/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod <anton@nsl.cz>
 */

#include "nnamedservice.h"
#include "fastcgi.h"

#include <QMetaMethod>

NNamedService::NNamedService(QObject *parent) :
    QObject(parent)
{
//    metaInfoParsed = false;
}

void NNamedService::parseMetaInfo()
{
    metaInfoParsed = true;
    // получаем мета-объект
    const QMetaObject *meta = metaObject();
    // нам не нужны вызовы обхектов из QObject
    for (int id = QObject::staticMetaObject.methodCount(); id < meta->methodCount(); ++id) {
        const QMetaMethod method = meta->method(id);
        if (method.methodType() == QMetaMethod::Slot &&
            method.access() == QMetaMethod::Public) {
            QByteArray signature = method.methodSignature();
            QByteArray methodName = signature.left(signature.indexOf('('));
            m_methodHash.insert(methodName, id);

            QList<int> paramTypes;
            paramTypes << QMetaType::type(method.typeName());

            foreach(QByteArray paramType, method.parameterTypes()) {
                paramTypes << QMetaType::type(paramType);
            }
            m_paramHash[id] = paramTypes;
        }
    }
}

QVariant NNamedService::process(QString method, QVariantList arguments)
{
    if (!metaInfoParsed)
        parseMetaInfo();

    QByteArray b_method = method.toLatin1();
    if (!m_methodHash.contains(b_method))
        throw NJsonRpcException(NJsonRpcException::code_methodNotFound, QString("Method '%1' not found").arg(method));

    int metaId = -1;
    QList<int> paramTypes;
    QList<int> indexes = m_methodHash.values(b_method);
    foreach (int methodIndex, indexes) {
        paramTypes = m_paramHash.value(methodIndex);
        if (arguments.size() == paramTypes.size() - 1) {
            metaId = methodIndex;
            break;
        }
    }

    if (metaId == -1)
        throw NJsonRpcException(NJsonRpcException::code_invalidParams);

    QVarLengthArray<void *, 10> parameters;
    parameters.reserve(paramTypes.count());

    // первый аргумент metacall - тип возврата
    QMetaType::Type returnType = static_cast<QMetaType::Type>(paramTypes[0]);
    void *returnData = QMetaType::create(returnType);

    QVariant returnValue(returnType, returnData);
    if (returnType == QMetaType::QVariant)
        parameters.append(&returnValue);
    else
        parameters.append(returnValue.data());

    // compile arguments
    QHash<void*, QMetaType::Type> cleanup;
    for (int i = 0; i < paramTypes.size() - 1; ++i) {
        int parameterType = paramTypes[i + 1];
        const QVariant &argument = arguments.at(i);
        if (!argument.isValid()) {
            // pass in a default constructed parameter in this case
            void *value = QMetaType::create(parameterType);
            parameters.append(value);
            cleanup.insert(value, static_cast<QMetaType::Type>(parameterType));
        } else {
            if (argument.userType() != parameterType &&
                parameterType != QMetaType::QVariant &&
                const_cast<QVariant*>(&argument)->canConvert(static_cast<QVariant::Type>(parameterType)))
                const_cast<QVariant*>(&argument)->convert(static_cast<QVariant::Type>(parameterType));
            parameters.append(const_cast<void *>(argument.constData()));
        }
    }

    bool success =
        const_cast<NNamedService*>(this)->qt_metacall(QMetaObject::InvokeMetaMethod, metaId, parameters.data()) < 0;
    if (!success)
        throw NJsonRpcException(NJsonRpcException::code_serverError, QString("process for method '%1' failed").arg(method));

    // cleanup and result
    QVariant returnCopy(returnValue);
    QMetaType::destroy(returnType, returnData);
    foreach (void *value, cleanup.keys()) {
        cleanup.remove(value);
        QMetaType::destroy(cleanup.value(value), value);
    }

    return returnCopy;
}

bool NNamedService::metaInfoParsed = false;
QMultiHash<QByteArray, int> NNamedService::m_methodHash;
QHash<int, QList<int> > NNamedService::m_paramHash;
