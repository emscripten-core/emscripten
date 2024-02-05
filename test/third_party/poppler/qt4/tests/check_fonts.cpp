#include <QtTest/QtTest>

#include <poppler-qt4.h>

#include <memory>

class TestFontsData: public QObject
{
    Q_OBJECT
private slots:
    void checkNoFonts();
    void checkType1();
    void checkType3();
    void checkTrueType();
    void checkFontIterator();
    void checkSecondDocumentQuery();
    void checkMultipleIterations();
    void checkScanForFonts();
};


QList<Poppler::FontInfo> loadFontsViaIterator( Poppler::Document *doc, int from = 0, int count = -1 )
{
    int num = count == -1 ? doc->numPages() - from : count;
    QList<Poppler::FontInfo> list;
    std::auto_ptr< Poppler::FontIterator > it( doc->newFontIterator( from ) );
    while ( it->hasNext() && num )
    {
        list += it->next();
        --num;
    }
    return list;
}

bool operator==( const Poppler::FontInfo &f1, const Poppler::FontInfo &f2 )
{
    if ( f1.name() != f2.name() )
        return false;
    if ( f1.file() != f2.file() )
        return false;
    if ( f1.isEmbedded() != f2.isEmbedded() )
        return false;
    if ( f1.isSubset() != f2.isSubset() )
        return false;
    if ( f1.type() != f2.type() )
        return false;
    if ( f1.typeName() != f2.typeName() )
        return false;
    return true;
}


void TestFontsData::checkNoFonts()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/image.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 0 );

    delete doc;
}

void TestFontsData::checkType1()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/text.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 1 );
    QCOMPARE( listOfFonts.at(0).name(), QString("Helvetica") );
    QCOMPARE( listOfFonts.at(0).type(), Poppler::FontInfo::Type1 );
    QCOMPARE( listOfFonts.at(0).typeName(), QString("Type 1") );

    QCOMPARE( listOfFonts.at(0).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(0).isSubset(), false );

    delete doc;
}

void TestFontsData::checkType3()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/type3.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 2 );
    QCOMPARE( listOfFonts.at(0).name(), QString("Helvetica") );
    QCOMPARE( listOfFonts.at(0).type(), Poppler::FontInfo::Type1 );
    QCOMPARE( listOfFonts.at(0).typeName(), QString("Type 1") );

    QCOMPARE( listOfFonts.at(0).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(0).isSubset(), false );

    QCOMPARE( listOfFonts.at(1).name(), QString("") );
    QCOMPARE( listOfFonts.at(1).type(), Poppler::FontInfo::Type3 );
    QCOMPARE( listOfFonts.at(1).typeName(), QString("Type 3") );

    QCOMPARE( listOfFonts.at(1).isEmbedded(), true );
    QCOMPARE( listOfFonts.at(1).isSubset(), false );

    delete doc;
}

void TestFontsData::checkTrueType()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 2 );
    QCOMPARE( listOfFonts.at(0).name(), QString("Arial-BoldMT") );
    QCOMPARE( listOfFonts.at(0).type(), Poppler::FontInfo::TrueType );
    QCOMPARE( listOfFonts.at(0).typeName(), QString("TrueType") );

    QCOMPARE( listOfFonts.at(0).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(0).isSubset(), false );

    QCOMPARE( listOfFonts.at(1).name(), QString("ArialMT") );
    QCOMPARE( listOfFonts.at(1).type(), Poppler::FontInfo::TrueType );
    QCOMPARE( listOfFonts.at(1).typeName(), QString("TrueType") );

    QCOMPARE( listOfFonts.at(1).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(1).isSubset(), false );

    delete doc;
}

void TestFontsData::checkFontIterator()
{
    // loading a 1-page document
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/type3.pdf");
    QVERIFY( doc );
    // loading a 6-pages document
    Poppler::Document *doc6 = Poppler::Document::load("../../../test/tests/cropbox.pdf");
    QVERIFY( doc6 );

    std::auto_ptr< Poppler::FontIterator > it;

    // some tests with the 1-page document:
    // - check a default iterator
    it.reset( doc->newFontIterator() );
    QVERIFY( it->hasNext() );
    // - check an iterator for negative pages to behave as 0
    it.reset( doc->newFontIterator( -1 ) );
    QVERIFY( it->hasNext() );
    // - check an iterator for pages out of the page limit
    it.reset( doc->newFontIterator( 1 ) );
    QVERIFY( !it->hasNext() );
    // - check that it reaches the end after 1 iteration
    it.reset( doc->newFontIterator() );
    QVERIFY( it->hasNext() );
    it->next();
    QVERIFY( !it->hasNext() );

    // some tests with the 6-page document:
    // - check a default iterator
    it.reset( doc6->newFontIterator() );
    QVERIFY( it->hasNext() );
    // - check an iterator for pages out of the page limit
    it.reset( doc6->newFontIterator( 6 ) );
    QVERIFY( !it->hasNext() );
    // - check that it reaches the end after 6 iterations
    it.reset( doc6->newFontIterator() );
    QVERIFY( it->hasNext() );
    it->next();
    QVERIFY( it->hasNext() );
    it->next();
    QVERIFY( it->hasNext() );
    it->next();
    QVERIFY( it->hasNext() );
    it->next();
    QVERIFY( it->hasNext() );
    it->next();
    QVERIFY( it->hasNext() );
    it->next();
    QVERIFY( !it->hasNext() );

    delete doc;
    delete doc6;
}

void TestFontsData::checkSecondDocumentQuery()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/type3.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 2 );
    // check we get the very same result when calling fonts() again (#19405)
    QList<Poppler::FontInfo> listOfFonts2 = doc->fonts();
    QCOMPARE( listOfFonts, listOfFonts2 );

    delete doc;
}

void TestFontsData::checkMultipleIterations()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/type3.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = loadFontsViaIterator( doc );
    QCOMPARE( listOfFonts.size(), 2 );
    QList<Poppler::FontInfo> listOfFonts2 = loadFontsViaIterator( doc );
    QCOMPARE( listOfFonts, listOfFonts2 );

    delete doc;
}

void TestFontsData::checkScanForFonts()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/fonts.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 3 );
    // check we get the very same result when gatering fonts using scanForFonts
    QList<Poppler::FontInfo> listOfFonts2;
    for ( int i = 0; i < doc->numPages(); ++i )
    {
        doc->scanForFonts( 1, &listOfFonts2 );
    }
    QCOMPARE( listOfFonts, listOfFonts2 );

   // check doing a second scanForFonts gives no result
    QList<Poppler::FontInfo> listOfFonts3;
    for ( int i = 0; i < doc->numPages(); ++i )
    {
        doc->scanForFonts( 1, &listOfFonts3 );
    }
    QVERIFY( listOfFonts3.isEmpty() );

    delete doc;
}

QTEST_MAIN(TestFontsData)
#include "check_fonts.moc"

