/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod <anton@nsl.cz>
 */

#include <QSocketNotifier>
#include <QDebug>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>

#include "fastcgi.h"

NFastCgi::NFastCgi(const char *socketPath, int serviceMetaType, QObject *parent) : QObject(parent), m_notifier(0), m_serviceMetaType(serviceMetaType)
{
    // инициализация метаданных до создания потоков, чтобы потом не повредить данные
    NNamedService *serviceInit = static_cast<NNamedService *>(QMetaType::create(m_serviceMetaType));
    serviceInit->parseMetaInfo();
    QMetaType::destroy(m_serviceMetaType, (void *) serviceInit);

    // создание пула потоков
    jobsPool = new QThreadPool(this);
    jobsPool->setMaxThreadCount(1024);

    // инициализаяиц fastcgi и подключения сигнала к сокету
    FCGX_Init();
    int sock = FCGX_OpenSocket(socketPath, 1024);
    this->m_notifier = new QSocketNotifier(sock, QSocketNotifier::Read);
    QObject::connect(this->m_notifier, SIGNAL(activated(int)), this, SLOT(connectionPending(int)));
}

void NFastCgi::connectionPending(int socket)
{
    QSocketNotifier* notifier = qobject_cast<QSocketNotifier*>(this->sender());
    Q_CHECK_PTR(notifier);

    notifier->setEnabled(false);

    FCGX_Request* req = new FCGX_Request;
    FCGX_InitRequest(req, socket, 0);
    int s = FCGX_Accept_r(req);
    if (s >= 0) {
        /* Здесь создаём и запускаем поток */
        NFastCgiJob *newJob = new NFastCgiJob(req, m_serviceMetaType);
        jobsPool->start(newJob);
    }

    notifier->setEnabled(true);
}

NFastCgiJob::NFastCgiJob(FCGX_Request *request, int serviceMetaType) : m_request(request), m_serviceMetaType(serviceMetaType), request_ip(0) { }

NFastCgiJob::~NFastCgiJob()
{
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
            throw NJsonRpcException(NJsonRpcException::code_invalidRequest);

        post_data = new char[len+1];
        if (!post_data)
            throw NJsonRpcException(NJsonRpcException::code_serverError, QString("falied to alloc %1 bytes").arg(len));

        if(FCGX_GetStr(post_data, len, m_request->in) != len)
            throw NJsonRpcException(NJsonRpcException::code_serverError, QString("falied to read %1 bytes").arg(len));

        // у нас уже есть данные, теперь можно попробовать распарсить jsonrpc-запрос

        QJsonParseError json_error;
        json_request = QJsonDocument::fromJson(QByteArray(post_data, len), &json_error);
        if (json_error.error != QJsonParseError::NoError)
            throw NJsonRpcException(NJsonRpcException::code_parseError);

        // далее имея json-документ необходимо проверить десяток условий верности запроса
        if (!json_request.isObject())
            throw NJsonRpcException(NJsonRpcException::code_invalidRequest);
        QJsonObject json_root = json_request.object();

        // нам нужен только jsonrpc версии 2.0
        if ((!json_root.contains("jsonrpc")) || (json_root.value("jsonrpc").toString("") != "2.0"))
            throw NJsonRpcException(NJsonRpcException::code_invalidRequest);

        if ((!json_root.contains("method")) || (!json_root.value("method").isString()))
            throw NJsonRpcException(NJsonRpcException::code_invalidRequest);

        if ((!json_root.contains("params")) || (!json_root.value("params").isArray()))
            throw NJsonRpcException(NJsonRpcException::code_invalidRequest);

        if ((!json_root.contains("id")) || (!json_root.value("id").isString()))
            throw NJsonRpcException(NJsonRpcException::code_invalidRequest);

        // TODO: тут будет авторизация :)

        // создание экземпляра класса сервиса
        NNamedService *service = static_cast<NNamedService *>(QMetaType::create(m_serviceMetaType));

        // а тут заполнение дополнительных данных!!!
        request_ip = FCGX_GetParam("REMOTE_ADDR", m_request->envp);
        if (request_ip) {
            service->params["remote_ip"] = QVariant(QString(request_ip));
        }

        // подготовка класса для ответа
        QVariantMap jresult;

        // собственно вызов функции
        jresult.insert("result", service->process(json_root.value("method").toString(), json_root.value("params").toArray().toVariantList()));

        // чистка памяти
        QMetaType::destroy(m_serviceMetaType, (void *) service);

        // дополнительные параметры ответа
        jresult.insert("jsonrpc", "2.0");
        jresult.insert("id", json_root.value("id").toVariant());

        QJsonDocument result(QJsonObject::fromVariantMap(jresult));

        FCGX_PutS(result.toJson().data(), m_request->out);

    } catch (NJsonRpcException &e) {
        FCGX_FPrintF(m_request->out, e.getJsonError());
        qDebug() << "NFastCgiJob::run() -> " << e.getJsonError();
    }
    if (post_data) delete post_data;
    FCGX_Finish_r(m_request);
}

const char* NJsonRpcException::getJsonError()
{
    if (!m_hasMessage) {
        switch (m_code) {
          case NJsonRpcException::code_parseError: m_message = "Parse error"; break;
          case NJsonRpcException::code_invalidRequest: m_message = "Invalid Request"; break;
          case NJsonRpcException::code_methodNotFound:m_message = "Method not found"; break;
          case NJsonRpcException::code_invalidParams: m_message = "Invalid params"; break;
          case NJsonRpcException::code_internalError: m_message = "Internal error"; break;
          case NJsonRpcException::code_serverError: m_message = "Server error"; break;
          default: m_message = "unknown error";
        }
    }

    return QString("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": %1, \"message\": \"%2\"}, \"id\": %3}")
                     .arg(m_code)
                     .arg(m_message)
                     .arg((m_id < 1) ? "null" : QString("\"%1\"").arg(m_id))
                     .toUtf8().data();
}
