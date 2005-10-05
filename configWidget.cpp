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

#include <kdebug.h>
#include <klineedit.h>
#include <dcopref.h>
#include <kurlrequester.h>
#include <qcheckbox.h>
#include <kcombobox.h>

#include <config.h>
extern "C" {
    // We don't have libWorkMan installed already, so get everything
    // from within our own directory
#include "libwm/include/wm_config.h"
}


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

    kcfg_cdDevice->comboBox()->setEditable(true);
    kcfg_cdDevice->comboBox()->insertItem(DEFAULT_CD_DEVICE);
    getMediaDevices();
    
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
    kcfg_AudioSystem->setEnabled(toggle);
    textLabel4->setEnabled(toggle);
    kcfg_AudioDevice->setEnabled(toggle);
    textLabel5->setEnabled(toggle);
}

void configWidget::getMediaDevices()
{
    DCOPRef ref("kded","mediamanager");
    DCOPReply rep = ref.call("fullList()");
    if (!rep.isValid()) {
        return;
    }
    QStringList list = rep;
    QStringList::const_iterator it = list.begin();
    QStringList::const_iterator itEnd = list.end();
    // it would be much better if libmediacommon was in kdelibs
    while (it != itEnd) {
        it++;
        if (it == itEnd) break;
        QString url="media:/"+(*it); // is it always right? ervin?
        kdDebug() << "checking " << url << endl;
        for (int i=0;i<9;i++) ++it;  // go to mimetype (MIME_TYPE-NAME from medium.h)
        kdDebug() << "Mime: " << *it << endl;
        if (it!=itEnd && (*it)=="media/audiocd") {
            kcfg_cdDevice->comboBox()->insertItem(url);
        }
        while (it !=itEnd && (*it)!="---") ++it;  // go to end of current device's properties
        ++it;
    }
}
 

void configWidget::kcfg_SelectEncoding_toggled(bool toggle)
{
    kcfg_SelectedEncoding->setEnabled(toggle);
}

void configWidget::configDone()
{
    Prefs::setAudioSystem(kcfg_AudioSystem->currentText());
}


#include "configWidget.moc"
