#include <QtTest/QtTest>

#include "PDFDoc.h"

#include <poppler-qt4.h>

class TestOptionalContent: public QObject
{
    Q_OBJECT
private slots:
    void checkVisPolicy();
    void checkNestedLayers();
    void checkNoOptionalContent();
    void checkIsVisible();
    void checkVisibilitySetting();
    void checkRadioButtons();
};

void TestOptionalContent::checkVisPolicy()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/vis_policy_test.pdf");
    QVERIFY( doc );

    QVERIFY( doc->hasOptionalContent() );

    Poppler::OptContentModel *optContent = doc->optionalContentModel();
    QModelIndex index;
    index = optContent->index( 0, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "A" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Checked );
    index = optContent->index( 1, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "B" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Checked );

    delete doc;
}

void TestOptionalContent::checkNestedLayers()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/NestedLayers.pdf");
    QVERIFY( doc );

    QVERIFY( doc->hasOptionalContent() );

    Poppler::OptContentModel *optContent = doc->optionalContentModel();
    QModelIndex index;

    index = optContent->index( 0, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Black Text and Green Snow" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    index = optContent->index( 1, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Mountains and Image" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Checked );

    // This is a sub-item of "Mountains and Image"
    QModelIndex subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Image" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Checked );

    index = optContent->index( 2, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Starburst" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Checked );

    index = optContent->index( 3, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Watermark" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    delete doc;
}

void TestOptionalContent::checkNoOptionalContent()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->hasOptionalContent(), false );

    delete doc;
}

void TestOptionalContent::checkIsVisible()
{
    GooString *fileName = new GooString("../../../test/unittestcases/vis_policy_test.pdf"); 
    PDFDoc *doc = new PDFDoc( fileName );
    QVERIFY( doc );

    OCGs *ocgs = doc->getOptContentConfig();
    QVERIFY( ocgs );

    XRef *xref = doc->getXRef();

    Object obj;

    // In this test, both Ref(21,0) and Ref(2,0) are set to On

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QVERIFY( ocgs->optContentIsVisible( &obj ) );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QVERIFY( ocgs->optContentIsVisible( &obj ) );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();


    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    delete doc;
}

void TestOptionalContent::checkVisibilitySetting()
{
    GooString *fileName = new GooString("../../../test/unittestcases/vis_policy_test.pdf"); 
    PDFDoc *doc = new PDFDoc( fileName );
    QVERIFY( doc );

    OCGs *ocgs = doc->getOptContentConfig();
    QVERIFY( ocgs );

    XRef *xref = doc->getXRef();

    Object obj;

    // In this test, both Ref(21,0) and Ref(28,0) start On,
    // based on the file settings
    Object ref21obj;
    ref21obj.initRef( 21, 0 );
    Ref ref21 = ref21obj.getRef();
    OptionalContentGroup *ocgA = ocgs->findOcgByRef( ref21 );
    QVERIFY( ocgA );

    QVERIFY( (ocgA->getName()->cmp("A")) == 0 );
    QCOMPARE( ocgA->getState(), OptionalContentGroup::On );

    Object ref28obj;
    ref28obj.initRef( 28, 0 );
    Ref ref28 = ref28obj.getRef();
    OptionalContentGroup *ocgB = ocgs->findOcgByRef( ref28 );
    QVERIFY( ocgB );

    QVERIFY( (ocgB->getName()->cmp("B")) == 0 );
    QCOMPARE( ocgB->getState(), OptionalContentGroup::On );

    // turn one Off
    ocgA->setState( OptionalContentGroup::Off );

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();


    // Turn the other one off as well (i.e. both are Off)
    ocgB->setState(OptionalContentGroup::Off);

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();


    // Turn the first one on again (21 is On, 28 is Off)
    ocgA->setState(OptionalContentGroup::On);

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    delete doc;
}

void TestOptionalContent::checkRadioButtons()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/ClarityOCGs.pdf");
    QVERIFY( doc );

    QVERIFY( doc->hasOptionalContent() );

    Poppler::OptContentModel *optContent = doc->optionalContentModel();
    QModelIndex index;

    index = optContent->index( 0, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Languages" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( index, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    // These are sub-items of the "Languages" label
    QModelIndex subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Checked );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    // RBGroup of languages, so turning on Japanese should turn off English
    bool result = optContent->setData( subindex, QVariant( true ), Qt::CheckStateRole );

    subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Checked );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    // and turning on French should turn off Japanese
    result = optContent->setData( subindex, QVariant( true ), Qt::CheckStateRole );

    subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Checked );


    // and turning off French should leave them all off
    result = optContent->setData( subindex, QVariant( false ), Qt::CheckStateRole );

    subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    QCOMPARE( static_cast<Qt::CheckState>( optContent->data( subindex, Qt::CheckStateRole ).toInt() ), Qt::Unchecked );

    delete doc;
}

QTEST_MAIN(TestOptionalContent)

#include "check_optcontent.moc"

