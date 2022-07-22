#include <QtTest/QtTest>

#include <poppler-qt4.h>

#include <memory>

class TestLinks : public QObject
{
    Q_OBJECT
private slots:
    void checkDocumentWithNoDests();
    void checkDests_xr01();
    void checkDests_xr02();
};

bool isDestinationValid_pageNumber( const Poppler::LinkDestination *dest, const Poppler::Document *doc )
{
    return dest->pageNumber() > 0 && dest->pageNumber() <= doc->numPages();
}

bool isDestinationValid_name( const Poppler::LinkDestination *dest )
{
    return !dest->destinationName().isEmpty();
}


void TestLinks::checkDocumentWithNoDests()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/WithAttachments.pdf");
    QVERIFY( doc );

    std::auto_ptr< Poppler::LinkDestination > dest;
    dest.reset( doc->linkDestination("no.dests.in.this.document") );
    QVERIFY( !isDestinationValid_pageNumber( dest.get(), doc ) );
    QVERIFY( isDestinationValid_name( dest.get() ) );

    delete doc;
}

void TestLinks::checkDests_xr01()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/xr01.pdf");
    QVERIFY( doc );

    Poppler::Page *page = doc->page(0);
    QVERIFY( page );

    QList< Poppler::Link* > links = page->links();
    QCOMPARE( links.count(), 2 );

    {
    QCOMPARE( links.at(0)->linkType(), Poppler::Link::Goto );
    Poppler::LinkGoto *link = static_cast< Poppler::LinkGoto * >( links.at(0) );
    const Poppler::LinkDestination dest = link->destination();
    QVERIFY( !isDestinationValid_pageNumber( &dest, doc ) );
    QVERIFY( isDestinationValid_name( &dest ) );
    QCOMPARE( dest.destinationName(), QString::fromLatin1("section.1") );
    }

    {
    QCOMPARE( links.at(1)->linkType(), Poppler::Link::Goto );
    Poppler::LinkGoto *link = static_cast< Poppler::LinkGoto * >( links.at(1) );
    const Poppler::LinkDestination dest = link->destination();
    QVERIFY( !isDestinationValid_pageNumber( &dest, doc ) );
    QVERIFY( isDestinationValid_name( &dest ) );
    QCOMPARE( dest.destinationName(), QString::fromLatin1("section.2") );
    }

    delete doc;
}

void TestLinks::checkDests_xr02()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/xr02.pdf");
    QVERIFY( doc );

    std::auto_ptr< Poppler::LinkDestination > dest;
    dest.reset( doc->linkDestination("section.1") );
    QVERIFY( isDestinationValid_pageNumber( dest.get(), doc ) );
    QVERIFY( !isDestinationValid_name( dest.get() ) );
    dest.reset( doc->linkDestination("section.2") );
    QVERIFY( isDestinationValid_pageNumber( dest.get(), doc ) );
    QVERIFY( !isDestinationValid_name( dest.get() ) );
    dest.reset( doc->linkDestination("section.3") );
    QVERIFY( !isDestinationValid_pageNumber( dest.get(), doc ) );
    QVERIFY( isDestinationValid_name( dest.get() ) );

    delete doc;
}

QTEST_MAIN(TestLinks)

#include "check_links.moc"
