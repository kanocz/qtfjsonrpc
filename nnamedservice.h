/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod anton@nsl.cz
 */

#ifndef NNAMEDSERVICE_H
#define NNAMEDSERVICE_H

#include <QObject>
#include <QMultiHash>
#include <QHash>
#include <QMap>
#include <QVariant>

class NNamedService : public QObject
{
    friend class NFastCgi;
    friend class NFastCgiJob;
    Q_OBJECT
public:
    explicit NNamedService(QObject *parent = 0);
    QVariant process(QString method, QVariantList arguments);

signals:

public slots:

private:
    static bool metaInfoParsed;
    void parseMetaInfo();
    static QMultiHash<QByteArray, int> m_methodHash;
    static QHash<int, QList<int> > m_paramHash;
protected:
    QMap<QString,QVariant> params;
};

#endif // NNAMEDSERVICE_H
