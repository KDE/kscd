/*
 *
 * kscd -- A simple CD player for the KDE project
 *
 * $Id$
 *
 * Copyright (C) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
 * wuebben@math.cornell.edu
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <qlayout.h>
#include <qfontmetrics.h>
#include <qvbox.h>

#include <kaboutdialog.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kwin.h>

#include "configdlg.h"
#include "configWidget.h"
#include "smtpconfig.h"
#include "CDDBSetup.h"
#include "kscd.h"
#include "version.h"

#if KSCDMAGIC
#include "mgconfdlg.h"
#endif

// little helper:
static inline QPixmap loadIcon( const char * name )
{
  return KGlobal::instance()->iconLoader()
    ->loadIcon( QString::fromLatin1(name), KIcon::NoGroup, KIcon::SizeMedium );
}

ConfigDlg::ConfigDlg(KSCD* player, const char* name, bool modal)
  :  KDialogBase(KDialogBase::IconList, i18n("CD Player Configuration"),
                 KDialogBase::Help |
                 KDialogBase::Ok |
                 KDialogBase::Apply |
                 KDialogBase::Cancel,
                 KDialogBase::Ok,
                 0, "configDialog", modal, true),
     mPlayer(player)
{
    setHelp(QString::null);
    KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
    connect(this, SIGNAL(finished()), this, SLOT(finis()));

    /*
     * kscd config page
     *
     */
    QVBox* page = addVBoxPage(i18n("CD Player"), i18n("KSCD Settings & Behavior"), loadIcon("kscd"));
    mKCSDConfig = new configWidget(mPlayer, page);
    
    /*
     * freedb page
     */
    page = addVBoxPage(QString("freedb"), i18n("Configure Fetching Items"), loadIcon("cdtrack"));
    mCDDBConfig = new CDDBSetup(page,"cddbsetupdialog");
    mPlayer->getCDDBOptions(mCDDBConfig);
    connect(mCDDBConfig, SIGNAL(updateCDDBServers()), mPlayer, SLOT(getCDDBservers()));
    connect(mCDDBConfig, SIGNAL(updateCurrentServer(const QString&)), 
            mPlayer, SLOT(updateCurrentCDDBServer(const QString&)));
    connect(mPlayer, SIGNAL(newServerList(const QStringList&)), 
            mCDDBConfig, SLOT(insertServerList(const QStringList&)));
    
    /*
     * SMTP page
     */
    page = addVBoxPage(QString("SMTP"), i18n("Mail Settings for Uploading CDDB Records"), loadIcon("email"));
    mSMTPConfig = new SMTPConfig(page, "smtpconfig", mPlayer->smtpData());

#if KSCDMAGIC
    /*
     * Magic page
     *
     * TODO: get an icon!
     */
    page = addVBoxPage(i18n("Magic"), i18n("KSCD Magic Display Settings"), loadIcon("kscdmagic"));
    MGConfigDlg* mMagicConfig;
    struct mgconfigstruct mgconfig;
    mPlayer->getMagicOptions(mgconfig);
    mMagicConfig = new MGConfigDlg(page, &mgconfig, "mgconfigdialg");
#endif
    
    /*
     * About page
     */
    page = addVBoxPage(i18n("Credits"), i18n("Primary Authors & Contributors"), loadIcon("help"));
    KAboutWidget* about = new KAboutWidget(page);
    about->setLogo(UserIcon("kscdlogo"));
    about->setVersion(KSCDVERSION);
    about->setMaintainer(i18n("Dirk Försterling"), "milliByte@gmx.net", QString::null, 
                         i18n("Workman library, current maintainer"));
    about->setAuthor("Bernd Johannes Wuebben", "wuebben@kde.org", QString::null, QString::null);
    about->addContributor("Aaron J. Seigo", "aseigo@olympusproject.org", QString::null, 
                          i18n("General UI issues"));
    about->addContributor("Steven Grimm", QString::null, QString::null, i18n("Workman library"));
    about->addContributor("Vadim Zaliva", QString::null, QString::null, i18n("HTTP proxy code"));
    about->addContributor("Paul Harrison", "pfh@yoyo.cc.monash.edu.au", QString::null, 
                          i18n("KSCD Magic based on Synaesthesia"));
    about->addContributor("freedb.org", QString::null, QString::null, 
                          i18n("Special thanks to freedb.org for "
                               "providing a free CDDB-like CD database"));
    about->adjust();
}

ConfigDlg::~ConfigDlg()
{
}

void ConfigDlg::updateGlobalSettings()
{
    mSMTPConfig->updateGlobalSettings();
}

void ConfigDlg::slotApply()
{
    mKCSDConfig->apply();
    mPlayer->setCDDBOptions(mCDDBConfig);
#if KSCDMAGIC
    mPlayer->setMagicOptions(*mMagicConfig->getData());
#endif
    mSMTPConfig->commitData();
}

void ConfigDlg::slotOk()
{
    slotApply();
    KDialogBase::slotOk();
}

void ConfigDlg::finis()
{
    delayedDestruct();
}

#include "configdlg.moc"




