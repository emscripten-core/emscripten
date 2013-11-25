/* poppler-annotation.h: qt interface to poppler
 * Copyright (C) 2006-2008, 2012 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006, 2008 Pino Toscano <pino@kde.org>
 * Copyright (C) 2007, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2010, Philip Lorenz <lorenzph+freedesktop@gmail.com>
 * Copyright (C) 2012, Tobias Koenig <tokoe@kdab.com>
 * Copyright (C) 2012, Guillermo A. Amaral B. <gamaral@kde.org>
 * Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
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
#include <QtCore/QSharedDataPointer>
#include <QtCore/QLinkedList>
#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QVector>
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
class ScreenAnnotationPrivate;
class WidgetAnnotationPrivate;
class EmbeddedFile;
class Link;
class SoundObject;
class MovieObject;
class LinkRendition;
class Page;

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
 *
 * \warning Different Annotation objects might point to the same annotation.
 *
 * \section annotCreation How to add annotations
 *
 * Create an Annotation object of the desired subclass (for example
 * TextAnnotation) and set its properties:
 * @code
 * Poppler::TextAnnotation* myann = new Poppler::TextAnnotation(Poppler::TextAnnotation::InPlace);
 * myann->setBoundary(QRectF(0.1, 0.1, 0.2, 0.2)); // normalized coordinates: (0,0) is top-left, (1,1) is bottom-right
 * myann->setContents("Hello, world!");
 * @endcode
 * \note Always set a boundary rectangle, or nothing will be shown!
 *
 * Obtain a pointer to the Page where you want to add the annotation (refer to
 * \ref req for instructions) and add the annotation:
 * @code
 * Poppler::Page* mypage = ...;
 * mypage->addAnnotation(myann);
 * @endcode
 *
 * You can keep on editing the annotation after it has been added to the page:
 * @code
 * myann->setContents("World, hello!"); // Let's change text...
 * myann->setAuthor("Your name here");  // ...and set an author too
 * @endcode
 *
 * When you're done with editing the annotation, you must destroy the Annotation
 * object:
 * @code
 * delete myann;
 * @endcode
 *
 * Use the PDFConverter class to save the modified document.
 *
 * \section annotFixedRotation FixedRotation flag specifics
 *
 * According to the PDF specification, annotations whose
 * Annotation::FixedRotation flag is set must always be shown in their original
 * orientation, no matter what the current rendering rotation or the page's
 * Page::orientation() values are. In comparison with regular annotations, such
 * annotations should therefore be transformed by an extra rotation at rendering
 * time to "undo" such context-related rotations, which is equal to
 * <code>-(rendering_rotation + page_orientation)</code>. The rotation pivot
 * is the top-left corner of the boundary rectangle.
 *
 * In practice, %Poppler's \ref Page::renderToImage only "unrotates" the
 * page orientation, and does <b>not</b> unrotate the rendering rotation.
 * This ensures consistent renderings at different Page::Rotation values:
 * annotations are always positioned as if they were being positioned at the
 * default page orientation.
 *
 * Just like regular annotations, %Poppler Qt4 exposes normalized coordinates
 * relative to the page's default orientation. However, behind the scenes, the
 * coordinate system is different and %Poppler transparently transforms each
 * shape. If you never call either Annotation::setFlags or
 * Annotation::setBoundary, you don't need to worry about this; but if you do
 * call them, then you need to adhere to the following rules:
 *  - Whenever you toggle the Annotation::FixedRotation flag, you <b>must</b>
 *    set again the boundary rectangle first, and then you <b>must</b> set
 *    again any other geometry-related property.
 *  - Whenever you modify the boundary rectangle of an annotation whose
 *    Annotation::FixedRotation flag is set, you <b>must</b> set again any other
 *    geometry-related property.
 *
 * These two rules are necessary to make %Poppler's transparent coordinate
 * conversion work properly.
 */
class POPPLER_QT4_EXPORT Annotation
{
  friend class AnnotationUtils;
  friend class LinkMovie;
  friend class LinkRendition;

  public:
    // enum definitions
    /**
     * Annotation subclasses
     *
     * \sa subType()
     */
    // WARNING!!! oKular uses that very same values so if you change them notify the author!
    enum SubType
    {
        AText = 1,            ///< TextAnnotation
        ALine = 2,            ///< LineAnnotation
        AGeom = 3,            ///< GeomAnnotation
        AHighlight = 4,       ///< HighlightAnnotation
        AStamp = 5,           ///< StampAnnotation
        AInk = 6,             ///< InkAnnotation
        ALink = 7,            ///< LinkAnnotation
        ACaret = 8,           ///< CaretAnnotation
        AFileAttachment = 9,  ///< FileAttachmentAnnotation
        ASound = 10,          ///< SoundAnnotation
        AMovie = 11,          ///< MovieAnnotation
        AScreen = 12,         ///< ScreenAnnotation \since 0.20
        AWidget = 13,         ///< WidgetAnnotation \since 0.22
        A_BASE = 0
    };

    /**
     * Annotation flags
     *
     * They can be OR'd together (e.g. Annotation::FixedRotation | Annotation::DenyPrint).
     *
     * \sa flags(), setFlags(int)
     */
    // NOTE: Only flags that are known to work are documented
    enum Flag
    {
        Hidden = 1,                ///< Do not display or print the annotation
        FixedSize = 2,
        FixedRotation = 4,         ///< Do not rotate the annotation according to page orientation and rendering rotation \warning Extra care is needed with this flag: see \ref annotFixedRotation
        DenyPrint = 8,             ///< Do not print the annotation
        DenyWrite = 16,
        DenyDelete = 32,
        ToggleHidingOnMouse = 64,
        External = 128
    };

    enum LineStyle { Solid = 1, Dashed = 2, Beveled = 4, Inset = 8, Underline = 16 };
    enum LineEffect { NoEffect = 1, Cloudy = 2};
    enum RevScope { Root = 0 /** \since 0.20 */, Reply = 1, Group = 2, Delete = 4 };
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

    /**
     * Returns this annotation's flags
     *
     * \sa Flag, setFlags(int)
     */
    int flags() const;
    /**
     * Sets this annotation's flags
     *
     * \sa Flag, flags(), \ref annotFixedRotation
     */
    void setFlags( int flags );

    /**
     * Returns this annotation's boundary rectangle in normalized coordinates
     *
     * \sa setBoundary(const QRectF&)
     */
    QRectF boundary() const;
    /**
     * Sets this annotation's boundary rectangle
     *
     * The boundary rectangle is the smallest rectangle that contains the
     * annotation.
     *
     * \warning This property is mandatory: you must always set this.
     *
     * \sa boundary(), \ref annotFixedRotation
     */
    void setBoundary( const QRectF &boundary );

    /**
     * \short Container class for Annotation style information
     *
     * \since 0.20
     */
    class POPPLER_QT4_EXPORT Style
    {
      public:
        Style();
        Style( const Style &other );
        Style& operator=( const Style &other );
        ~Style();

        // appearance properties
        QColor color() const;                     // black
        void setColor(const QColor &color);
        double opacity() const;                   // 1.0
        void setOpacity(double opacity);

        // pen properties
        double width() const;                     // 1.0
        void setWidth(double width);
        LineStyle lineStyle() const;              // LineStyle::Solid
        void setLineStyle(LineStyle style);
        double xCorners() const;                  // 0.0
        void setXCorners(double radius);
        double yCorners() const;                  // 0.0
        void setYCorners(double radius);
        const QVector<double>& dashArray() const; // [ 3 ]
        void setDashArray(const QVector<double> &array);

        // pen effects
        LineEffect lineEffect() const;            // LineEffect::NoEffect
        void setLineEffect(LineEffect effect);
        double effectIntensity() const;           // 1.0
        void setEffectIntensity(double intens);

      private:
        class Private;
        QSharedDataPointer<Private> d;
    };

    /// \since 0.20
    Style style() const;
    /// \since 0.20
    void setStyle( const Style& style );

    /**
     * \short Container class for Annotation pop-up window information
     *
     * \since 0.20
     */
    class POPPLER_QT4_EXPORT Popup
    {
      public:
        Popup();
        Popup( const Popup &other );
        Popup& operator=( const Popup &other );
        ~Popup();

        // window state (Hidden, FixedRotation, Deny* flags allowed)
        int flags() const;       // -1 (never initialized) -> 0 (if inited and shown)
        void setFlags( int flags );

        // geometric properties
        QRectF geometry() const; // no default
        void setGeometry( const QRectF &geom );

        // window contens/override properties
        QString title() const;   // '' text in the titlebar (overrides author)
        void setTitle( const QString &title );
        QString summary() const; // '' short description (displayed if not empty)
        void setSummary( const QString &summary );
        QString text() const;    // '' text for the window (overrides annot->contents)
        void setText( const QString &text );

      private:
        class Private;
        QSharedDataPointer<Private> d;
    };

    /// \since 0.20
    Popup popup() const;
    /// \warning Currently does nothing \since 0.20
    void setPopup( const Popup& popup );

    /// \cond PRIVATE
    // This field is deprecated and not used any more. Use popup
    Q_DECL_DEPRECATED struct { int width, height; } window; // Always set to zero
    /// \endcond

    /// \since 0.20
    RevScope revisionScope() const; // Root

    /// \since 0.20
    RevType revisionType() const;   // None

    /**
     * Returns the revisions of this annotation
     *
     * \note The caller owns the returned annotations and they should
     *       be deleted when no longer required.
     *
     * \since 0.20
     */
    QList<Annotation*> revisions() const;

    /**
     * The type of the annotation.
     */
    virtual SubType subType() const = 0;

    /**
     * Destructor.
     */
    virtual ~Annotation();

    /**
     * Describes the flags from an annotations 'AA' dictionary.
     *
     * This flag is used by the additionalAction() method for ScreenAnnotation
     * and WidgetAnnotation.
     *
     * \since 0.22
     */
    enum AdditionalActionType
    {
        CursorEnteringAction, ///< Performed when the cursor enters the annotation's active area
        CursorLeavingAction,  ///< Performed when the cursor exists the annotation's active area
        MousePressedAction,   ///< Performed when the mouse button is pressed inside the annotation's active area
        MouseReleasedAction,  ///< Performed when the mouse button is released inside the annotation's active area
        FocusInAction,        ///< Performed when the annotation receives the input focus
        FocusOutAction,       ///< Performed when the annotation loses the input focus
        PageOpeningAction,    ///< Performed when the page containing the annotation is opened
        PageClosingAction,    ///< Performed when the page containing the annotation is closed
        PageVisibleAction,    ///< Performed when the page containing the annotation becomes visible
        PageInvisibleAction   ///< Performed when the page containing the annotation becomes invisible
    };

  protected:
    /// \cond PRIVATE
    Annotation( AnnotationPrivate &dd );
    Annotation( AnnotationPrivate &dd, const QDomNode &description );
    void storeBaseAnnotationProperties( QDomNode & parentNode, QDomDocument & document ) const;
    Q_DECLARE_PRIVATE( Annotation )
    QExplicitlySharedDataPointer<AnnotationPrivate> d_ptr;
    /// \endcond

  private:
    virtual void store( QDomNode & parentNode, QDomDocument & document ) const = 0;
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
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    // local enums
    enum TextType { Linked, InPlace };
    enum InplaceIntent { Unknown, Callout, TypeWriter };

    TextAnnotation( TextType type );
    virtual ~TextAnnotation();
    virtual SubType subType() const;

    /**
       The type of text annotation represented by this object
    */
    TextType textType() const;

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

    /**
       Synonym for contents()

       \deprecated Use contents() instead
    */
    QString inplaceText() const;
    /**
       Synonym for setContents()

       \deprecated Use setContents() instead
    */
    void setInplaceText( const QString &text );

    QPointF calloutPoint( int id ) const;
    /// \since 0.20
    QVector<QPointF> calloutPoints() const;
    /// \since 0.20
    void setCalloutPoints( const QVector<QPointF> &points );

    InplaceIntent inplaceIntent() const;
    void setInplaceIntent( InplaceIntent intent );

  private:
    TextAnnotation( const QDomNode &node );
    TextAnnotation( TextAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    void setTextType( TextType type );
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
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    // local enums
    /// \since 0.20
    enum LineType { StraightLine, Polyline };
    enum TermStyle { Square, Circle, Diamond, OpenArrow, ClosedArrow, None,
                     Butt, ROpenArrow, RClosedArrow, Slash };
    enum LineIntent { Unknown, Arrow, Dimension, PolygonCloud };

    /// \since 0.20
    LineAnnotation( LineType type );
    virtual ~LineAnnotation();
    virtual SubType subType() const;

    /// \since 0.20
    LineType lineType() const;

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
    LineAnnotation( const QDomNode &node );
    LineAnnotation( LineAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    void setLineType( LineType type );
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
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    GeomAnnotation();
    virtual ~GeomAnnotation();
    virtual SubType subType() const;

    // common enums
    enum GeomType { InscribedSquare, InscribedCircle };

    GeomType geomType() const;
    void setGeomType( GeomType style );

    QColor geomInnerColor() const;
    void setGeomInnerColor( const QColor &color );

  private:
    GeomAnnotation( const QDomNode &node );
    GeomAnnotation( GeomAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
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
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    HighlightAnnotation();
    virtual ~HighlightAnnotation();
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
    HighlightAnnotation( const QDomNode &node );
    HighlightAnnotation( HighlightAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
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
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    StampAnnotation();
    virtual ~StampAnnotation();
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
    StampAnnotation( const QDomNode &node );
    StampAnnotation( StampAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
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
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    InkAnnotation();
    virtual ~InkAnnotation();
    virtual SubType subType() const;

    QList< QLinkedList<QPointF> > inkPaths() const;
    void setInkPaths( const QList< QLinkedList<QPointF> > &paths );

  private:
    InkAnnotation( const QDomNode &node );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    InkAnnotation(InkAnnotationPrivate &dd);
    Q_DECLARE_PRIVATE( InkAnnotation )
    Q_DISABLE_COPY( InkAnnotation )
};

class POPPLER_QT4_EXPORT LinkAnnotation : public Annotation
{
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    virtual ~LinkAnnotation();
    virtual SubType subType() const;

    // local enums
    enum HighlightMode { None, Invert, Outline, Push };

    /** \since 0.20 */
    Link* linkDestination() const;
    void setLinkDestination( Link *link );

    HighlightMode linkHighlightMode() const;
    void setLinkHighlightMode( HighlightMode mode );

    QPointF linkRegionPoint( int id ) const;
    void setLinkRegionPoint( int id, const QPointF &point );

  private:
    LinkAnnotation();
    LinkAnnotation( const QDomNode &node );
    LinkAnnotation( LinkAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
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
  friend class AnnotationUtils;
  friend class AnnotationPrivate;

  public:
    CaretAnnotation();
    virtual ~CaretAnnotation();
    virtual SubType subType() const;

    /**
     * The symbols for the caret annotation.
     */
    enum CaretSymbol { None, P };

    CaretSymbol caretSymbol() const;
    void setCaretSymbol( CaretSymbol symbol );

  private:
    CaretAnnotation( const QDomNode &node );
    CaretAnnotation( CaretAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
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
  friend class AnnotationPrivate;

  public:
    virtual ~FileAttachmentAnnotation();
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
    FileAttachmentAnnotation();
    FileAttachmentAnnotation( const QDomNode &node );
    FileAttachmentAnnotation( FileAttachmentAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
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
  friend class AnnotationPrivate;

  public:
    virtual ~SoundAnnotation();
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
    SoundAnnotation();
    SoundAnnotation( const QDomNode &node );
    SoundAnnotation( SoundAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
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
  friend class AnnotationPrivate;

  public:
    virtual ~MovieAnnotation();
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
    MovieAnnotation();
    MovieAnnotation( const QDomNode &node );
    MovieAnnotation( MovieAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const;
    Q_DECLARE_PRIVATE( MovieAnnotation )
    Q_DISABLE_COPY( MovieAnnotation )
};

/**
 * \short Screen annotation.
 *
 * The screen annotation represents a screen to be played when activated.
 *
 * \since 0.20
 */
class POPPLER_QT4_EXPORT ScreenAnnotation : public Annotation
{
  friend class AnnotationPrivate;

  public:
    virtual ~ScreenAnnotation();

    virtual SubType subType() const;

    /**
     * Returns the LinkRendition of this annotation.
     */
    LinkRendition* action() const;

    /**
     * Sets a new LinkRendition for this annotation.
     *
     * \note ScreenAnnotation takes ownership of the object
     */
    void setAction( LinkRendition *action );

    /**
     * Returns the title of the screen of this annotation.
     */
    QString screenTitle() const;

    /**
     * Sets a new title for the screen of this annotation.
     */
    void setScreenTitle( const QString &title );

    /**
     * Returns the additional action of the given @p type fo the annotation or
     * @c 0 if no action has been defined.
     *
     * \since 0.22
     */
    Link* additionalAction( AdditionalActionType type ) const;

  private:
    ScreenAnnotation();
    ScreenAnnotation( ScreenAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const; // stub
    Q_DECLARE_PRIVATE( ScreenAnnotation )
    Q_DISABLE_COPY( ScreenAnnotation )
};

/**
 * \short Widget annotation.
 *
 * The widget annotation represents a widget (form field) on a page.
 *
 * \note This class is just provided for consistency of the annotation API,
 *       use the FormField classes to get all the form-related information.
 *
 * \since 0.22
 */
class POPPLER_QT4_EXPORT WidgetAnnotation : public Annotation
{
  friend class AnnotationPrivate;

  public:
    virtual ~WidgetAnnotation();

    virtual SubType subType() const;

    /**
     * Returns the additional action of the given @p type fo the annotation or
     * @c 0 if no action has been defined.
     *
     * \since 0.22
     */
    Link* additionalAction( AdditionalActionType type ) const;

  private:
    WidgetAnnotation();
    WidgetAnnotation( WidgetAnnotationPrivate &dd );
    virtual void store( QDomNode &parentNode, QDomDocument &document ) const; // stub
    Q_DECLARE_PRIVATE( WidgetAnnotation )
    Q_DISABLE_COPY( WidgetAnnotation )
};

}

#endif
