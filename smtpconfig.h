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

#ifndef SMTPCONFIG_H
#define SMTPCONFIG_H

#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <kcombobox.h>
#include <kemailsettings.h>

struct SMTPConfigData 
{
//public:
    bool enabled;
    QString serverHost;
    QString serverPort;
    QString senderAddress;
    QString senderReplyTo;
    QString mailProfile;
};

class SMTPConfig:public QWidget
{
    Q_OBJECT
public:

    SMTPConfig(QWidget *parent = NULL, const char *name = NULL, struct SMTPConfigData *_configData = NULL);
    ~SMTPConfig() {};

public slots:
    void commitData();
    void enableClicked();
    void mailProfileChanged(const QString &name);

signals:

protected:
    QGroupBox       *mainBox;
    QCheckBox       *enableCB;
    QLabel          *mailProfileLabel;
    KComboBox       *mailProfileCombo;
    QLabel          *serverHostLabel;
    QLineEdit       *serverHostEdit;
    QLabel          *serverPortLabel;
    QLineEdit       *serverPortEdit;
    QLabel          *senderAddressLabel;
    QLineEdit       *senderAddressEdit;
    QLabel          *senderReplyToLabel;
    QLineEdit       *senderReplyToEdit;
    KEMailSettings  *kes;

private:
    SMTPConfigData *configData;
};
#endif
