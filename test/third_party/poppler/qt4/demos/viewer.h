/*
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
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

#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QtGui/QMainWindow>

class QAction;
class QActionGroup;
class QLabel;
class DocumentObserver;
namespace Poppler {
class Document;
}

class PdfViewer : public QMainWindow
{
    Q_OBJECT

    friend class DocumentObserver;

public:
    PdfViewer();
    ~PdfViewer();

    /*virtual*/ QSize sizeHint() const;

    void loadDocument(const QString &file);
    void closeDocument();

private Q_SLOTS:
    void slotOpenFile();
    void slotSaveCopy();
    void slotAbout();
    void slotAboutQt();
    void slotToggleTextAA(bool value);
    void slotToggleGfxAA(bool value);
    void slotRenderBackend(QAction *act);

private:
    void setPage(int page);
    int page() const;

    int m_currentPage;

    QAction *m_fileOpenAct;
    QAction *m_fileSaveCopyAct;
    QAction *m_settingsTextAAAct;
    QAction *m_settingsGfxAAAct;
    QActionGroup *m_settingsRenderBackendGrp;

    QList<DocumentObserver *> m_observers;

    Poppler::Document *m_doc;
};

#endif
