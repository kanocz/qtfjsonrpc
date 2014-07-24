/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod <anton@nsl.cz>
 */

#include <QSocketNotifier>
#include <QDebug>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>

#include "nfastcgi.h"

Q_LOGGING_CATEGORY(LOG_NFASTCGI, "NFastCgi")

NFastCgi::NFastCgi(const char *socketPath, int serviceMetaType, int threadCount = 0, QObject *parent) : QObject(parent), m_notifier(0), m_serviceMetaType(serviceMetaType)
{
    // инициализация метаданных до создания потоков, чтобы потом не повредить данные
    qCDebug(LOG_NFASTCGI) << "NNamedService init";
    NNamedService *serviceInit = static_cast<NNamedService *>(QMetaType::create(m_serviceMetaType));
    Q_CHECK_PTR(serviceInit);
    serviceInit->parseMetaInfo();
    QMetaType::destroy(m_serviceMetaType, (void *) serviceInit);

    // создание пула потоков
    qCDebug(LOG_NFASTCGI) << "QThreadPool init";
    jobsPool = new QThreadPool(this);
    jobsPool->setMaxThreadCount(threadCount == 0 ? QThread::idealThreadCount() : threadCount);

    // инициализаяиц fastcgi и подключения сигнала к сокету
    qCDebug(LOG_NFASTCGI) << "FastCGI init";
    FCGX_Init();
    int sock = FCGX_OpenSocket(socketPath, 1024);
    this->m_notifier = new QSocketNotifier(sock, QSocketNotifier::Read);
    QObject::connect(this->m_notifier, SIGNAL(activated(int)), this, SLOT(connectionPending(int)));
}

void NFastCgi::connectionPending(int socket)
{
    qCDebug(LOG_NFASTCGI) << "connectionPending";
    QSocketNotifier* notifier = qobject_cast<QSocketNotifier*>(this->sender());
    Q_CHECK_PTR(notifier);

    notifier->setEnabled(false);

    FCGX_Request* req = new FCGX_Request;
    FCGX_InitRequest(req, socket, 0);
    int s = FCGX_Accept_r(req);
    if (s >= 0) {
        /* Здесь создаём и запускаем поток */
        NFastCgiJob *newJob = new NFastCgiJob(req, m_serviceMetaType);
        qCDebug(LOG_NFASTCGI) << "New thread" << newJob;
        jobsPool->start(newJob);
    }

    notifier->setEnabled(true);
}

const char *NFastCgiJob::getJsonError(NNamedService::NSException *e)
{
  int code = e->getCode();
  int id = e->getId();
  QString message = e->getMessage();

  if (message.isEmpty()) {
      switch (code) {
        case NNamedService::NSException::code_parseError: message = "Parse error"; break;
        case NNamedService::NSException::code_invalidRequest: message = "Invalid Request"; break;
        case NNamedService::NSException::code_methodNotFound:message = "Method not found"; break;
        case NNamedService::NSException::code_invalidParams: message = "Invalid params"; break;
        case NNamedService::NSException::code_internalError: message = "Internal error"; break;
        case NNamedService::NSException::code_serverError: message = "Server error"; break;
        default: message = "unknown error";
      }
  }

  return QString("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": %1, \"message\": \"%2\"}, \"id\": %3}")
                   .arg(code)
                   .arg(message)
                   .arg((id < 1) ? "null" : QString("\"%1\"").arg(id))
                   .toUtf8().data();

}

NFastCgiJob::NFastCgiJob(FCGX_Request *request, int serviceMetaType)
    : m_request(request), m_serviceMetaType(serviceMetaType)
{
    rpc = new QJsonRpcSerivce(serviceMetaType);
}

NFastCgiJob::~NFastCgiJob()
{
    delete rpc;
    FCGX_Finish_r(m_request);
}

void NFastCgiJob::run()
{
    FCGX_FPrintF(m_request->out, "Content-type: application/json; charset=UTF-8\r\n");
    FCGX_FPrintF(m_request->out, "Expires: Wed, 23 Mar 1983 12:15:00 GMT\r\n"\
                 "Cache-Control: no-store, no-cache, must-revalidate\r\n"\
                 "Cache-Control: post-check=0, pre-check=0\r\n"\
                 "Pragma: no-cache\r\n"\
                 "\r\n");

    char *post_data = 0;
    try {
        // для начала читаем весь массив пост-запроса, чтобы иметь возможность спарсить json
        char *contentLength = FCGX_GetParam("CONTENT_LENGTH", m_request->envp);
        int len = 0;
        if (contentLength != NULL) { len = strtol(contentLength, NULL, 10); }

        if ((len < 1) || (len > NFastCgiJob::MAX_REQUEST_SIZE))
            throw NNamedService::NSException(NNamedService::NSException::code_invalidRequest);

        post_data = new char[len+1];
        if (!post_data)
            throw NNamedService::NSException(NNamedService::NSException::code_serverError, QString("falied to alloc %1 bytes").arg(len));

        if(FCGX_GetStr(post_data, len, m_request->in) != len)
            throw NNamedService::NSException(NNamedService::NSException::code_serverError, QString("falied to read %1 bytes").arg(len));

        // у нас уже есть данные, теперь можно попробовать распарсить jsonrpc-запрос

        QVariantMap info;
        info["ip"] = QString(FCGX_GetParam("REMOTE_ADDR", m_request->envp));
        QByteArray result = rpc->processRequest(QByteArray(post_data, len), info);

        FCGX_PutS(result.data(), m_request->out);

    } catch (NNamedService::NSException &e) {
        FCGX_FPrintF(m_request->out, getJsonError(&e));
        qCWarning(LOG_NFASTCGI) << "NFastCgiJob::run() -> " << getJsonError(&e);
    }
    if (post_data) delete post_data;
    FCGX_Finish_r(m_request);
}
