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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <kdebug.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <qcheckbox.h>

#include <config.h>

#include "configWidget.h"
#include "kscd.h"
#include "prefs.h"

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
    
#if defined(BUILD_CDDA)
    kcfg_DigitalPlayback_toggled(Prefs::digitalPlayback());
    
    // fill ComboBox audioBackend
    kcfg_AudioSystem->insertStringList(mPlayer->audioSystems());

    int t = kcfg_AudioSystem->listBox()->index(kcfg_AudioSystem->listBox()->findItem(Prefs::audioSystem()));
    if(t != -1)
        kcfg_AudioSystem->setCurrentItem(t);

    kcfg_AudioDevice->lineEdit()->setText(Prefs::audioDevice());
    
    kcfg_DigitalPlayback->setChecked(Prefs::digitalPlayback());
#else
    kcfg_DigitalPlayback_toggled(false);
    
    kcfg_DigitalPlayback->setChecked(false);
    kcfg_DigitalPlayback->hide();
#endif
    kcfg_SelectEncoding_toggled(Prefs::selectEncoding());
}

configWidget::~configWidget()
{
}

void configWidget::kcfg_DigitalPlayback_toggled(bool toggle)
{
    if(toggle) {
        kcfg_AudioSystem->show();
        textLabel4->show();
        kcfg_AudioDevice->show();
        textLabel5->show();
    } else {
        kcfg_AudioSystem->hide();
        textLabel4->hide();
        kcfg_AudioDevice->hide();
        textLabel5->hide();
    }
}

void configWidget::kcfg_SelectEncoding_toggled(bool toggle)
{
    if(toggle) {
        kcfg_SelectedEncoding->show();
    } else {
        kcfg_SelectedEncoding->hide();
    }
}

void configWidget::configDone()
{
    Prefs::setAudioSystem(kcfg_AudioSystem->currentText());
}


#include "configWidget.moc"
