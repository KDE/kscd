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
    kcfg_CdDevice->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
    kcfg_CdDevice->comboBox()->insertItems(0, KCompactDisc::devices());
    kcfg_AudioSystem->insertItems(0, KCompactDisc::audioSystems());

    connect(kcfg_DigitalPlayback, SIGNAL(toggled(bool)), this,SLOT(kcfg_DigitalPlayback_toggled(bool)));
    connect(kcfg_SelectEncoding, SIGNAL(toggled(bool)), this, SLOT(kcfg_SelectEncoding_toggled(bool)));

    kcfg_DigitalPlayback_toggled(false);
    kcfg_SelectEncoding_toggled(false);

    load();
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

void configWidget::load(void)
{
    int i;

    i = kcfg_CdDevice->comboBox()->findText(Prefs::cdDevice());
    if(i == -1)
        kcfg_CdDevice->setPath(Prefs::cdDevice());
    else
        kcfg_CdDevice->comboBox()->setCurrentIndex(i);
    kcfg_DigitalPlayback->setChecked(Prefs::digitalPlayback());
    kcfg_AudioSystem->setCurrentIndex(Prefs::audioSystem());
    kcfg_AudioDevice->setText(Prefs::audioDevice());
    kcfg_SelectEncoding->setChecked(Prefs::selectEncoding());
}

void configWidget::defaults(void)
{
    int i;

    i = kcfg_CdDevice->comboBox()->findText(KCompactDisc::defaultDevice);
    if(i == -1)
        kcfg_CdDevice->setPath(KCompactDisc::defaultDevice);
    else
        kcfg_CdDevice->comboBox()->setCurrentIndex(i);
    kcfg_DigitalPlayback->setChecked(true);
    kcfg_AudioSystem->setCurrentIndex(Prefs::EnumAudioSystem::phonon);
    kcfg_AudioDevice->setText(QString());
    kcfg_SelectEncoding->setChecked(false);
}

void configWidget::save(void)
{
kDebug() << "kcfg_CdDevice->url().path()" << kcfg_CdDevice->url().path() << endl;
    Prefs::setCdDevice(kcfg_CdDevice->url().path());
    Prefs::setDigitalPlayback(kcfg_DigitalPlayback->isChecked());
    Prefs::setAudioSystem(kcfg_AudioSystem->currentIndex());
    Prefs::setAudioDevice(kcfg_AudioDevice->text());
    Prefs::setSelectEncoding(kcfg_SelectEncoding->isChecked());

    Prefs::writeConfig();
}

#include "configWidget.moc"
