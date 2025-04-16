#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <iostream>

#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );               // QApplication required!

    if (!( argc == 2 ))
    {
	qWarning() << "usage: poppler-fonts filename";
	exit(1);
    }
  
    Poppler::Document *doc = Poppler::Document::load(argv[1]);
    if (!doc)
    {
	qWarning() << "doc not loaded";
	exit(1);
    }

    std::cout << "name                                 type         emb sub font file";
    std::cout << std::endl;
    std::cout << "------------------------------------ ------------ --- --- ---------";
    std::cout << std::endl;
  
    foreach( const Poppler::FontInfo &font, doc->fonts() ) {
	if (font.name().isNull()) {
	    std::cout << qPrintable( QString("%1").arg(QString("[none]"), -37) );
	} else {
	    std::cout << qPrintable( QString("%1").arg(font.name(), -37) );
	}
	switch( font.type() ) {
	case Poppler::FontInfo::unknown:
	    std::cout << "unknown           ";
	    break;
	case Poppler::FontInfo::Type1:
	    std::cout << "Type 1            ";
	    break;
	case Poppler::FontInfo::Type1C:
	    std::cout << "Type 1C           ";
	    break;
	case Poppler::FontInfo::Type3:
	    std::cout << "Type 3            ";
	    break;
	case Poppler::FontInfo::TrueType:
	    std::cout << "TrueType          ";
	    break;
	case Poppler::FontInfo::CIDType0:
	    std::cout << "CID Type 0        ";
	    break;
	case Poppler::FontInfo::CIDType0C:
	    std::cout << "CID Type 0C       ";
	    break;
	case Poppler::FontInfo::CIDTrueType:
	    std::cout << "CID TrueType      ";
	    break;
	case Poppler::FontInfo::Type1COT:
	    std::cout << "Type 1C (OT)      ";
	    break;
	case Poppler::FontInfo::TrueTypeOT:
	    std::cout << "TrueType (OT)     ";
	    break;
	case Poppler::FontInfo::CIDType0COT:
	    std::cout << "CID Type 0C (OT)  ";
	    break;
	case Poppler::FontInfo::CIDTrueTypeOT:
	    std::cout << "CID TrueType (OT) ";
	    break;
	}

	if ( font.isEmbedded() ) {
	    std::cout << "yes ";
	} else {
	    std::cout << "no  ";
	}
	if ( font.isSubset() ) {
	    std::cout << "yes ";
	} else {
	    std::cout << "no  ";
	}
	std::cout << qPrintable( QString("%1").arg(font.file()) );
	std::cout << std::endl;
    }
    delete doc;
}
