/*
 * Kscd - A simple cd player for the KDE Project
 *
 * $Id$
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyrithg (c) 2001 Dirk Försterling milliByte@gmx.net
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <klocale.h>
#include <kapplication.h>

#include <smtpconfig.h>

#include <qlayout.h>
#include <qfontmetrics.h>
#include <qvalidator.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <kemailsettings.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kurllabel.h>

bool SMTPConfigData::isValid() const
{
    // simple validation
    return !serverHost.isEmpty() && !serverPort.isEmpty() &&
           senderAddress.contains("@") &&
           (senderReplyTo.contains("@") || senderReplyTo.isEmpty());
}

void SMTPConfigData::loadGlobalSettings()
{
    KEMailSettings kes;
    kes.setProfile( i18n("Default") );
    senderAddress = kes.getSetting( KEMailSettings::EmailAddress );
    senderReplyTo = kes.getSetting( KEMailSettings::ReplyToAddress );
}


SMTPConfig::SMTPConfig(QWidget *parent, const char *name, struct SMTPConfigData *_configData)
    : QWidget(parent, name)
{
    configData = _configData;
    QFontMetrics fm(font());

    QVBoxLayout* lay1 = new QVBoxLayout(this);
    QGroupBox* mainBox = new QGroupBox(this, "mainBox");
    lay1->addWidget(mainBox);

    QVBoxLayout* lay2 = new QVBoxLayout(mainBox, KDialog::marginHint());
    enableCB = new QCheckBox(i18n("Enable submission via SMTP"), mainBox, "enableCB");
    lay2->addWidget(enableCB);
    enableCB->setChecked(configData->enabled);
    connect(enableCB, SIGNAL(clicked()), this, SLOT(enableClicked()));

    lay2->addSpacing(20);

    // SMTP server settings
    const int LAYOUT_INDENT = 30;

    smtpServerLabel = new QLabel(i18n("SMTP server"), mainBox, "smtpServerLabel");
    lay2->addWidget(smtpServerLabel);

    QGridLayout* serverLayout = new QGridLayout(lay2, 2, 3, KDialog::spacingHint());
    serverLayout->addColSpacing(0, LAYOUT_INDENT);
    serverLayout->setColStretch(2, 1);

    serverHostLabel = new QLabel(i18n("Host:"), mainBox, "serverHostLabel");
    serverLayout->addWidget(serverHostLabel, 0, 1);
    serverHostEdit = new QLineEdit(mainBox, "serverHostEdit");
    serverLayout->addWidget(serverHostEdit, 0, 2);
    serverHostEdit->setText(configData->serverHost);

    serverPortLabel = new QLabel(i18n("Port:"), mainBox, "serverPortLabel");
    serverLayout->addWidget(serverPortLabel, 1, 1);
    serverPortEdit = new QLineEdit(mainBox, "serverPortEdit");
    serverPortEdit->setValidator( new QIntValidator(this) );
    serverLayout->addWidget(serverPortEdit, 1, 2);
    serverPortEdit->setText(configData->serverPort);

    // Global email address settings
    globalRadio = new QRadioButton(i18n("Use email addresses from Control Center"),
        mainBox, "globalRadio");
    lay2->addWidget(globalRadio);

    QGridLayout* globalLayout = new QGridLayout(lay2, 3, 4, KDialog::spacingHint());
    globalLayout->addColSpacing(0, LAYOUT_INDENT);
    globalLayout->setColStretch(3, 1);

    globalAddressLabel = new QLabel(i18n("From:"), mainBox, "globalAddressLabel");
    globalLayout->addWidget(globalAddressLabel, 0, 1);
    globalAddressSetting = new QLabel(mainBox, "globalAddressSetting");
    globalLayout->addWidget(globalAddressSetting, 0, 3);

    globalReplyToLabel = new QLabel(i18n("Reply-To:"), mainBox, "globalReplyToLabel");
    globalLayout->addWidget(globalReplyToLabel, 1, 1);
    globalReplyToSetting = new QLabel(mainBox, "globalReplyToSetting");
    globalLayout->addWidget(globalReplyToSetting, 1, 3);
    
    controlCenterLink = new KURLLabel(mainBox, "controlCenterLink");
    controlCenterLink->setText(i18n("Open the email address control panel"));
    globalLayout->addMultiCellWidget(controlCenterLink, 2, 2, 1, 3);
    connect(controlCenterLink, SIGNAL(leftClickedURL()), this, SLOT(launchControlCenter()));

    updateGlobalSettings();

    // KSCD-specific email address settings
    localRadio = new QRadioButton( i18n("Use the following email addresses"),
        mainBox, "localRadio");
    lay2->addWidget(localRadio);

    QButtonGroup* radioGroup = new QButtonGroup;
    radioGroup->insert(globalRadio);
    radioGroup->insert(localRadio);

    QGridLayout* localLayout = new QGridLayout(lay2, 2, 3, KDialog::spacingHint());
    localLayout->addColSpacing(0, 30);
    localLayout->setColStretch(2, 1);

    senderAddressLabel = new QLabel(i18n("From:"), mainBox, "senderAddressLabel");
    localLayout->addWidget(senderAddressLabel, 0, 1);
    senderAddressEdit = new QLineEdit(mainBox, "senderAddressEdit");
    localLayout->addWidget(senderAddressEdit, 0, 2);
    if(!configData->useGlobalSettings)
        senderAddressEdit->setText(configData->senderAddress);

    senderReplyToLabel = new QLabel(i18n("Reply-To:"), mainBox, "senderReplyToLabel");
    localLayout->addWidget(senderReplyToLabel, 1, 1);
    senderReplyToEdit = new QLineEdit(mainBox, "senderReplyToEdit");
    localLayout->addWidget(senderReplyToEdit, 1, 2);
    if(!configData->useGlobalSettings)
        senderReplyToEdit->setText(configData->senderReplyTo);
    
    lay1->addStretch(1);

    if(configData->useGlobalSettings)
        globalRadio->setChecked(true);
    else
        localRadio->setChecked(true);

    enableClicked();
}

void SMTPConfig::commitData(void)
{
    configData->enabled = enableCB->isChecked();
    configData->serverHost = serverHostEdit->text();
    configData->serverPort = serverPortEdit->text();
    configData->useGlobalSettings = globalRadio->isChecked();
    if(configData->useGlobalSettings)
    {
        configData->senderAddress = globalAddressSetting->text();
        configData->senderReplyTo = globalReplyToSetting->text();
    }
    else
    {
        configData->senderAddress = senderAddressEdit->text();
        configData->senderReplyTo = senderReplyToEdit->text();
    }

    if( configData->enabled && !configData->isValid() )
    {
        KMessageBox::sorry(this, i18n("freedb submissions via SMTP have been disabled\n"
                                      "because the email details you have entered are\n"
                                      "incomplete. Please review your email settings\n"
                                      "and try again."), i18n("Freedb Submissions Disabled"));
        configData->enabled = false;
        enableCB->setChecked(false);
        enableClicked();
    }

} // commitData

void SMTPConfig::enableClicked(void)
{
    bool enable = enableCB->isChecked();

    smtpServerLabel->setEnabled(enable);
    serverHostLabel->setEnabled(enable);
    serverHostEdit->setEnabled(enable);
    serverPortLabel->setEnabled(enable);
    serverPortEdit->setEnabled(enable);

    globalRadio->setEnabled(enable);
    globalAddressLabel->setEnabled(enable);
    globalAddressSetting->setEnabled(enable);
    globalReplyToLabel->setEnabled(enable);
    globalReplyToSetting->setEnabled(enable);
    controlCenterLink->setEnabled(enable);

    localRadio->setEnabled(enable);
    senderAddressLabel->setEnabled(enable);
    senderAddressEdit->setEnabled(enable);
    senderReplyToLabel->setEnabled(enable);
    senderReplyToEdit->setEnabled(enable);

} // enableClicked

void SMTPConfig::updateGlobalSettings()
{
    SMTPConfigData globalData;
    globalData.loadGlobalSettings();
    globalAddressSetting->setText(globalData.senderAddress);
    globalReplyToSetting->setText(globalData.senderReplyTo);

} // updateGlobalSettings

void SMTPConfig::launchControlCenter()
{
    KApplication::kdeinitExec("kcmshell", "email");

} // launchControlCenter

#include <smtpconfig.moc>
