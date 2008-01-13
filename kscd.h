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

#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QCloseEvent>

// CD support.
class KCompactDisc;

// CDDB support via libkcddb
#include <libkcddb/kcddb.h>
#include <libkcddb/client.h>

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
#include "ledlamp.h"
#include "ui_panel.h"
#include "hwcontroler.h"
#include "prefs.h"
#include "configWidget.h"
#include <kapplication.h>
#include <kconfigdialog.h>
#include <k3process.h>
#include <krandomsequence.h>
#include <kglobalaccel.h>
#include <ksessionmanager.h>

#include "ihm/kscdwindow.h"

class CDDBDlg;
class DockWidget;
class QGridLayout;
class KActionCollection;
class KToggleAction;

class KSCD : public KscdWindow, public KSessionManager {

	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KSCD")

public:
	explicit KSCD(QWidget *parent = 0);
	~KSCD();
	virtual bool saveState(QSessionManager& sm);
	void writeSettings();

protected:
	void populateSongList();

private:
	HWControler* devices;

	KCompactDisc* m_cd; // kept for CDDB compatibility

	// Info from CDDB
	CDDBDlg* m_cddialog;
	KCDDB::CDInfo m_cddbInfo;
	KCDDB::Client* m_cddb;


signals:
	void picture(QString,StateButton);
	void CDDBClicked();

public slots:
	void actionButton(QString);
	void lookupCDDB();

private slots:
	void CDDialogSelected();
	void CDDialogDone();
	void lookupCDDBDone(KCDDB::Result);
	void setCDInfo(KCDDB::CDInfo);
	void showArtistLabel(QString);
	void restoreArtistLabel();

};

#endif
