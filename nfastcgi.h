/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod <anton@nsl.cz>
 */

#ifndef NFASTCGI_H
#define NFASTCGI_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <QJsonDocument>
#include <QMetaType>
#include <fcgiapp.h>
#include <QException>
#include <QLoggingCategory>

#include "nnamedservice.h"
#include "qjsonrpcserivce.h"

class QSocketNotifier;

class NFastCgi : public QObject
{
    Q_OBJECT
  public:
    explicit NFastCgi(const char *socketPath, int serviceMetaType, int threadCount, QObject *parent = 0);

  private:
    QSocketNotifier* m_notifier;
    QThreadPool *jobsPool;
    int m_serviceMetaType;

  private slots:
    void connectionPending(int socket);
};

class NFastCgiJob : public QRunnable
{
  private:
    FCGX_Request *m_request;
    int m_serviceMetaType;

    static const int MAX_REQUEST_SIZE = 16 * 1024 * 1024;

    const char *getJsonError(NNamedService::NSException *e);

    QJsonRpcSerivce *rpc;

  public:

    NFastCgiJob(FCGX_Request *request, int serviceMetaType);
    ~NFastCgiJob();
    void run();
};

Q_DECLARE_LOGGING_CATEGORY(LOG_NFASTCGI)

#endif // NFASTCGI_H
