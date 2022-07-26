#include <QtTest/QtTest>

#include <poppler-qt4.h>

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
    doc = Poppler::Document::load("../../../test/unittestcases/bug7063.pdf");
    QVERIFY( doc );
   
    Poppler::Page *page = doc->page(0);
    QRectF pageRegion( QPointF(0,0), page->pageSize() );

    QCOMPARE( page->search(QString("non-ascii:"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );

    QCOMPARE( page->search(QString("Ascii"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );
    QCOMPARE( page->search(QString("Ascii"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseInsensitive), true );

    QCOMPARE( page->search(QString("latin1:"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );

    QCOMPARE( page->search(QString::fromUtf8("é"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("à"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("ç"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("search \"é\", \"à\" or \"ç\""), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("¥µ©"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("¥©"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );
  
    delete doc;
}

void TestSearch::testNextAndPrevious()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/xr01.pdf");
    QVERIFY( doc );
  
    Poppler::Page *page = doc->page(0);
    QRectF region( QPointF(0,0), page->pageSize() );
    
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(region.x() - 161.44) < 0.01 );
    QVERIFY( qAbs(region.y() - 127.85) < 0.01 );
    QVERIFY( qAbs(region.width() - 6.70) < 0.01 );
    QVERIFY( qAbs(region.height() - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(region.x() - 171.46) < 0.01 );
    QVERIFY( qAbs(region.y() - 127.85) < 0.01 );
    QVERIFY( qAbs(region.width() - 6.70) < 0.01 );
    QVERIFY( qAbs(region.height() - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(region.x() - 161.44) < 0.01 );
    QVERIFY( qAbs(region.y() - 139.81) < 0.01 );
    QVERIFY( qAbs(region.width() - 6.70) < 0.01 );
    QVERIFY( qAbs(region.height() - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(region.x() - 171.46) < 0.01 );
    QVERIFY( qAbs(region.y() - 139.81) < 0.01 );
    QVERIFY( qAbs(region.width() - 6.70) < 0.01 );
    QVERIFY( qAbs(region.height() - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::NextResult, Poppler::Page::CaseSensitive), false );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(region.x() - 161.44) < 0.01 );
    QVERIFY( qAbs(region.y() - 139.81) < 0.01 );
    QVERIFY( qAbs(region.width() - 6.70) < 0.01 );
    QVERIFY( qAbs(region.height() - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(region.x() - 171.46) < 0.01 );
    QVERIFY( qAbs(region.y() - 127.85) < 0.01 );
    QVERIFY( qAbs(region.width() - 6.70) < 0.01 );
    QVERIFY( qAbs(region.height() - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), true );
    QVERIFY( qAbs(region.x() - 161.44) < 0.01 );
    QVERIFY( qAbs(region.y() - 127.85) < 0.01 );
    QVERIFY( qAbs(region.width() - 6.70) < 0.01 );
    QVERIFY( qAbs(region.height() - 8.85) < 0.01 );
    QCOMPARE( page->search(QString("is"), region, Poppler::Page::PreviousResult, Poppler::Page::CaseSensitive), false );
    
    delete doc;
}

QTEST_MAIN(TestSearch)
#include "check_search.moc"

