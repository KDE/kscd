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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "docking.h"
#include "kscd.h"

#include <q3hbox.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QLabel>
#include <QWheelEvent>
#include <Q3PopupMenu>

#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kpassivepopup.h>

#include <kdebug.h>

DockWidget::DockWidget( KSCD* parent, const char *name)
    : KSystemTray( parent, name )
{
    m_popup = 0;
    setPixmap( loadIcon("cdsmall") );

    KActionCollection* actionCollection = parent->actionCollection();
    m_backAction = actionCollection->action("Previous");
    m_forwardAction = actionCollection->action("Next");
    m_backPix = loadIcon("player_start");
    m_forwardPix = loadIcon("player_end");

    // popup menu for right mouse button
    Q3PopupMenu* popup = contextMenu();

    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_play", KIcon::Small), i18n("Play/Pause"), parent, SLOT(playClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_stop", KIcon::Small), i18n("Stop"), parent, SLOT(stopClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_end", KIcon::Small), i18n("Next"), parent, SLOT(nextClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_start", KIcon::Small), i18n("Previous"), parent, SLOT(prevClicked()));
    popup->insertItem(KGlobal::iconLoader()->loadIconSet("player_eject", KIcon::Small), i18n("Eject"), parent, SLOT(ejectClicked()));

    QToolTip::add(this, kapp->aboutData()->programName());
}

DockWidget::~DockWidget()
{
}

void DockWidget::createPopup(const QString &songName, bool addButtons)
{
    if (!Prefs::trackAnnouncement())
        return;

    delete m_popup;
    m_popup = new KPassivePopup(this);

    Q3HBox* box = new Q3HBox(m_popup);
    
    if (addButtons)
    {
        QPushButton* backButton = new QPushButton(m_backPix, 0, box, "popup_back");
        backButton->setFlat(true);
        connect(backButton, SIGNAL(clicked()), m_backAction, SLOT(activate()));
    }

    QLabel* l = new QLabel(songName, box);
    l->setMargin(3);

    if (addButtons)
    {
        QPushButton* forwardButton = new QPushButton(m_forwardPix, 0, box, "popup_forward");
        forwardButton->setFlat(true);
        connect(forwardButton, SIGNAL(clicked()), m_forwardAction, SLOT(activate()));
    }

    m_popup->setView(box);
    m_popup->setAutoDelete(false);
    m_popup->show();
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

void DockWidget::wheelEvent(QWheelEvent *e)
{
    if (e->orientation() == Qt::Horizontal)
        return;

    KSCD* kscd = dynamic_cast<KSCD*>(parent());
    if (kscd == 0)
        return;

    switch (e->state())
    {
	    case QT::ShiftButton:
        {
            if (e->delta() > 0)
            {
                kscd->incVolume();
            }
            else
            {
                kscd->decVolume();
            }
            break;
        }
        default:
        {
            if (e->delta() > 0)
            {
                kscd->nextClicked();
            }
            else
            {
                kscd->prevClicked();
            }
        }
    }
}

#include "docking.moc"
