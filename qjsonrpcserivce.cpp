#include "qjsonrpcserivce.h"
#include "nnamedservice.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

Q_LOGGING_CATEGORY(LOG_QJSONRPCSERVICE, "QJsonRpcService")

QJsonRpcSerivce::QJsonRpcSerivce(int serviceMetaType) :
    m_serviceMetaType(serviceMetaType)
{
}

QByteArray QJsonRpcSerivce::processRequest(QByteArray request, QVariantMap info)
{
    qCDebug(LOG_QJSONRPCSERVICE) << "processRequest" << QString(request) << info;
    QJsonParseError json_error;
    QJsonDocument json_request = QJsonDocument::fromJson(request, &json_error);
    if (json_error.error != QJsonParseError::NoError)
        throw NNamedService::NSException(NNamedService::NSException::code_parseError);

    qCDebug(LOG_QJSONRPCSERVICE) << json_request;

    // далее имея json-документ необходимо проверить десяток условий верности запроса
    if (!json_request.isObject())
        throw NNamedService::NSException(NNamedService::NSException::code_invalidRequest);
    QJsonObject json_root = json_request.object();

    // нам нужен только jsonrpc версии 2.0
    if ((!json_root.contains("jsonrpc")) || (json_root.value("jsonrpc").toString("") != "2.0"))
        throw NNamedService::NSException(NNamedService::NSException::code_invalidRequest);

    if ((!json_root.contains("method")) || (!json_root.value("method").isString()))
        throw NNamedService::NSException(NNamedService::NSException::code_invalidRequest);

    if ((!json_root.contains("params")) || (!json_root.value("params").isArray()))
        throw NNamedService::NSException(NNamedService::NSException::code_invalidRequest);

    if ((!json_root.contains("id")) || (!json_root.value("id").isString()))
        throw NNamedService::NSException(NNamedService::NSException::code_invalidRequest);

    // TODO: тут будет авторизация :)

    // создание экземпляра класса сервиса
    NNamedService *service = static_cast<NNamedService *>(QMetaType::create(m_serviceMetaType));

    // а тут заполнение дополнительных данных!!!
    if (info.contains("remote_ip"))
        service->params["remote_ip"] = info["remote_ip"];

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

    qCDebug(LOG_QJSONRPCSERVICE) << result;

    return result.toJson();
}
