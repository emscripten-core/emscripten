#include <QtTest/QtTest>

#include <poppler-qt4.h>

class TestPassword: public QObject
{
    Q_OBJECT
private slots:
    void password1();
    void password1a();
    void password2();
    void password2a();
    void password2b();
    void password3();
};


// BUG:4557
void TestPassword::password1()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load(QString::fromUtf8("../../../test/unittestcases/Gday garçon - open.pdf"), "", QString::fromUtf8("garçon").toLatin1() );
    QVERIFY( doc );
    QVERIFY( !doc->isLocked() );

    delete doc;
}


void TestPassword::password1a()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load(QString::fromUtf8("../../../test/unittestcases/Gday garçon - open.pdf") );
    QVERIFY( doc );
    QVERIFY( doc->isLocked() );
    QVERIFY( !doc->unlock( "", QString::fromUtf8("garçon").toLatin1() ) );
    QVERIFY( !doc->isLocked() );

    delete doc;
}

void TestPassword::password2()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load(QString::fromUtf8("../../../test/unittestcases/Gday garçon - owner.pdf"), QString::fromUtf8("garçon").toLatin1(), "" );
    QVERIFY( doc );
    QVERIFY( !doc->isLocked() );

    delete doc;
}

void TestPassword::password2a()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load(QString::fromUtf8("../../../test/unittestcases/Gday garçon - owner.pdf"), QString::fromUtf8("garçon").toLatin1() );
    QVERIFY( doc );
    QVERIFY( !doc->isLocked() );

    delete doc;
}

void TestPassword::password2b()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load(QString::fromUtf8("../../../test/unittestcases/Gday garçon - owner.pdf") );
    QVERIFY( doc );
    QVERIFY( !doc->isLocked() );
    QVERIFY( !doc->unlock( QString::fromUtf8("garçon").toLatin1(), "" ) );
    QVERIFY( !doc->isLocked() );

    delete doc;
}

void TestPassword::password3()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load( QString::fromUtf8("../../../test/unittestcases/PasswordEncrypted.pdf") );
    QVERIFY( doc );
    QVERIFY( doc->isLocked() );
    QVERIFY( !doc->unlock( "", "password" ) );
    QVERIFY( !doc->isLocked() );

    delete doc;
}

QTEST_MAIN(TestPassword)
#include "check_password.moc"

