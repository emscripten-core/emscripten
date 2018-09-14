// Copyright 2011 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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

