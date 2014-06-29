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

#include "nnamedservice.h"

class QSocketNotifier;

class NFastCgi : public QObject
{
    Q_OBJECT
  public:
    explicit NFastCgi(const char *socketPath, int serviceMetaType, QObject *parent = 0);

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
  public:

    char *request_ip;

    QJsonDocument json_request;

    NFastCgiJob(FCGX_Request *request, int serviceMetaType);
    ~NFastCgiJob();
    void run();
};

class NJsonRpcException : public QException
{
  private:
    int m_code;
    QString m_message;
    bool m_hasMessage;
    int m_id;

  public:
    static const int code_parseError = -32700;
    static const int code_invalidRequest = -32600;
    static const int code_methodNotFound = -32601;
    static const int code_invalidParams = -32602;
    static const int code_internalError = -32603;
    static const int code_serverError = -32000;

    const char *getJsonError();

    void raise() const { throw *this; }
    NJsonRpcException *clone() const { return new NJsonRpcException(*this); }

    ~NJsonRpcException() throw() {}

    NJsonRpcException(int code = -3200) : m_code(code), m_hasMessage(false), m_id(-1) { }
    NJsonRpcException(int code, QString message, int id = -1) : m_code(code), m_id(id) { m_message = message; m_hasMessage = true;}
};


#endif // NFASTCGI_H
