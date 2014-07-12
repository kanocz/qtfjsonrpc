#ifndef QJSONRPCSERIVCE_H
#define QJSONRPCSERIVCE_H

#include <QVariantMap>
#include <QByteArray>

class QJsonRpcSerivce
{
public:
    explicit QJsonRpcSerivce(int serviceMetaType);

    QByteArray processRequest(QByteArray request, QVariantMap info);

private:
    int m_serviceMetaType;

};

#endif // QJSONRPCSERIVCE_H
