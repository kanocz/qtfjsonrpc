#include <QCoreApplication>
#include <QTest>

#include "test_nnamedservice.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return QTest::qExec(new Test_NNamedService, argc, argv);
}
