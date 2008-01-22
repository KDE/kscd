/*
   Kscd - A simple cd player for the KDE Project

   $Id$

   Copyright (c) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (c) 2002 Aaron J. Seigo <aseigo@kde.org>
   Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __KSCD__
#define __KSCD__

// Solid implementation
#include <QLabel>
#include <qdialog.h>
#include <qapplication.h>
#include <QTimer>
#include <QComboBox>
#include <qscrollbar.h>
#include <qslider.h>
#include <QToolTip>
#include <QMenu>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QMenu>
#include <QtDBus>
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QCloseEvent>

// CD support.
class KCompactDisc;

// CDDB support via libkcddb
#include <libkcddb/cddb.h>
#include <libkcddb/kcddb.h>
#include <libkcddb/client.h>

#include <phonon/phononnamespace.h>
#include <phonon/seekslider.h>

#include "ihm/kscdwindow.h"
#include "hwcontroler.h"
#include "cddbmanager.h"
#include "prefs.h"
#include "cddbdlg.h"
#include "configWidget.h"
#include "docking.h"
#include <config-alsa.h>


#include <kapplication.h>
#include <kconfigdialog.h>
#include <k3process.h>
#include <krandomsequence.h>
#include <kglobalaccel.h>
#include <ksessionmanager.h>
#include <kcompactdisc.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kcmdlineargs.h>
#include <khelpmenu.h>
#include <kshortcutsdialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <kuniqueapplication.h>
#include <kcmoduleloader.h>
#include <ktoolinvocation.h>

class CDDBDlg;
class DockWidget;
class QGridLayout;
class KActionCollection;
class KToggleAction;

class KSCD : public KscdWindow, public KSessionManager {

	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KSCD")

private:
	HWControler* devices;
	KCompactDisc* m_cd; // kept for CDDB compatibility
	CDDBManager* m_cddbManager;
	KConfigDialog* configDialog;

public:
	explicit KSCD(QWidget *parent = 0);
	~KSCD();
	virtual bool saveState(QSessionManager& sm);
	void writeSettings();

protected:
	void populateSongList();

signals:
	void picture(QString,QString);

public slots:
	void test();
	void refreshCDDB();
	void lookupCDDB();
	void restoreArtistLabel();
	void restoreTrackinfoLabel();
	void actionButton(QString);

};

#endif
