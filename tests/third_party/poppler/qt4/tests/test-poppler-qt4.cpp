#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QToolTip>
#include <QtGui/QWidget>

#include <poppler-qt4.h>

class PDFDisplay : public QWidget           // picture display widget
{
public:
    PDFDisplay( Poppler::Document *d, bool arthur );
    ~PDFDisplay();
    void setShowTextRects(bool show);
    void display();
protected:
    void paintEvent( QPaintEvent * );
    void keyPressEvent( QKeyEvent * );
    void mousePressEvent( QMouseEvent * );
private:
    int m_currentPage;
    QImage image;
    Poppler::Document *doc;
    QString backendString;
    bool showTextRects;
    QList<Poppler::TextBox*> textRects;
};

PDFDisplay::PDFDisplay( Poppler::Document *d, bool arthur )
{
    showTextRects = false;
    doc = d;
    m_currentPage = 0;
    if (arthur)
    {
        backendString = "Arthur";
        doc->setRenderBackend(Poppler::Document::ArthurBackend);
    }
    else
    {
        backendString = "Splash";
        doc->setRenderBackend(Poppler::Document::SplashBackend);
    }
    doc->setRenderHint(Poppler::Document::Antialiasing, true);
    doc->setRenderHint(Poppler::Document::TextAntialiasing, true);
}

void PDFDisplay::setShowTextRects(bool show)
{
    showTextRects = show;
}

void PDFDisplay::display()
{
    if (doc) {
        Poppler::Page *page = doc->page(m_currentPage);
        if (page) {
            qDebug() << "Displaying page using" << backendString << "backend: " << m_currentPage;
            QTime t = QTime::currentTime();
            image = page->renderToImage();
            qDebug() << "Rendering took" << t.msecsTo(QTime::currentTime()) << "msecs";
            qDeleteAll(textRects);
            if (showTextRects)
            {
                QPainter painter(&image);
                painter.setPen(Qt::red);
                textRects = page->textList();
                foreach(Poppler::TextBox *tb, textRects)
                {
                    painter.drawRect(tb->boundingBox());
                }
            }
            else textRects.clear();
            update();
            delete page;
        }
    } else {
        qWarning() << "doc not loaded";
    }
}

PDFDisplay::~PDFDisplay()
{
    qDeleteAll(textRects);
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

void PDFDisplay::mousePressEvent( QMouseEvent *e )
{
  int i = 0;
  foreach(Poppler::TextBox *tb, textRects)
  {
    if (tb->boundingBox().contains(e->pos()))
    {
      QString tt = QString("Text: \"%1\"\nIndex in text list: %2").arg(tb->text()).arg(i);
      QToolTip::showText(e->globalPos(), tt, this);
      break;
    }
    ++i;
  }
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );               // QApplication required!

    if ( argc < 2 ||
        (argc == 3 && strcmp(argv[2], "-extract") != 0 && strcmp(argv[2], "-arthur") != 0 && strcmp(argv[2], "-textRects") != 0) ||
        argc > 3)
    {
	// use argument as file name
	qWarning() << "usage: test-poppler-qt filename [-extract|-arthur|-textRects]";
	exit(1);
    }
  
    Poppler::Document *doc = Poppler::Document::load(QFile::decodeName(argv[1]));
    if (!doc)
    {
	qWarning() << "doc not loaded";
	exit(1);
    }

    if (doc->isLocked())
    {
	qWarning() << "document locked (needs password)";
	exit(0);
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
    qDebug() << "       Metadata: " << doc->metadata();
    QStringList fontNameList;
    foreach( const Poppler::FontInfo &font, doc->fonts() )
	fontNameList += font.name();
    qDebug() << "          Fonts: " << fontNameList.join( ", " );

    if ( doc->hasEmbeddedFiles() ) {
        qDebug() << "Embedded files:";
        foreach( Poppler::EmbeddedFile *file, doc->embeddedFiles() ) {
	    qDebug() << "   " << file->name();
	}
	qDebug();
    } else {
        qDebug() << "No embedded files";
    }

    if (doc->numPages() <= 0)
    {
        delete doc;
        qDebug() << "Doc has no pages";
        return 0;
    }

    Poppler::Page *page = doc->page(0);
    qDebug() << "Page 1 size: " << page->pageSize().width()/72 << "inches x " << page->pageSize().height()/72 << "inches";
    delete page;

    if (argc == 2 || (argc == 3 && strcmp(argv[2], "-arthur") == 0) || (argc == 3 && strcmp(argv[2], "-textRects") == 0))
    {
        bool useArthur = (argc == 3 && strcmp(argv[2], "-arthur") == 0);
        PDFDisplay test( doc, useArthur );        // create picture display
        test.setWindowTitle("Poppler-Qt4 Test");
        test.setShowTextRects(argc == 3 && strcmp(argv[2], "-textRects") == 0);
        test.display();
        test.show();                            // show it

        return a.exec();                        // start event loop
    }
    else
    {
	Poppler::Page *page = doc->page(0);

	QLabel *l = new QLabel(page->text(QRectF()), 0);
	l->show();
	delete page;
	delete doc;
	return a.exec();
    }
}
