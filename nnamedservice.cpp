/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod <anton@nsl.cz>
 */

#include "nnamedservice.h"

#include <QMetaMethod>

void NNamedService::parseMetaInfo()
{
    metaInfoParsed = true;
    // get meta-object
    const QMetaObject *meta = metaObject();
    // ignoring methods of QObject and so on
    for (int id = QObject::staticMetaObject.methodCount(); id < meta->methodCount(); ++id) {
        const QMetaMethod method = meta->method(id);
        // we need only public slots (remove check to provide all methods)
        if (method.methodType() == QMetaMethod::Slot && method.access() == QMetaMethod::Public) {
            m_methodHash.insert(method.name(), id);

            // max 10 of int
            QList<int> paramTypes;

            // first one is return type
            paramTypes << method.returnType();

            // and then types of parameters
            for (int p = 0; p < method.parameterCount(); p++)
                paramTypes << method.parameterType(p);

            m_paramHash[id] = paramTypes;
        }
    }
}

QVariant NNamedService::process(QString method, QVariantList arguments)
{
    // we can't do this in constructor, but have to check if parsed
    if (!metaInfoParsed)
        parseMetaInfo();

    // check if such method is in our array
    QByteArray b_method = method.toLatin1();
    if (!m_methodHash.contains(b_method))
        throw NNamedServriceException(NNamedServriceException::code_methodNotFound, QString("Method '%1' not found").arg(method));


    int metaId = -1;
    QList<int> paramTypes;
    // we need to get method not only by name, but also by parameters count - default parameters not accepted!
    QList<int> indexes = m_methodHash.values(b_method);
    foreach (int methodIndex, indexes) {
        paramTypes = m_paramHash.value(methodIndex);
        if (arguments.size() == paramTypes.size() - 1) {
            metaId = methodIndex;
            break;
        }
    }

    // error if not found
    if (metaId == -1)
        throw NNamedServriceException(NNamedServriceException::code_invalidParams);

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
            // in case if we unable to parse argument we pass default value
            void *value = QMetaType::create(parameterType);
            parameters.append(value);
            cleanup.insert(value, static_cast<QMetaType::Type>(parameterType));
        } else {
            if (argument.userType() != parameterType && parameterType != QMetaType::QVariant) {
                // check if we need convert argument
                if (const_cast<QVariant*>(&argument)->canConvert(static_cast<QVariant::Type>(parameterType))) {
                  const_cast<QVariant*>(&argument)->convert(static_cast<QVariant::Type>(parameterType));
                  parameters.append(const_cast<void *>(argument.constData()));
                } else {
                    // default value if not
                    void *value = QMetaType::create(parameterType);
                    parameters.append(value);
                    cleanup.insert(value, static_cast<QMetaType::Type>(parameterType));
                }

            } else {
                parameters.append(const_cast<void *>(argument.constData()));
            }
        }
    }

    // invoking method...
    bool success =
        const_cast<NNamedService*>(this)->qt_metacall(QMetaObject::InvokeMetaMethod, metaId, parameters.data()) < 0;
    if (!success)
        throw NNamedServriceException(NNamedServriceException::code_serverError, QString("process for method '%1' failed").arg(method));

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
