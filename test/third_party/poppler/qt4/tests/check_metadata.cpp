#include <QtTest/QtTest>

#include <poppler-qt4.h>

class TestMetaData: public QObject
{
    Q_OBJECT
private slots:
    void checkStrings_data();
    void checkStrings();
    void checkStrings2_data();
    void checkStrings2();
    void checkStringKeys();
    void checkLinearised();
    void checkNumPages();
    void checkDate();
    void checkPageSize();
    void checkPortraitOrientation();
    void checkLandscapeOrientation();
    void checkUpsideDownOrientation();
    void checkSeascapeOrientation();
    void checkVersion();
    void checkPdfId();
    void checkNoPdfId();
};

void TestMetaData::checkStrings_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("value");

    QTest::newRow( "Author" ) << "Author" << "Brad Hards";
    QTest::newRow( "Title" ) << "Title" << "Two pages";
    QTest::newRow( "Subject" ) << "Subject"
			       << "A two page layout for poppler testing";
    QTest::newRow( "Keywords" ) << "Keywords" << "Qt4 bindings";
    QTest::newRow( "Creator" ) << "Creator" << "iText: cgpdftops CUPS filter";
    QTest::newRow( "Producer" ) << "Producer" << "Acrobat Distiller 7.0 for Macintosh";
}

void TestMetaData::checkStrings()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    QVERIFY( doc );

    QFETCH( QString, key );
    QFETCH( QString, value );
    QCOMPARE( doc->info(key), value );

    delete doc;
}

void TestMetaData::checkStrings2_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("value");

    QTest::newRow( "Title" ) << "Title" << "Malaga hotels";
    QTest::newRow( "Author" ) << "Author" << "Brad Hards";
    QTest::newRow( "Creator" ) << "Creator" << "Safari: cgpdftops CUPS filter";
    QTest::newRow( "Producer" )  << "Producer" << "Acrobat Distiller 7.0 for Macintosh";
    QTest::newRow( "Keywords" ) << "Keywords" << "First\rSecond\rthird";
    QTest::newRow( "Custom1" ) << "Custom1" << "CustomValue1";
    QTest::newRow( "Custom2" ) << "Custom2" << "CustomValue2";
}

void TestMetaData::checkStrings2()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );

    QFETCH( QString, key );
    QFETCH( QString, value );
    QCOMPARE( doc->info(key), value );

    delete doc;
}

void TestMetaData::checkStringKeys()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );

    QStringList keyList;
    keyList << "Title" << "Author" << "Creator" << "Keywords" << "CreationDate";
    keyList << "Producer" << "ModDate" << "Custom1" << "Custom2";
    keyList.sort();
    QStringList keysInDoc = doc->infoKeys();
    keysInDoc.sort();
    QCOMPARE( keysInDoc, keyList );

    delete doc;
}

void TestMetaData::checkLinearised()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    QVERIFY( doc->isLinearized() );

    delete doc;

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->isLinearized(), false );

    delete doc;
}

void TestMetaData::checkPortraitOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );
  
    Poppler::Page *page = doc->page(0);
    QCOMPARE( page->orientation(), Poppler::Page::Portrait );

    delete page;
    delete doc;
}

void TestMetaData::checkNumPages()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->numPages(), 2 );

    delete doc;

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->numPages(), 1 );

    delete doc;
}

void TestMetaData::checkDate()
{
    Poppler::Document *doc;

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->date("ModDate"), QDateTime(QDate(2005, 12, 5), QTime(9,44,46), Qt::UTC ) );
    QCOMPARE( doc->date("CreationDate"), QDateTime(QDate(2005, 8, 13), QTime(1,12,11), Qt::UTC ) );

    delete doc;
}

void TestMetaData::checkPageSize()
{
    Poppler::Document *doc;

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    Poppler::Page *page = doc->page(0);
    QCOMPARE( page->pageSize(), QSize(595, 842) );
    QCOMPARE( page->pageSizeF(), QSizeF(595.22, 842) );

    delete page;
    delete doc;
}


void TestMetaData::checkLandscapeOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );
  
    Poppler::Page *page = doc->page(1);
    QCOMPARE( page->orientation(), Poppler::Page::Landscape );

    delete page;
    delete doc;
}

void TestMetaData::checkUpsideDownOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    Poppler::Page *page = doc->page(2);
    QCOMPARE( page->orientation(), Poppler::Page::UpsideDown );

    delete page;
    delete doc;
}

void TestMetaData::checkSeascapeOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    Poppler::Page *page = doc->page(3);
    QCOMPARE( page->orientation(), Poppler::Page::Seascape );

    delete page;
    delete doc;
}

void TestMetaData::checkVersion()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->pdfVersion(), 1.6 );
    int major = 0, minor = 0;
    doc->getPdfVersion( &major, &minor );
    QCOMPARE( major, 1 );
    QCOMPARE( minor, 6 );

    delete doc;
}

void TestMetaData::checkPdfId()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/A6EmbeddedFiles.pdf");
    QVERIFY( doc );

    const QByteArray referencePermanentId( "00C9D5B6D8FB11D7A902003065D630AA" );
    const QByteArray referenceUpdateId( "39AECAE6D8FB11D7A902003065D630AA" );

    {
    // no IDs wanted, just existance check
    QVERIFY( doc->getPdfId( 0, 0 ) );
    }
    {
    // only permanent ID
    QByteArray permanentId;
    QVERIFY( doc->getPdfId( &permanentId, 0 ) );
    QCOMPARE( permanentId.toUpper(), referencePermanentId );
    }
    {
    // only update ID
    QByteArray updateId;
    QVERIFY( doc->getPdfId( 0, &updateId ) );
    QCOMPARE( updateId.toUpper(), referenceUpdateId );
    }
    {
    // both IDs
    QByteArray permanentId;
    QByteArray updateId;
    QVERIFY( doc->getPdfId( &permanentId, &updateId ) );
    QCOMPARE( permanentId.toUpper(), referencePermanentId );
    QCOMPARE( updateId.toUpper(), referenceUpdateId );
    }

    delete doc;
}

void TestMetaData::checkNoPdfId()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/WithActualText.pdf");
    QVERIFY( doc );

    QVERIFY( !doc->getPdfId( 0, 0 ) );

    delete doc;
}

QTEST_MAIN(TestMetaData)
#include "check_metadata.moc"

