/*
 *
 * ConfigDlg - the configuration dialog for kscd
 *
 * $Id$
 *
 * Copyright (C) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
 * 
 * there was a class of the same name written in 1997 by 
 * Bernd Johannes Wuebben, but it was rewritten.
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef _CONFIG_DLG_H_
#define _CONFIG_DLG_H_

#include <kdialogbase.h>

class KSCD;
class configWidget;
class CDDBSetup;
class SMTPConfig;
class MGConfigDlg;

class ConfigDlg : public KDialogBase 
{
    Q_OBJECT

    public:
        ConfigDlg(KSCD* player, const char* name = 0, bool modal = false);
        ~ConfigDlg();

        /*
         * TODO: remove this last of the mo-hack-ans someday with cleverness
         */
        CDDBSetup* cddb() { return mCDDBConfig; }

        void updateGlobalSettings();

    protected slots: 
        void slotApply();
        void slotOk();
        void finis();

    private:
        configWidget* mKCSDConfig;
        CDDBSetup* mCDDBConfig;
        SMTPConfig* mSMTPConfig;
        KSCD* mPlayer;
#ifdef KSCDMAGIC
        MGConfigDlg* mMagicConfig;
#endif

        enum pages
        {
            KSCDPAGE  = 0,
            CDDBPAGE  = 1,
            SMTPPAGE  = 2,
            ABOUTPAGE = 3
        };
};
#endif
