#ifndef QJSONRPCSERIVCE_H
#define QJSONRPCSERIVCE_H

#include <QVariantMap>
#include <QByteArray>
#include <QLoggingCategory>

class QJsonRpcSerivce
{
public:
    explicit QJsonRpcSerivce(int serviceMetaType);

    QByteArray processRequest(QByteArray request, QVariantMap info);

private:
    int m_serviceMetaType;

};

Q_DECLARE_LOGGING_CATEGORY(LOG_QJSONRPCSERVICE)

#endif // QJSONRPCSERIVCE_H
