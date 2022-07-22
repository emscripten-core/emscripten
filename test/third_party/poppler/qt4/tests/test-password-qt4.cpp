#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QWidget>

#include <poppler-qt4.h>

class PDFDisplay : public QWidget           // picture display widget
{
public:
    PDFDisplay( Poppler::Document *d );
    ~PDFDisplay();
protected:
    void paintEvent( QPaintEvent * );
    void keyPressEvent( QKeyEvent * );
private:
    void display();
    int m_currentPage;
    QImage image;
    Poppler::Document *doc;
};

PDFDisplay::PDFDisplay( Poppler::Document *d )
{
    doc = d;
    m_currentPage = 0;
    display();
}

void PDFDisplay::display()
{
    if (doc) {
	Poppler::Page *page = doc->page(m_currentPage);
	if (page) {
	    qDebug() << "Displaying page: " << m_currentPage;
	    image = page->renderToImage();
	    update();
	    delete page;
	}
    } else {
	qWarning() << "doc not loaded";
    }
}

PDFDisplay::~PDFDisplay()
{
    delete doc;
}

void PDFDisplay::paintEvent( QPaintEvent *e )
{
    QPainter paint( this );                     // paint widget
    if (!image.isNull()) {
	paint.drawImage(0, 0, image);
    } else {
	qWarning() << "null image";
    }
}

void PDFDisplay::keyPressEvent( QKeyEvent *e )
{
  if (e->key() == Qt::Key_Down)
  {
    if (m_currentPage + 1 < doc->numPages())
    {
      m_currentPage++;
      display();
    }
  }
  else if (e->key() == Qt::Key_Up)
  {
    if (m_currentPage > 0)
    {
      m_currentPage--;
      display();
    }
  }
  else if (e->key() == Qt::Key_Q)
  {
      exit(0);
  }
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );               // QApplication required!

    if ( argc != 3)
    {
	qWarning() << "usage: test-password-qt4 owner-password filename";
	exit(1);
    }
  
    Poppler::Document *doc = Poppler::Document::load(argv[2], argv[1]);
    if (!doc)
    {
	qWarning() << "doc not loaded";
	exit(1);
    }
  
    // output some meta-data
    int major = 0, minor = 0;
    doc->getPdfVersion( &major, &minor );
    qDebug() << "    PDF Version: " << qPrintable(QString::fromLatin1("%1.%2").arg(major).arg(minor));
    qDebug() << "          Title: " << doc->info("Title");
    qDebug() << "        Subject: " << doc->info("Subject");
    qDebug() << "         Author: " << doc->info("Author");
    qDebug() << "      Key words: " << doc->info("Keywords");
    qDebug() << "        Creator: " << doc->info("Creator");
    qDebug() << "       Producer: " << doc->info("Producer");
    qDebug() << "   Date created: " << doc->date("CreationDate").toString();
    qDebug() << "  Date modified: " << doc->date("ModDate").toString();
    qDebug() << "Number of pages: " << doc->numPages();
    qDebug() << "     Linearised: " << doc->isLinearized();
    qDebug() << "      Encrypted: " << doc->isEncrypted();
    qDebug() << "    OK to print: " << doc->okToPrint();
    qDebug() << "     OK to copy: " << doc->okToCopy();
    qDebug() << "   OK to change: " << doc->okToChange();
    qDebug() << "OK to add notes: " << doc->okToAddNotes();
    qDebug() << "      Page mode: " << doc->pageMode();
    QStringList fontNameList;
    foreach( const Poppler::FontInfo &font, doc->fonts() )
	fontNameList += font.name();
    qDebug() << "          Fonts: " << fontNameList.join( ", " );

    Poppler::Page *page = doc->page(0);
    qDebug() << "    Page 1 size: " << page->pageSize().width()/72 << "inches x " << page->pageSize().height()/72 << "inches";

    PDFDisplay test( doc );        // create picture display
    test.setWindowTitle("Poppler-Qt4 Test");
    test.show();                            // show it

    return a.exec();                        // start event loop
}
