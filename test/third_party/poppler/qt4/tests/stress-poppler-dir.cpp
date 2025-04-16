#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTime>
#include <QtGui/QApplication>
#include <QtGui/QImage>

#include <iostream>

#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );               // QApplication required!

    QTime t;
    t.start();

    QDir directory( argv[1] );
    foreach ( const QString &fileName, directory.entryList() ) {
        if (fileName.endsWith("pdf") ) {
	    qDebug() << "Doing" << fileName.toLatin1().data() << ":";
	    Poppler::Document *doc = Poppler::Document::load( directory.canonicalPath()+"/"+fileName );
	    if (!doc) {
		qWarning() << "doc not loaded";
	    } else if ( doc->isLocked() ) {
	        if (! doc->unlock( "", "password" ) ) {
		    qWarning() << "couldn't unlock document";
		    delete doc;
		}
	    } else {
		int major = 0, minor = 0;
		doc->getPdfVersion( &major, &minor );
		doc->info("Title");
		doc->info("Subject");
		doc->info("Author");
		doc->info("Keywords");
		doc->info("Creator");
		doc->info("Producer");
		doc->date("CreationDate").toString();
		doc->date("ModDate").toString();
		doc->numPages();
		doc->isLinearized();
		doc->isEncrypted();
		doc->okToPrint();
		doc->okToCopy();
		doc->okToChange();
		doc->okToAddNotes();
		doc->pageMode();

		for( int index = 0; index < doc->numPages(); ++index ) {
		    Poppler::Page *page = doc->page( index );
		    QImage image = page->renderToImage();
		    page->pageSize();
		    page->orientation();
		    delete page;
		    std::cout << ".";
		    std::cout.flush();
		}
		std::cout << std::endl;
		delete doc;
	    }
	}
    }

    std::cout << "Elapsed time: " << (t.elapsed()/1000) << "seconds" << std::endl;

}
