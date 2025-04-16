#include <QtTest/QtTest>

Q_DECLARE_METATYPE(QDate)
Q_DECLARE_METATYPE(QTime)

#include <poppler-qt4.h>

class TestDateConv: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void checkDates_data();
    void checkDates();
    void checkInvalidDates_data();
    void checkInvalidDates();
};

void TestDateConv::initTestCase()
{
    qRegisterMetaType<QDate>("QDate");
    qRegisterMetaType<QTime>("QTime");
}

void TestDateConv::checkDates_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QDate>("day");
    QTest::addColumn<QTime>("time");

    // This is a typical case - all data provided
    QTest::newRow("D:20040101121110")
      << QByteArray("D:20040101121110Z")
      << QDate( 2004, 1, 1)
      << QTime( 12, 11, 10);

    // The D: is strongly recommended, but optional
    QTest::newRow("20040101121110")
      << QByteArray("20040101121110Z")
      << QDate( 2004, 1, 1)
      << QTime( 12, 11, 10);

    // Only the year is actually required
    QTest::newRow("D:2006")
      << QByteArray("D:2006")
      << QDate( 2006, 1, 1)
      << QTime( 0, 0, 0);

    QTest::newRow("D:200602")
      << QByteArray("D:200602")
      << QDate( 2006, 2, 1)
      << QTime( 0, 0, 0);

    QTest::newRow("D:20060304")
      << QByteArray("D:20060304")
      << QDate( 2006, 3, 4)
      << QTime( 0, 0, 0);

    QTest::newRow("D:2006030405")
      << QByteArray("D:2006030405")
      << QDate( 2006, 3, 4)
      << QTime( 5, 0, 0);

    QTest::newRow("D:200603040512")
      << QByteArray("D:200603040512")
      << QDate( 2006, 3, 4)
      << QTime( 5, 12, 0);

    // If the timezone isn't specified, I assume UTC
    QTest::newRow("D:20060304051226")
      << QByteArray("D:20060304051226")
      << QDate( 2006, 3, 4)
      << QTime( 5, 12, 26);

    // Check for real timezone conversions
    QTest::newRow("D:20030131115258-04'00'")
      << QByteArray("D:20030131115258-04'00'")
      << QDate( 2003, 1, 31)
      << QTime( 15, 52, 58);

    QTest::newRow("D:20030131115258+05'00'")
      << QByteArray("D:20030131115258+05'00'")
      << QDate( 2003, 1, 31)
      << QTime( 6, 52, 58);

    // There are places that have non-hour offsets
    // Yep, that means you Adelaide.
    QTest::newRow("D:20030131115258+08'30'")
      << QByteArray("D:20030131115258+08'30'")
      << QDate( 2003, 1, 31)
      << QTime( 3, 22, 58);

    QTest::newRow("D:20030131115258-08'30'")
      << QByteArray("D:20030131115258-08'30'")
      << QDate( 2003, 1, 31)
      << QTime( 20, 22, 58);
}

void TestDateConv::checkDates()
{
    QFETCH(QByteArray, input);
    QFETCH(QDate, day);
    QFETCH(QTime, time);

    QCOMPARE( Poppler::convertDate(input.data()), QDateTime(day, time, Qt::UTC) );
}

void TestDateConv::checkInvalidDates_data()
{
    QTest::addColumn<QByteArray>("input");

    // Null data
    QTest::newRow("Null data")
      << QByteArray();

    // Empty data
    QTest::newRow("Empty data")
      << QByteArray("");

    // Empty data
    QTest::newRow("One character")
      << QByteArray("D");

    // Empty data
    QTest::newRow("'D:'")
      << QByteArray("D:");

    // Empty data
    QTest::newRow("Not a date")
      << QByteArray("D:IAmNotAValidDate");
}

void TestDateConv::checkInvalidDates()
{
    QFETCH(QByteArray, input);

    QCOMPARE(Poppler::convertDate(input.data()), QDateTime());
}

QTEST_MAIN(TestDateConv)

#include "check_dateConversion.moc"
