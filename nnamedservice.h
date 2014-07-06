/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod <anton@nsl.cz>
 */

#ifndef NNAMEDSERVICE_H
#define NNAMEDSERVICE_H

#include <QObject>
#include <QMultiHash>
#include <QHash>
#include <QMap>
#include <QVariant>
#include <QException>

class NNamedService : public QObject
{
    friend class NFastCgi;
    friend class NFastCgiJob;
    Q_OBJECT
public:
    explicit NNamedService(QObject *parent = 0) : QObject(parent) {}
    QVariant process(QString method, QVariantList arguments);

signals:

public slots:

protected:
    static bool metaInfoParsed;
    void parseMetaInfo();
    static QMultiHash<QByteArray, int> m_methodHash;
    static QHash<int, QList<int> > m_paramHash;
    QMap<QString,QVariant> params;
};

class NNamedServriceException : public QException
{
  protected:
    int m_id, m_code;
    QString m_message;

  public:
    static const int code_parseError = -32700;
    static const int code_invalidRequest = -32600;
    static const int code_methodNotFound = -32601;
    static const int code_invalidParams = -32602;
    static const int code_internalError = -32603;
    static const int code_serverError = -32000;

    void raise() const { throw *this; }
    NNamedServriceException *clone() const { return new NNamedServriceException(*this); }

    ~NNamedServriceException() throw() {}

    NNamedServriceException(int code = -32000, QString message = QString(), int id = -1) : m_id(id), m_code(code), m_message(message) { }

    int getId() { return m_id; }
    int getCode() { return m_code; }
    QString getMessage() { return m_message; }
};


#endif // NNAMEDSERVICE_H
