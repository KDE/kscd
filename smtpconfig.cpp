/*
 * Kscd - A simple cd player for the KDE Project
 *
 * $Id$
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
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

SMTPConfig::SMTPConfig(QWidget *parent, const char *name, struct SMTPConfigData *_configData)
    : QDialog(parent, name)
{
    configData = _configData;
    QFontMetrics fm ( font() );
    
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

    serverHostLabel = new QLabel(i18n("SMTP Address:Port"), mainBox, "serverHostLabel");
    glay->addWidget ( serverHostLabel, 0, 0 );
    serverHostEdit = new QLineEdit(mainBox, "serverHostEdit");
    glay->addWidget ( serverHostEdit, 0, 1 );
    serverHostEdit->setText(configData->serverHost);
    serverHostEdit->setEnabled(configData->enabled);
    serverPortLabel = new QLabel(":", mainBox, "serverPortLabel");
    glay->addWidget ( serverPortLabel, 0, 2 );
    serverPortEdit = new QLineEdit(mainBox, "serverPortEdit");
    serverPortEdit->setFixedWidth ( 5 * fm.maxWidth() );
    glay->addWidget ( serverPortEdit, 0, 3 );
    serverPortEdit->setGeometry(475, 40, 45, 25);
    serverPortEdit->setText(configData->serverPort);
    serverPortEdit->setEnabled(configData->enabled);

    senderAddressLabel = new QLabel(i18n("Your Email Address"), mainBox, "senderAddressLabel");
    glay->addWidget ( senderAddressLabel, 1, 0 );
    senderAddressEdit = new QLineEdit(mainBox, "senderAddressEdit");
    glay->addMultiCellWidget ( senderAddressEdit, 1,1, 1,3 );
    senderAddressEdit->setText(configData->senderAddress);
    senderAddressEdit->setEnabled(configData->enabled);

    lay1->addStretch ( 1 );
}

void SMTPConfig::commitData(void)
{
    configData->enabled = enableCB->isChecked();
    configData->serverHost = serverHostEdit->text();
    configData->serverPort = serverPortEdit->text();
    configData->senderAddress = senderAddressEdit->text();
}

void SMTPConfig::enableClicked(void)
{
    bool c;

    c = enableCB->isChecked();
    serverHostEdit->setEnabled(c);
    serverPortEdit->setEnabled(c);
    senderAddressEdit->setEnabled(c);
}
#include <smtpconfig.moc>
