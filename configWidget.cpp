/*
 * configWidget - the config dialog page for KSCD settings
 *
 * $Id:
 *
 * Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "configWidget.h"
#include "kscd.h"

#include <kaccel.h>
#include <kcolorbutton.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

/*
 *  Constructs a configWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
configWidget::configWidget(KSCD* player, QWidget* parent, const char* name)
    : configWidgetUI(parent, name),
      mPlayer(player)
{
    if (!name)
    {
        setName("configWidget");
    }

    bgColorBtn->setColor(mPlayer->bgColor());
    ledColorBtn->setColor(mPlayer->ledColor());
    systrayChckbx->setChecked(mPlayer->dock());
    skipInterval->setValue(mPlayer->skipInterval());
    autoplayChkbx->setChecked(mPlayer->autoPlay());
    ejectChkbx->setChecked(mPlayer->ejectOnFinish());
    stopOnExitChckbx->setChecked(mPlayer->stopOnExit());
    cdDevice->setURL(mPlayer->devicePath());
    digitalPlaybackChckbx->setChecked(mPlayer->digitalPlayback());
}

configWidget::~configWidget()
{
}

void configWidget::apply()
{
    mPlayer->setColors(ledColorBtn->color(), bgColorBtn->color());
    mPlayer->setDocking(systrayChckbx->isChecked());
    mPlayer->setSkipInterval(skipInterval->value());
    mPlayer->setAutoplay(autoplayChkbx->isChecked());
    mPlayer->setEjectOnFinish(ejectChkbx->isChecked());
    mPlayer->setStopOnExit(stopOnExitChckbx->isChecked());
    mPlayer->setDevicePath(cdDevice->lineEdit()->text());
    mPlayer->setDigitalPlayback(digitalPlaybackChckbx->isChecked());
}
