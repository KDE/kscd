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
#include <kapp.h>

#include <smtpconfig.h>

#include <qlayout.h>
#include <qfontmetrics.h>

#include <kemailsettings.h>
#include <kmessagebox.h>

SMTPConfig::SMTPConfig(QWidget *parent, const char *name, struct SMTPConfigData *_configData)
    : QDialog(parent, name)
{
    configData = _configData;
    QFontMetrics fm ( font() );
    kes = new KEMailSettings();

    kes->setProfile( configData->mailProfile );
    configData->serverHost = kes->getSetting( KEMailSettings::OutServer );
    configData->serverPort = "25";
    configData->senderAddress = kes->getSetting( KEMailSettings::EmailAddress );
    configData->senderReplyTo = kes->getSetting( KEMailSettings::ReplyToAddress );
    // Don't accept obviously bogus settings.
    if( (configData->serverHost == "") || (!configData->senderAddress.contains("@")))
      {
	configData->enabled = false;
      }
    

    QBoxLayout * lay1 = new QVBoxLayout ( this, 10 );
    mainBox = new QGroupBox(this, "mainBox");
    lay1->addWidget ( mainBox );

    QBoxLayout * lay2 = new QVBoxLayout ( mainBox, 10 );
    enableCB = new QCheckBox(i18n("Enable submission via SMTP"), mainBox, "enableCB");
    lay2->addWidget ( enableCB );
    enableCB->setChecked(configData->enabled);
    connect(enableCB, SIGNAL(clicked()), this, SLOT(enableClicked()));

    QGridLayout * glay = new QGridLayout ( lay2, 2, 4, 5 );
    glay->setColStretch ( 1, 1 );

    mailProfileLabel = new QLabel(i18n("Current E-Mail Profile"), mainBox, "mailProfileLabel");
    glay->addWidget ( mailProfileLabel, 0, 0 );
    mailProfileCombo = new KComboBox( FALSE, mainBox, "mailProfileCombo" );
    glay->addMultiCellWidget( mailProfileCombo, 0,0, 1,3);
    mailProfileCombo->insertStringList(kes->profiles());
    mailProfileCombo->setEnabled(configData->enabled);
    connect(mailProfileCombo, SIGNAL(activated(const QString &)), this, SLOT(mailProfileChanged(const QString &)));
    // *yuck*
    int i = 0;
    for( i=0; i < mailProfileCombo->count(); i++ )
      {
	if( configData->mailProfile == mailProfileCombo->text(i) )
	  {
	    mailProfileCombo->setCurrentItem( i );
	  }
      }

    serverHostLabel = new QLabel(i18n("SMTP Address:Port"), mainBox, "serverHostLabel");
    glay->addWidget ( serverHostLabel, 1, 0 );
    serverHostEdit = new QLineEdit(mainBox, "serverHostEdit");
    glay->addWidget ( serverHostEdit, 1, 1 );
    serverHostEdit->setText(configData->serverHost);
    serverHostEdit->setEnabled(configData->enabled);
    serverHostEdit->setReadOnly( true );
    serverPortLabel = new QLabel(":", mainBox, "serverPortLabel");
    glay->addWidget ( serverPortLabel, 1, 2 );
    serverPortEdit = new QLineEdit(mainBox, "serverPortEdit");
    serverPortEdit->setFixedWidth ( 5 * fm.maxWidth() );
    glay->addWidget ( serverPortEdit, 1, 3 );
    serverPortEdit->setGeometry(475, 40, 45, 25);
    serverPortEdit->setText(configData->serverPort);
    serverPortEdit->setEnabled(configData->enabled);
    serverPortEdit->setReadOnly( true );

    senderAddressLabel = new QLabel(i18n("Your Email Address"), mainBox, "senderAddressLabel");
    glay->addWidget ( senderAddressLabel, 2, 0 );
    senderAddressEdit = new QLineEdit(mainBox, "senderAddressEdit");
    glay->addMultiCellWidget ( senderAddressEdit, 2,2, 1,3 );
    senderAddressEdit->setText(configData->senderAddress);
    senderAddressEdit->setEnabled(configData->enabled);
    senderAddressEdit->setReadOnly( true );

    senderReplyToLabel = new QLabel(i18n("Your Reply Address"), mainBox, "senderReplyToLabel");
    glay->addWidget ( senderReplyToLabel, 3, 0 );
    senderReplyToEdit = new QLineEdit(mainBox, "senderReplyToEdit");
    glay->addMultiCellWidget ( senderReplyToEdit, 3,3, 1,3 );
    senderReplyToEdit->setText(configData->senderReplyTo);
    senderReplyToEdit->setEnabled(configData->enabled);
    senderReplyToEdit->setReadOnly( true );
    
    lay1->addStretch ( 1 );
}

void SMTPConfig::commitData(void)
{
    configData->enabled = enableCB->isChecked();
    configData->serverHost = serverHostEdit->text();
    configData->serverPort = serverPortEdit->text();
    configData->senderAddress = senderAddressEdit->text();
    configData->senderReplyTo = senderReplyToEdit->text();
    configData->mailProfile = mailProfileCombo->currentText();
    if( (configData->serverHost == "") || (!configData->senderAddress.contains("@")))
      {
	KMessageBox::sorry(this, i18n("CDDB submissions via SMTP have been disabled\n"
				      "because the E-Mail profile you selected is\n"
				      "incomplete. Please review your E-Mail settings\n"
				      "and try again."), i18n("CDDB submissions disabled"));
	configData->enabled = false;
      } 
} // commitData

void SMTPConfig::enableClicked(void)
{
    bool c;

    c = enableCB->isChecked();
    mailProfileCombo->setEnabled(c);
    serverHostEdit->setEnabled(c);
    serverPortEdit->setEnabled(c);
    senderAddressEdit->setEnabled(c);
    senderReplyToEdit->setEnabled(c);
} // enableClicked

void SMTPConfig::mailProfileChanged( const QString &name )
{
    kes->setProfile( name );
    configData->serverHost = kes->getSetting( KEMailSettings::OutServer );
    configData->senderAddress = kes->getSetting( KEMailSettings::EmailAddress );
    configData->senderReplyTo = kes->getSetting( KEMailSettings::ReplyToAddress );
    serverHostEdit->setText( configData->serverHost );
    senderAddressEdit->setText( configData->senderAddress );
    senderReplyToEdit->setText( configData->senderReplyTo );
} // mailProfileChanged

#include <smtpconfig.moc>
