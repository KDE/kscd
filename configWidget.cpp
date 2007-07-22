/*
 * configWidget - the config dialog page for KSCD settings
 *
 * $Id:
 *
 * Copyright (c) 2002 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "configWidget.h"
#include "kscd.h"
#include "prefs.h"
#include "kcompactdisc.h"

#include <QCheckBox>

#include <kdebug.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kcombobox.h>


/*
 *  Constructs a configWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
configWidget::configWidget(KSCD* player, QWidget* parent)
    : configWidgetUI(parent),
      mPlayer(player)
{
    kcfg_CdDevice->insertItems(0, KCompactDisc::cdromDeviceNames());
    kcfg_AudioSystem->insertItems(0, KCompactDisc::audioSystems());

    connect(kcfg_DigitalPlayback, SIGNAL(toggled(bool)), this,SLOT(kcfg_DigitalPlayback_toggled(bool)));
    connect(kcfg_SelectEncoding, SIGNAL(toggled(bool)), this, SLOT(kcfg_SelectEncoding_toggled(bool)));
}

configWidget::~configWidget()
{
}

void configWidget::kcfg_DigitalPlayback_toggled(bool toggle)
{
    kcfg_AudioSystem->setEnabled(toggle);
    textLabel4->setEnabled(toggle);
    kcfg_AudioDevice->setEnabled(toggle);
    textLabel5->setEnabled(toggle);
}

void configWidget::kcfg_SelectEncoding_toggled(bool toggle)
{
    kcfg_SelectedEncoding->setEnabled(toggle);
}

#include "configWidget.moc"
