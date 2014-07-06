/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod anton@nsl.cz
 */

#ifndef TESTSERVICE_H
#define TESTSERVICE_H

#include "nnamedservice.h"
#include <QDebug>
#include <QMetaType>

class TestService : public NNamedService
{
    Q_OBJECT
  public:
    TestService(QObject *parent = 0) : NNamedService(parent) {}
    TestService(const TestService &other) : NNamedService(other.parent()) { instance = other.instance;  }
    ~TestService() {}

  public slots:
    int speedTest() { return 0; }
    int test1() { qDebug() << "test1 called"; return 0; }
    int test2(const QString &first, int second) { qDebug() << "test 2 called with " << first << " and " << second; return 1; }
    QString test3(const QString &param) { qDebug() << "test3 called with " << param; return "result from test3"; }
    QString testA(const QVariantMap &param) { qDebug() << "testA called with " << param; return "result from testA"; }
    QString testB(const QVariantMap &param)
    {
        qDebug() << params;
        qDebug() << "testA called with " << param;
        return "result from testA";
    }

  private:
    int instance;
};

Q_DECLARE_METATYPE(TestService)

#endif // TESTSERVICE_H
