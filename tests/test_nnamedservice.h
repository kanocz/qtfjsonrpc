#include <QObject>
#include "nnamedservice.h"

class Test_NNamedService : public QObject
{
    Q_OBJECT
public:
    explicit Test_NNamedService(QObject *parent = 0);

private slots:
    void process();
    void parseMetaInfo();
    void exceptions();
};

class TSrv : public NNamedService
{
    Q_OBJECT
    friend class Test_NNamedService;

public slots:
    int return1() { return 1; }
    int return2X(int x) { return 2*x; }
    int returnSum(int a, int b) { return a+b; }
    QString returnHello() { return "Hello"; }
    QString returnConcat(QString a, QString b) { return a+b; }

};
