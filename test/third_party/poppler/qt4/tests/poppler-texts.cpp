#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <iostream>

#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );               // QApplication required!

    if (!( argc == 2 ))
    {
	qWarning() << "usage: poppler-texts filename";
	exit(1);
    }
  
    Poppler::Document *doc = Poppler::Document::load(argv[1]);
    if (!doc)
    {
	qWarning() << "doc not loaded";
	exit(1);
    }

    for ( int i = 0; i < doc->numPages(); i++ )
    {
      int j = 0;
      std::cout << "*** Page " << i << std::endl;
      std::cout << std::flush;

      Poppler::Page *page = doc->page(i);
      const QByteArray utf8str = page->text( QRectF(), Poppler::Page::RawOrderLayout ).toUtf8();
      std::cout << std::flush;
      for ( j = 0; j < utf8str.size(); j++ )
        std::cout << utf8str[j];
      std::cout << std::endl;
      delete page;
    }
    delete doc;
}
