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
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <qlayout.h>
#include <qfontmetrics.h>
#include <qvbox.h>

#include <kaboutdialog.h>
#include <kapplication.h>
#include <kcmoduleloader.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kwin.h>

#include "configdlg.h"
#include "configWidget.h"
#include "smtpconfig.h"
#include "kscd.h"
#include "version.h"

#ifdef KSCDMAGIC
#include "mgconfdlg.h"
#endif

// little helper:
static inline QPixmap loadIcon( const char * name )
{
  return KGlobal::instance()->iconLoader()
    ->loadIcon( QString::fromLocal8Bit(name), KIcon::NoGroup, KIcon::SizeMedium );
}

ConfigDlg::ConfigDlg(KSCD* player, const char*, bool modal)
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
     * libkcddb page
     *
     */

    KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
    if (libkcddb->isValid())
    {
        KCModuleInfo info(libkcddb->desktopEntryPath(), "settings");
        if (info.service()->isValid())
        {
            KCModule *m = KCModuleLoader::loadModule(info);
            if (m)
            {
                m->load();
                page = addVBoxPage(QString("freedb"), i18n("Configure Fetching Items"), loadIcon("cdtrack"));
                m->reparent(page, 0, QPoint(0, 0));
                connect(this, SIGNAL(okClicked()), m, SLOT(save()));
                connect(this, SIGNAL(applyClicked()), m, SLOT(save()));
                connect(this, SIGNAL(defaultClicked()), m, SLOT(defaults()));
                page->setStretchFactor(m, 1);
            }
        }
    }

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
