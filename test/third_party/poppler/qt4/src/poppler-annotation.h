/* poppler-annotation.h: qt interface to poppler
 * Copyright (C) 2006-2008, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006, 2008 Pino Toscano <pino@kde.org>
 * Copyright (C) 2007, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2010, Philip Lorenz <lorenzph+freedesktop@gmail.com>
 * Adapting code from
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

#ifndef _POPPLER_ANNOTATION_H_
#define _POPPLER_ANNOTATION_H_

#include <QtCore/QDateTime>
#include <QtCore/QLinkedList>
#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtXml/QDomDocument>
#include "poppler-export.h"

namespace Poppler {

class Annotation;
class AnnotationPrivate;
class TextAnnotationPrivate;
class LineAnnotationPrivate;
class GeomAnnotationPrivate;
class HighlightAnnotationPrivate;
class StampAnnotationPrivate;
class InkAnnotationPrivate;
class LinkAnnotationPrivate;
class CaretAnnotationPrivate;
class FileAttachmentAnnotationPrivate;
class SoundAnnotationPrivate;
class MovieAnnotationPrivate;
class EmbeddedFile;
class Link;
class SoundObject;
class MovieObject;

/**
 * \short Helper class for (recursive) Annotation retrieval/storage.
 *
 */
class POPPLER_QT4_EXPORT AnnotationUtils
{
    public:
        /**
         * Restore an Annotation (with revisions if needed) from the DOM
         * element \p annElement.
         * \returns a pointer to the complete Annotation or 0 if element is
         * invalid.
         */
        static Annotation * createAnnotation( const QDomElement & annElement );

        /**
         * Save the Annotation \p ann as a child of \p annElement taking
         * care of saving all revisions if \p ann has any.
         */
        static void storeAnnotation( const Annotation * ann,
            QDomElement & annElement, QDomDocument & document );

        /**
         * Returns an element called \p name from the direct children of
         * \p parentNode or a null element if not found.
         */
        static QDomElement findChildElement( const QDomNode & parentNode,
            const QString & name );
};


/**
 * \short Annotation class holding properties shared by all annotations.
 *
 * An Annotation is an object (text note, highlight, sound, popup window, ..)
 * contained by a Page in the document.
 */
class POPPLER_QT4_EXPORT Annotation
{
  public:
    // enum definitions
    // WARNING!!! oKular uses that very same values so if you change them notify the author!
    enum SubType { AText = 1, ALine = 2, AGeom = 3, AHighlight = 4, AStamp = 5,
                   AInk = 6, ALink = 7, ACaret = 8, AFileAttachment = 9, ASound = 10,
                   AMovie = 11, A_BASE = 0 };
    enum Flag { Hidden = 1, FixedSize = 2, FixedRotation = 4, DenyPrint = 8,
                DenyWrite = 16, DenyDelete = 32, ToggleHidingOnMouse = 64, External = 128 };
    enum LineStyle { Solid = 1, Dashed = 2, Beveled = 4, Inset = 8, Underline = 16 };
    enum LineEffect { NoEffect = 1, Cloudy = 2};
    enum RevScope { Reply = 1, Group = 2, Delete = 4 };
    enum RevType { None = 1,  Marked = 2, Unmarked = 4,  Accepted = 8, Rejected = 16, Cancelled = 32, Completed = 64 };

    /**
     * Returns the author of the annotation.
     */
    QString author() const;
    /**
     * Sets a new author for the annotation.
     */
    void setAuthor( const QString &author );

    QString contents() const;
    void setContents( const QString &contents );

    /**
     * Returns the unique name (ID) of the annotation.
     */
    QString uniqueName() const;
    /**
     * Sets a new unique name for the annotation.
     *
     * \note no check of the new uniqueName is done
     */
    void setUniqueName( const QString &uniqueName );

    QDateTime modificationDate() const;
    void setModificationDate( const QDateTime &date );

    QDateTime creationDate() const;
    void setCreationDate( const QDateTime &date );

    int flags() const;
    void setFlags( int flags );

    QRectF boundary() const;
    void setBoundary( const QRectF &boundary );

    struct POPPLER_QT4_EXPORT Style
    {
        // appearance properties
        QColor          color;              // black
        double          opacity;            // 1.0
        // pen properties
        double          width;              // 1.0
        LineStyle       style;              // LineStyle::Solid
        double          xCorners;           // 0.0
        double          yCorners;           // 0.0
        int             marks;              // 3
        int             spaces;             // 0
        // pen effects
        LineEffect      effect;             // LineEffect::NoEffect
        double          effectIntensity;    // 1.0
        // default initializer
        Style();
    }               style;

    /* properties: popup window */
    struct POPPLER_QT4_EXPORT Window
    {
        // window state (Hidden, FixedRotation, Deny* flags allowed)
        int             flags;              // -1 (never initialized) -> 0 (if inited and shown)
        // geometric properties
        QPointF         topLeft;            // no default, inited to boundary.topLeft
        int             width;              // no default
        int             height;             // no default
        // window contens/override properties
        QString         title;              // '' text in the titlebar (overrides author)
        QString         summary;            // '' short description (displayed if not empty)
        QString         text;               // '' text for the window (overrides annot->contents)
        // default initializer
        Window();
    }               window;

    /* properties: versioning */
    struct POPPLER_QT4_EXPORT Revision
    {
        // child revision
        Annotation *    annotation;         // not null
        // scope and type of revision
        RevScope        scope;              // Reply
        RevType         type;               // None
        // default initializer
        Revision();
    };

    QLinkedList< Revision >& revisions();
    const QLinkedList< Revision >& revisions() const;

    // methods: query annotation's type for runtime type identification
    /**
     * The type of the annotation.
     */
    virtual SubType subType() const = 0;

    /**
     * Save this annotation as a child of \p parentNode.
     */
    virtual void store( QDomNode & parentNode, QDomDocument & document ) const;

    /**
     * Destructor.
     */
    virtual ~Annotation();

  protected:
    /// \cond PRIVATE
    Annotation( AnnotationPrivate &dd );
    Annotation( AnnotationPrivate &dd, const QDomNode &description );
    Q_DECLARE_PRIVATE( Annotation )
    AnnotationPrivate *d_ptr;
    /// \endcond

  private:
    Q_DISABLE_COPY( Annotation )
};

/**
 * \short Annotation containing text.
 *
 * A text annotation is an object showing some text directly on the page, or
 * linked to the contents using an icon shown on a page.
 */
class POPPLER_QT4_EXPORT TextAnnotation : public Annotation
{
  public:
    TextAnnotation();
    TextAnnotation( const QDomNode &node );
    virtual ~TextAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    // local enums
    enum TextType { Linked, InPlace };
    enum InplaceIntent { Unknown, Callout, TypeWriter };

    /**
       The type of text annotation represented by this object
    */
    TextType textType() const;

    /**
       Set the type of text annotation represented by this object
    */
    void setTextType( TextType type );

    /**
       The name of the icon for this text annotation.

       Standard names for text annotation icons are:
       - Comment
       - Help
       - Insert
       - Key
       - NewParagraph
       - Note (this is the default icon to use)
       - Paragraph
    */
    QString textIcon() const;

    /**
       Set the name of the icon to use for this text annotation.

       \sa textIcon for the list of standard names
    */
    void setTextIcon( const QString &icon );

    QFont textFont() const;
    void setTextFont( const QFont &font );

    int inplaceAlign() const;
    void setInplaceAlign( int align );

    QString inplaceText() const;
    void setInplaceText( const QString &text );

    QPointF calloutPoint( int id ) const;
    void setCalloutPoint( int id, const QPointF &point );

    InplaceIntent inplaceIntent() const;
    void setInplaceIntent( InplaceIntent intent );

  private:
    Q_DECLARE_PRIVATE( TextAnnotation )
    Q_DISABLE_COPY( TextAnnotation )
};

/**
 * \short Polygon/polyline annotation.
 *
 * This annotation represents a polygon (or polyline) to be drawn on a page.
 */
class POPPLER_QT4_EXPORT LineAnnotation : public Annotation
{
  public:
    LineAnnotation();
    LineAnnotation( const QDomNode &node );
    virtual ~LineAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    // local enums
    enum TermStyle { Square, Circle, Diamond, OpenArrow, ClosedArrow, None,
                     Butt, ROpenArrow, RClosedArrow, Slash };
    enum LineIntent { Unknown, Arrow, Dimension, PolygonCloud };

    QLinkedList<QPointF> linePoints() const;
    void setLinePoints( const QLinkedList<QPointF> &points );

    TermStyle lineStartStyle() const;
    void setLineStartStyle( TermStyle style );

    TermStyle lineEndStyle() const;
    void setLineEndStyle( TermStyle style );

    bool isLineClosed() const;
    void setLineClosed( bool closed );

    QColor lineInnerColor() const;
    void setLineInnerColor( const QColor &color );

    double lineLeadingForwardPoint() const;
    void setLineLeadingForwardPoint( double point );

    double lineLeadingBackPoint() const;
    void setLineLeadingBackPoint( double point );

    bool lineShowCaption() const;
    void setLineShowCaption( bool show );

    LineIntent lineIntent() const;
    void setLineIntent( LineIntent intent );

  private:
    Q_DECLARE_PRIVATE( LineAnnotation )
    Q_DISABLE_COPY( LineAnnotation )
};

/**
 * \short Geometric annotation.
 *
 * The geometric annotation represents a geometric figure, like a rectangle or
 * an ellipse.
 */
class POPPLER_QT4_EXPORT GeomAnnotation : public Annotation
{
  public:
    GeomAnnotation();
    GeomAnnotation( const QDomNode &node );
    virtual ~GeomAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    // common enums
    enum GeomType { InscribedSquare, InscribedCircle };

    GeomType geomType() const;
    void setGeomType( GeomType style );

    QColor geomInnerColor() const;
    void setGeomInnerColor( const QColor &color );

    int geomPointWidth() const;
    void setGeomPointWidth( int width );

  private:
    Q_DECLARE_PRIVATE( GeomAnnotation )
    Q_DISABLE_COPY( GeomAnnotation )
};

/**
 * \short Text highlight annotation.
 *
 * The higlight annotation represents some areas of text being "highlighted".
 */
class POPPLER_QT4_EXPORT HighlightAnnotation : public Annotation
{
  public:
    HighlightAnnotation();
    HighlightAnnotation( const QDomNode &node );
    virtual ~HighlightAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    /**
       The type of highlight
    */
    enum HighlightType { Highlight, ///< highlighter pen style annotation
			 Squiggly,  ///< jagged or squiggly underline
			 Underline, ///< straight line underline
			 StrikeOut  ///< straight line through-line
    };

    /**
       Structure corresponding to a QuadPoints array. This matches a
       quadrilateral that describes the area around a word (or set of
       words) that are to be highlighted.
    */
    struct Quad
    {
        QPointF         points[4];          // 8 valid coords
        bool            capStart;           // false (vtx 1-4) [K]
        bool            capEnd;             // false (vtx 2-3) [K]
        double          feather;            // 0.1 (in range 0..1) [K]
    };

    /**
       The type (style) of highlighting to use for this area
       or these areas.
    */
    HighlightType highlightType() const;

    /**
       Set the type of highlighting to use for the given area
       or areas.
    */
    void setHighlightType( HighlightType type );

    /**
       The list of areas to highlight.
    */
    QList< Quad > highlightQuads() const;

    /**
       Set the areas to highlight.
    */
    void setHighlightQuads( const QList< Quad > &quads );

  private:
    Q_DECLARE_PRIVATE( HighlightAnnotation )
    Q_DISABLE_COPY( HighlightAnnotation )
};

/**
 * \short Stamp annotation.
 *
 * A simple annotation drawing a stamp on a page.
 */
class POPPLER_QT4_EXPORT StampAnnotation : public Annotation
{
  public:
    StampAnnotation();
    StampAnnotation( const QDomNode &node );
    virtual ~StampAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    /**
       The name of the icon for this stamp annotation.

       Standard names for stamp annotation icons are:
       - Approved
       - AsIs
       - Confidential
       - Departmental
       - Draft (this is the default icon type)
       - Experimental
       - Expired
       - Final
       - ForComment
       - ForPublicRelease
       - NotApproved
       - NotForPublicRelease
       - Sold
       - TopSecret
    */
    QString stampIconName() const;

    /**
       Set the icon type for this stamp annotation.

       \sa stampIconName for the list of standard icon names
    */
    void setStampIconName( const QString &name );

  private:
    Q_DECLARE_PRIVATE( StampAnnotation )
    Q_DISABLE_COPY( StampAnnotation )
};

/**
 * \short Ink Annotation.
 *
 * Annotation representing an ink path on a page.
 */
class POPPLER_QT4_EXPORT InkAnnotation : public Annotation
{
  public:
    InkAnnotation();
    InkAnnotation( const QDomNode &node );
    virtual ~InkAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    QList< QLinkedList<QPointF> > inkPaths() const;
    void setInkPaths( const QList< QLinkedList<QPointF> > &paths );

  private:
    Q_DECLARE_PRIVATE( InkAnnotation )
    Q_DISABLE_COPY( InkAnnotation )
};

class POPPLER_QT4_EXPORT LinkAnnotation : public Annotation
{
  public:
    LinkAnnotation();
    LinkAnnotation( const QDomNode &node );
    virtual ~LinkAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    // local enums
    enum HighlightMode { None, Invert, Outline, Push };

    Link* linkDestionation() const;
    void setLinkDestination( Link *link );

    HighlightMode linkHighlightMode() const;
    void setLinkHighlightMode( HighlightMode mode );

    QPointF linkRegionPoint( int id ) const;
    void setLinkRegionPoint( int id, const QPointF &point );

  private:
    Q_DECLARE_PRIVATE( LinkAnnotation )
    Q_DISABLE_COPY( LinkAnnotation )
};

/**
 * \short Caret annotation.
 *
 * The caret annotation represents a symbol to indicate the presence of text.
 */
class POPPLER_QT4_EXPORT CaretAnnotation : public Annotation
{
  public:
    CaretAnnotation();
    CaretAnnotation( const QDomNode &node );
    virtual ~CaretAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    /**
     * The symbols for the caret annotation.
     */
    enum CaretSymbol { None, P };

    CaretSymbol caretSymbol() const;
    void setCaretSymbol( CaretSymbol symbol );

  private:
    Q_DECLARE_PRIVATE( CaretAnnotation )
    Q_DISABLE_COPY( CaretAnnotation )
};

/**
 * \short File attachment annotation.
 *
 * The file attachment annotation represents a file embedded in the document.
 *
 * \since 0.10
 */
class POPPLER_QT4_EXPORT FileAttachmentAnnotation : public Annotation
{
  public:
    FileAttachmentAnnotation();
    FileAttachmentAnnotation( const QDomNode &node );
    virtual ~FileAttachmentAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    /**
     * Returns the name of the icon of this annotation.
     */
    QString fileIconName() const;
    /**
     * Sets a new name for the icon of this annotation.
     */
    void setFileIconName( const QString &icon );

    /**
     * Returns the EmbeddedFile of this annotation.
     */
    EmbeddedFile* embeddedFile() const;
    /**
     * Sets a new EmbeddedFile for this annotation.
     *
     * \note FileAttachmentAnnotation takes ownership of the object
     */
    void setEmbeddedFile( EmbeddedFile *ef );

  private:
    Q_DECLARE_PRIVATE( FileAttachmentAnnotation )
    Q_DISABLE_COPY( FileAttachmentAnnotation )
};

/**
 * \short Sound annotation.
 *
 * The sound annotation represents a sound to be played when activated.
 *
 * \since 0.10
 */
class POPPLER_QT4_EXPORT SoundAnnotation : public Annotation
{
  public:
    SoundAnnotation();
    SoundAnnotation( const QDomNode &node );
    virtual ~SoundAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    /**
     * Returns the name of the icon of this annotation.
     */
    QString soundIconName() const;
    /**
     * Sets a new name for the icon of this annotation.
     */
    void setSoundIconName( const QString &icon );

    /**
     * Returns the SoundObject of this annotation.
     */
    SoundObject* sound() const;
    /**
     * Sets a new SoundObject for this annotation.
     *
     * \note SoundAnnotation takes ownership of the object
     */
    void setSound( SoundObject *ef );

  private:
    Q_DECLARE_PRIVATE( SoundAnnotation )
    Q_DISABLE_COPY( SoundAnnotation )
};

/**
 * \short Movie annotation.
 *
 * The movie annotation represents a movie to be played when activated.
 *
 * \since 0.10
 */
class POPPLER_QT4_EXPORT MovieAnnotation : public Annotation
{
  public:
    MovieAnnotation();
    MovieAnnotation( const QDomNode &node );
    virtual ~MovieAnnotation();
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    virtual SubType subType() const;

    /**
     * Returns the MovieObject of this annotation.
     */
    MovieObject* movie() const;
    /**
     * Sets a new MovieObject for this annotation.
     *
     * \note MovieAnnotation takes ownership of the object
     */
    void setMovie( MovieObject *movie );

    /**
     * Returns the title of the movie of this annotation.
     */
    QString movieTitle() const;
    /**
     * Sets a new title for the movie of this annotation.
     */
    void setMovieTitle( const QString &title );

  private:
    Q_DECLARE_PRIVATE( MovieAnnotation )
    Q_DISABLE_COPY( MovieAnnotation )
};

}

#endif
