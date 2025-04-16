#include <QtTest/QtTest>

#include <poppler-qt4.h>

class TestPageMode: public QObject
{
    Q_OBJECT
private slots:
    void checkNone();
    void checkFullScreen();
    void checkAttachments();
    void checkThumbs();
    void checkOC();
};

void TestPageMode::checkNone()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseNone.pdf");
    QVERIFY( doc );
  
    QCOMPARE( doc->pageMode(), Poppler::Document::UseNone );

    delete doc;
}

void TestPageMode::checkFullScreen()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/FullScreen.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->pageMode(), Poppler::Document::FullScreen );

    delete doc;
}

void TestPageMode::checkAttachments()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseAttachments.pdf");
    QVERIFY( doc );
  
    QCOMPARE( doc->pageMode(), Poppler::Document::UseAttach );

    delete doc;
}

void TestPageMode::checkThumbs()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseThumbs.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->pageMode(), Poppler::Document::UseThumbs );

    delete doc;
}

void TestPageMode::checkOC()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseOC.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->pageMode(), Poppler::Document::UseOC );

    delete doc;
}

QTEST_MAIN(TestPageMode)
#include "check_pagemode.moc"

