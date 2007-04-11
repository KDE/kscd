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
#include <kurlrequester.h>
#include <QCheckBox>
#include <kcombobox.h>
#include <QLayout>
#include <QtDBus>
#include <config.h>

#include "configWidget.h"
#include "kscd.h"
#include "prefs.h"
#include "kcompactdisc.h"

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
    CdDeviceList->comboBox()->setEditable(true);

    load();

    connect(kcfg_DigitalPlayback, SIGNAL(toggled(bool)), this,SLOT(kcfg_DigitalPlayback_toggled(bool)));
    connect(kcfg_SelectEncoding, SIGNAL(toggled(bool)), this, SLOT(kcfg_SelectEncoding_toggled(bool)));
}

configWidget::~configWidget()
{
}

void configWidget::kcfg_DigitalPlayback_toggled(bool toggle)
{
    AudioSystemList->setEnabled(toggle);
    textLabel4->setEnabled(toggle);
    AudioDevice->setEnabled(toggle);
    textLabel5->setEnabled(toggle);
}

void configWidget::kcfg_SelectEncoding_toggled(bool toggle)
{
    kcfg_SelectedEncoding->setEnabled(toggle);
}

void configWidget::getMediaDevices(void)
{
     QDBusInterface mediamanager( "org.kde.kded", "/modules/mediamanager", "org.kde.MediaManager" );
     QDBusReply<QStringList> reply = mediamanager.call( "fullList" );
     if (!reply.isValid()) {
         return;
     }
    QStringList list = reply;
    QStringList::const_iterator it = list.begin();
    QStringList::const_iterator itEnd = list.end();
    // it would be much better if libmediacommon was in kdelibs
    while (it != itEnd) {
        it++;
        if (it == itEnd) break;
        QString url="media:/"+(*it); // is it always right? ervin?
        kDebug() << "checking " << url << endl;
        for (int i=0;i<9;i++) ++it;  // go to mimetype (MIME_TYPE-NAME from medium.h)
        kDebug() << "Mime: " << *it << endl;
        if (it!=itEnd && (*it)=="media/audiocd") {
            CdDeviceList->comboBox()->addItem(url);
        }
        while (it !=itEnd && (*it)!="---") ++it;  // go to end of current device's properties
        ++it;
    }
}

void configWidget::load(void)
{
    QStringList::Iterator it;
    int i;

    getMediaDevices();

    CdDeviceList->comboBox()->setCurrentText(Prefs::cdDevice());

    if(mPlayer->audioSystems().empty()) {
        kcfg_DigitalPlayback_toggled(false);

        kcfg_DigitalPlayback->setChecked(false);
        kcfg_DigitalPlayback->hide();
    } else {
        kcfg_DigitalPlayback_toggled(Prefs::digitalPlayback());

        // fill ComboBox audioBackend
        AudioSystemList->insertStringList(mPlayer->audioSystems());
        i = AudioSystemList->findText(Prefs::audioSystem());
        if(i >= -1)
            AudioSystemList->setCurrentIndex(1);

        AudioDevice->setText(Prefs::audioDevice());
    }
    kcfg_SelectEncoding_toggled(Prefs::selectEncoding());
}

void configWidget::defaults(void)
{
    getMediaDevices();
    CdDeviceList->comboBox()->setCurrentText(KCompactDisc::defaultDevice);

    kcfg_DigitalPlayback_toggled(false);

    // fill ComboBox audioBackend
    AudioSystemList->insertStringList(mPlayer->audioSystems());
    AudioSystemList->setCompletedText("phonon");
    AudioDevice->setText("");
}

void configWidget::save(void)
{
    Prefs::setCdDevice(CdDeviceList->comboBox()->currentText());
    Prefs::setAudioSystem(AudioSystemList->currentText());
    Prefs::setAudioDevice(AudioDevice->text());
    Prefs::writeConfig();
}

#include "configWidget.moc"
