/* poppler-annotation-private.h: qt interface to poppler
 * Copyright (C) 2007, Pino Toscano <pino@kde.org>
 * Copyright (C) 2012, Tobias Koenig <tokoe@kdab.com>
 * Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2012, Albert Astals Cid <aacid@kde.org>
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

#ifndef _POPPLER_ANNOTATION_PRIVATE_H_
#define _POPPLER_ANNOTATION_PRIVATE_H_

#include <QtCore/QLinkedList>
#include <QtCore/QPointF>
#include <QtCore/QSharedDataPointer>

#include "poppler-annotation.h"

#include <Object.h>

class Annot;
class AnnotPath;
class Link;
class Page;
class PDFRectangle;

namespace Poppler
{
class DocumentData;

class AnnotationPrivate : public QSharedData
{
    public:
        AnnotationPrivate();
        virtual ~AnnotationPrivate();

        void addRevision(Annotation *ann, Annotation::RevScope scope, Annotation::RevType type);

        /* Returns an Annotation of the right subclass whose d_ptr points to
         * this AnnotationPrivate */
        virtual Annotation * makeAlias() = 0;

        /* properties: contents related */
        QString author;
        QString contents;
        QString uniqueName;
        QDateTime modDate;       // before or equal to currentDateTime()
        QDateTime creationDate;  // before or equal to modifyDate

        /* properties: look/interaction related */
        int flags;
        QRectF boundary;

        /* style and popup */
        Annotation::Style style;
        Annotation::Popup popup;

        /* revisions */
        Annotation::RevScope revisionScope;
        Annotation::RevType revisionType;
        QList<Annotation*> revisions;

        /* After this call, the Annotation object will behave like a wrapper for
         * the specified Annot object. All cached values are discarded */
        void tieToNativeAnnot(Annot *ann, ::Page *page, DocumentData *doc);

        /* Creates a new Annot object on the specified page, flushes current
         * values to that object and ties this Annotation to that object */
        virtual Annot* createNativeAnnot(::Page *destPage, DocumentData *doc) = 0;

        /* Inited to 0 (i.e. untied annotation) */
        Annot *pdfAnnot;
        ::Page *pdfPage;
        DocumentData * parentDoc;

        /* The following helpers only work if pdfPage is set */
        void flushBaseAnnotationProperties();
        void fillNormalizationMTX(double MTX[6], int pageRotation) const;
        void fillTransformationMTX(double MTX[6]) const;
        QRectF fromPdfRectangle(const PDFRectangle &r) const;
        PDFRectangle boundaryToPdfRectangle(const QRectF &r, int flags) const;
        AnnotPath * toAnnotPath(const QLinkedList<QPointF> &l) const;

        /* Scan page for annotations, parentId=0 searches for root annotations */
        static QList<Annotation*> findAnnotations(::Page *pdfPage, DocumentData *doc, int parentId = 0);

        /* Add given annotation to given page */
        static void addAnnotationToPage(::Page *pdfPage, DocumentData *doc, const Annotation * ann);

        /* Remove annotation from page and destroy ann */
        static void removeAnnotationFromPage(::Page *pdfPage, const Annotation * ann);

        Ref pdfObjectReference() const;

        Link* additionalAction( Annotation::AdditionalActionType type ) const;
};

}

#endif
