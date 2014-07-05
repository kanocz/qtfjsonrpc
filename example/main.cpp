/*
 * FastCGI processing module for QT5
 * @autor Anton Skorokhod anton@nsl.cz
 */

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QStringList>
#include <QVariantList>

#include "nnamedservice.h"
#include "nfastcgi.h"
#include "testservice.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NFastCgi fastCgi(":9000", qRegisterMetaType<TestService>(), 32, &a);
    return a.exec();
}
