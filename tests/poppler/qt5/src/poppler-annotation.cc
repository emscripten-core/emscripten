/* poppler-annotation.cc: qt interface to poppler
 * Copyright (C) 2006, 2009, 2012, 2013 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006, 2008, 2010 Pino Toscano <pino@kde.org>
 * Copyright (C) 2012, Guillermo A. Amaral B. <gamaral@kde.org>
 * Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2012, Tobias Koenig <tokoe@kdab.com>
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

// qt/kde includes
#include <QtCore/QRegExp>
#include <QtCore/QtAlgorithms>
#include <QtXml/QDomElement>
#include <QtGui/QColor>
#include <QtGui/QTransform>

// local includes
#include "poppler-annotation.h"
#include "poppler-link.h"
#include "poppler-qt5.h"
#include "poppler-annotation-helper.h"
#include "poppler-annotation-private.h"
#include "poppler-page-private.h"
#include "poppler-private.h"

// poppler includes
#include <Page.h>
#include <Annot.h>
#include <Gfx.h>
#include <Error.h>
#include <FileSpec.h>
#include <Link.h>

/* Almost all getters directly query the underlying poppler annotation, with
 * the exceptions of link, file attachment, sound, movie and screen annotations,
 * Whose data retrieval logic has not been moved yet. Their getters return
 * static data set at creation time by findAnnotations
 */

namespace Poppler {

//BEGIN AnnotationUtils implementation
Annotation * AnnotationUtils::createAnnotation( const QDomElement & annElement )
{
    // safety check on annotation element
    if ( !annElement.hasAttribute( "type" ) )
        return 0;

    // build annotation of given type
    Annotation * annotation = 0;
    int typeNumber = annElement.attribute( "type" ).toInt();
    switch ( typeNumber )
    {
        case Annotation::AText:
            annotation = new TextAnnotation( annElement );
            break;
        case Annotation::ALine:
            annotation = new LineAnnotation( annElement );
            break;
        case Annotation::AGeom:
            annotation = new GeomAnnotation( annElement );
            break;
        case Annotation::AHighlight:
            annotation = new HighlightAnnotation( annElement );
            break;
        case Annotation::AStamp:
            annotation = new StampAnnotation( annElement );
            break;
        case Annotation::AInk:
            annotation = new InkAnnotation( annElement );
            break;
        case Annotation::ACaret:
            annotation = new CaretAnnotation( annElement );
            break;
    }

    // return created annotation
    return annotation;
}

void AnnotationUtils::storeAnnotation( const Annotation * ann, QDomElement & annElement,
    QDomDocument & document )
{
    // save annotation's type as element's attribute
    annElement.setAttribute( "type", (uint)ann->subType() );

    // append all annotation data as children of this node
    ann->store( annElement, document );
}

QDomElement AnnotationUtils::findChildElement( const QDomNode & parentNode,
    const QString & name )
{
    // loop through the whole children and return a 'name' named element
    QDomNode subNode = parentNode.firstChild();
    while( subNode.isElement() )
    {
        QDomElement element = subNode.toElement();
        if ( element.tagName() == name )
            return element;
        subNode = subNode.nextSibling();
    }
    // if the name can't be found, return a dummy null element
    return QDomElement();
}
//END AnnotationUtils implementation


//BEGIN Annotation implementation
AnnotationPrivate::AnnotationPrivate()
    : flags( 0 ), revisionScope ( Annotation::Root ),
    revisionType ( Annotation::None ), pdfAnnot ( 0 ), pdfPage ( 0 ),
    parentDoc ( 0 )
{
}

void AnnotationPrivate::addRevision( Annotation *ann, Annotation::RevScope scope, Annotation::RevType type )
{
    /* Since ownership stays with the caller, create an alias of ann */
    revisions.append( ann->d_ptr->makeAlias() );

    /* Set revision properties */
    revisionScope = scope;
    revisionType = type;
}

AnnotationPrivate::~AnnotationPrivate()
{
    // Delete all children revisions
    qDeleteAll( revisions );

    // Release Annot object
    if (pdfAnnot)
        pdfAnnot->decRefCnt();
}

void AnnotationPrivate::tieToNativeAnnot(Annot *ann, ::Page *page, Poppler::DocumentData * doc)
{
    if (pdfAnnot)
    {
        error(errIO, -1, "Annotation is already tied");
        return;
    }

    pdfAnnot = ann;
    pdfPage = page;
    parentDoc = doc;

    pdfAnnot->incRefCnt();
}

/* This method is called when a new annotation is created, after pdfAnnot and
 * pdfPage have been set */
void AnnotationPrivate::flushBaseAnnotationProperties()
{
    Q_ASSERT ( pdfPage );

    Annotation *q = makeAlias(); // Setters are defined in the public class

    // Since pdfAnnot has been set, this calls will write in the Annot object
    q->setAuthor(author);
    q->setContents(contents);
    q->setUniqueName(uniqueName);
    q->setModificationDate(modDate);
    q->setCreationDate(creationDate);
    q->setFlags(flags);
    //q->setBoundary(boundary); -- already set by subclass-specific code
    q->setStyle(style);
    q->setPopup(popup);

    // Flush revisions
    foreach (Annotation *r, revisions)
    {
        // TODO: Flush revision
        delete r; // Object is no longer needed
    }

    delete q;

    // Clear some members to save memory
    author.clear();
    contents.clear();
    uniqueName.clear();
    revisions.clear();
}

// Returns matrix to convert from user space coords (oriented according to the
// specified rotation) to normalized coords
void AnnotationPrivate::fillNormalizationMTX(double MTX[6], int pageRotation) const
{
    Q_ASSERT ( pdfPage );

    // build a normalized transform matrix for this page at 100% scale
    GfxState * gfxState = new GfxState( 72.0, 72.0, pdfPage->getCropBox(), pageRotation, gTrue );
    double * gfxCTM = gfxState->getCTM();

    double w = pdfPage->getCropWidth();
    double h = pdfPage->getCropHeight();

    // Swap width and height if the page is rotated landscape or seascape
    if ( pageRotation == 90 || pageRotation == 270 )
    {
        double t = w;
        w = h;
        h = t;
    }

    for ( int i = 0; i < 6; i+=2 )
    {
        MTX[i] = gfxCTM[i] / w;
        MTX[i+1] = gfxCTM[i+1] / h;
    }
    delete gfxState;
}

// Returns matrix to convert from user space coords (i.e. those that are stored
// in the PDF file) to normalized coords (i.e. those that we expose to clients).
// This method also applies a rotation around the top-left corner if the
// FixedRotation flag is set.
void AnnotationPrivate::fillTransformationMTX(double MTX[6]) const
{
    Q_ASSERT ( pdfPage );
    Q_ASSERT ( pdfAnnot );

    const int pageRotate = pdfPage->getRotate();

    if ( pageRotate == 0 || ( pdfAnnot->getFlags() & Annot::flagNoRotate ) == 0 )
    {
        // Use the normalization matrix for this page's rotation
        fillNormalizationMTX( MTX, pageRotate );
    }
    else
    {
        // Clients expect coordinates relative to this page's rotation, but
        // FixedRotation annotations internally use unrotated coordinates:
        // construct matrix to both normalize and rotate coordinates using the
        // top-left corner as rotation pivot

        double MTXnorm[6];
        fillNormalizationMTX( MTXnorm, pageRotate );

        QTransform transform( MTXnorm[0], MTXnorm[1], MTXnorm[2],
                              MTXnorm[3], MTXnorm[4], MTXnorm[5] );
        transform.translate( +pdfAnnot->getXMin(), +pdfAnnot->getYMax() );
        transform.rotate( pageRotate );
        transform.translate( -pdfAnnot->getXMin(), -pdfAnnot->getYMax() );

        MTX[0] = transform.m11();
        MTX[1] = transform.m12();
        MTX[2] = transform.m21();
        MTX[3] = transform.m22();
        MTX[4] = transform.dx();
        MTX[5] = transform.dy();
    }
}

QRectF AnnotationPrivate::fromPdfRectangle(const PDFRectangle &r) const
{
    double swp, MTX[6];
    fillTransformationMTX(MTX);

    QPointF p1, p2;
    XPDFReader::transform( MTX, r.x1, r.y1, p1 );
    XPDFReader::transform( MTX, r.x2, r.y2, p2 );

    double tl_x = p1.x();
    double tl_y = p1.y();
    double br_x = p2.x();
    double br_y = p2.y();

    if (tl_x > br_x)
    {
        swp = tl_x;
        tl_x = br_x;
        br_x = swp;
    }

    if (tl_y > br_y)
    {
        swp = tl_y;
        tl_y = br_y;
        br_y = swp;
    }

    return QRectF( QPointF(tl_x,tl_y) , QPointF(br_x,br_y) );
}

// This function converts a boundary QRectF in normalized coords to a
// PDFRectangle in user coords. If the FixedRotation flag is set, this function
// also applies a rotation around the top-left corner: it's the inverse of
// the transformation produced by fillTransformationMTX, but we can't use
// fillTransformationMTX here because it relies on the native annotation
// object's boundary rect to be already set up.
PDFRectangle AnnotationPrivate::boundaryToPdfRectangle(const QRectF &r, int flags) const
{
    Q_ASSERT ( pdfPage );

    const int pageRotate = pdfPage->getRotate();

    double MTX[6];
    fillNormalizationMTX( MTX, pageRotate );

    double tl_x, tl_y, br_x, br_y, swp;
    XPDFReader::invTransform( MTX, r.topLeft(), tl_x, tl_y );
    XPDFReader::invTransform( MTX, r.bottomRight(), br_x, br_y );

    if (tl_x > br_x)
    {
        swp = tl_x;
        tl_x = br_x;
        br_x = swp;
    }

    if (tl_y > br_y)
    {
        swp = tl_y;
        tl_y = br_y;
        br_y = swp;
    }

    const int rotationFixUp = ( flags & Annotation::FixedRotation ) ? pageRotate : 0;
    const double width = br_x - tl_x;
    const double height = br_y - tl_y;

    if ( rotationFixUp == 0 )
        return PDFRectangle(tl_x, tl_y, br_x, br_y);
    else if ( rotationFixUp == 90 )
        return PDFRectangle(tl_x, tl_y - width, tl_x + height, tl_y);
    else if ( rotationFixUp == 180 )
        return PDFRectangle(br_x, tl_y - height, br_x + width, tl_y);
    else // rotationFixUp == 270
        return PDFRectangle(br_x, br_y - width, br_x + height, br_y);
}

AnnotPath * AnnotationPrivate::toAnnotPath(const QLinkedList<QPointF> &list) const
{
    const int count = list.size();
    AnnotCoord **ac = (AnnotCoord **) gmallocn(count, sizeof(AnnotCoord*));

    double MTX[6];
    fillTransformationMTX(MTX);

    int pos = 0;
    foreach (const QPointF &p, list)
    {
        double x, y;
        XPDFReader::invTransform( MTX, p, x, y );
        ac[pos++] = new AnnotCoord(x, y);
    }

    return new AnnotPath(ac, count);
}

QList<Annotation*> AnnotationPrivate::findAnnotations(::Page *pdfPage, DocumentData *doc, int parentID)
{
    Annots* annots = pdfPage->getAnnots();
    const uint numAnnotations = annots->getNumAnnots();
    if ( numAnnotations == 0 )
    {
        return QList<Annotation*>();
    }

    // Create Annotation objects and tie to their native Annot
    QList<Annotation*> res;
    for ( uint j = 0; j < numAnnotations; j++ )
    {
        // get the j-th annotation
        Annot * ann = annots->getAnnot( j );
        if ( !ann )
        {
            error(errInternal, -1, "Annot %u is null", j);
            continue;
        }

        // Check parent annotation
        AnnotMarkup * markupann = dynamic_cast< AnnotMarkup * >( ann );
        if (!markupann)
        {
            // Assume it's a root annotation, and skip if user didn't request it
            if (parentID != 0)
                continue;
        }
        else if (markupann->getInReplyToID() != parentID)
            continue;

        /* Create Annotation of the right subclass */
        Annotation * annotation = 0;
        Annot::AnnotSubtype subType = ann->getType();

        switch ( subType )
        {
            case Annot::typeText:
                annotation = new TextAnnotation(TextAnnotation::Linked);
                break;
            case Annot::typeFreeText:
                annotation = new TextAnnotation(TextAnnotation::InPlace);
                break;
            case Annot::typeLine:
                annotation = new LineAnnotation(LineAnnotation::StraightLine);
                break;
            case Annot::typePolygon:
            case Annot::typePolyLine:
                annotation = new LineAnnotation(LineAnnotation::Polyline);
                break;
            case Annot::typeSquare:
            case Annot::typeCircle:
                annotation = new GeomAnnotation();
                break;
            case Annot::typeHighlight:
            case Annot::typeUnderline:
            case Annot::typeSquiggly:
            case Annot::typeStrikeOut:
                annotation = new HighlightAnnotation();
                break;
            case Annot::typeStamp:
                annotation = new StampAnnotation();
                break;
            case Annot::typeInk:
                annotation = new InkAnnotation();
                break;
            case Annot::typeLink: /* TODO: Move logic to getters */
            {
                // parse Link params
                AnnotLink * linkann = static_cast< AnnotLink * >( ann );
                LinkAnnotation * l = new LinkAnnotation();
                annotation = l;

                // -> hlMode
                l->setLinkHighlightMode( (LinkAnnotation::HighlightMode)linkann->getLinkEffect() );

                // -> link region
                // TODO

                // reading link action
                if ( linkann->getAction() )
                {
                    Link * popplerLink = PageData::convertLinkActionToLink( linkann->getAction(), doc, QRectF() );
                    if ( popplerLink )
                    {
                        l->setLinkDestination( popplerLink );
                    }
                }
                break;
            }
            case Annot::typeCaret:
                annotation = new CaretAnnotation();
                break;
            case Annot::typeFileAttachment: /* TODO: Move logic to getters */
            {
                AnnotFileAttachment * attachann = static_cast< AnnotFileAttachment * >( ann );
                FileAttachmentAnnotation * f = new FileAttachmentAnnotation();
                annotation = f;
                // -> fileIcon
                f->setFileIconName( QString::fromLatin1( attachann->getName()->getCString() ) );
                // -> embeddedFile
                FileSpec *filespec = new FileSpec( attachann->getFile() );
                f->setEmbeddedFile( new EmbeddedFile( *new EmbeddedFileData( filespec ) ) );
                break;
            }
            case Annot::typeSound: /* TODO: Move logic to getters */
            {
                AnnotSound * soundann = static_cast< AnnotSound * >( ann );
                SoundAnnotation * s = new SoundAnnotation();
                annotation = s;

                // -> soundIcon
                s->setSoundIconName( QString::fromLatin1( soundann->getName()->getCString() ) );
                // -> sound
                s->setSound( new SoundObject( soundann->getSound() ) );
                break;
            }
            case Annot::typeMovie: /* TODO: Move logic to getters */
            {
                AnnotMovie * movieann = static_cast< AnnotMovie * >( ann );
                MovieAnnotation * m = new MovieAnnotation();
                annotation = m;

                // -> movie
                MovieObject *movie = new MovieObject( movieann );
                m->setMovie( movie );
                // -> movieTitle
                GooString * movietitle = movieann->getTitle();
                if ( movietitle )
                    m->setMovieTitle( QString::fromLatin1( movietitle->getCString() ) );
                break;
            }
            case Annot::typeScreen:
            {
                AnnotScreen * screenann = static_cast< AnnotScreen * >( ann );
                if (!screenann->getAction())
                  continue;
                ScreenAnnotation * s = new ScreenAnnotation();
                annotation = s;

                // -> screen
                Link * popplerLink = PageData::convertLinkActionToLink( screenann->getAction(), doc, QRectF() );
                s->setAction( static_cast<Poppler::LinkRendition *>(popplerLink) );

                // -> screenTitle
                GooString * screentitle = screenann->getTitle();
                if ( screentitle )
                    s->setScreenTitle( UnicodeParsedString( screentitle ) );
                break;
            }
            case Annot::typePopup:
                continue; // popups are parsed by Annotation's window() getter
            case Annot::typeUnknown:
                continue; // special case for ignoring unknown annotations
            case Annot::typeWidget:
                annotation = new WidgetAnnotation();
                break;
            default:
            {
#define CASE_FOR_TYPE( thetype ) \
                    case Annot::type ## thetype: \
                        error(errUnimplemented, -1, "Annotation " #thetype " not supported"); \
                        break;
                switch ( subType )
                {
                    CASE_FOR_TYPE( PrinterMark )
                    CASE_FOR_TYPE( TrapNet )
                    CASE_FOR_TYPE( Watermark )
                    CASE_FOR_TYPE( 3D )
                    default: error(errUnimplemented, -1, "Annotation %u not supported", subType);
                }
                continue;
#undef CASE_FOR_TYPE
            }
        }

        annotation->d_ptr->tieToNativeAnnot(ann, pdfPage, doc);
        res.append(annotation);
    }

    return res;
}

Ref AnnotationPrivate::pdfObjectReference() const
{
    if (pdfAnnot == 0)
    {
        const Ref invalid_ref = { -1, -1 };
        return invalid_ref;
    }

    return pdfAnnot->getRef();
}

Link* AnnotationPrivate::additionalAction( Annotation::AdditionalActionType type ) const
{
    if ( pdfAnnot->getType() != Annot::typeScreen && pdfAnnot->getType() != Annot::typeWidget )
        return 0;

    Annot::AdditionalActionsType actionType = Annot::actionCursorEntering;
    switch ( type )
    {
        case Annotation::CursorEnteringAction: actionType = Annot::actionCursorEntering; break;
        case Annotation::CursorLeavingAction: actionType = Annot::actionCursorLeaving; break;
        case Annotation::MousePressedAction: actionType = Annot::actionMousePressed; break;
        case Annotation::MouseReleasedAction: actionType = Annot::actionMouseReleased; break;
        case Annotation::FocusInAction: actionType = Annot::actionFocusIn; break;
        case Annotation::FocusOutAction: actionType = Annot::actionFocusOut; break;
        case Annotation::PageOpeningAction: actionType = Annot::actionPageOpening; break;
        case Annotation::PageClosingAction: actionType = Annot::actionPageClosing; break;
        case Annotation::PageVisibleAction: actionType = Annot::actionPageVisible; break;
        case Annotation::PageInvisibleAction: actionType = Annot::actionPageInvisible; break;
    }

    ::LinkAction *linkAction = 0;
    if ( pdfAnnot->getType() == Annot::typeScreen )
        linkAction = static_cast<AnnotScreen*>( pdfAnnot )->getAdditionalAction( actionType );
    else
        linkAction = static_cast<AnnotWidget*>( pdfAnnot )->getAdditionalAction( actionType );

    Link *link = 0;

    if ( linkAction )
        link = PageData::convertLinkActionToLink( linkAction, parentDoc, QRectF() );

    return link;
}

void AnnotationPrivate::addAnnotationToPage(::Page *pdfPage, DocumentData *doc, const Annotation * ann)
{
    if (ann->d_ptr->pdfAnnot != 0)
    {
        error(errIO, -1, "Annotation is already tied");
        return;
    }

    // Unimplemented annotations can't be created by the user because their ctor
    // is private. Therefore, createNativeAnnot will never return 0
    Annot *nativeAnnot = ann->d_ptr->createNativeAnnot(pdfPage, doc);
    Q_ASSERT(nativeAnnot);
    pdfPage->addAnnot(nativeAnnot);
}

void AnnotationPrivate::removeAnnotationFromPage(::Page *pdfPage, const Annotation * ann)
{
    if (ann->d_ptr->pdfAnnot == 0)
    {
        error(errIO, -1, "Annotation is not tied");
        return;
    }

    if (ann->d_ptr->pdfPage != pdfPage)
    {
        error(errIO, -1, "Annotation doesn't belong to the specified page");
        return;
    }

    // Remove annotation
    pdfPage->removeAnnot(ann->d_ptr->pdfAnnot);

    // Destroy object
    delete ann;
}

class Annotation::Style::Private : public QSharedData
{
  public:
    Private()
        : opacity( 1.0 ), width( 1.0 ), lineStyle( Solid ), xCorners( 0.0 ),
        yCorners( 0.0 ), lineEffect( NoEffect ), effectIntensity( 1.0 )
        {
            dashArray.resize(1);
            dashArray[0] = 3;
        }

    QColor color;
    double opacity;
    double width;
    Annotation::LineStyle lineStyle;
    double xCorners;
    double yCorners;
    QVector<double> dashArray;
    Annotation::LineEffect lineEffect;
    double effectIntensity;
};

Annotation::Style::Style()
    : d ( new Private )
{
}

Annotation::Style::Style( const Style &other )
    : d( other.d )
{
}

Annotation::Style& Annotation::Style::operator=( const Style &other )
{
    if ( this != &other )
        d = other.d;

    return *this;
}

Annotation::Style::~Style()
{
}

QColor Annotation::Style::color() const
{
    return d->color;
}

void Annotation::Style::setColor(const QColor &color)
{
    d->color = color;
}

double Annotation::Style::opacity() const
{
    return d->opacity;
}

void Annotation::Style::setOpacity(double opacity)
{
    d->opacity = opacity;
}

double Annotation::Style::width() const
{
    return d->width;
}

void Annotation::Style::setWidth(double width)
{
    d->width = width;
}

Annotation::LineStyle Annotation::Style::lineStyle() const
{
    return d->lineStyle;
}

void Annotation::Style::setLineStyle(Annotation::LineStyle style)
{
    d->lineStyle = style;
}

double Annotation::Style::xCorners() const
{
    return d->xCorners;
}

void Annotation::Style::setXCorners(double radius)
{
    d->xCorners = radius;
}

double Annotation::Style::yCorners() const
{
    return d->yCorners;
}

void Annotation::Style::setYCorners(double radius)
{
    d->yCorners = radius;
}

const QVector<double>& Annotation::Style::dashArray() const
{
    return d->dashArray;
}

void Annotation::Style::setDashArray(const QVector<double> &array)
{
    d->dashArray = array;
}

Annotation::LineEffect Annotation::Style::lineEffect() const
{
    return d->lineEffect;
}

void Annotation::Style::setLineEffect(Annotation::LineEffect effect)
{
    d->lineEffect = effect;
}

double Annotation::Style::effectIntensity() const
{
    return d->effectIntensity;
}

void Annotation::Style::setEffectIntensity(double intens)
{
    d->effectIntensity = intens;
}

class Annotation::Popup::Private : public QSharedData
{
  public:
    Private()
        : flags( -1 ) {}

    int flags;
    QRectF geometry;
    QString title;
    QString summary;
    QString text;
};

Annotation::Popup::Popup()
    : d ( new Private )
{
}

Annotation::Popup::Popup( const Popup &other )
    : d( other.d )
{
}

Annotation::Popup& Annotation::Popup::operator=( const Popup &other )
{
    if ( this != &other )
        d = other.d;

    return *this;
}

Annotation::Popup::~Popup()
{
}

int Annotation::Popup::flags() const
{
    return d->flags;
}

void Annotation::Popup::setFlags( int flags )
{
    d->flags = flags;
}

QRectF Annotation::Popup::geometry() const
{
    return d->geometry;
}

void Annotation::Popup::setGeometry( const QRectF &geom )
{
    d->geometry = geom;
}

QString Annotation::Popup::title() const
{
    return d->title;
}

void Annotation::Popup::setTitle( const QString &title )
{
    d->title = title;
}

QString Annotation::Popup::summary() const
{
    return d->summary;
}

void Annotation::Popup::setSummary( const QString &summary )
{
    d->summary = summary;
}

QString Annotation::Popup::text() const
{
    return d->text;
}

void Annotation::Popup::setText( const QString &text )
{
    d->text = text;
}

Annotation::Annotation( AnnotationPrivate &dd )
    : d_ptr( &dd )
{
}

Annotation::~Annotation()
{
}

Annotation::Annotation( AnnotationPrivate &dd, const QDomNode &annNode )
    : d_ptr( &dd )
{
    Q_D( Annotation );

    // get the [base] element of the annotation node
    QDomElement e = AnnotationUtils::findChildElement( annNode, "base" );
    if ( e.isNull() )
        return;

    Style s;
    Popup w;

    // parse -contents- attributes
    if ( e.hasAttribute( "author" ) )
        setAuthor(e.attribute( "author" ));
    if ( e.hasAttribute( "contents" ) )
        setContents(e.attribute( "contents" ));
    if ( e.hasAttribute( "uniqueName" ) )
        setUniqueName(e.attribute( "uniqueName" ));
    if ( e.hasAttribute( "modifyDate" ) )
        setModificationDate(QDateTime::fromString( e.attribute( "modifyDate" ) ));
    if ( e.hasAttribute( "creationDate" ) )
        setCreationDate(QDateTime::fromString( e.attribute( "creationDate" ) ));

    // parse -other- attributes
    if ( e.hasAttribute( "flags" ) )
        setFlags(e.attribute( "flags" ).toInt());
    if ( e.hasAttribute( "color" ) )
        s.setColor(QColor( e.attribute( "color" ) ));
    if ( e.hasAttribute( "opacity" ) )
        s.setOpacity(e.attribute( "opacity" ).toDouble());

    // parse -the-subnodes- (describing Style, Window, Revision(s) structures)
    // Note: all subnodes if present must be 'attributes complete'
    QDomNode eSubNode = e.firstChild();
    while ( eSubNode.isElement() )
    {
        QDomElement ee = eSubNode.toElement();
        eSubNode = eSubNode.nextSibling();

        // parse boundary
        if ( ee.tagName() == "boundary" )
        {
            QRectF brect;
            brect.setLeft(ee.attribute( "l" ).toDouble());
            brect.setTop(ee.attribute( "t" ).toDouble());
            brect.setRight(ee.attribute( "r" ).toDouble());
            brect.setBottom(ee.attribute( "b" ).toDouble());
            setBoundary(brect);
        }
        // parse penStyle if not default
        else if ( ee.tagName() == "penStyle" )
        {
            s.setWidth(ee.attribute( "width" ).toDouble());
            s.setLineStyle((LineStyle)ee.attribute( "style" ).toInt());
            s.setXCorners(ee.attribute( "xcr" ).toDouble());
            s.setYCorners(ee.attribute( "ycr" ).toDouble());

            // Try to parse dash array (new format)
            QVector<double> dashArray;

            QDomNode eeSubNode = ee.firstChild();
            while ( eeSubNode.isElement() )
            {
                QDomElement eee = eeSubNode.toElement();
                eeSubNode = eeSubNode.nextSibling();

                if ( eee.tagName() != "dashsegm" )
                    continue;

                dashArray.append(eee.attribute( "len" ).toDouble());
            }

            // If no segments were found use marks/spaces (old format)
            if ( dashArray.size() == 0 )
            {
                dashArray.append(ee.attribute( "marks" ).toDouble());
                dashArray.append(ee.attribute( "spaces" ).toDouble());
            }

            s.setDashArray(dashArray);
        }
        // parse effectStyle if not default
        else if ( ee.tagName() == "penEffect" )
        {
            s.setLineEffect((LineEffect)ee.attribute( "effect" ).toInt());
            s.setEffectIntensity(ee.attribute( "intensity" ).toDouble());
        }
        // parse window if present
        else if ( ee.tagName() == "window" )
        {
            QRectF geom;
            geom.setX(ee.attribute( "top" ).toDouble());
            geom.setY(ee.attribute( "left" ).toDouble());

            if (ee.hasAttribute("widthDouble"))
                geom.setWidth(ee.attribute( "widthDouble" ).toDouble());
            else
                geom.setWidth(ee.attribute( "width" ).toDouble());

            if (ee.hasAttribute("widthDouble"))
                geom.setHeight(ee.attribute( "heightDouble" ).toDouble());
            else
                geom.setHeight(ee.attribute( "height" ).toDouble());

            w.setGeometry(geom);

            w.setFlags(ee.attribute( "flags" ).toInt());
            w.setTitle(ee.attribute( "title" ));
            w.setSummary(ee.attribute( "summary" ));
            // parse window subnodes
            QDomNode winNode = ee.firstChild();
            for ( ; winNode.isElement(); winNode = winNode.nextSibling() )
            {
                QDomElement winElement = winNode.toElement();
                if ( winElement.tagName() == "text" )
                    w.setText(winElement.firstChild().toCDATASection().data());
            }
        }
    }

    setStyle(s);  // assign parsed style
    setPopup(w); // assign parsed window

    // get the [revisions] element of the annotation node
    QDomNode revNode = annNode.firstChild();
    for ( ; revNode.isElement(); revNode = revNode.nextSibling() )
    {
        QDomElement revElement = revNode.toElement();
        if ( revElement.tagName() != "revision" )
            continue;

        Annotation *reply = AnnotationUtils::createAnnotation( revElement );

        if (reply) // if annotation is valid, add as a revision of this annotation
        {
            RevScope scope = (RevScope)revElement.attribute( "revScope" ).toInt();
            RevType type = (RevType)revElement.attribute( "revType" ).toInt();
            d->addRevision(reply, scope, type);
            delete reply;
        }
    }
}

void Annotation::storeBaseAnnotationProperties( QDomNode & annNode, QDomDocument & document ) const
{
    // create [base] element of the annotation node
    QDomElement e = document.createElement( "base" );
    annNode.appendChild( e );

    const Style s = style();
    const Popup w = popup();

    // store -contents- attributes
    if ( !author().isEmpty() )
        e.setAttribute( "author", author() );
    if ( !contents().isEmpty() )
        e.setAttribute( "contents", contents() );
    if ( !uniqueName().isEmpty() )
        e.setAttribute( "uniqueName", uniqueName() );
    if ( modificationDate().isValid() )
        e.setAttribute( "modifyDate", modificationDate().toString() );
    if ( creationDate().isValid() )
        e.setAttribute( "creationDate", creationDate().toString() );

    // store -other- attributes
    if ( flags() )
        e.setAttribute( "flags", flags() );
    if ( s.color().isValid() )
        e.setAttribute( "color", s.color().name() );
    if ( s.opacity() != 1.0 )
        e.setAttribute( "opacity", QString::number( s.opacity() ) );

    // Sub-Node-1 - boundary
    const QRectF brect = boundary();
    QDomElement bE = document.createElement( "boundary" );
    e.appendChild( bE );
    bE.setAttribute( "l", QString::number( (double)brect.left() ) );
    bE.setAttribute( "t", QString::number( (double)brect.top() ) );
    bE.setAttribute( "r", QString::number( (double)brect.right() ) );
    bE.setAttribute( "b", QString::number( (double)brect.bottom() ) );

    // Sub-Node-2 - penStyle
    const QVector<double> dashArray = s.dashArray();
    if ( s.width() != 1 || s.lineStyle() != Solid || s.xCorners() != 0 ||
         s.yCorners() != 0.0 || dashArray.size() != 1 || dashArray[0] != 3 )
    {
        QDomElement psE = document.createElement( "penStyle" );
        e.appendChild( psE );
        psE.setAttribute( "width", QString::number( s.width() ) );
        psE.setAttribute( "style", (int)s.lineStyle() );
        psE.setAttribute( "xcr", QString::number( s.xCorners() ) );
        psE.setAttribute( "ycr", QString::number( s.yCorners() ) );

        int marks = 3, spaces = 0; // Do not break code relying on marks/spaces
        if (dashArray.size() != 0)
            marks = (int)dashArray[0];
        if (dashArray.size() > 1)
            spaces = (int)dashArray[1];

        psE.setAttribute( "marks", marks );
        psE.setAttribute( "spaces", spaces );

        foreach (double segm, dashArray)
        {
            QDomElement pattE = document.createElement( "dashsegm" );
            pattE.setAttribute( "len", QString::number( segm ) );
            psE.appendChild(pattE);
        }
    }

    // Sub-Node-3 - penEffect
    if ( s.lineEffect() != NoEffect || s.effectIntensity() != 1.0 )
    {
        QDomElement peE = document.createElement( "penEffect" );
        e.appendChild( peE );
        peE.setAttribute( "effect", (int)s.lineEffect() );
        peE.setAttribute( "intensity", QString::number( s.effectIntensity() ) );
    }

    // Sub-Node-4 - window
    if ( w.flags() != -1 || !w.title().isEmpty() || !w.summary().isEmpty() ||
         !w.text().isEmpty() )
    {
        QDomElement wE = document.createElement( "window" );
        const QRectF geom = w.geometry();
        e.appendChild( wE );
        wE.setAttribute( "flags", w.flags() );
        wE.setAttribute( "top", QString::number( geom.x() ) );
        wE.setAttribute( "left", QString::number( geom.y() ) );
        wE.setAttribute( "width", (int)geom.width() );
        wE.setAttribute( "height", (int)geom.height() );
        wE.setAttribute( "widthDouble", QString::number( geom.width() ) );
        wE.setAttribute( "heightDouble", QString::number( geom.height() ) );
        wE.setAttribute( "title", w.title() );
        wE.setAttribute( "summary", w.summary() );
        // store window.text as a subnode, because we need escaped data
        if ( !w.text().isEmpty() )
        {
            QDomElement escapedText = document.createElement( "text" );
            wE.appendChild( escapedText );
            QDomCDATASection textCData = document.createCDATASection( w.text() );
            escapedText.appendChild( textCData );
        }
    }

    const QList<Annotation*> revs = revisions();

    // create [revision] element of the annotation node (if any)
    if ( revs.isEmpty() )
        return;

    // add all revisions as children of revisions element
    foreach (const Annotation *rev, revs)
    {
        QDomElement r = document.createElement( "revision" );
        annNode.appendChild( r );
        // set element attributes
        r.setAttribute( "revScope", (int)rev->revisionScope() );
        r.setAttribute( "revType", (int)rev->revisionType() );
        // use revision as the annotation element, so fill it up
        AnnotationUtils::storeAnnotation( rev, r, document );
        delete rev;
    }
}

QString Annotation::author() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->author;

    const AnnotMarkup *markupann = dynamic_cast<const AnnotMarkup*>(d->pdfAnnot);
    return markupann ? UnicodeParsedString( markupann->getLabel() ) : QString();
}

void Annotation::setAuthor( const QString &author )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->author = author;
        return;
    }

    AnnotMarkup *markupann = dynamic_cast<AnnotMarkup*>(d->pdfAnnot);
    if (markupann)
    {
        GooString *s = QStringToUnicodeGooString(author);
        markupann->setLabel(s);
        delete s;
    }
}

QString Annotation::contents() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->contents;

    return UnicodeParsedString( d->pdfAnnot->getContents() );
}

void Annotation::setContents( const QString &contents )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->contents = contents;
        return;
    }

    GooString *s = QStringToUnicodeGooString(contents);
    d->pdfAnnot->setContents(s);
    delete s;
}

QString Annotation::uniqueName() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->uniqueName;

    return UnicodeParsedString( d->pdfAnnot->getName() );
}

void Annotation::setUniqueName( const QString &uniqueName )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->uniqueName = uniqueName;
        return;
    }

    QByteArray ascii = uniqueName.toLatin1();
    GooString s(ascii.constData());
    d->pdfAnnot->setName(&s);
}

QDateTime Annotation::modificationDate() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->modDate;

    if ( d->pdfAnnot->getModified() )
        return convertDate( d->pdfAnnot->getModified()->getCString() );
    else
        return QDateTime();
}

void Annotation::setModificationDate( const QDateTime &date )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->modDate = date;
        return;
    }

#if 0 // TODO: Conversion routine is broken
    if (d->pdfAnnot)
    {
        time_t t = date.toTime_t();
        GooString *s = timeToDateString(&t);
        d->pdfAnnot->setModified(s);
        delete s;
    }
#endif
}

QDateTime Annotation::creationDate() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->creationDate;

    const AnnotMarkup *markupann = dynamic_cast<const AnnotMarkup*>(d->pdfAnnot);

    if (markupann && markupann->getDate())
        return convertDate( markupann->getDate()->getCString() );

    return modificationDate();
}

void Annotation::setCreationDate( const QDateTime &date )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->creationDate = date;
        return;
    }

#if 0 // TODO: Conversion routine is broken
    AnnotMarkup *markupann = dynamic_cast<AnnotMarkup*>(d->pdfAnnot);
    if (markupann)
    {
        time_t t = date.toTime_t();
        GooString *s = timeToDateString(&t);
        markupann->setDate(s);
        delete s;
    }
#endif
}

static int fromPdfFlags(int flags)
{
    int qtflags = 0;

    if ( flags & Annot::flagHidden )
        qtflags |= Annotation::Hidden;
    if ( flags & Annot::flagNoZoom )
        qtflags |= Annotation::FixedSize;
    if ( flags & Annot::flagNoRotate )
        qtflags |= Annotation::FixedRotation;
    if ( !( flags & Annot::flagPrint ) )
        qtflags |= Annotation::DenyPrint;
    if ( flags & Annot::flagReadOnly )
        qtflags |= (Annotation::DenyWrite | Annotation::DenyDelete);
    if ( flags & Annot::flagLocked )
        qtflags |= Annotation::DenyDelete;
    if ( flags & Annot::flagToggleNoView )
        qtflags |= Annotation::ToggleHidingOnMouse;

    return qtflags;
}

static int toPdfFlags(int qtflags)
{
    int flags = 0;

    if ( qtflags & Annotation::Hidden )
        flags |= Annot::flagHidden;
    if ( qtflags & Annotation::FixedSize )
        flags |= Annot::flagNoZoom;
    if ( qtflags & Annotation::FixedRotation )
        flags |= Annot::flagNoRotate;
    if ( !( qtflags & Annotation::DenyPrint ) )
        flags |= Annot::flagPrint;
    if ( qtflags & Annotation::DenyWrite )
        flags |= Annot::flagReadOnly;
    if ( qtflags & Annotation::DenyDelete )
        flags |= Annot::flagLocked;
    if ( qtflags & Annotation::ToggleHidingOnMouse )
        flags |= Annot::flagToggleNoView;

    return flags;
}

int Annotation::flags() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->flags;

    return fromPdfFlags( d->pdfAnnot->getFlags() );
}

void Annotation::setFlags( int flags )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->flags = flags;
        return;
    }

    d->pdfAnnot->setFlags(toPdfFlags( flags ));
}

QRectF Annotation::boundary() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->boundary;

    const PDFRectangle * rect = d->pdfAnnot->getRect();
    return d->fromPdfRectangle( *rect );
}

void Annotation::setBoundary( const QRectF &boundary )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->boundary = boundary;
        return;
    }

    PDFRectangle rect = d->boundaryToPdfRectangle( boundary, flags() );
    d->pdfAnnot->setRect(&rect);
}

Annotation::Style Annotation::style() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->style;

    Style s;
    s.setColor(convertAnnotColor( d->pdfAnnot->getColor() ));

    const AnnotMarkup *markupann = dynamic_cast<const AnnotMarkup*>(d->pdfAnnot);
    if (markupann)
        s.setOpacity( markupann->getOpacity() );

    const AnnotBorder *border = d->pdfAnnot->getBorder();
    if (border)
    {
        if ( border->getType() == AnnotBorder::typeArray )
        {
            const AnnotBorderArray *border_array = static_cast<const AnnotBorderArray*>(border);
            s.setXCorners( border_array->getHorizontalCorner() );
            s.setYCorners( border_array->getVerticalCorner() );
        }

        s.setWidth( border->getWidth() );
        s.setLineStyle((Annotation::LineStyle)( 1 << border->getStyle() ));

        const int dashArrLen = border->getDashLength();
        const double* dashArrData = border->getDash();
        QVector<double> dashArrVect( dashArrLen );
        for (int i = 0; i < dashArrLen; ++i)
            dashArrVect[i] = dashArrData[i];
        s.setDashArray(dashArrVect);
    }

    AnnotBorderEffect *border_effect;
    switch (d->pdfAnnot->getType())
    {
        case Annot::typeFreeText:
            border_effect = static_cast<AnnotFreeText*>(d->pdfAnnot)->getBorderEffect();
            break;
        case Annot::typeSquare:
        case Annot::typeCircle:
            border_effect = static_cast<AnnotGeometry*>(d->pdfAnnot)->getBorderEffect();
            break;
        default:
            border_effect = 0;
    }
    if (border_effect)
    {
        s.setLineEffect( (Annotation::LineEffect)border_effect->getEffectType() );
        s.setEffectIntensity( border_effect->getIntensity() );
    }

    return s;
}

void Annotation::setStyle( const Annotation::Style& style )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->style = style;
        return;
    }

    d->pdfAnnot->setColor(convertQColor( style.color() ));

    AnnotMarkup *markupann = dynamic_cast<AnnotMarkup*>(d->pdfAnnot);
    if (markupann)
        markupann->setOpacity( style.opacity() );

    AnnotBorderArray * border = new AnnotBorderArray();
    border->setWidth( style.width() );
    border->setHorizontalCorner( style.xCorners() );
    border->setVerticalCorner( style.yCorners() );
    d->pdfAnnot->setBorder(border);
}

Annotation::Popup Annotation::popup() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->popup;

    Popup w;
    AnnotPopup *popup = 0;
    int flags = -1; // Not initialized

    const AnnotMarkup *markupann = dynamic_cast<const AnnotMarkup*>(d->pdfAnnot);
    if (markupann)
    {
        popup = markupann->getPopup();
        w.setSummary(UnicodeParsedString( markupann->getSubject() ));
    }

    if (popup)
    {
        flags = fromPdfFlags( popup->getFlags() ) & ( Annotation::Hidden |
                Annotation::FixedSize | Annotation::FixedRotation );

        if (!popup->getOpen())
            flags |= Annotation::Hidden;

        const PDFRectangle * rect = popup->getRect();
        w.setGeometry( d->fromPdfRectangle( *rect ) );
    }

    if (d->pdfAnnot->getType() == Annot::typeText)
    {
        const AnnotText * textann = static_cast<const AnnotText*>(d->pdfAnnot);

        // Text annotations default to same rect as annotation
        if (flags == -1)
        {
            flags = 0;
            w.setGeometry( boundary() );
        }

        // If text is not 'opened', force window hiding. if the window
        // was parsed from popup, the flag should already be set
        if ( !textann->getOpen() && flags != -1 )
            flags |= Annotation::Hidden;
    }

    w.setFlags(flags);

    return w;
}

void Annotation::setPopup( const Annotation::Popup& popup )
{
    Q_D( Annotation );

    if (!d->pdfAnnot)
    {
        d->popup = popup;
        return;
    }

#if 0 /* TODO: Remove old popup and add AnnotPopup to page */
    AnnotMarkup *markupann = dynamic_cast<AnnotMarkup*>(d->pdfAnnot);
    if (!markupann)
        return;

    // Create a new AnnotPopup and assign it to pdfAnnot
    PDFRectangle rect = d->toPdfRectangle( popup.geometry() );
    AnnotPopup * p = new AnnotPopup( d->pdfPage->getDoc(), &rect );
    p->setOpen( !(popup.flags() & Annotation::Hidden) );
    if (!popup.summary().isEmpty())
    {
        GooString *s = QStringToUnicodeGooString(popup.summary());
        markupann->setLabel(s);
        delete s;
    }
    markupann->setPopup(p);
#endif
}

Annotation::RevScope Annotation::revisionScope() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->revisionScope;

    const AnnotMarkup *markupann = dynamic_cast<const AnnotMarkup*>(d->pdfAnnot);

    if (markupann && markupann->getInReplyToID() != 0)
    {
        switch (markupann->getReplyTo())
        {
            case AnnotMarkup::replyTypeR:
                return Annotation::Reply;
            case AnnotMarkup::replyTypeGroup:
                return Annotation::Group;
        }
    }

    return Annotation::Root; // It's not a revision
}

Annotation::RevType Annotation::revisionType() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
        return d->revisionType;

    const AnnotText *textann = dynamic_cast<const AnnotText*>(d->pdfAnnot);

    if (textann && textann->getInReplyToID() != 0)
    {
        switch (textann->getState())
        {
            case AnnotText::stateMarked:
                return Annotation::Marked;
            case AnnotText::stateUnmarked:
                return Annotation::Unmarked;
            case AnnotText::stateAccepted:
                return Annotation::Accepted;
            case AnnotText::stateRejected:
                return Annotation::Rejected;
            case AnnotText::stateCancelled:
                return Annotation::Cancelled;
            case AnnotText::stateCompleted:
                return Annotation::Completed;
            default:
                break;
        }
    }

    return Annotation::None;
}

QList<Annotation*> Annotation::revisions() const
{
    Q_D( const Annotation );

    if (!d->pdfAnnot)
    {
        /* Return aliases, whose ownership goes to the caller */
        QList<Annotation*> res;
        foreach (Annotation *rev, d->revisions)
            res.append( rev->d_ptr->makeAlias() );
        return res;
    }

    /* If the annotation doesn't live in a object on its own (eg bug51361), it
     * has no ref, therefore it can't have revisions */
    if ( !d->pdfAnnot->getHasRef() )
        return QList<Annotation*>();

    return AnnotationPrivate::findAnnotations( d->pdfPage, d->parentDoc, d->pdfAnnot->getId() );
}

//END Annotation implementation


/** TextAnnotation [Annotation] */
class TextAnnotationPrivate : public AnnotationPrivate
{
    public:
        TextAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        TextAnnotation::TextType textType;
        QString textIcon;
        QFont textFont;
        int inplaceAlign; // 0:left, 1:center, 2:right
        QVector<QPointF> inplaceCallout;
        TextAnnotation::InplaceIntent inplaceIntent;

        // Helper
        static GooString * toAppearanceString(const QFont &font);
};

TextAnnotationPrivate::TextAnnotationPrivate()
    : AnnotationPrivate(), textType( TextAnnotation::Linked ),
    textIcon( "Note" ), inplaceAlign( 0 ),
    inplaceIntent( TextAnnotation::Unknown )
{
}

Annotation * TextAnnotationPrivate::makeAlias()
{
    return new TextAnnotation(*this);
}

GooString * TextAnnotationPrivate::toAppearanceString(const QFont &font)
{
    GooString * s = GooString::format("/Invalid_font {0:d} Tf", font.pointSize());
    // TODO: Font family, style (bold, italic, ...) and pointSize as float
    return s;
}

Annot* TextAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    // Setters are defined in the public class
    TextAnnotation *q = static_cast<TextAnnotation*>( makeAlias() );

    // Set page and contents
    pdfPage = destPage;
    parentDoc = doc;

    // Set pdfAnnot
    PDFRectangle rect = boundaryToPdfRectangle(boundary, flags);
    if (textType == TextAnnotation::Linked)
    {
        pdfAnnot = new AnnotText(destPage->getDoc(), &rect);
    }
    else
    {
        GooString * da = toAppearanceString(textFont);
        pdfAnnot = new AnnotFreeText(destPage->getDoc(), &rect, da);
        delete da;
    }

    // Set properties
    flushBaseAnnotationProperties();
    q->setTextIcon(textIcon);
    q->setInplaceAlign(inplaceAlign);
    q->setCalloutPoints(inplaceCallout);
    q->setInplaceIntent(inplaceIntent);

    delete q;

    inplaceCallout.clear(); // Free up memory

    return pdfAnnot;
}

TextAnnotation::TextAnnotation( TextAnnotation::TextType type )
    : Annotation( *new TextAnnotationPrivate() )
{
    setTextType( type );
}

TextAnnotation::TextAnnotation(TextAnnotationPrivate &dd)
    : Annotation( dd )
{}

TextAnnotation::TextAnnotation( const QDomNode & node )
    : Annotation( *new TextAnnotationPrivate, node )
{
    // loop through the whole children looking for a 'text' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "text" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "type" ) )
            setTextType((TextAnnotation::TextType)e.attribute( "type" ).toInt());
        if ( e.hasAttribute( "icon" ) )
            setTextIcon(e.attribute( "icon" ));
        if ( e.hasAttribute( "font" ) )
        {
            QFont font;
            font.fromString( e.attribute( "font" ) );
            setTextFont(font);
        }
        if ( e.hasAttribute( "align" ) )
            setInplaceAlign(e.attribute( "align" ).toInt());
        if ( e.hasAttribute( "intent" ) )
            setInplaceIntent((TextAnnotation::InplaceIntent)e.attribute( "intent" ).toInt());

        // parse the subnodes
        QDomNode eSubNode = e.firstChild();
        while ( eSubNode.isElement() )
        {
            QDomElement ee = eSubNode.toElement();
            eSubNode = eSubNode.nextSibling();

            if ( ee.tagName() == "escapedText" )
            {
                setContents(ee.firstChild().toCDATASection().data());
            }
            else if ( ee.tagName() == "callout" )
            {
                QVector<QPointF> points(3);
                points[0] = QPointF(ee.attribute( "ax" ).toDouble(),
                                    ee.attribute( "ay" ).toDouble());
                points[1] = QPointF(ee.attribute( "bx" ).toDouble(),
                                    ee.attribute( "by" ).toDouble());
                points[2] = QPointF(ee.attribute( "cx" ).toDouble(),
                                    ee.attribute( "cy" ).toDouble());
                setCalloutPoints(points);
            }
        }

        // loading complete
        break;
    }
}

TextAnnotation::~TextAnnotation()
{
}

void TextAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [text] element
    QDomElement textElement = document.createElement( "text" );
    node.appendChild( textElement );

    // store the optional attributes
    if ( textType() != Linked )
        textElement.setAttribute( "type", (int)textType() );
    if ( textIcon() != "Note" )
        textElement.setAttribute( "icon", textIcon() );
    if ( inplaceAlign() )
        textElement.setAttribute( "align", inplaceAlign() );
    if ( inplaceIntent() != Unknown )
        textElement.setAttribute( "intent", (int)inplaceIntent() );

    textElement.setAttribute( "font", textFont().toString() );

    // Sub-Node-1 - escapedText
    if ( !contents().isEmpty() )
    {
        QDomElement escapedText = document.createElement( "escapedText" );
        textElement.appendChild( escapedText );
        QDomCDATASection textCData = document.createCDATASection( contents() );
        escapedText.appendChild( textCData );
    }

    // Sub-Node-2 - callout
    if ( calloutPoint(0).x() != 0.0 )
    {
        QDomElement calloutElement = document.createElement( "callout" );
        textElement.appendChild( calloutElement );
        calloutElement.setAttribute( "ax", QString::number( calloutPoint(0).x() ) );
        calloutElement.setAttribute( "ay", QString::number( calloutPoint(0).y() ) );
        calloutElement.setAttribute( "bx", QString::number( calloutPoint(1).x() ) );
        calloutElement.setAttribute( "by", QString::number( calloutPoint(1).y() ) );
        calloutElement.setAttribute( "cx", QString::number( calloutPoint(2).x() ) );
        calloutElement.setAttribute( "cy", QString::number( calloutPoint(2).y() ) );
    }
}

Annotation::SubType TextAnnotation::subType() const
{
    return AText;
}

TextAnnotation::TextType TextAnnotation::textType() const
{
    Q_D( const TextAnnotation );

    if (!d->pdfAnnot)
        return d->textType;

    return d->pdfAnnot->getType() == Annot::typeText ?
        TextAnnotation::Linked : TextAnnotation::InPlace;
}

void TextAnnotation::setTextType( TextAnnotation::TextType type )
{
    Q_D( TextAnnotation );

    if (!d->pdfAnnot)
    {
        d->textType = type;
        return;
    }

    // Type cannot be changed if annotation is already tied
}

QString TextAnnotation::textIcon() const
{
    Q_D( const TextAnnotation );

    if (!d->pdfAnnot)
        return d->textIcon;

    if (d->pdfAnnot->getType() == Annot::typeText)
    {
        const AnnotText * textann = static_cast<const AnnotText*>(d->pdfAnnot);
        return QString::fromLatin1( textann->getIcon()->getCString() );
    }

    return QString();
}

void TextAnnotation::setTextIcon( const QString &icon )
{
    Q_D( TextAnnotation );

    if (!d->pdfAnnot)
    {
        d->textIcon = icon;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeText)
    {
        AnnotText * textann = static_cast<AnnotText*>(d->pdfAnnot);
        QByteArray encoded = icon.toLatin1();
        GooString s(encoded.constData());
        textann->setIcon(&s);
    }
}

QFont TextAnnotation::textFont() const
{
    Q_D( const TextAnnotation );

    if (!d->pdfAnnot)
        return d->textFont;

    QFont font;

    if (d->pdfAnnot->getType() == Annot::typeFreeText)
    {
        const AnnotFreeText * ftextann = static_cast<const AnnotFreeText*>(d->pdfAnnot);
        const GooString * da = ftextann->getAppearanceString();
        if (da)
        {
            // At the moment, only font size is parsed
            QString style = QString::fromLatin1( da->getCString() );
            QRegExp rx("(\\d+)(\\.\\d*)? Tf");
            if (rx.indexIn(style) != -1)
                font.setPointSize( rx.cap(1).toInt() );
            // TODO: Other properties
        }
    }

    return font;
}

void TextAnnotation::setTextFont( const QFont &font )
{
    Q_D( TextAnnotation );

    if (!d->pdfAnnot)
    {
        d->textFont = font;
        return;
    }

    if (d->pdfAnnot->getType() != Annot::typeFreeText)
        return;

    AnnotFreeText * ftextann = static_cast<AnnotFreeText*>(d->pdfAnnot);
    GooString * da = TextAnnotationPrivate::toAppearanceString(font);
    ftextann->setAppearanceString(da);
    delete da;
}

int TextAnnotation::inplaceAlign() const
{
    Q_D( const TextAnnotation );

    if (!d->pdfAnnot)
        return d->inplaceAlign;

    if (d->pdfAnnot->getType() == Annot::typeFreeText)
    {
        const AnnotFreeText * ftextann = static_cast<const AnnotFreeText*>(d->pdfAnnot);
        return ftextann->getQuadding();
    }

    return 0;
}

void TextAnnotation::setInplaceAlign( int align )
{
    Q_D( TextAnnotation );

    if (!d->pdfAnnot)
    {
        d->inplaceAlign = align;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeFreeText)
    {
        AnnotFreeText * ftextann = static_cast<AnnotFreeText*>(d->pdfAnnot);
        ftextann->setQuadding((AnnotFreeText::AnnotFreeTextQuadding)align);
    }
}

QPointF TextAnnotation::calloutPoint( int id ) const
{
    const QVector<QPointF> points = calloutPoints();
    if ( id < 0 || id >= points.size() )
        return QPointF();
    else
        return points[id];
}

QVector<QPointF> TextAnnotation::calloutPoints() const
{
    Q_D( const TextAnnotation );

    if (!d->pdfAnnot)
        return d->inplaceCallout;

    if (d->pdfAnnot->getType() == Annot::typeText)
        return QVector<QPointF>();

    const AnnotFreeText * ftextann = static_cast<const AnnotFreeText*>(d->pdfAnnot);
    const AnnotCalloutLine *callout = ftextann->getCalloutLine();

    if (!callout)
        return QVector<QPointF>();

    double MTX[6];
    d->fillTransformationMTX(MTX);

    const AnnotCalloutMultiLine * callout_v6 = dynamic_cast<const AnnotCalloutMultiLine*>(callout);
    QVector<QPointF> res(callout_v6 ? 3 : 2);
    XPDFReader::transform(MTX, callout->getX1(), callout->getY1(), res[0]);
    XPDFReader::transform(MTX, callout->getX2(), callout->getY2(), res[1]);
    if (callout_v6)
        XPDFReader::transform(MTX, callout_v6->getX3(), callout_v6->getY3(), res[2]);
    return res;
}

void TextAnnotation::setCalloutPoints( const QVector<QPointF> &points )
{
    Q_D( TextAnnotation );
    if (!d->pdfAnnot)
    {
        d->inplaceCallout = points;
        return;
    }

    if (d->pdfAnnot->getType() != Annot::typeFreeText)
        return;

    AnnotFreeText * ftextann = static_cast<AnnotFreeText*>(d->pdfAnnot);
    const int count = points.size();

    if (count == 0)
    {
        ftextann->setCalloutLine(0);
        return;
    }

    if (count != 2 && count != 3)
    {
        error(errSyntaxError, -1, "Expected zero, two or three points for callout");
        return;
    }

    AnnotCalloutLine *callout;
    double x1, y1, x2, y2;
    double MTX[6];
    d->fillTransformationMTX(MTX);

    XPDFReader::invTransform( MTX, points[0], x1, y1 );
    XPDFReader::invTransform( MTX, points[1], x2, y2 );
    if (count == 3)
    {
        double x3, y3;
        XPDFReader::invTransform( MTX, points[2], x3, y3 );
        callout = new AnnotCalloutMultiLine(x1, y1, x2, y2, x3, y3);
    }
    else
    {
        callout = new AnnotCalloutLine(x1, y1, x2, y2);
    }

    ftextann->setCalloutLine(callout);
    delete callout;
}

TextAnnotation::InplaceIntent TextAnnotation::inplaceIntent() const
{
    Q_D( const TextAnnotation );

    if (!d->pdfAnnot)
        return d->inplaceIntent;

    if (d->pdfAnnot->getType() == Annot::typeFreeText)
    {
        const AnnotFreeText * ftextann = static_cast<const AnnotFreeText*>(d->pdfAnnot);
        return (TextAnnotation::InplaceIntent)ftextann->getIntent();
    }

    return TextAnnotation::Unknown;
}

void TextAnnotation::setInplaceIntent( TextAnnotation::InplaceIntent intent )
{
    Q_D( TextAnnotation );

    if (d->pdfAnnot)
    {
        d->inplaceIntent = intent;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeFreeText)
    {
        AnnotFreeText * ftextann = static_cast<AnnotFreeText*>(d->pdfAnnot);
        ftextann->setIntent((AnnotFreeText::AnnotFreeTextIntent)intent);
    }
}


/** LineAnnotation [Annotation] */
class LineAnnotationPrivate : public AnnotationPrivate
{
    public:
        LineAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields (note uses border for rendering style)
        QLinkedList<QPointF> linePoints;
        LineAnnotation::TermStyle lineStartStyle;
        LineAnnotation::TermStyle lineEndStyle;
        bool lineClosed : 1;  // (if true draw close shape)
        bool lineShowCaption : 1;
        LineAnnotation::LineType lineType;
        QColor lineInnerColor;
        double lineLeadingFwdPt;
        double lineLeadingBackPt;
        LineAnnotation::LineIntent lineIntent;
};

LineAnnotationPrivate::LineAnnotationPrivate()
    : AnnotationPrivate(), lineStartStyle( LineAnnotation::None ),
    lineEndStyle( LineAnnotation::None ), lineClosed( false ),
    lineShowCaption( false ), lineLeadingFwdPt( 0 ),
    lineLeadingBackPt( 0 ), lineIntent( LineAnnotation::Unknown )
{
}

Annotation * LineAnnotationPrivate::makeAlias()
{
    return new LineAnnotation(*this);
}

Annot* LineAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    // Setters are defined in the public class
    LineAnnotation *q = static_cast<LineAnnotation*>( makeAlias() );

    // Set page and document
    pdfPage = destPage;
    parentDoc = doc;

    // Set pdfAnnot
    PDFRectangle rect = boundaryToPdfRectangle(boundary, flags);
    if (lineType == LineAnnotation::StraightLine)
    {
        pdfAnnot = new AnnotLine(doc->doc, &rect);
    }
    else
    {
        pdfAnnot = new AnnotPolygon(doc->doc, &rect,
                lineClosed ? Annot::typePolygon : Annot::typePolyLine );
    }

    // Set properties
    flushBaseAnnotationProperties();
    q->setLinePoints(linePoints);
    q->setLineStartStyle(lineStartStyle);
    q->setLineEndStyle(lineEndStyle);
    q->setLineInnerColor(lineInnerColor);
    q->setLineLeadingForwardPoint(lineLeadingFwdPt);
    q->setLineLeadingBackPoint(lineLeadingBackPt);
    q->setLineShowCaption(lineShowCaption);
    q->setLineIntent(lineIntent);

    delete q;

    linePoints.clear(); // Free up memory

    return pdfAnnot;
}

LineAnnotation::LineAnnotation( LineAnnotation::LineType type )
    : Annotation( *new LineAnnotationPrivate() )
{
    setLineType(type);
}

LineAnnotation::LineAnnotation(LineAnnotationPrivate &dd)
    : Annotation( dd )
{}

LineAnnotation::LineAnnotation( const QDomNode & node )
    : Annotation( *new LineAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'line' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "line" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "startStyle" ) )
            setLineStartStyle((LineAnnotation::TermStyle)e.attribute( "startStyle" ).toInt());
        if ( e.hasAttribute( "endStyle" ) )
            setLineEndStyle((LineAnnotation::TermStyle)e.attribute( "endStyle" ).toInt());
        if ( e.hasAttribute( "closed" ) )
            setLineClosed(e.attribute( "closed" ).toInt());
        if ( e.hasAttribute( "innerColor" ) )
            setLineInnerColor(QColor( e.attribute( "innerColor" ) ));
        if ( e.hasAttribute( "leadFwd" ) )
            setLineLeadingForwardPoint(e.attribute( "leadFwd" ).toDouble());
        if ( e.hasAttribute( "leadBack" ) )
            setLineLeadingBackPoint(e.attribute( "leadBack" ).toDouble());
        if ( e.hasAttribute( "showCaption" ) )
            setLineShowCaption(e.attribute( "showCaption" ).toInt());
        if ( e.hasAttribute( "intent" ) )
            setLineIntent((LineAnnotation::LineIntent)e.attribute( "intent" ).toInt());

        // parse all 'point' subnodes
        QLinkedList<QPointF> points;
        QDomNode pointNode = e.firstChild();
        while ( pointNode.isElement() )
        {
            QDomElement pe = pointNode.toElement();
            pointNode = pointNode.nextSibling();

            if ( pe.tagName() != "point" )
                continue;

            QPointF p(pe.attribute( "x", "0.0" ).toDouble(), pe.attribute( "y", "0.0" ).toDouble());
            points.append( p );
        }
        setLinePoints(points);
        setLineType(points.size() == 2 ? StraightLine : Polyline);

        // loading complete
        break;
    }
}

LineAnnotation::~LineAnnotation()
{
}

void LineAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [line] element
    QDomElement lineElement = document.createElement( "line" );
    node.appendChild( lineElement );

    // store the attributes
    if ( lineStartStyle() != None )
        lineElement.setAttribute( "startStyle", (int)lineStartStyle() );
    if ( lineEndStyle() != None )
        lineElement.setAttribute( "endStyle", (int)lineEndStyle() );
    if ( isLineClosed() )
        lineElement.setAttribute( "closed", isLineClosed() );
    if ( lineInnerColor().isValid() )
        lineElement.setAttribute( "innerColor", lineInnerColor().name() );
    if ( lineLeadingForwardPoint() != 0.0 )
        lineElement.setAttribute( "leadFwd", QString::number( lineLeadingForwardPoint() ) );
    if ( lineLeadingBackPoint() != 0.0 )
        lineElement.setAttribute( "leadBack", QString::number( lineLeadingBackPoint() ) );
    if ( lineShowCaption() )
        lineElement.setAttribute( "showCaption", lineShowCaption() );
    if ( lineIntent() != Unknown )
        lineElement.setAttribute( "intent", lineIntent() );

    // append the list of points
    const QLinkedList<QPointF> points = linePoints();
    if ( points.count() > 1 )
    {
        QLinkedList<QPointF>::const_iterator it = points.begin(), end = points.end();
        while ( it != end )
        {
            const QPointF & p = *it;
            QDomElement pElement = document.createElement( "point" );
            lineElement.appendChild( pElement );
            pElement.setAttribute( "x", QString::number( p.x() ) );
            pElement.setAttribute( "y", QString::number( p.y() ) );
            ++it;
        }
    }
}

Annotation::SubType LineAnnotation::subType() const
{
    return ALine;
}

LineAnnotation::LineType LineAnnotation::lineType() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineType;

    return (d->pdfAnnot->getType() == Annot::typeLine) ?
        LineAnnotation::StraightLine : LineAnnotation::Polyline;
}

void LineAnnotation::setLineType( LineAnnotation::LineType type )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineType = type;
        return;
    }

    // Type cannot be changed if annotation is already tied
}

QLinkedList<QPointF> LineAnnotation::linePoints() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->linePoints;

    double MTX[6];
    d->fillTransformationMTX(MTX);

    QLinkedList<QPointF> res;
    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        QPointF p;
        XPDFReader::transform(MTX, lineann->getX1(), lineann->getY1(), p);
        res.append(p);
        XPDFReader::transform(MTX, lineann->getX2(), lineann->getY2(), p);
        res.append(p);
    }
    else
    {
        const AnnotPolygon * polyann = static_cast<const AnnotPolygon*>(d->pdfAnnot);
        const AnnotPath * vertices = polyann->getVertices();

        for (int i = 0; i < vertices->getCoordsLength(); ++i)
        {
            QPointF p;
            XPDFReader::transform(MTX, vertices->getX(i), vertices->getY(i), p);
            res.append(p);
        }
    }

    return res;
}

void LineAnnotation::setLinePoints( const QLinkedList<QPointF> &points )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->linePoints = points;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine *lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        if (points.size() != 2)
        {
            error(errSyntaxError, -1, "Expected two points for a straight line");
            return;
        }
        double x1, y1, x2, y2;
        double MTX[6];
        d->fillTransformationMTX(MTX);
        XPDFReader::invTransform( MTX, points.first(), x1, y1 );
        XPDFReader::invTransform( MTX, points.last(), x2, y2 );
        lineann->setVertices(x1, y1, x2, y2);
    }
    else
    {
        AnnotPolygon *polyann = static_cast<AnnotPolygon*>(d->pdfAnnot);
        AnnotPath * p = d->toAnnotPath(points);
        polyann->setVertices(p);
        delete p;
    }
}

LineAnnotation::TermStyle LineAnnotation::lineStartStyle() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineStartStyle;

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        return (LineAnnotation::TermStyle)lineann->getStartStyle();
    }
    else
    {
        const AnnotPolygon * polyann = static_cast<const AnnotPolygon*>(d->pdfAnnot);
        return (LineAnnotation::TermStyle)polyann->getStartStyle();
    }
}

void LineAnnotation::setLineStartStyle( LineAnnotation::TermStyle style )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineStartStyle = style;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine *lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        lineann->setStartEndStyle((AnnotLineEndingStyle)style, lineann->getEndStyle());
    }
    else
    {
        AnnotPolygon *polyann = static_cast<AnnotPolygon*>(d->pdfAnnot);
        polyann->setStartEndStyle((AnnotLineEndingStyle)style, polyann->getEndStyle());
    }
}

LineAnnotation::TermStyle LineAnnotation::lineEndStyle() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineEndStyle;

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        return (LineAnnotation::TermStyle)lineann->getEndStyle();
    }
    else
    {
        const AnnotPolygon * polyann = static_cast<const AnnotPolygon*>(d->pdfAnnot);
        return (LineAnnotation::TermStyle)polyann->getEndStyle();
    }
}

void LineAnnotation::setLineEndStyle( LineAnnotation::TermStyle style )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineEndStyle = style;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine *lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        lineann->setStartEndStyle(lineann->getStartStyle(), (AnnotLineEndingStyle)style);
    }
    else
    {
        AnnotPolygon *polyann = static_cast<AnnotPolygon*>(d->pdfAnnot);
        polyann->setStartEndStyle(polyann->getStartStyle(), (AnnotLineEndingStyle)style);
    }
}

bool LineAnnotation::isLineClosed() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineClosed;

    return d->pdfAnnot->getType() == Annot::typePolygon;
}

void LineAnnotation::setLineClosed( bool closed )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineClosed = closed;
        return;
    }

    if (d->pdfAnnot->getType() != Annot::typeLine)
    {
        AnnotPolygon *polyann = static_cast<AnnotPolygon*>(d->pdfAnnot);

        // Set new subtype and switch intent if necessary
        if (closed)
        {
            polyann->setType(Annot::typePolygon);
            if (polyann->getIntent() == AnnotPolygon::polylineDimension)
                polyann->setIntent( AnnotPolygon::polygonDimension );
        }
        else
        {
            polyann->setType(Annot::typePolyLine);
            if (polyann->getIntent() == AnnotPolygon::polygonDimension)
                polyann->setIntent( AnnotPolygon::polylineDimension );
        }
    }
}

QColor LineAnnotation::lineInnerColor() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineInnerColor;

    AnnotColor * c;

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        c = lineann->getInteriorColor();
    }
    else
    {
        const AnnotPolygon * polyann = static_cast<const AnnotPolygon*>(d->pdfAnnot);
        c = polyann->getInteriorColor();
    }

    return convertAnnotColor(c);
}

void LineAnnotation::setLineInnerColor( const QColor &color )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineInnerColor = color;
        return;
    }

    AnnotColor * c = convertQColor(color);

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine *lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        lineann->setInteriorColor(c);
    }
    else
    {
        AnnotPolygon *polyann = static_cast<AnnotPolygon*>(d->pdfAnnot);
        polyann->setInteriorColor(c);
    }
}

double LineAnnotation::lineLeadingForwardPoint() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineLeadingFwdPt;

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        return lineann->getLeaderLineLength();
    }

    return 0;
}

void LineAnnotation::setLineLeadingForwardPoint( double point )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineLeadingFwdPt = point;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine *lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        lineann->setLeaderLineLength(point);
    }
}

double LineAnnotation::lineLeadingBackPoint() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineLeadingBackPt;

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        return lineann->getLeaderLineExtension();
    }

    return 0;
}

void LineAnnotation::setLineLeadingBackPoint( double point )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineLeadingBackPt = point;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine *lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        lineann->setLeaderLineExtension(point);
    }
}

bool LineAnnotation::lineShowCaption() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineShowCaption;

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        return lineann->getCaption();
    }

    return false;
}

void LineAnnotation::setLineShowCaption( bool show )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineShowCaption = show;
        return;
    }

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine *lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        lineann->setCaption(show);
    }
}

LineAnnotation::LineIntent LineAnnotation::lineIntent() const
{
    Q_D( const LineAnnotation );

    if (!d->pdfAnnot)
        return d->lineIntent;

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        const AnnotLine * lineann = static_cast<const AnnotLine*>(d->pdfAnnot);
        return (LineAnnotation::LineIntent)( lineann->getIntent() + 1 );
    }
    else
    {
        const AnnotPolygon * polyann = static_cast<const AnnotPolygon*>(d->pdfAnnot);
        if ( polyann->getIntent() == AnnotPolygon::polygonCloud )
            return LineAnnotation::PolygonCloud;
        else // AnnotPolygon::polylineDimension, AnnotPolygon::polygonDimension
            return LineAnnotation::Dimension;
    }
}

void LineAnnotation::setLineIntent( LineAnnotation::LineIntent intent )
{
    Q_D( LineAnnotation );

    if (!d->pdfAnnot)
    {
        d->lineIntent = intent;
        return;
    }

    if ( intent == LineAnnotation::Unknown )
        return; // Do not set (actually, it should clear the property)

    if (d->pdfAnnot->getType() == Annot::typeLine)
    {
        AnnotLine * lineann = static_cast<AnnotLine*>(d->pdfAnnot);
        lineann->setIntent((AnnotLine::AnnotLineIntent)( intent - 1 ));
    }
    else
    {
        AnnotPolygon * polyann = static_cast<AnnotPolygon*>(d->pdfAnnot);
        if ( intent == LineAnnotation::PolygonCloud)
            polyann->setIntent( AnnotPolygon::polygonCloud );
        else // LineAnnotation::Dimension
        {
            if ( d->pdfAnnot->getType() == Annot::typePolygon )
                polyann->setIntent( AnnotPolygon::polygonDimension );
            else // Annot::typePolyLine
                polyann->setIntent( AnnotPolygon::polylineDimension );
        }
    }
}


/** GeomAnnotation [Annotation] */
class GeomAnnotationPrivate : public AnnotationPrivate
{
    public:
        GeomAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields (note uses border for rendering style)
        GeomAnnotation::GeomType geomType;
        QColor geomInnerColor;
};

GeomAnnotationPrivate::GeomAnnotationPrivate()
    : AnnotationPrivate(), geomType( GeomAnnotation::InscribedSquare )
{
}

Annotation * GeomAnnotationPrivate::makeAlias()
{
    return new GeomAnnotation(*this);
}

Annot* GeomAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    // Setters are defined in the public class
    GeomAnnotation *q = static_cast<GeomAnnotation*>( makeAlias() );

    // Set page and document
    pdfPage = destPage;
    parentDoc = doc;

    Annot::AnnotSubtype type;
    if (geomType == GeomAnnotation::InscribedSquare)
        type = Annot::typeSquare;
    else // GeomAnnotation::InscribedCircle
        type = Annot::typeCircle;

    // Set pdfAnnot
    PDFRectangle rect = boundaryToPdfRectangle(boundary, flags);
    pdfAnnot = new AnnotGeometry(destPage->getDoc(), &rect, type);

    // Set properties
    flushBaseAnnotationProperties();
    q->setGeomInnerColor(geomInnerColor);

    delete q;
    return pdfAnnot;
}

GeomAnnotation::GeomAnnotation()
    : Annotation( *new GeomAnnotationPrivate() )
{}

GeomAnnotation::GeomAnnotation(GeomAnnotationPrivate &dd)
    : Annotation( dd )
{}

GeomAnnotation::GeomAnnotation( const QDomNode & node )
    : Annotation( *new GeomAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'geom' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "geom" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "type" ) )
            setGeomType((GeomAnnotation::GeomType)e.attribute( "type" ).toInt());
        if ( e.hasAttribute( "color" ) )
            setGeomInnerColor(QColor( e.attribute( "color" ) ));

        // loading complete
        break;
    }
}

GeomAnnotation::~GeomAnnotation()
{
}

void GeomAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [geom] element
    QDomElement geomElement = document.createElement( "geom" );
    node.appendChild( geomElement );

    // append the optional attributes
    if ( geomType() != InscribedSquare )
        geomElement.setAttribute( "type", (int)geomType() );
    if ( geomInnerColor().isValid() )
        geomElement.setAttribute( "color", geomInnerColor().name() );
}

Annotation::SubType GeomAnnotation::subType() const
{
    return AGeom;
}

GeomAnnotation::GeomType GeomAnnotation::geomType() const
{
    Q_D( const GeomAnnotation );

    if (!d->pdfAnnot)
        return d->geomType;

    if (d->pdfAnnot->getType() == Annot::typeSquare)
        return GeomAnnotation::InscribedSquare;
    else // Annot::typeCircle
        return GeomAnnotation::InscribedCircle;
}

void GeomAnnotation::setGeomType( GeomAnnotation::GeomType type )
{
    Q_D( GeomAnnotation );

    if (!d->pdfAnnot)
    {
        d->geomType = type;
        return;
    }

    AnnotGeometry * geomann = static_cast<AnnotGeometry*>(d->pdfAnnot);
    if (type == GeomAnnotation::InscribedSquare)
        geomann->setType(Annot::typeSquare);
    else // GeomAnnotation::InscribedCircle
        geomann->setType(Annot::typeCircle);
}

QColor GeomAnnotation::geomInnerColor() const
{
    Q_D( const GeomAnnotation );

    if (!d->pdfAnnot)
        return d->geomInnerColor;

    const AnnotGeometry * geomann = static_cast<const AnnotGeometry*>(d->pdfAnnot);
    return convertAnnotColor( geomann->getInteriorColor() );
}

void GeomAnnotation::setGeomInnerColor( const QColor &color )
{
    Q_D( GeomAnnotation );

    if (!d->pdfAnnot)
    {
        d->geomInnerColor = color;
        return;
    }

    AnnotGeometry * geomann = static_cast<AnnotGeometry*>(d->pdfAnnot);
    geomann->setInteriorColor(convertQColor( color ));
}


/** HighlightAnnotation [Annotation] */
class HighlightAnnotationPrivate : public AnnotationPrivate
{
    public:
        HighlightAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        HighlightAnnotation::HighlightType highlightType;
        QList< HighlightAnnotation::Quad > highlightQuads; // not empty

        // helpers
        static Annot::AnnotSubtype toAnnotSubType( HighlightAnnotation::HighlightType type );
        QList< HighlightAnnotation::Quad > fromQuadrilaterals(AnnotQuadrilaterals *quads) const;
        AnnotQuadrilaterals * toQuadrilaterals(const QList< HighlightAnnotation::Quad > &quads) const;
};

HighlightAnnotationPrivate::HighlightAnnotationPrivate()
    : AnnotationPrivate(), highlightType( HighlightAnnotation::Highlight )
{
}

Annotation * HighlightAnnotationPrivate::makeAlias()
{
    return new HighlightAnnotation(*this);
}

Annot::AnnotSubtype HighlightAnnotationPrivate::toAnnotSubType( HighlightAnnotation::HighlightType type )
{
    switch (type)
    {
        default: // HighlightAnnotation::Highlight:
            return Annot::typeHighlight;
        case HighlightAnnotation::Underline:
            return Annot::typeUnderline;
        case HighlightAnnotation::Squiggly:
            return Annot::typeSquiggly;
        case HighlightAnnotation::StrikeOut:
            return Annot::typeStrikeOut;
    }
}

QList< HighlightAnnotation::Quad > HighlightAnnotationPrivate::fromQuadrilaterals(AnnotQuadrilaterals *hlquads) const
{
    QList< HighlightAnnotation::Quad > quads;

    if ( !hlquads || !hlquads->getQuadrilateralsLength() )
        return quads;
    const int quadsCount = hlquads->getQuadrilateralsLength();

    double MTX[6];
    fillTransformationMTX(MTX);

    for (int q = 0; q < quadsCount; ++q)
    {
        HighlightAnnotation::Quad quad;
        XPDFReader::transform( MTX, hlquads->getX1( q ), hlquads->getY1( q ), quad.points[ 0 ] );
        XPDFReader::transform( MTX, hlquads->getX2( q ), hlquads->getY2( q ), quad.points[ 1 ] );
        XPDFReader::transform( MTX, hlquads->getX3( q ), hlquads->getY3( q ), quad.points[ 2 ] );
        XPDFReader::transform( MTX, hlquads->getX4( q ), hlquads->getY4( q ), quad.points[ 3 ] );
        // ### PDF1.6 specs says that point are in ccw order, but in fact
        // points 3 and 4 are swapped in every PDF around!
        QPointF tmpPoint = quad.points[ 2 ];
        quad.points[ 2 ] = quad.points[ 3 ];
        quad.points[ 3 ] = tmpPoint;
        // initialize other properties and append quad
        quad.capStart = true;       // unlinked quads are always capped
        quad.capEnd = true;         // unlinked quads are always capped
        quad.feather = 0.1;         // default feather
        quads.append( quad );
    }

    return quads;
}

AnnotQuadrilaterals * HighlightAnnotationPrivate::toQuadrilaterals(const QList< HighlightAnnotation::Quad > &quads) const
{
    const int count = quads.size();
    AnnotQuadrilaterals::AnnotQuadrilateral **ac =
        (AnnotQuadrilaterals::AnnotQuadrilateral**)
            gmallocn( count, sizeof(AnnotQuadrilaterals::AnnotQuadrilateral*) );

    double MTX[6];
    fillTransformationMTX(MTX);

    int pos = 0;
    foreach (const HighlightAnnotation::Quad &q, quads)
    {
        double x1, y1, x2, y2, x3, y3, x4, y4;
        XPDFReader::invTransform( MTX, q.points[0], x1, y1 );
        XPDFReader::invTransform( MTX, q.points[1], x2, y2 );
        // Swap points 3 and 4 (see HighlightAnnotationPrivate::fromQuadrilaterals)
        XPDFReader::invTransform( MTX, q.points[3], x3, y3 );
        XPDFReader::invTransform( MTX, q.points[2], x4, y4 );
        ac[pos++] = new AnnotQuadrilaterals::AnnotQuadrilateral(x1, y1, x2, y2, x3, y3, x4, y4);
    }

    return new AnnotQuadrilaterals(ac, count);
}

Annot* HighlightAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    // Setters are defined in the public class
    HighlightAnnotation *q = static_cast<HighlightAnnotation*>( makeAlias() );

    // Set page and document
    pdfPage = destPage;
    parentDoc = doc;

    // Set pdfAnnot
    PDFRectangle rect = boundaryToPdfRectangle(boundary, flags);
    pdfAnnot = new AnnotTextMarkup(destPage->getDoc(), &rect, toAnnotSubType(highlightType));

    // Set properties
    flushBaseAnnotationProperties();
    q->setHighlightQuads(highlightQuads);

    highlightQuads.clear(); // Free up memory

    delete q;

    return pdfAnnot;
}

HighlightAnnotation::HighlightAnnotation()
    : Annotation( *new HighlightAnnotationPrivate() )
{}

HighlightAnnotation::HighlightAnnotation(HighlightAnnotationPrivate &dd)
    : Annotation( dd )
{}

HighlightAnnotation::HighlightAnnotation( const QDomNode & node )
    : Annotation( *new HighlightAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'hl' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "hl" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "type" ) )
            setHighlightType((HighlightAnnotation::HighlightType)e.attribute( "type" ).toInt());

        // parse all 'quad' subnodes
        QList<HighlightAnnotation::Quad> quads;
        QDomNode quadNode = e.firstChild();
        for ( ; quadNode.isElement(); quadNode = quadNode.nextSibling() )
        {
            QDomElement qe = quadNode.toElement();
            if ( qe.tagName() != "quad" )
                continue;

            Quad q;
            q.points[0].setX(qe.attribute( "ax", "0.0" ).toDouble());
            q.points[0].setY(qe.attribute( "ay", "0.0" ).toDouble());
            q.points[1].setX(qe.attribute( "bx", "0.0" ).toDouble());
            q.points[1].setY(qe.attribute( "by", "0.0" ).toDouble());
            q.points[2].setX(qe.attribute( "cx", "0.0" ).toDouble());
            q.points[2].setY(qe.attribute( "cy", "0.0" ).toDouble());
            q.points[3].setX(qe.attribute( "dx", "0.0" ).toDouble());
            q.points[3].setY(qe.attribute( "dy", "0.0" ).toDouble());
            q.capStart = qe.hasAttribute( "start" );
            q.capEnd = qe.hasAttribute( "end" );
            q.feather = qe.attribute( "feather", "0.1" ).toDouble();
            quads.append( q );
        }
        setHighlightQuads(quads);

        // loading complete
        break;
    }
}

HighlightAnnotation::~HighlightAnnotation()
{
}

void HighlightAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [hl] element
    QDomElement hlElement = document.createElement( "hl" );
    node.appendChild( hlElement );

    // append the optional attributes
    if ( highlightType() != Highlight )
        hlElement.setAttribute( "type", (int)highlightType() );

    const QList<HighlightAnnotation::Quad> quads = highlightQuads();
    if ( quads.count() < 1 )
        return;
    // append highlight quads, all children describe quads
    QList< HighlightAnnotation::Quad >::const_iterator it = quads.begin(), end = quads.end();
    for ( ; it != end; ++it )
    {
        QDomElement quadElement = document.createElement( "quad" );
        hlElement.appendChild( quadElement );
        const Quad & q = *it;
        quadElement.setAttribute( "ax", QString::number( q.points[0].x() ) );
        quadElement.setAttribute( "ay", QString::number( q.points[0].y() ) );
        quadElement.setAttribute( "bx", QString::number( q.points[1].x() ) );
        quadElement.setAttribute( "by", QString::number( q.points[1].y() ) );
        quadElement.setAttribute( "cx", QString::number( q.points[2].x() ) );
        quadElement.setAttribute( "cy", QString::number( q.points[2].y() ) );
        quadElement.setAttribute( "dx", QString::number( q.points[3].x() ) );
        quadElement.setAttribute( "dy", QString::number( q.points[3].y() ) );
        if ( q.capStart )
            quadElement.setAttribute( "start", 1 );
        if ( q.capEnd )
            quadElement.setAttribute( "end", 1 );
        quadElement.setAttribute( "feather", QString::number( q.feather ) );
    }
}

Annotation::SubType HighlightAnnotation::subType() const
{
    return AHighlight;
}

HighlightAnnotation::HighlightType HighlightAnnotation::highlightType() const
{
    Q_D( const HighlightAnnotation );

    if (!d->pdfAnnot)
        return d->highlightType;

    Annot::AnnotSubtype subType = d->pdfAnnot->getType();

    if ( subType == Annot::typeHighlight )
        return HighlightAnnotation::Highlight;
    else if ( subType == Annot::typeUnderline )
        return HighlightAnnotation::Underline;
    else if ( subType == Annot::typeSquiggly )
        return HighlightAnnotation::Squiggly;
    else // Annot::typeStrikeOut
        return HighlightAnnotation::StrikeOut;
}

void HighlightAnnotation::setHighlightType( HighlightAnnotation::HighlightType type )
{
    Q_D( HighlightAnnotation );

    if (!d->pdfAnnot)
    {
        d->highlightType = type;
        return;
    }

    AnnotTextMarkup * hlann = static_cast<AnnotTextMarkup*>(d->pdfAnnot);
    hlann->setType(HighlightAnnotationPrivate::toAnnotSubType( type ));
}

QList< HighlightAnnotation::Quad > HighlightAnnotation::highlightQuads() const
{
    Q_D( const HighlightAnnotation );

    if (!d->pdfAnnot)
        return d->highlightQuads;

    const AnnotTextMarkup * hlann = static_cast<AnnotTextMarkup*>(d->pdfAnnot);
    return d->fromQuadrilaterals( hlann->getQuadrilaterals() );
}

void HighlightAnnotation::setHighlightQuads( const QList< HighlightAnnotation::Quad > &quads )
{
    Q_D( HighlightAnnotation );

    if (!d->pdfAnnot)
    {
        d->highlightQuads = quads;
        return;
    }

    AnnotTextMarkup * hlann = static_cast<AnnotTextMarkup*>(d->pdfAnnot);
    AnnotQuadrilaterals * quadrilaterals = d->toQuadrilaterals(quads);
    hlann->setQuadrilaterals(quadrilaterals);
    delete quadrilaterals;
}


/** StampAnnotation [Annotation] */
class StampAnnotationPrivate : public AnnotationPrivate
{
    public:
        StampAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        QString stampIconName;
};

StampAnnotationPrivate::StampAnnotationPrivate()
    : AnnotationPrivate(), stampIconName( "Draft" )
{
}

Annotation * StampAnnotationPrivate::makeAlias()
{
    return new StampAnnotation(*this);
}

Annot* StampAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    StampAnnotation *q = static_cast<StampAnnotation*>( makeAlias() );

    // Set page and document
    pdfPage = destPage;
    parentDoc = doc;

    // Set pdfAnnot
    PDFRectangle rect = boundaryToPdfRectangle(boundary, flags);
    pdfAnnot = new AnnotStamp(destPage->getDoc(), &rect);

    // Set properties
    flushBaseAnnotationProperties();
    q->setStampIconName(stampIconName);

    delete q;

    stampIconName.clear(); // Free up memory

    return pdfAnnot;
}

StampAnnotation::StampAnnotation()
    : Annotation( *new StampAnnotationPrivate() )
{}

StampAnnotation::StampAnnotation(StampAnnotationPrivate &dd)
    : Annotation( dd )
{}

StampAnnotation::StampAnnotation( const QDomNode & node )
    : Annotation( *new StampAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'stamp' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "stamp" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "icon" ) )
            setStampIconName(e.attribute( "icon" ));

        // loading complete
        break;
    }
}

StampAnnotation::~StampAnnotation()
{
}

void StampAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [stamp] element
    QDomElement stampElement = document.createElement( "stamp" );
    node.appendChild( stampElement );

    // append the optional attributes
    if ( stampIconName() != "Draft" )
        stampElement.setAttribute( "icon", stampIconName() );
}

Annotation::SubType StampAnnotation::subType() const
{
    return AStamp;
}

QString StampAnnotation::stampIconName() const
{
    Q_D( const StampAnnotation );

    if (!d->pdfAnnot)
        return d->stampIconName;

    const AnnotStamp * stampann = static_cast<const AnnotStamp*>(d->pdfAnnot);
    return QString::fromLatin1( stampann->getIcon()->getCString() );
}

void StampAnnotation::setStampIconName( const QString &name )
{
    Q_D( StampAnnotation );

    if (!d->pdfAnnot)
    {
        d->stampIconName = name;
        return;
    }

    AnnotStamp * stampann = static_cast<AnnotStamp*>(d->pdfAnnot);
    QByteArray encoded = name.toLatin1();
    GooString s(encoded.constData());
    stampann->setIcon(&s);
}

/** InkAnnotation [Annotation] */
class InkAnnotationPrivate : public AnnotationPrivate
{
    public:
        InkAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        QList< QLinkedList<QPointF> > inkPaths;

        // helper
        AnnotPath **toAnnotPaths(const QList< QLinkedList<QPointF> > &inkPaths);
};

InkAnnotationPrivate::InkAnnotationPrivate()
    : AnnotationPrivate()
{
}

Annotation * InkAnnotationPrivate::makeAlias()
{
    return new InkAnnotation(*this);
}

// Note: Caller is required to delete array elements and the array itself after use
AnnotPath **InkAnnotationPrivate::toAnnotPaths(const QList< QLinkedList<QPointF> > &inkPaths)
{
    const int pathsNumber = inkPaths.size();
    AnnotPath **res = new AnnotPath*[pathsNumber];
    for (int i = 0; i < pathsNumber; ++i)
        res[i] = toAnnotPath( inkPaths[i] );
    return res;
}

Annot* InkAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    // Setters are defined in the public class
    InkAnnotation *q = static_cast<InkAnnotation*>( makeAlias() );

    // Set page and document
    pdfPage = destPage;
    parentDoc = doc;

    // Set pdfAnnot
    PDFRectangle rect = boundaryToPdfRectangle(boundary, flags);
    pdfAnnot = new AnnotInk(destPage->getDoc(), &rect);

    // Set properties
    flushBaseAnnotationProperties();
    q->setInkPaths(inkPaths);

    inkPaths.clear(); // Free up memory

    delete q;

    return pdfAnnot;
}

InkAnnotation::InkAnnotation()
    : Annotation( *new InkAnnotationPrivate() )
{}

InkAnnotation::InkAnnotation(InkAnnotationPrivate &dd)
    : Annotation( dd )
{}

InkAnnotation::InkAnnotation( const QDomNode & node )
    : Annotation( *new InkAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'ink' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "ink" )
            continue;

        // parse the 'path' subnodes
        QList< QLinkedList<QPointF> > paths;
        QDomNode pathNode = e.firstChild();
        while ( pathNode.isElement() )
        {
            QDomElement pathElement = pathNode.toElement();
            pathNode = pathNode.nextSibling();

            if ( pathElement.tagName() != "path" )
                continue;

            // build each path parsing 'point' subnodes
            QLinkedList<QPointF> path;
            QDomNode pointNode = pathElement.firstChild();
            while ( pointNode.isElement() )
            {
                QDomElement pointElement = pointNode.toElement();
                pointNode = pointNode.nextSibling();

                if ( pointElement.tagName() != "point" )
                    continue;

                QPointF p(pointElement.attribute( "x", "0.0" ).toDouble(), pointElement.attribute( "y", "0.0" ).toDouble());
                path.append( p );
            }

            // add the path to the path list if it contains at least 2 nodes
            if ( path.count() >= 2 )
                paths.append( path );
        }
        setInkPaths(paths);

        // loading complete
        break;
    }
}

InkAnnotation::~InkAnnotation()
{
}

void InkAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [ink] element
    QDomElement inkElement = document.createElement( "ink" );
    node.appendChild( inkElement );

    // append the optional attributes
    const QList< QLinkedList<QPointF> > paths = inkPaths();
    if ( paths.count() < 1 )
        return;
    QList< QLinkedList<QPointF> >::const_iterator pIt = paths.begin(), pEnd = paths.end();
    for ( ; pIt != pEnd; ++pIt )
    {
        QDomElement pathElement = document.createElement( "path" );
        inkElement.appendChild( pathElement );
        const QLinkedList<QPointF> & path = *pIt;
        QLinkedList<QPointF>::const_iterator iIt = path.begin(), iEnd = path.end();
        for ( ; iIt != iEnd; ++iIt )
        {
            const QPointF & point = *iIt;
            QDomElement pointElement = document.createElement( "point" );
            pathElement.appendChild( pointElement );
            pointElement.setAttribute( "x", QString::number( point.x() ) );
            pointElement.setAttribute( "y", QString::number( point.y() ) );
        }
    }
}

Annotation::SubType InkAnnotation::subType() const
{
    return AInk;
}

QList< QLinkedList<QPointF> > InkAnnotation::inkPaths() const
{
    Q_D( const InkAnnotation );

    if (!d->pdfAnnot)
        return d->inkPaths;

    const AnnotInk * inkann = static_cast<const AnnotInk *>(d->pdfAnnot);

    const AnnotPath * const* paths = inkann->getInkList();
    if ( !paths || !inkann->getInkListLength() )
        return QList< QLinkedList<QPointF> >();

    double MTX[6];
    d->fillTransformationMTX(MTX);

    const int pathsNumber = inkann->getInkListLength();
    QList< QLinkedList<QPointF> > inkPaths;
    for (int m = 0; m < pathsNumber; ++m)
    {
        // transform each path in a list of normalized points ..
        QLinkedList<QPointF> localList;
        const AnnotPath * path = paths[ m ];
        const int pointsNumber = path ? path->getCoordsLength() : 0;
        for (int n = 0; n < pointsNumber; ++n)
        {
            QPointF point;
            XPDFReader::transform(MTX, path->getX(n), path->getY(n), point);
            localList.append(point);
        }
        // ..and add it to the annotation
        inkPaths.append( localList );
    }
    return inkPaths;
}

void InkAnnotation::setInkPaths( const QList< QLinkedList<QPointF> > &paths )
{
    Q_D( InkAnnotation );

    if (!d->pdfAnnot)
    {
        d->inkPaths = paths;
        return;
    }

    AnnotInk * inkann = static_cast<AnnotInk*>(d->pdfAnnot);
    AnnotPath **annotpaths = d->toAnnotPaths(paths);
    const int pathsNumber = paths.size();
    inkann->setInkList(annotpaths, pathsNumber);

    for (int i = 0; i < pathsNumber; ++i)
        delete annotpaths[i];
    delete[] annotpaths;
}


/** LinkAnnotation [Annotation] */
class LinkAnnotationPrivate : public AnnotationPrivate
{
    public:
        LinkAnnotationPrivate();
        ~LinkAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        Link * linkDestination;
        LinkAnnotation::HighlightMode linkHLMode;
        QPointF linkRegion[4];
};

LinkAnnotationPrivate::LinkAnnotationPrivate()
    : AnnotationPrivate(), linkDestination( 0 ), linkHLMode( LinkAnnotation::Invert )
{
}

LinkAnnotationPrivate::~LinkAnnotationPrivate()
{
    delete linkDestination;
}

Annotation * LinkAnnotationPrivate::makeAlias()
{
    return new LinkAnnotation(*this);
}

Annot* LinkAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    return 0; // Not implemented
}

LinkAnnotation::LinkAnnotation()
    : Annotation( *new LinkAnnotationPrivate() )
{}

LinkAnnotation::LinkAnnotation(LinkAnnotationPrivate &dd)
    : Annotation( dd )
{}

LinkAnnotation::LinkAnnotation( const QDomNode & node )
    : Annotation( *new LinkAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'link' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "link" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "hlmode" ) )
            setLinkHighlightMode((LinkAnnotation::HighlightMode)e.attribute( "hlmode" ).toInt());

        // parse all 'quad' subnodes
        QDomNode quadNode = e.firstChild();
        for ( ; quadNode.isElement(); quadNode = quadNode.nextSibling() )
        {
            QDomElement qe = quadNode.toElement();
            if ( qe.tagName() == "quad" )
            {
                setLinkRegionPoint(0, QPointF(qe.attribute( "ax", "0.0" ).toDouble(),
                                              qe.attribute( "ay", "0.0" ).toDouble()));
                setLinkRegionPoint(1, QPointF(qe.attribute( "bx", "0.0" ).toDouble(),
                                              qe.attribute( "by", "0.0" ).toDouble()));
                setLinkRegionPoint(2, QPointF(qe.attribute( "cx", "0.0" ).toDouble(),
                                              qe.attribute( "cy", "0.0" ).toDouble()));
                setLinkRegionPoint(3, QPointF(qe.attribute( "dx", "0.0" ).toDouble(),
                                              qe.attribute( "dy", "0.0" ).toDouble()));
            }
            else if ( qe.tagName() == "link" )
            {
                QString type = qe.attribute( "type" );
                if ( type == "GoTo" )
                {
                    Poppler::LinkGoto * go = new Poppler::LinkGoto( QRect(), qe.attribute( "filename" ), LinkDestination( qe.attribute( "destination" ) ) );
                    setLinkDestination(go);
                }
                else if ( type == "Exec" )
                {
                    Poppler::LinkExecute * exec = new Poppler::LinkExecute( QRect(), qe.attribute( "filename" ), qe.attribute( "parameters" ) );
                    setLinkDestination(exec);
                }
                else if ( type == "Browse" )
                {
                    Poppler::LinkBrowse * browse = new Poppler::LinkBrowse( QRect(), qe.attribute( "url" ) );
                    setLinkDestination(browse);
                }
                else if ( type == "Action" )
                {
                    Poppler::LinkAction::ActionType act;
                    QString actString = qe.attribute( "action" );
                    bool found = true;
                    if ( actString == "PageFirst" )
                        act = Poppler::LinkAction::PageFirst;
                    else if ( actString == "PagePrev" )
                        act = Poppler::LinkAction::PagePrev;
                    else if ( actString == "PageNext" )
                        act = Poppler::LinkAction::PageNext;
                    else if ( actString == "PageLast" )
                        act = Poppler::LinkAction::PageLast;
                    else if ( actString == "HistoryBack" )
                        act = Poppler::LinkAction::HistoryBack;
                    else if ( actString == "HistoryForward" )
                        act = Poppler::LinkAction::HistoryForward;
                    else if ( actString == "Quit" )
                        act = Poppler::LinkAction::Quit;
                    else if ( actString == "Presentation" )
                        act = Poppler::LinkAction::Presentation;
                    else if ( actString == "EndPresentation" )
                        act = Poppler::LinkAction::EndPresentation;
                    else if ( actString == "Find" )
                        act = Poppler::LinkAction::Find;
                    else if ( actString == "GoToPage" )
                        act = Poppler::LinkAction::GoToPage;
                    else if ( actString == "Close" )
                        act = Poppler::LinkAction::Close;
                    else if ( actString == "Print" )
                        act = Poppler::LinkAction::Print;
                    else
                        found = false;
                    if (found)
                    {
                        Poppler::LinkAction * action = new Poppler::LinkAction( QRect(), act );
                        setLinkDestination(action);
                    }
                }
#if 0
                else if ( type == "Movie" )
                {
                    Poppler::LinkMovie * movie = new Poppler::LinkMovie( QRect() );
                    setLinkDestination(movie);
                }
#endif
            }
        }

        // loading complete
        break;
    }
}

LinkAnnotation::~LinkAnnotation()
{
}

void LinkAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [hl] element
    QDomElement linkElement = document.createElement( "link" );
    node.appendChild( linkElement );

    // append the optional attributes
    if ( linkHighlightMode() != Invert )
        linkElement.setAttribute( "hlmode", (int)linkHighlightMode() );

    // saving region
    QDomElement quadElement = document.createElement( "quad" );
    linkElement.appendChild( quadElement );
    quadElement.setAttribute( "ax", QString::number( linkRegionPoint(0).x() ) );
    quadElement.setAttribute( "ay", QString::number( linkRegionPoint(0).y() ) );
    quadElement.setAttribute( "bx", QString::number( linkRegionPoint(1).x() ) );
    quadElement.setAttribute( "by", QString::number( linkRegionPoint(1).y() ) );
    quadElement.setAttribute( "cx", QString::number( linkRegionPoint(2).x() ) );
    quadElement.setAttribute( "cy", QString::number( linkRegionPoint(2).y() ) );
    quadElement.setAttribute( "dx", QString::number( linkRegionPoint(3).x() ) );
    quadElement.setAttribute( "dy", QString::number( linkRegionPoint(3).y() ) );

    // saving link
    QDomElement hyperlinkElement = document.createElement( "link" );
    linkElement.appendChild( hyperlinkElement );
    if ( linkDestination() )
    {
        switch( linkDestination()->linkType() )
        {
            case Poppler::Link::Goto:
            {
                Poppler::LinkGoto * go = static_cast< Poppler::LinkGoto * >( linkDestination() );
                hyperlinkElement.setAttribute( "type", "GoTo" );
                hyperlinkElement.setAttribute( "filename", go->fileName() );
                hyperlinkElement.setAttribute( "destionation", go->destination().toString() ); // TODO remove for poppler 0.28
                hyperlinkElement.setAttribute( "destination", go->destination().toString() );
                break;
            }
            case Poppler::Link::Execute:
            {
                Poppler::LinkExecute * exec = static_cast< Poppler::LinkExecute * >( linkDestination() );
                hyperlinkElement.setAttribute( "type", "Exec" );
                hyperlinkElement.setAttribute( "filename", exec->fileName() );
                hyperlinkElement.setAttribute( "parameters", exec->parameters() );
                break;
            }
            case Poppler::Link::Browse:
            {
                Poppler::LinkBrowse * browse = static_cast< Poppler::LinkBrowse * >( linkDestination() );
                hyperlinkElement.setAttribute( "type", "Browse" );
                hyperlinkElement.setAttribute( "url", browse->url() );
                break;
            }
            case Poppler::Link::Action:
            {
                Poppler::LinkAction * action = static_cast< Poppler::LinkAction * >( linkDestination() );
                hyperlinkElement.setAttribute( "type", "Action" );
                switch ( action->actionType() )
                {
                    case Poppler::LinkAction::PageFirst:
                        hyperlinkElement.setAttribute( "action", "PageFirst" );
                        break;
                    case Poppler::LinkAction::PagePrev:
                        hyperlinkElement.setAttribute( "action", "PagePrev" );
                        break;
                    case Poppler::LinkAction::PageNext:
                        hyperlinkElement.setAttribute( "action", "PageNext" );
                        break;
                    case Poppler::LinkAction::PageLast:
                        hyperlinkElement.setAttribute( "action", "PageLast" );
                        break;
                    case Poppler::LinkAction::HistoryBack:
                        hyperlinkElement.setAttribute( "action", "HistoryBack" );
                        break;
                    case Poppler::LinkAction::HistoryForward:
                        hyperlinkElement.setAttribute( "action", "HistoryForward" );
                        break;
                    case Poppler::LinkAction::Quit:
                        hyperlinkElement.setAttribute( "action", "Quit" );
                        break;
                    case Poppler::LinkAction::Presentation:
                        hyperlinkElement.setAttribute( "action", "Presentation" );
                        break;
                    case Poppler::LinkAction::EndPresentation:
                        hyperlinkElement.setAttribute( "action", "EndPresentation" );
                        break;
                    case Poppler::LinkAction::Find:
                        hyperlinkElement.setAttribute( "action", "Find" );
                        break;
                    case Poppler::LinkAction::GoToPage:
                        hyperlinkElement.setAttribute( "action", "GoToPage" );
                        break;
                    case Poppler::LinkAction::Close:
                        hyperlinkElement.setAttribute( "action", "Close" );
                        break;
                    case Poppler::LinkAction::Print:
                        hyperlinkElement.setAttribute( "action", "Print" );
                        break;
                }
                break;
            }
            case Poppler::Link::Movie:
            {
                hyperlinkElement.setAttribute( "type", "Movie" );
                break;
            }
            case Poppler::Link::Rendition:
            {
                hyperlinkElement.setAttribute( "type", "Rendition" );
                break;
            }
            case Poppler::Link::Sound:
            {
                // FIXME: implement me
                break;
            }
            case Poppler::Link::None:
                break;
        }
    }
}

Annotation::SubType LinkAnnotation::subType() const
{
    return ALink;
}

Link* LinkAnnotation::linkDestination() const
{
    Q_D( const LinkAnnotation );
    return d->linkDestination;
}

void LinkAnnotation::setLinkDestination( Link *link )
{
    Q_D( LinkAnnotation );
    delete d->linkDestination;
    d->linkDestination = link;
}

LinkAnnotation::HighlightMode LinkAnnotation::linkHighlightMode() const
{
    Q_D( const LinkAnnotation );
    return d->linkHLMode;
}

void LinkAnnotation::setLinkHighlightMode( LinkAnnotation::HighlightMode mode )
{
    Q_D( LinkAnnotation );
    d->linkHLMode = mode;
}

QPointF LinkAnnotation::linkRegionPoint( int id ) const
{
    if ( id < 0 || id >= 4 )
        return QPointF();

    Q_D( const LinkAnnotation );
    return d->linkRegion[id];
}

void LinkAnnotation::setLinkRegionPoint( int id, const QPointF &point )
{
    if ( id < 0 || id >= 4 )
        return;

    Q_D( LinkAnnotation );
    d->linkRegion[id] = point;
}

/** CaretAnnotation [Annotation] */
class CaretAnnotationPrivate : public AnnotationPrivate
{
    public:
        CaretAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        CaretAnnotation::CaretSymbol symbol;
};

static QString caretSymbolToString( CaretAnnotation::CaretSymbol symbol )
{
    switch ( symbol )
    {
        case CaretAnnotation::None:
            return QString::fromLatin1( "None" );
        case CaretAnnotation::P:
            return QString::fromLatin1( "P" );
    }
    return QString();
}

static CaretAnnotation::CaretSymbol caretSymbolFromString( const QString &symbol )
{
    if ( symbol == QLatin1String( "None" ) )
        return CaretAnnotation::None;
    else if ( symbol == QLatin1String( "P" ) )
        return CaretAnnotation::P;
    return CaretAnnotation::None;
}

CaretAnnotationPrivate::CaretAnnotationPrivate()
    : AnnotationPrivate(), symbol( CaretAnnotation::None )
{
}

Annotation * CaretAnnotationPrivate::makeAlias()
{
    return new CaretAnnotation(*this);
}

Annot* CaretAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    // Setters are defined in the public class
    CaretAnnotation *q = static_cast<CaretAnnotation*>( makeAlias() );

    // Set page and document
    pdfPage = destPage;
    parentDoc = doc;

    // Set pdfAnnot
    PDFRectangle rect = boundaryToPdfRectangle(boundary, flags);
    pdfAnnot = new AnnotCaret(destPage->getDoc(), &rect);

    // Set properties
    flushBaseAnnotationProperties();
    q->setCaretSymbol(symbol);

    delete q;
    return pdfAnnot;
}

CaretAnnotation::CaretAnnotation()
    : Annotation( *new CaretAnnotationPrivate() )
{
}

CaretAnnotation::CaretAnnotation(CaretAnnotationPrivate &dd)
    : Annotation( dd )
{
}

CaretAnnotation::CaretAnnotation( const QDomNode & node )
    : Annotation( *new CaretAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'caret' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "caret" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "symbol" ) )
            setCaretSymbol(caretSymbolFromString( e.attribute( "symbol" ) ));

        // loading complete
        break;
    }
}

CaretAnnotation::~CaretAnnotation()
{
}

void CaretAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [caret] element
    QDomElement caretElement = document.createElement( "caret" );
    node.appendChild( caretElement );

    // append the optional attributes
    if ( caretSymbol() != CaretAnnotation::None )
        caretElement.setAttribute( "symbol", caretSymbolToString( caretSymbol() ) );
}

Annotation::SubType CaretAnnotation::subType() const
{
    return ACaret;
}

CaretAnnotation::CaretSymbol CaretAnnotation::caretSymbol() const
{
    Q_D( const CaretAnnotation );

    if (!d->pdfAnnot)
        return d->symbol;

    const AnnotCaret * caretann = static_cast<const AnnotCaret *>(d->pdfAnnot);
    return (CaretAnnotation::CaretSymbol)caretann->getSymbol();
}

void CaretAnnotation::setCaretSymbol( CaretAnnotation::CaretSymbol symbol )
{
    Q_D( CaretAnnotation );

    if (!d->pdfAnnot)
    {
        d->symbol = symbol;
        return;
    }

    AnnotCaret * caretann = static_cast<AnnotCaret *>(d->pdfAnnot);
    caretann->setSymbol((AnnotCaret::AnnotCaretSymbol)symbol);
}

/** FileAttachmentAnnotation [Annotation] */
class FileAttachmentAnnotationPrivate : public AnnotationPrivate
{
    public:
        FileAttachmentAnnotationPrivate();
        ~FileAttachmentAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        QString icon;
        EmbeddedFile *embfile;
};

FileAttachmentAnnotationPrivate::FileAttachmentAnnotationPrivate()
    : AnnotationPrivate(), icon( "PushPin" ), embfile( 0 )
{
}

FileAttachmentAnnotationPrivate::~FileAttachmentAnnotationPrivate()
{
    delete embfile;
}

Annotation * FileAttachmentAnnotationPrivate::makeAlias()
{
    return new FileAttachmentAnnotation(*this);
}

Annot* FileAttachmentAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    return 0; // Not implemented
}

FileAttachmentAnnotation::FileAttachmentAnnotation()
    : Annotation( *new FileAttachmentAnnotationPrivate() )
{
}

FileAttachmentAnnotation::FileAttachmentAnnotation(FileAttachmentAnnotationPrivate &dd)
    : Annotation( dd )
{
}

FileAttachmentAnnotation::FileAttachmentAnnotation( const QDomNode & node )
    : Annotation( *new FileAttachmentAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'fileattachment' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "fileattachment" )
            continue;

        // loading complete
        break;
    }
}

FileAttachmentAnnotation::~FileAttachmentAnnotation()
{
}

void FileAttachmentAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [fileattachment] element
    QDomElement fileAttachmentElement = document.createElement( "fileattachment" );
    node.appendChild( fileAttachmentElement );
}

Annotation::SubType FileAttachmentAnnotation::subType() const
{
    return AFileAttachment;
}

QString FileAttachmentAnnotation::fileIconName() const
{
    Q_D( const FileAttachmentAnnotation );
    return d->icon;
}

void FileAttachmentAnnotation::setFileIconName( const QString &icon )
{
    Q_D( FileAttachmentAnnotation );
    d->icon = icon;
}

EmbeddedFile* FileAttachmentAnnotation::embeddedFile() const
{
    Q_D( const FileAttachmentAnnotation );
    return d->embfile;
}

void FileAttachmentAnnotation::setEmbeddedFile( EmbeddedFile *ef )
{
    Q_D( FileAttachmentAnnotation );
    d->embfile = ef;
}

/** SoundAnnotation [Annotation] */
class SoundAnnotationPrivate : public AnnotationPrivate
{
    public:
        SoundAnnotationPrivate();
        ~SoundAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        QString icon;
        SoundObject *sound;
};

SoundAnnotationPrivate::SoundAnnotationPrivate()
    : AnnotationPrivate(), icon( "Speaker" ), sound( 0 )
{
}

SoundAnnotationPrivate::~SoundAnnotationPrivate()
{
    delete sound;
}

Annotation * SoundAnnotationPrivate::makeAlias()
{
    return new SoundAnnotation(*this);
}

Annot* SoundAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    return 0; // Not implemented
}

SoundAnnotation::SoundAnnotation()
    : Annotation( *new SoundAnnotationPrivate() )
{
}

SoundAnnotation::SoundAnnotation(SoundAnnotationPrivate &dd)
    : Annotation( dd )
{
}

SoundAnnotation::SoundAnnotation( const QDomNode & node )
    : Annotation( *new SoundAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'sound' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "sound" )
            continue;

        // loading complete
        break;
    }
}

SoundAnnotation::~SoundAnnotation()
{
}

void SoundAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [sound] element
    QDomElement soundElement = document.createElement( "sound" );
    node.appendChild( soundElement );
}

Annotation::SubType SoundAnnotation::subType() const
{
    return ASound;
}

QString SoundAnnotation::soundIconName() const
{
    Q_D( const SoundAnnotation );
    return d->icon;
}

void SoundAnnotation::setSoundIconName( const QString &icon )
{
    Q_D( SoundAnnotation );
    d->icon = icon;
}

SoundObject* SoundAnnotation::sound() const
{
    Q_D( const SoundAnnotation );
    return d->sound;
}

void SoundAnnotation::setSound( SoundObject *s )
{
    Q_D( SoundAnnotation );
    d->sound = s;
}

/** MovieAnnotation [Annotation] */
class MovieAnnotationPrivate : public AnnotationPrivate
{
    public:
        MovieAnnotationPrivate();
        ~MovieAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        MovieObject *movie;
        QString title;
};

MovieAnnotationPrivate::MovieAnnotationPrivate()
    : AnnotationPrivate(), movie( 0 )
{
}

MovieAnnotationPrivate::~MovieAnnotationPrivate()
{
    delete movie;
}

Annotation * MovieAnnotationPrivate::makeAlias()
{
    return new MovieAnnotation(*this);
}

Annot* MovieAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    return 0; // Not implemented
}

MovieAnnotation::MovieAnnotation()
    : Annotation( *new MovieAnnotationPrivate() )
{
}

MovieAnnotation::MovieAnnotation(MovieAnnotationPrivate &dd)
    : Annotation( dd )
{
}

MovieAnnotation::MovieAnnotation( const QDomNode & node )
    : Annotation( *new MovieAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'movie' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "movie" )
            continue;

        // loading complete
        break;
    }
}

MovieAnnotation::~MovieAnnotation()
{
}

void MovieAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [movie] element
    QDomElement movieElement = document.createElement( "movie" );
    node.appendChild( movieElement );
}

Annotation::SubType MovieAnnotation::subType() const
{
    return AMovie;
}

MovieObject* MovieAnnotation::movie() const
{
    Q_D( const MovieAnnotation );
    return d->movie;
}

void MovieAnnotation::setMovie( MovieObject *movie )
{
    Q_D( MovieAnnotation );
    d->movie = movie;
}

QString MovieAnnotation::movieTitle() const
{
    Q_D( const MovieAnnotation );
    return d->title;
}

void MovieAnnotation::setMovieTitle( const QString &title )
{
    Q_D( MovieAnnotation );
    d->title = title;
}

/** ScreenAnnotation [Annotation] */
class ScreenAnnotationPrivate : public AnnotationPrivate
{
    public:
        ScreenAnnotationPrivate();
        ~ScreenAnnotationPrivate();
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);

        // data fields
        LinkRendition *action;
        QString title;
};

ScreenAnnotationPrivate::ScreenAnnotationPrivate()
    : AnnotationPrivate(), action( 0 )
{
}

ScreenAnnotationPrivate::~ScreenAnnotationPrivate()
{
    delete action;
}

ScreenAnnotation::ScreenAnnotation(ScreenAnnotationPrivate &dd)
    : Annotation( dd )
{}

Annotation * ScreenAnnotationPrivate::makeAlias()
{
    return new ScreenAnnotation(*this);
}

Annot* ScreenAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    return 0; // Not implemented
}

ScreenAnnotation::ScreenAnnotation()
    : Annotation( *new ScreenAnnotationPrivate() )
{
}

ScreenAnnotation::~ScreenAnnotation()
{
}

void ScreenAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [screen] element
    QDomElement screenElement = document.createElement( "screen" );
    node.appendChild( screenElement );
}

Annotation::SubType ScreenAnnotation::subType() const
{
    return AScreen;
}

LinkRendition* ScreenAnnotation::action() const
{
    Q_D( const ScreenAnnotation );
    return d->action;
}

void ScreenAnnotation::setAction( LinkRendition *action )
{
    Q_D( ScreenAnnotation );
    d->action = action;
}

QString ScreenAnnotation::screenTitle() const
{
    Q_D( const ScreenAnnotation );
    return d->title;
}

void ScreenAnnotation::setScreenTitle( const QString &title )
{
    Q_D( ScreenAnnotation );
    d->title = title;
}

Link* ScreenAnnotation::additionalAction( AdditionalActionType type ) const
{
    Q_D( const ScreenAnnotation );
    return d->additionalAction( type );
}

/** WidgetAnnotation [Annotation] */
class WidgetAnnotationPrivate : public AnnotationPrivate
{
    public:
        Annotation * makeAlias();
        Annot* createNativeAnnot(::Page *destPage, DocumentData *doc);
};

Annotation * WidgetAnnotationPrivate::makeAlias()
{
    return new WidgetAnnotation(*this);
}

Annot* WidgetAnnotationPrivate::createNativeAnnot(::Page *destPage, DocumentData *doc)
{
    return 0; // Not implemented
}

WidgetAnnotation::WidgetAnnotation(WidgetAnnotationPrivate &dd)
    : Annotation( dd )
{}

WidgetAnnotation::WidgetAnnotation()
    : Annotation( *new WidgetAnnotationPrivate() )
{
}

WidgetAnnotation::~WidgetAnnotation()
{
}

void WidgetAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // store base annotation properties
    storeBaseAnnotationProperties( node, document );

    // create [widget] element
    QDomElement widgetElement = document.createElement( "widget" );
    node.appendChild( widgetElement );
}

Annotation::SubType WidgetAnnotation::subType() const
{
    return AWidget;
}

Link* WidgetAnnotation::additionalAction( AdditionalActionType type ) const
{
    Q_D( const WidgetAnnotation );
    return d->additionalAction( type );
}

//BEGIN utility annotation functions
QColor convertAnnotColor( AnnotColor *color )
{
    if ( !color )
        return QColor();

    QColor newcolor;
    const double *color_data = color->getValues();
    switch ( color->getSpace() )
    {
        case AnnotColor::colorTransparent: // = 0,
            newcolor = Qt::transparent;
            break;
        case AnnotColor::colorGray: // = 1,
            newcolor.setRgbF( color_data[0], color_data[0], color_data[0] );
            break;
        case AnnotColor::colorRGB: // = 3,
            newcolor.setRgbF( color_data[0], color_data[1], color_data[2] );
            break;
        case AnnotColor::colorCMYK: // = 4
            newcolor.setCmykF( color_data[0], color_data[1], color_data[2], color_data[3] );
            break;
    }
    return newcolor;
}

AnnotColor* convertQColor( const QColor &c )
{
    if (!c.isValid() || c.alpha() == 0)
        return new AnnotColor(); // Transparent
    else
        return new AnnotColor(c.redF(), c.greenF(), c.blueF());
}
//END utility annotation functions

}
