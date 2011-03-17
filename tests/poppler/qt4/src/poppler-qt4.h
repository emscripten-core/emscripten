/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, 2007, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2005-2010, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Stefan Kebekus <stefan.kebekus@math.uni-koeln.de>
 * Copyright (C) 2006-2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2009 Shawn Rutledge <shawn.t.rutledge@gmail.com>
 * Copyright (C) 2010 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
 * Copyright (C) 2010 Matthias Fauconneau <matthias.fauconneau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __POPPLER_QT_H__
#define __POPPLER_QT_H__

#include "poppler-annotation.h"
#include "poppler-link.h"
#include "poppler-optcontent.h"
#include "poppler-page-transition.h"

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QSet>
#include <QtXml/QDomDocument>
#include "poppler-export.h"

class EmbFile;
class Sound;
class AnnotMovie;

/**
   The %Poppler Qt4 binding.
*/
namespace Poppler {

    class Document;
    class DocumentData;

    class PageData;

    class FormField;

    class TextBoxData;

    class PDFConverter;
    class PSConverter;

    /**
	Debug/error function.

	This function type is used for debugging & error output;
	the first parameter is the actual message, the second is the unaltered
	closure argument which was passed to the setDebugErrorFunction call.

	\since 0.16
    */
    typedef void (*PopplerDebugFunc)(const QString & /*message*/, const QVariant & /*closure*/);

    /**
	Set a new debug/error output function.

	If not set, by default error and debug messages will be sent to the
	Qt \p qDebug() function.

	\param debugFunction the new debug function
	\param closure user data which will be passes as-is to the debug function

	\since 0.16
    */
    POPPLER_QT4_EXPORT void setDebugErrorFunction(PopplerDebugFunc debugFunction, const QVariant &closure);

    /**
        Describes the physical location of text on a document page
       
        This very simple class describes the physical location of text
	on the page. It consists of
	- a QString that contains the text
	- a QRectF that gives a box that describes where on the page
	the text is found.
    */
    class POPPLER_QT4_EXPORT TextBox {
    friend class Page;
    public:
      /**
	 The default constructor sets the \p text and the rectangle that
	 contains the text. Coordinated for the \p bBox are in points =
	 1/72 of an inch.
      */
      TextBox(const QString& text, const QRectF &bBox);
      /**
	Destructor.
      */
      ~TextBox();

      /**
	  Returns the text of this text box
      */
      QString text() const;

      /**
	  Returns the position of the text, in point, i.e., 1/72 of
	 an inch

	 \since 0.8
      */
      QRectF boundingBox() const;

      /**
	  Returns the pointer to the next text box, if there is one.

	  Otherwise, it returns a null pointer.
      */
      TextBox *nextWord() const;

      /**
	  Returns the bounding box of the \p i -th characted of the word.
      */
      QRectF charBoundingBox(int i) const;

      /**
	  Returns whether there is a space character after this text box
      */
      bool hasSpaceAfter() const;

    private:
        Q_DISABLE_COPY(TextBox)

	TextBoxData *m_data;
    };


    class FontInfoData;
    /**
       Container class for information about a font within a PDF
       document
    */
    class POPPLER_QT4_EXPORT FontInfo {
    friend class Document;
    public:
	/**
	   The type of font.
	*/
	enum Type {
		unknown,
		Type1,
		Type1C,
		Type1COT,
		Type3,
		TrueType,
		TrueTypeOT,
		CIDType0,
		CIDType0C,
		CIDType0COT,
		CIDTrueType,
		CIDTrueTypeOT
	};
	
	/// \cond PRIVATE
	/**
	   Create a new font information container.
	*/
	FontInfo();
	
	/**
	   Create a new font information container.
	*/
	FontInfo( const FontInfoData &fid );
	/// \endcond
	
	/**
	   Copy constructor.
	*/
	FontInfo( const FontInfo &fi );
	
	/**
	   Destructor.
	*/
	~FontInfo();

	/**
	   The name of the font. Can be QString::null if the font has no name
	*/
	QString name() const;

	/**
	   The path of the font file used to represent this font on this system,
	   or a null string is the font is embedded
	*/
	QString file() const;

	/**
	   Whether the font is embedded in the file, or not

	   \return true if the font is embedded
	*/
	bool isEmbedded() const;

	/**
	   Whether the font provided is only a subset of the full
	   font or not. This only has meaning if the font is embedded.

	   \return true if the font is only a subset
	*/
	bool isSubset() const;

	/**
	   The type of font encoding

	   \return a enumerated value corresponding to the font encoding used

	   \sa typeName for a string equivalent
	*/
	Type type() const;

	/**
	   The name of the font encoding used

	   \note if you are looking for the name of the font (as opposed to the
	   encoding format used), you probably want name().

	   \sa type for a enumeration version
	*/
	QString typeName() const;

	/**
	   Standard assignment operator
	*/
	FontInfo& operator=( const FontInfo &fi );

    private:
	FontInfoData *m_data;
    };


    class FontIteratorData;
    /**
       Iterator for reading the fonts in a document.

       FontIterator provides a Java-style iterator for reading the fonts in a
       document.

       You can use it in the following way:
       \code
Poppler::FontIterator* it = doc->newFontIterator();
while (it->hasNext()) {
  QList<Poppler::FontInfo> fonts = it->next();
  // do something with the fonts
}
// after doing the job, the iterator must be freed
delete it;
       \endcode

       \since 0.12
    */
    class POPPLER_QT4_EXPORT FontIterator {
    friend class Document;
    friend class DocumentData;
    public:
	/**
	   Destructor.
	*/
	~FontIterator();

	/**
	   Returns the fonts of the current page and then advances the iterator
	   to the next page.
	*/
	QList<FontInfo> next();

	/**
	   Checks whether there is at least one more page to iterate, ie returns
	   false when the iterator is beyond the last page.
	*/
	bool hasNext() const;

	/**
	   Returns the current page where the iterator is.
	*/
	int currentPage() const;

    private:
	Q_DISABLE_COPY( FontIterator )
	FontIterator( int, DocumentData *dd );

	FontIteratorData *d;
    };


    class EmbeddedFileData;
    /**
       Container class for an embedded file with a PDF document
    */
    class POPPLER_QT4_EXPORT EmbeddedFile {
    public:
	/// \cond PRIVATE
	EmbeddedFile(EmbFile *embfile);
	/// \endcond
	
	/**
	   Destructor.
	*/
	~EmbeddedFile();

	/**
	   The name associated with the file
	*/
	QString name() const;

	/**
	   The description associated with the file, if any.

	   This will return an empty QString if there is no description element
	*/
	QString description() const;

	/**
	   The size of the file.
	
	   This will return < 0 if there is no size element
	*/
	int size() const;

	/**
	   The modification date for the embedded file, if known.
	*/
	QDateTime modDate() const;

	/**
	   The creation date for the embedded file, if known.
	*/
	QDateTime createDate() const;
	
	/**
	   The MD5 checksum of the file.
		
	   This will return an empty QByteArray if there is no checksum element.
	*/
	QByteArray checksum() const;

	/**
	   The MIME type of the file, if known.
	
	   \since 0.8
	*/
	QString mimeType() const;

	/**
	   The data as a byte array
	*/
	QByteArray data();

	/**
	   Is the embedded file valid?
	   
	   \since 0.12
	*/
	bool isValid() const;

	/**
	   A QDataStream for the actual data?
	*/
	//QDataStream dataStream() const;

    private:
	Q_DISABLE_COPY(EmbeddedFile)

	EmbeddedFileData *m_embeddedFile;
    };


    /**
       \brief A page in a document.

       The Page class represents a single page within a PDF document.

       You cannot construct a Page directly, but you have to use the Document
       functions that return a new Page out of an index or a label.
    */
    class POPPLER_QT4_EXPORT Page {
	friend class Document;
    public:
	/**
	   Destructor.
	*/
	~Page();

	/**
	   The type of rotation to apply for an operation
	*/
	enum Rotation { Rotate0 = 0,   ///< Do not rotate
			Rotate90 = 1,  ///< Rotate 90 degrees clockwise
			Rotate180 = 2, ///< Rotate 180 degrees
			Rotate270 = 3  ///< Rotate 270 degrees clockwise (90 degrees counterclockwise)
	};

	/**
	   The kinds of page actions
	*/
	enum PageAction {
	    Opening,   ///< The action when a page is "opened"
	    Closing    ///< The action when a page is "closed"
	};
	
	/**
	   How the text is going to be returned
	   \since 0.16
	*/
	enum TextLayout {
	    PhysicalLayout,   ///< The text is layouted to resemble the real page layout
	    RawOrderLayout          ///< The text is returned without any type of processing
	};

        /**
           Additional flags for the renderToPainter method
           \since 0.16
        */
        enum PainterFlag {
            /**
               Do not save/restore the caller-owned painter.

               renderToPainter() by default preserves, using save() + restore(),
               the state of the painter specified; if this is not needed, this
               flag can avoid this job
             */
            DontSaveAndRestore = 0x00000001
        };
        Q_DECLARE_FLAGS( PainterFlags, PainterFlag )

	/** 
	   Render the page to a QImage using the current
	   \link Document::renderBackend() Document renderer\endlink.
	   
	   If \p x = \p y = \p w = \p h = -1, the method will automatically
           compute the size of the image from the horizontal and vertical
           resolutions specified in \p xres and \p yres. Otherwise, the
           method renders only a part of the page, specified by the
           parameters (\p x, \p y, \p w, \p h) in pixel coordinates. The returned
           QImage then has size (\p w, \p h), independent of the page
           size.

	   \param x specifies the left x-coordinate of the box, in
	   pixels.

	   \param y specifies the top y-coordinate of the box, in
	   pixels.

	   \param w specifies the width of the box, in pixels.

	   \param h specifies the height of the box, in pixels.

	   \param xres horizontal resolution of the graphics device,
	   in dots per inch

	   \param yres vertical resolution of the graphics device, in
	   dots per inch
	
	   \param rotate how to rotate the page

	   \warning The parameter (\p x, \p y, \p w, \p h) are not
	   well-tested. Unusual or meaningless parameters may lead to
	   rather unexpected results.

	   \returns a QImage of the page, or a null image on failure.

	   \since 0.6
        */
	QImage renderToImage(double xres=72.0, double yres=72.0, int x=-1, int y=-1, int w=-1, int h=-1, Rotation rotate = Rotate0) const;

        /**
           Render the page to the specified QPainter using the current
           \link Document::renderBackend() Document renderer\endlink.

           If \p x = \p y = \p w = \p h = -1, the method will automatically
           compute the size of the page area from the horizontal and vertical
           resolutions specified in \p xres and \p yres. Otherwise, the
           method renders only a part of the page, specified by the
           parameters (\p x, \p y, \p w, \p h) in pixel coordinates.

           \param painter the painter to paint on

           \param x specifies the left x-coordinate of the box, in
           pixels.

           \param y specifies the top y-coordinate of the box, in
           pixels.

           \param w specifies the width of the box, in pixels.

           \param h specifies the height of the box, in pixels.

           \param xres horizontal resolution of the graphics device,
           in dots per inch

           \param yres vertical resolution of the graphics device, in
           dots per inch

           \param rotate how to rotate the page

           \param flags additional painter flags

           \warning The parameter (\p x, \p y, \p w, \p h) are not
           well-tested. Unusual or meaningless parameters may lead to
           rather unexpected results.

           \returns whether the painting succeeded

           \note This method is only supported for Arthur

           \since 0.16
        */
        bool renderToPainter(QPainter* painter, double xres=72.0, double yres=72.0, int x=-1, int y=-1, int w=-1, int h=-1,
                             Rotation rotate = Rotate0, PainterFlags flags = 0) const;

	/**
	   Get the page thumbnail if it exists.

	   \return a QImage of the thumbnail, or a null image
	   if the PDF does not contain one for this page

	   \since 0.12
	*/
	QImage thumbnail() const;

	/**
	   Returns the text that is inside a specified rectangle

	   \param rect the rectangle specifying the area of interest,
	   with coordinates given in points, i.e., 1/72th of an inch.
	   If rect is null, all text on the page is given
	
	   \since 0.16
	**/
	QString text(const QRectF &rect, TextLayout textLayout) const;

	/**
	   Returns the text that is inside a specified rectangle.
	   The text is returned using the physical layout of the page

	   \param rect the rectangle specifying the area of interest,
	   with coordinates given in points, i.e., 1/72th of an inch.
	   If rect is null, all text on the page is given
	**/
	QString text(const QRectF &rect) const;
	
	/**
	   The starting point for a search
	*/
	enum SearchDirection { FromTop,        ///< Start sorting at the top of the document
			       NextResult,     ///< Find the next result, moving "down the page"
			       PreviousResult  ///< Find the previous result, moving "up the page"
	};

	/**
	   The type of search to perform
	*/
	enum SearchMode { CaseSensitive,   ///< Case differences cause no match in searching
			  CaseInsensitive  ///< Case differences are ignored in matching
	};
	
	/**
	   Returns true if the specified text was found.

	   \param text the text the search
	   \param rect in all directions is used to return where the text was found, for NextResult and PreviousResult
	               indicates where to continue searching for
	   \param direction in which direction do the search
	   \param caseSensitive be case sensitive?
	   \param rotate the rotation to apply for the search order
	**/
	Q_DECL_DEPRECATED bool search(const QString &text, QRectF &rect, SearchDirection direction, SearchMode caseSensitive, Rotation rotate = Rotate0) const;
	
	/**
	   Returns true if the specified text was found.

	   \param text the text the search
	   \param rectXXX in all directions is used to return where the text was found, for NextResult and PreviousResult
	               indicates where to continue searching for
	   \param direction in which direction do the search
	   \param caseSensitive be case sensitive?
	   \param rotate the rotation to apply for the search order
	   \since 0.14
	**/
	bool search(const QString &text, double &rectLeft, double &rectTop, double &rectRight, double &rectBottom, SearchDirection direction, SearchMode caseSensitive, Rotation rotate = Rotate0) const;

	/**
	   Returns a list of text of the page

	   This method returns a QList of TextBoxes that contain all
	   the text of the page, with roughly one text word of text
	   per TextBox item.
	   
	   For text written in western languages (left-to-right and
	   up-to-down), the QList contains the text in the proper
	   order.

	   \note The caller owns the text boxes and they should
	         be deleted when no longer required.

	   \warning This method is not tested with Asian scripts
	*/
	QList<TextBox*> textList(Rotation rotate = Rotate0) const;

	/**
	   \return The dimensions (cropbox) of the page, in points (i.e. 1/72th on an inch)
	*/
	QSizeF pageSizeF() const;

	/**
	   \return The dimensions (cropbox) of the page, in points (i.e. 1/72th on an inch)
	*/
	QSize pageSize() const;

	/**
	  Returns the transition of this page

	  \returns a pointer to a PageTransition structure that
	  defines how transition to this page shall be performed.

	  \note The PageTransition structure is owned by this page, and will
	  automatically be destroyed when this page class is
	  destroyed.
	**/
	PageTransition *transition() const;
	
	/**
	  Gets the page action specified, or NULL if there is no action.

	  \since 0.6
	**/
	Link *action( PageAction act ) const;
	
	/**
	   Types of orientations that are possible
	*/
	enum Orientation {
	    Landscape, ///< Landscape orientation (portrait, with 90 degrees clockwise rotation )
	    Portrait, ///< Normal portrait orientation
	    Seascape, ///< Seascape orientation (portrait, with 270 degrees clockwise rotation)
	    UpsideDown ///< Upside down orientation (portrait, with 180 degrees rotation)
	};

	/**
	   The orientation of the page
	*/
	Orientation orientation() const;
	
	/**
	  The default CTM
	*/
	void defaultCTM(double *CTM, double dpiX, double dpiY, int rotate, bool upsideDown);
	
	/**
	  Gets the links of the page
	*/
	QList<Link*> links() const;
	
	/**
	 Returns the annotations of the page
	*/
	QList<Annotation*> annotations() const;

	/**
	 Returns the form fields on the page

	 \since 0.6
	*/
	QList<FormField*> formFields() const;

	/**
	 Returns the page duration. That is the time, in seconds, that the page
	 should be displayed before the presentation automatically advances to the next page.
	 Returns < 0 if duration is not set.

	 \since 0.6
	*/
	double duration() const;
	
	/**
	   Returns the label of the page, or a null string is the page has no label.

	 \since 0.6
	**/
	QString label() const;
	
    private:
	Q_DISABLE_COPY(Page)

	Page(DocumentData *doc, int index);
	PageData *m_page;
    };

/**
   \brief PDF document.

   The Document class represents a PDF document: its pages, and all the global
   properties, metadata, etc.

   \section ownership Ownership of the returned objects

   All the functions that returns class pointers create new object, and the
   responsability of those is given to the callee.

   The only exception is \link Poppler::Page::transition() Page::transition()\endlink.

   \section document-loading Loading

   To get a Document, you have to load it via the load() & loadFromData()
   functions.

   In all the functions that have passwords as arguments, they \b must be Latin1
   encoded. If you have a password that is a UTF-8 string, you need to use
   QString::toLatin1() (or similar) to convert the password first.
   If you have a UTF-8 character array, consider converting it to a QString first
   (QString::fromUtf8(), or similar) before converting to Latin1 encoding.

   \section document-rendering Rendering

   To render pages of a document, you have different Document functions to set
   various options.

   \subsection document-rendering-backend Backends

   %Poppler offers a different backends for rendering the pages. Currently
   there are two backends (see #RenderBackend), but only the Splash engine works
   well and has been tested.

   The available rendering backends can be discovered via availableRenderBackends().
   The current rendering backend can be changed using setRenderBackend().
   Please note that setting a backend not listed in the available ones
   will always result in null QImage's.

   \section document-cms Color management support

   %Poppler, if compiled with this support, provides functions to handle color
   profiles.

   To know whether the %Poppler version you are using has support for color
   management, you can query Poppler::isCmsAvailable(). In case it is not
   avilable, all the color management-related functions will either do nothing
   or return null.
*/
    class POPPLER_QT4_EXPORT Document {
	friend class Page;
	friend class DocumentData;
  
    public:
	/**
	   The page mode
	*/
	enum PageMode {
	    UseNone,     ///< No mode - neither document outline nor thumbnail images are visible
	    UseOutlines, ///< Document outline visible
	    UseThumbs,   ///< Thumbnail images visible
	    FullScreen,  ///< Fullscreen mode (no menubar, windows controls etc)
	    UseOC,       ///< Optional content group panel visible
	    UseAttach    ///< Attachments panel visible
	};
  
	/**
	   The page layout
	*/
	enum PageLayout {
	    NoLayout,   ///< Layout not specified
	    SinglePage, ///< Display a single page
	    OneColumn,  ///< Display a single column of pages
	    TwoColumnLeft, ///< Display the pages in two columns, with odd-numbered pages on the left
	    TwoColumnRight, ///< Display the pages in two columns, with odd-numbered pages on the right
	    TwoPageLeft, ///< Display the pages two at a time, with odd-numbered pages on the left
	    TwoPageRight ///< Display the pages two at a time, with odd-numbered pages on the right
	};

	/**
	   The render backends available

	   \since 0.6
	*/
	enum RenderBackend {
	    SplashBackend,   ///< Splash backend
	    ArthurBackend   ///< Arthur (Qt4) backend
	};

	/**
	   The render hints available

	   \since 0.6
	*/
	enum RenderHint {
	    Antialiasing = 0x00000001,      ///< Antialiasing for graphics
	    TextAntialiasing = 0x00000002,  ///< Antialiasing for text
	    TextHinting = 0x00000004        ///< Hinting for text \since 0.12.1
	};
	Q_DECLARE_FLAGS( RenderHints, RenderHint )

	/**
	  Set a color display profile for the current document.

	  \param outputProfileA is a \c cmsHPROFILE of the LCMS library.

	   \since 0.12
	*/
	void setColorDisplayProfile(void *outputProfileA);
	/**
	  Set a color display profile for the current document.

	  \param name is the name of the display profile to set.

	   \since 0.12
	*/
	void setColorDisplayProfileName(const QString &name);
	/**
	  Return the current RGB profile.

	  \return a \c cmsHPROFILE of the LCMS library.

	   \since 0.12
	*/
	void* colorRgbProfile() const;
	/**
	  Return the current display profile.

	  \return a \c cmsHPROFILE of the LCMS library.

	   \since 0.12
	*/
	void *colorDisplayProfile() const;

	/**
	   Load the document from a file on disk

	   \param filePath the name (and path, if required) of the file to load
	   \param ownerPassword the Latin1-encoded owner password to use in
	   loading the file
	   \param userPassword the Latin1-encoded user ("open") password
	   to use in loading the file

	   \return the loaded document, or NULL on error

	   \note The caller owns the pointer to Document, and this should
	   be deleted when no longer required.
	
	   \warning The returning document may be locked if a password is required
	   to open the file, and one is not provided (as the userPassword).
	*/
	static Document *load(const QString & filePath,
			      const QByteArray &ownerPassword=QByteArray(),
			      const QByteArray &userPassword=QByteArray());
	
	/**
	   Load the document from memory

	   \param fileContents the file contents. They are copied so there is no need 
	                       to keep the byte array around for the full life time of 
	                       the document.
	   \param ownerPassword the Latin1-encoded owner password to use in
	   loading the file
	   \param userPassword the Latin1-encoded user ("open") password
	   to use in loading the file

	   \return the loaded document, or NULL on error

	   \note The caller owns the pointer to Document, and this should
	   be deleted when no longer required.
	
	   \warning The returning document may be locked if a password is required
	   to open the file, and one is not provided (as the userPassword).

	   \since 0.6
	*/
	static Document *loadFromData(const QByteArray &fileContents,
			      const QByteArray &ownerPassword=QByteArray(),
			      const QByteArray &userPassword=QByteArray());
  
	/**
	   Get a specified Page
     
	   Note that this follows the PDF standard of being zero based - if you
	   want the first page, then you need an index of zero.
	
	   The caller gets the ownership of the returned object.

	   \param index the page number index
	*/
	Page *page(int index) const;

	/**
	   \overload


	   The intent is that you can pass in a label like \c "ix" and
	   get the page with that label (which might be in the table of
	   contents), or pass in \c "1" and get the page that the user
	   expects (which might not be the first page, if there is a
	   title page and a table of contents).

	   \param label the page label
	*/
	Page *page(const QString &label) const;

	/**
	   The number of pages in the document
	*/
	int numPages() const;
  
	/**
	   The type of mode that should be used by the application
	   when the document is opened. Note that while this is 
	   called page mode, it is really viewer application mode.
	*/
	PageMode pageMode() const;

	/**
	   The layout that pages should be shown in when the document
	   is first opened.  This basically describes how pages are
	   shown relative to each other.
	*/
	PageLayout pageLayout() const;

	/**
	   Provide the passwords required to unlock the document

	   \param ownerPassword the Latin1-encoded owner password to use in
	   loading the file
	   \param userPassword the Latin1-encoded user ("open") password
	   to use in loading the file
	*/
	bool unlock(const QByteArray &ownerPassword, const QByteArray &userPassword);

	/**
	   Determine if the document is locked
	*/
	bool isLocked() const;

	/**
	   The date associated with the document

	   You would use this method with something like:
	   \code
QDateTime created = m_doc->date("CreationDate");
QDateTime modified = m_doc->date("ModDate");
	   \endcode

	   The available dates are:
	   - CreationDate: the date of creation of the document
	   - ModDate: the date of the last change in the document

	   \param data the type of date that is required
	*/
	QDateTime date( const QString & data ) const;

	/**
	   Get specified information associated with the document

	   You would use this method with something like:
	   \code
QString title = m_doc->info("Title");
QString subject = m_doc->info("Subject");
	   \endcode

	   In addition to \c Title and \c Subject, other information that may
	   be available include \c Author, \c Keywords, \c Creator and \c Producer.

	   \param data the information that is required

	   \sa infoKeys() to get a list of the available keys
	*/
	QString info( const QString & data ) const;

	/**
	   Obtain a list of the available string information keys.
	*/
	QStringList infoKeys() const;

	/**
	   Test if the document is encrypted
	*/
	bool isEncrypted() const;

	/**
	   Test if the document is linearised

	   In some cases, this is called "fast web view", since it
	   is mostly an optimisation for viewing over the Web.
	*/
	bool isLinearized() const;

	/**
	   Test if the permissions on the document allow it to be
	   printed
	*/
	bool okToPrint() const;

	/**
	   Test if the permissions on the document allow it to be
	   printed at high resolution
	*/
	bool okToPrintHighRes() const;

	/**
	   Test if the permissions on the document allow it to be
	   changed.

	   \note depending on the type of change, it may be more
	   appropriate to check other properties as well.
	*/
	bool okToChange() const;

	/**
	   Test if the permissions on the document allow the
	   contents to be copied / extracted
	*/
	bool okToCopy() const;

	/**
	   Test if the permissions on the document allow annotations
	   to be added or modified, and interactive form fields (including
	   signature fields) to be completed.
	*/
	bool okToAddNotes() const;

	/**
	   Test if the permissions on the document allow interactive
	   form fields (including signature fields) to be completed.

	   \note this can be true even if okToAddNotes() is false - this
	   means that only form completion is permitted.
	*/
	bool okToFillForm() const;

	/**
	   Test if the permissions on the document allow interactive
	   form fields (including signature fields) to be set, created and
	   modified
	*/
	bool okToCreateFormFields() const;

	/**
	   Test if the permissions on the document allow content extraction
	   (text and perhaps other content) for accessibility usage (eg for
	   a screen reader)
	*/
	bool okToExtractForAccessibility() const;

	/**
	   Test if the permissions on the document allow it to be
	   "assembled" - insertion, rotation and deletion of pages;
	   or creation of bookmarks and thumbnail images.

	   \note this can be true even if okToChange() is false
	*/
	bool okToAssemble() const;

	/**
	   The version of the PDF specification that the document
	   conforms to

	   \deprecated use getPdfVersion and avoid float point
	   comparisons/handling
	*/
	Q_DECL_DEPRECATED double pdfVersion() const;

	/**
	   The version of the PDF specification that the document
	   conforms to

	   \param major an optional pointer to a variable where store the
	   "major" number of the version
	   \param minor an optional pointer to a variable where store the
	   "minor" number of the version

	   \since 0.12
	*/
	void getPdfVersion(int *major, int *minor) const;
  
	/**
	   The fonts within the PDF document.

	   This is a shorthand for getting all the fonts at once.

	   \note this can take a very long time to run with a large
	   document. You may wish to use a FontIterator if you have more
	   than say 20 pages

	   \see newFontIterator()
	*/
	QList<FontInfo> fonts() const;

	/**
	   Scans for fonts within the PDF document.

	   \param numPages the number of pages to scan
	   \param fontList pointer to the list where the font information
	   should be placed

	   \note with this method you can scan for fonts only \em once for each
	   document; once the end is reached, no more scanning with this method
	   can be done

	   \return false if the end of the document has been reached

	   \deprecated this function is quite limited in its job (see note),
	   better use fonts() or newFontIterator()

	   \see fonts(), newFontIterator()
	*/
	Q_DECL_DEPRECATED bool scanForFonts( int numPages, QList<FontInfo> *fontList ) const;

	/**
	   Creates a new FontIterator object for font scanning.

	   The new iterator can be used for reading the font information of the
	   document, reading page by page.

	   The caller is responsible for the returned object, ie it should freed
	   it when no more useful.

	   \param startPage the initial page from which start reading fonts

	   \see fonts()

	   \since 0.12
	*/
	FontIterator* newFontIterator( int startPage = 0 ) const;

	/**
	   The font data if the font is an embedded one.

	   \since 0.10
	*/
	QByteArray fontData(const FontInfo &font) const;

	/**
	   The documents embedded within the PDF document.

	   \note there are two types of embedded document - this call
	   only accesses documents that are embedded at the document level.
	*/
	QList<EmbeddedFile*> embeddedFiles() const;

	/**
	   Whether there are any documents embedded in this PDF document.
	*/
	bool hasEmbeddedFiles() const;
	
	/**
	  Gets the table of contents (TOC) of the Document.
	
	  The caller is responsable for the returned object.
	
	  In the tree the tag name is the 'screen' name of the entry. A tag can have
	  attributes. Here follows the list of tag attributes with meaning:
	  - Destination: A string description of the referred destination
	  - DestinationName: A 'named reference' to the viewport
	  - ExternalFileName: A link to a external filename
	  - Open: A bool value that tells whether the subbranch of the item is open or not
	
	  Resolving the final destination for each item can be done in the following way:
	  - first, checking for 'Destination': if not empty, then a LinkDestination
	    can be constructed straight with it
	  - as second step, if the 'DestinationName' is not empty, then the destination
	    can be resolved using linkDestination()
	
	  Note also that if 'ExternalFileName' is not emtpy, then the destination refers
	  to that document (and not to the current one).
	
	  \returns the TOC, or NULL if the Document does not have one
	*/
	QDomDocument *toc() const;
	
	/**
	   Tries to resolve the named destination \p name.
	
	   \note this operation starts a search through the whole document
	
	   \returns a new LinkDestination object if the named destination was
	   actually found, or NULL otherwise
	*/
	LinkDestination *linkDestination( const QString &name );
	
	/**
	  Sets the paper color

	  \param color the new paper color
	 */
	void setPaperColor(const QColor &color);
	/**
	  The paper color

	  The default color is white.
	 */
	QColor paperColor() const;

	/**
	 Sets the backend used to render the pages.

	 \param backend the new rendering backend

	 \since 0.6
	 */
	void setRenderBackend( RenderBackend backend );
	/**
	  The currently set render backend

	  The default backend is \ref SplashBackend

	  \since 0.6
	 */
	RenderBackend renderBackend() const;

	/**
	  The available rendering backends.

	  \since 0.6
	 */
	static QSet<RenderBackend> availableRenderBackends();

	/**
	 Sets the render \p hint .

	 \note some hints may not be supported by some rendering backends.

	 \param on whether the flag should be added or removed.

	 \since 0.6
	 */
	void setRenderHint( RenderHint hint, bool on = true );
	/**
	  The currently set render hints.

	  \since 0.6
	 */
	RenderHints renderHints() const;
	
	/**
	  Gets a new PS converter for this document.

	  The caller gets the ownership of the returned converter.

	  \since 0.6
	 */
	PSConverter *psConverter() const;
	
	/**
	  Gets a new PDF converter for this document.

	  The caller gets the ownership of the returned converter.

	  \since 0.8
	 */
	PDFConverter *pdfConverter() const;
	
	/**
	  Gets the metadata stream contents

	  \since 0.6
	*/
	QString metadata() const;

	/**
	   Test whether this document has "optional content".

	   Optional content is used to optionally turn on (display)
	   and turn off (not display) some elements of the document.
	   The most common use of this is for layers in design
	   applications, but it can be used for a range of things,
	   such as not including some content in printing, and
	   displaying content in the appropriate language.

	   \since 0.8
	*/
	bool hasOptionalContent() const;

	/**
	   Itemviews model for optional content.

	   The model is owned by the document.

	   \since 0.8
	*/
	OptContentModel *optionalContentModel();

	/**
	   Document-level JavaScript scripts.

	   Returns the list of document level JavaScript scripts to be always
	   executed before any other script.

	   \since 0.10
	*/
	QStringList scripts() const;

	/**
	   The PDF identifiers.

	   \param permanentId an optional pointer to a variable where store the
	   permanent ID of the document
	   \param updateId an optional pointer to a variable where store the
	   update ID of the document

	   \return whether the document has the IDs

	   \since 0.16
	*/
	bool getPdfId(QByteArray *permanentId, QByteArray *updateId) const;

	/**
	   Destructor.
	*/
	~Document();
  
    private:
	Q_DISABLE_COPY(Document)

	DocumentData *m_doc;
	
	Document(DocumentData *dataA);
    };
    
    class BaseConverterPrivate;
    class PSConverterPrivate;
    class PDFConverterPrivate;
    /**
       \brief Base converter.

       This is the base class for the converters.

       \since 0.8
    */
    class POPPLER_QT4_EXPORT BaseConverter
    {
        friend class Document;
        public:
            /**
              Destructor.
            */
            virtual ~BaseConverter();

            /** Sets the output file name. You must set this or the output device. */
            void setOutputFileName(const QString &outputFileName);

            /**
             * Sets the output device. You must set this or the output file name.
             *
             * \since 0.8
             */
            void setOutputDevice(QIODevice *device);

            /**
              Does the conversion.

              \return whether the conversion succeeded
            */
            virtual bool convert() = 0;
            
            enum Error
            {
                NoError,
                FileLockedError,
                OpenOutputError,
                NotSupportedInputFileError
            };
            
            /**
              Returns the last error
              \since 0.12.1
            */
            Error lastError() const;

        protected:
            /// \cond PRIVATE
            BaseConverter(BaseConverterPrivate &dd);
            Q_DECLARE_PRIVATE(BaseConverter)
            BaseConverterPrivate *d_ptr;
            /// \endcond

        private:
            Q_DISABLE_COPY(BaseConverter)
    };

    /**
       Converts a PDF to PS

       Sizes have to be in Points (1/72 inch)

       If you are using QPrinter you can get paper size by doing:
       \code
QPrinter dummy(QPrinter::PrinterResolution);
dummy.setFullPage(true);
dummy.setPageSize(myPageSize);
width = dummy.width();
height = dummy.height();
       \endcode

       \since 0.6
    */
    class POPPLER_QT4_EXPORT PSConverter : public BaseConverter
    {
        friend class Document;
        public:
            /**
              Options for the PS export.

              \since 0.10
             */
            enum PSOption {
                Printing = 0x00000001,              ///< The PS is generated for priting purpouses
                StrictMargins = 0x00000002,
                ForceRasterization = 0x00000004
            };
            Q_DECLARE_FLAGS( PSOptions, PSOption )

            /**
              Destructor.
            */
            ~PSConverter();

            /** Sets the list of pages to print. Mandatory. */
            void setPageList(const QList<int> &pageList);

            /**
              Sets the title of the PS Document. Optional
            */
            void setTitle(const QString &title);

            /**
              Sets the horizontal DPI. Defaults to 72.0
            */
            void setHDPI(double hDPI);

            /**
              Sets the vertical DPI. Defaults to 72.0
            */
            void setVDPI(double vDPI);

            /**
              Sets the rotate. Defaults to not rotated
            */
            void setRotate(int rotate);

            /**
              Sets the output paper width. Has to be set.
            */
            void setPaperWidth(int paperWidth);

            /**
              Sets the output paper height. Has to be set.
            */
            void setPaperHeight(int paperHeight);

            /**
              Sets the output right margin. Defaults to 0
            */
            void setRightMargin(int marginRight);

            /**
              Sets the output bottom margin. Defaults to 0
            */
            void setBottomMargin(int marginBottom);

            /**
              Sets the output left margin. Defaults to 0
            */
            void setLeftMargin(int marginLeft);

            /**
              Sets the output top margin. Defaults to 0
            */
            void setTopMargin(int marginTop);

            /**
              Defines if margins have to be strictly followed (even if that
              means changing aspect ratio), or if the margins can be adapted
              to keep aspect ratio.

              Defaults to false.
            */
            void setStrictMargins(bool strictMargins);

            /** Defines if the page will be rasterized to an image before printing. Defaults to false */
            void setForceRasterize(bool forceRasterize);

            /**
              Sets the options for the PS export.

              \since 0.10
             */
            void setPSOptions(PSOptions options);

            /**
              The currently set options for the PS export.

              The default flags are: Printing.

              \since 0.10
             */
            PSOptions psOptions() const;

            /**
              Sets a function that will be called each time a page is converted.

              The payload belongs to the caller.

              \since 0.16
             */
            void setPageConvertedCallback(void (* callback)(int page, void *payload), void *payload);

            bool convert();

        private:
            Q_DECLARE_PRIVATE(PSConverter)
            Q_DISABLE_COPY(PSConverter)

            PSConverter(DocumentData *document);
    };

    /**
       Converts a PDF to PDF (thus saves a copy of the document).

       \since 0.8
    */
    class POPPLER_QT4_EXPORT PDFConverter : public BaseConverter
    {
        friend class Document;
        public:
            /**
              Options for the PDF export.
             */
            enum PDFOption {
                WithChanges = 0x00000001        ///< The changes done to the document are saved as well
            };
            Q_DECLARE_FLAGS( PDFOptions, PDFOption )

            /**
              Destructor.
            */
            virtual ~PDFConverter();

            /**
              Sets the options for the PDF export.
             */
            void setPDFOptions(PDFOptions options);
            /**
              The currently set options for the PDF export.
             */
            PDFOptions pdfOptions() const;

            bool convert();

        private:
            Q_DECLARE_PRIVATE(PDFConverter)
            Q_DISABLE_COPY(PDFConverter)

            PDFConverter(DocumentData *document);
    };

    /**
       Conversion from PDF date string format to QDateTime
    */
    POPPLER_QT4_EXPORT QDateTime convertDate( char *dateString );

    /**
       Whether the color management functions are available.

       \since 0.12
    */
    POPPLER_QT4_EXPORT bool isCmsAvailable();

    class SoundData;
    /**
       Container class for a sound file in a PDF document.

	A sound can be either External (in that case should be loaded the file
       whose url is represented by url() ), or Embedded, and the player has to
       play the data contained in data().

       \since 0.6
    */
    class POPPLER_QT4_EXPORT SoundObject {
    public:
	/**
	   The type of sound
	*/
	enum SoundType {
	    External,     ///< The real sound file is external
	    Embedded      ///< The sound is contained in the data
	};

	/**
	   The encoding format used for the sound
	*/
	enum SoundEncoding {
	    Raw,          ///< Raw encoding, with unspecified or unsigned values in the range [ 0, 2^B - 1 ]
	    Signed,       ///< Twos-complement values
	    muLaw,        ///< mu-law-encoded samples
	    ALaw          ///< A-law-encoded samples
	};

	/// \cond PRIVATE
	SoundObject(Sound *popplersound);
	/// \endcond
	
	~SoundObject();

	/**
	   Is the sound embedded (SoundObject::Embedded) or external (SoundObject::External)?
	*/
	SoundType soundType() const;

	/**
	   The URL of the sound file to be played, in case of SoundObject::External
	*/
	QString url() const;

	/**
	   The data of the sound, in case of SoundObject::Embedded
	*/
	QByteArray data() const;

	/**
	   The sampling rate of the sound
	*/
	double samplingRate() const;

	/**
	   The number of sound channels to use to play the sound
	*/
	int channels() const;

	/**
	   The number of bits per sample value per channel
	*/
	int bitsPerSample() const;

	/**
	   The encoding used for the sound
	*/
	SoundEncoding soundEncoding() const;

    private:
	Q_DISABLE_COPY(SoundObject)

	SoundData *m_soundData;
    };

    class MovieData;
    /**
       Container class for a movie object in a PDF document.

       \since 0.10
    */
    class POPPLER_QT4_EXPORT MovieObject {
    friend class Page;
    public:
	/**
	   The play mode for playing the movie
	*/
	enum PlayMode {
	    PlayOnce,         ///< Play the movie once, closing the movie controls at the end
	    PlayOpen,         ///< Like PlayOnce, but leaving the controls open
	    PlayRepeat,       ///< Play continuously until stopped
	    PlayPalindrome    ///< Play forward, then backward, then again foward and so on until stopped
	};

	~MovieObject();

	/**
	   The URL of the movie to be played
	*/
	QString url() const;

	/**
	   The size of the movie
	*/
	QSize size() const;

	/**
	   The rotation (either 0, 90, 180, or 270 degrees clockwise) for the movie,
	*/
	int rotation() const;

	/**
	   Whether show a bar with movie controls
	*/
	bool showControls() const;

	/**
	   How to play the movie
	*/
	PlayMode playMode() const;

    private:
	/// \cond PRIVATE
	MovieObject( AnnotMovie *ann );
	/// \endcond
	
	Q_DISABLE_COPY(MovieObject)

	MovieData *m_movieData;
    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Poppler::Page::PainterFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Poppler::Document::RenderHints)
Q_DECLARE_OPERATORS_FOR_FLAGS(Poppler::PDFConverter::PDFOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(Poppler::PSConverter::PSOptions)

#endif
