/*
 *              KSCD -- a simpole cd player for the KDE project
 *
 * $Id$
 *
 *              Copyright (C) 1997 Bernd Johannes Wuebben
 *                      wuebben@math.cornell.edu
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
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "docking.h"
#include "kscd.h"

#include <qtooltip.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

DockWidget::DockWidget( KSCD* parent, const char *name)
    : KSystemTray( parent, name )
{

    setPixmap( loadIcon("cdsmall") );

      // popup menu for right mouse button
    QPopupMenu* popup = contextMenu();

    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_play", KIcon::Small), i18n("Play/Pause"), parent, SLOT(playClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_stop", KIcon::Small), i18n("Stop"), parent, SLOT(stopClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_end", KIcon::Small), i18n("Next"), parent, SLOT(nextClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_start", KIcon::Small), i18n("Previous"), parent, SLOT(prevClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_eject", KIcon::Small), i18n("Eject"), parent, SLOT(ejectClicked()));

    QToolTip::add(this, kapp->aboutData()->programName());
}

DockWidget::~DockWidget() {
}

void DockWidget::setToolTip(const QString& text)
{
    if (tip == text)
    {
        return;
    }

    tip = text;
    QToolTip::remove(this);

    if (text.isEmpty())
    {
        QToolTip::add(this, kapp->aboutData()->programName());
    }
    else
    {
        QToolTip::add(this, text);
    }
}


#include "docking.moc"
