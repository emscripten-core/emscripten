#include <QtTest/QtTest>

#include <poppler-qt4.h>

#include <QtCore/QFile>

class TestActualText: public QObject
{
    Q_OBJECT
private slots:
    void checkActualText1();
};

void TestActualText::checkActualText1()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/WithActualText.pdf");
    QVERIFY( doc );

    Poppler::Page *page = doc->page(0);
    QVERIFY( page );

    QCOMPARE( page->text(QRectF()), QString("The slow brown fox jumps over the black dog.") );

    delete page;

    delete doc;
}

QTEST_MAIN(TestActualText)

#include "check_actualtext.moc"

