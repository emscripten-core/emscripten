/*
 * Copyright (C) 2008-2009, Pino Toscano <pino@kde.org>
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

#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include <QtGui/QToolBar>

#include "documentobserver.h"

class QAction;
class QComboBox;

class NavigationToolBar : public QToolBar, public DocumentObserver
{
    Q_OBJECT

public:
    NavigationToolBar(QWidget *parent = 0);
    ~NavigationToolBar();

    /*virtual*/ void documentLoaded();
    /*virtual*/ void documentClosed();
    /*virtual*/ void pageChanged(int page);

Q_SIGNALS:
    void zoomChanged(qreal value);

private Q_SLOTS:
    void slotGoFirst();
    void slotGoPrev();
    void slotGoNext();
    void slotGoLast();
    void slotComboActivated(int index);
    void slotZoomComboChanged(const QString &text);

private:
    QAction *m_firstAct;
    QAction *m_prevAct;
    QComboBox *m_pageCombo;
    QAction *m_nextAct;
    QAction *m_lastAct;
    QComboBox *m_zoomCombo;
};

#endif
