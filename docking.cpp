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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "kscd.h"

#include <qtooltip.h>
#include <kapp.h>

#include "docking.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kpopmenu.h>

DockWidget::DockWidget( KSCD* parent, const char *name)
  : KDockWindow( parent, name ) 
{

    setPixmap( BarIcon("cdsmall.xpm") );
      
      // popup menu for right mouse button
    QPopupMenu* popup = contextMenu();

    popup->insertItem(i18n("Play/Pause"), parent, SLOT(playClicked()));
    popup->insertItem(i18n("Stop"), parent, SLOT(stopClicked()));
    popup->insertItem(i18n("Forward"), parent, SLOT(fwdClicked()));
    popup->insertItem(i18n("Backward"), parent, SLOT(bwdClicked()));
    popup->insertItem(i18n("Next"), parent, SLOT(nextClicked()));
    popup->insertItem(i18n("Previous"), parent, SLOT(prevClicked()));
    popup->insertItem(i18n("Eject"), parent, SLOT(ejectClicked()));
}

DockWidget::~DockWidget() {
}


#include "docking.moc"






