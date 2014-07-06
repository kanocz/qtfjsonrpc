#include <QTest>
#include <QVariant>
#include <QVariantList>
#include "test_nnamedservice.h"

#include <QDebug>

Test_NNamedService::Test_NNamedService(QObject *parent) : QObject(parent)
{

}

void Test_NNamedService::process()
{
    TSrv test;
    QCOMPARE(test.process("return1", QVariantList()), QVariant(1));
    QCOMPARE(test.process("return2X", QVariantList() << 2), QVariant(4));
    QCOMPARE(test.process("returnSum", QVariantList() << 2 << 3), QVariant(5));
    QCOMPARE(test.process("returnHello", QVariantList()), QVariant("Hello"));
    QCOMPARE(test.process("returnConcat", QVariantList() << "Hello" << "World"), QVariant("HelloWorld"));
}


void Test_NNamedService::parseMetaInfo()
{
    TSrv test;
    test.parseMetaInfo();
    QVERIFY(test.metaInfoParsed);

    QVERIFY(test.m_methodHash.contains("return1"));
    QVERIFY(test.m_methodHash.contains("return2X"));
    QVERIFY(test.m_methodHash.contains("returnHello"));
    QVERIFY(test.m_methodHash.contains("returnSum"));
    QVERIFY(test.m_methodHash.contains("returnConcat"));

    int f;
    QList<int> p;

    f = test.m_methodHash.value("return1");
    p = test.m_paramHash.value(f);

    QCOMPARE(p.count(), 1);
    QCOMPARE(p.value(0), 2);

    f = test.m_methodHash.value("return2X");
    p = test.m_paramHash.value(f);

    QCOMPARE(p.count(), 2);
    QCOMPARE(p.value(0), 2);
    QCOMPARE(p.value(1), 2);

    f = test.m_methodHash.value("returnConcat");
    p = test.m_paramHash.value(f);

    QCOMPARE(p.count(), 3);
    QCOMPARE(p.value(0), 10);
    QCOMPARE(p.value(1), 10);
    QCOMPARE(p.value(2), 10);
}

void Test_NNamedService::exceptions()
{
    TSrv test;

    bool hasException;

    hasException = false;
    try { test.process("invalid", QVariantList()); } catch (NNamedServriceException &e) { QCOMPARE(e.getCode(), -32601); hasException = true; }
    QVERIFY2(hasException, "No 32601 exception");

    hasException = false;
    try { test.process("return1", QVariantList() << 1); } catch (NNamedServriceException &e) { QCOMPARE(e.getCode(), -32602); hasException = true; }
    QVERIFY2(hasException, "No 32602 exception");
}
