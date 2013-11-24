#include <QtTest/QtTest>

#include <poppler-qt5.h>

class TestSearch: public QObject
{
    Q_OBJECT
private slots:
    void bug7063();
    void testNextAndPrevious();
};

void TestSearch::bug7063()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load(TESTDATADIR "/unittestcases/bug7063.pdf");
    QVERIFY( doc );
   
    Poppler::Page *page = doc->page(0);
    double rectLeft, rectTop, rectRight, rectBottom;
    rectLeft = 0;
    rectTop = 0;
    rectRight = page->pageSize().width();
    rectBottom = page->pageSize().height();
    QCOMPARE( page->search(QString("non-ascii:"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );

    QCOMPARE( page->search(QString("Ascii"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );
    QCOMPARE( page->search(QString("Ascii"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseInsensitive), true );

    QCOMPARE( page->search(QString("latin1:"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );

    QCOMPARE( page->search(QString::fromUtf8("é"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("à"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("ç"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("search \"é\", \"à\" or \"ç\""), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("¥µ©"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("¥©"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );
  
    delete doc;
}

void TestSearch::testNextAndPrevious()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load(TESTDATADIR "/unittestcases/xr01.pdf");
    QVERIFY( doc );
  
    Poppler::Page *page = doc->page(0);
    double rectLeft, rectTop, rectRight, rectBottom;
    rectLeft = 0;
    rectTop = 0;
    rectRight = page->pageSize().width();
    rectBottom = page->pageSize().height();
    
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(rectLeft - 161.44) < 0.01 );
    QVERIFY( qAbs(rectTop - 127.85) < 0.01 );
    QVERIFY( qAbs(rectRight - rectLeft - 6.70) < 0.01 );
    QVERIFY( qAbs(rectBottom - rectTop - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(rectLeft - 171.46) < 0.01 );
    QVERIFY( qAbs(rectTop - 127.85) < 0.01 );
    QVERIFY( qAbs(rectRight - rectLeft - 6.70) < 0.01 );
    QVERIFY( qAbs(rectBottom - rectTop - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(rectLeft - 161.44) < 0.01 );
    QVERIFY( qAbs(rectTop - 139.81) < 0.01 );
    QVERIFY( qAbs(rectRight - rectLeft - 6.70) < 0.01 );
    QVERIFY( qAbs(rectBottom - rectTop - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(rectLeft - 171.46) < 0.01 );
    QVERIFY( qAbs(rectTop - 139.81) < 0.01 );
    QVERIFY( qAbs(rectRight - rectLeft - 6.70) < 0.01 );
    QVERIFY( qAbs(rectBottom - rectTop - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), false );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(rectLeft - 161.44) < 0.01 );
    QVERIFY( qAbs(rectTop - 139.81) < 0.01 );
    QVERIFY( qAbs(rectRight - rectLeft - 6.70) < 0.01 );
    QVERIFY( qAbs(rectBottom - rectTop - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(rectLeft - 171.46) < 0.01 );
    QVERIFY( qAbs(rectTop - 127.85) < 0.01 );
    QVERIFY( qAbs(rectRight - rectLeft - 6.70) < 0.01 );
    QVERIFY( qAbs(rectBottom - rectTop - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(rectLeft - 161.44) < 0.01 );
    QVERIFY( qAbs(rectTop - 127.85) < 0.01 );
    QVERIFY( qAbs(rectRight - rectLeft - 6.70) < 0.01 );
    QVERIFY( qAbs(rectBottom - rectTop - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), rectLeft, rectTop, rectRight, rectBottom, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), false );
    
    delete doc;
}

QTEST_MAIN(TestSearch)
#include "check_search.moc"

