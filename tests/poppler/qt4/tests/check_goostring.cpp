#include <QtTest/QtTest>

#include "goo/GooString.h"

class TestGooString : public QObject
{
    Q_OBJECT
private slots:
    void testInsertData_data();
    void testInsertData();
    void testInsert();
};

void TestGooString::testInsertData_data()
{
    QTest::addColumn<QByteArray>("string");
    QTest::addColumn<QByteArray>("addition");
    QTest::addColumn<int>("position");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("foo") << QByteArray("foo") << QByteArray("bar") << 0 << QByteArray("barfoo");
    QTest::newRow("<empty>") << QByteArray() << QByteArray("bar") << 0 << QByteArray("bar");
    QTest::newRow("foo+bar #1") << QByteArray("f+bar") << QByteArray("oo") << 1 << QByteArray("foo+bar");
    QTest::newRow("foo+bar #2") << QByteArray("fobar") << QByteArray("o+") << 2 << QByteArray("foo+bar");
    QTest::newRow("foo+bar #last") << QByteArray("foo+r") << QByteArray("ba") << 4 << QByteArray("foo+bar");
    QTest::newRow("foo+bar #end") << QByteArray("foo+") << QByteArray("bar") << 4 << QByteArray("foo+bar");
    QTest::newRow("long #start") << QByteArray("very string") << QByteArray("long long long long long ") << 5 << QByteArray("very long long long long long string");
}

void TestGooString::testInsertData()
{
    QFETCH(QByteArray, string);
    QFETCH(QByteArray, addition);
    QFETCH(int, position);
    QFETCH(QByteArray, result);

    GooString goo(string.constData());
    QCOMPARE(goo.getCString(), string.constData());
    goo.insert(position, addition.constData());
    QCOMPARE(goo.getCString(), result.constData());
}

void TestGooString::testInsert()
{
    {
    GooString goo;
    goo.insert(0, ".");
    goo.insert(0, "This is a very long long test string");
    QCOMPARE(goo.getCString(), "This is a very long long test string.");
    }
    {
    GooString goo;
    goo.insert(0, "second-part-third-part");
    goo.insert(0, "first-part-");
    QCOMPARE(goo.getCString(), "first-part-second-part-third-part");
    }
}

QTEST_MAIN(TestGooString)
#include "check_goostring.moc"

