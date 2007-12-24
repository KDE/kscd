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

#include <QLabel>
#include <QPushButton>
#include <QWheelEvent>
#include <QMenu>

#include <kaboutdata.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kpassivepopup.h>

DockWidget::DockWidget( KSCD* parent, const char *name)
    : KSystemTrayIcon( parent )
{
    setObjectName(name);
    m_popup = 0;
    setIcon(loadIcon("kscd-dock"));

    // popup menu for right mouse button
    QMenu* popup = contextMenu();

    popup->addAction(SmallIcon("media-playback-start"), i18n("Play/Pause"), parent, SLOT(playClicked()));
    popup->addAction(SmallIcon("media-playback-stop"), i18n("Stop"), parent, SLOT(stopClicked()));
    m_forwardAction = popup->addAction(SmallIcon("media-skip-forward"), i18n("Next"), parent, SLOT(nextClicked()));
    m_backAction = popup->addAction(SmallIcon("media-skip-backward"), i18n("Previous"), parent, SLOT(prevClicked()));
    popup->addAction(SmallIcon("media-eject"), i18n("Eject"), parent, SLOT(ejectClicked()));

    this->setToolTip(KGlobal::mainComponent().aboutData()->programName());
}

DockWidget::~DockWidget()
{
}

void DockWidget::createPopup(const QString &songName, bool addButtons)
{
    if (!Prefs::trackAnnouncement())
        return;

    delete m_popup;
    m_popup = new KPassivePopup(parentWidget());

    KHBox* box = new KHBox(m_popup);

    if (addButtons)
    {
        QPushButton* backButton = new QPushButton(loadIcon("media-skip-backward"), 0, box);
        backButton->setFlat(true);
        connect(backButton, SIGNAL(clicked()), m_backAction, SLOT(activate()));
    }

    QLabel* l = new QLabel(songName, box);
    l->setMargin(3);

    if (addButtons)
    {
        QPushButton* forwardButton = new QPushButton(loadIcon("media-skip-forward"), 0, box);
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

    if (text.isEmpty())
    {
        setToolTip(KGlobal::mainComponent().aboutData()->programName());
    }
    else
    {
        setToolTip(text);
    }
}

void DockWidget::wheelEvent(QWheelEvent *e)
{
    if (e->orientation() == Qt::Horizontal)
        return;

    KSCD* kscd = dynamic_cast<KSCD*>(parent());
    if (kscd == 0)
        return;

    switch (e->modifiers())
    {
    case Qt::ShiftModifier:
        if (e->delta() > 0)
        {
            kscd->incVolume();
        }
        else
        {
            kscd->decVolume();
        }
        break;
    default:
        if (e->delta() > 0)
        {
            kscd->nextClicked();
        }
        else
        {
            kscd->prevClicked();
        }
        break;
    }
}

#include "docking.moc"
