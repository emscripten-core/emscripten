#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <iostream>

#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );               // QApplication required!

    if (!( argc == 2 ))
    {
	qWarning() << "usage: poppler-attachments filename";
	exit(1);
    }
  
    Poppler::Document *doc = Poppler::Document::load(argv[1]);
    if (!doc)
    {
	qWarning() << "doc not loaded";
	exit(1);
    }

    if (doc->hasEmbeddedFiles()) {
	std::cout << "Embedded files: " << std::endl;
	foreach(Poppler::EmbeddedFile *file, doc->embeddedFiles()) {
	    std::cout << "    " << qPrintable(file->name()) << std::endl;
	    std::cout << "    desc:" << qPrintable(file->description()) << std::endl;
	    QByteArray data = file->data();
	    std::cout << "       data: " << data.constData() << std::endl;
	}
	
    } else {
	std::cout << "There are no embedded document at the top level" << std::endl;
    }
    delete doc;
  
}
