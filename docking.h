/*
 *              kPPP: A pppd Front End for the KDE project
 *
 * $Id$
 *
 *              Copyright (C) 1997 Bernd Johannes Wuebben
 *                      wuebben@math.cornell.edu
 *
 * This file was contributed by Harri Porten <porten@tu-harburg.de>
 * Latest changes (dynamic tooltips) by Cristian Tibirna <tibirna@kde.org>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef _DOCKING_H_
#define _DOCKING_H_

#include <stdio.h>
#include <qapplication.h>
#include <QIcon>
#include <QTimer>
#include <q3popupmenu.h>
#include <QPoint>
//Added by qt3to4:
#include <QWheelEvent>
#include <ksystemtrayicon.h>

class KSCD;

class QAction;
class KToggleAction;
class KPassivePopup;

class DockWidget : public KSystemTrayIcon
{
    Q_OBJECT

public:
    DockWidget( KSCD* parent, const char *name=0);
    ~DockWidget();

public slots:
    void setToolTip(const QString& text);
    void createPopup(const QString& songName, bool addButtons = true);

private:
    virtual void wheelEvent( QWheelEvent *e);

    KPassivePopup* m_popup;

    QAction* m_forwardAction;
    QAction* m_backAction;

    QIcon m_backPix;
    QIcon m_forwardPix;

    QString tip;
};

#endif



