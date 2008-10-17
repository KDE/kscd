/*
   Kscd - A simple cd player for the KDE Project

   $Id: kscd.h 818722 2008-06-09 12:01:16Z krzywda $

   Copyright (c) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (c) 2002 Aaron J. Seigo <aseigo@kde.org>
   Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
   Copyright (c) 2008 Amine Bouchikhi <bouchikhi.amine@gmail.com>

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

#ifndef KSCD_H
#define KSCD_H

// Solid implementation
#include <qdialog.h>
#include <qapplication.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <QMenu>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPoint>
// Phonon libs
#include <phonon/phononnamespace.h>
#include <phonon/seekslider.h>

#include "gui/kscdwindow.h"
#include "hwcontroler.h"
#include "mbmanager.h"
#include <config-alsa.h>

#include <kapplication.h>
#include <kconfigdialog.h>
#include <krandomsequence.h>
#include <kglobalaccel.h>
#include <ksessionmanager.h>
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
#include <kaction.h>
#include "panel.h"

#include "prefs.h"
#include "ui_generalSettings.h"
#include "ui_interfaceSettings.h"

#include <kshortcutseditor.h>

class KSCD : public KscdWindow, public KSessionManager {

	Q_OBJECT
public:
	explicit KSCD(QWidget *parent = 0);
	~KSCD();

	virtual bool saveState(QSessionManager& sm);
	void writeSettings();
	HWControler * getDevices() const;

protected:
	void setupActions();
	void setupContextMenu();


signals:
	void picture(const QString&,const QString&);
	void infoPanel(const QString&);

public slots:


	void showContextMenu( const QPoint & );

	void restoreArtistLabel();
	void restoreTrackinfoLabel();
	void changeVolume(qreal);
	void playTrack(int);

	void ejectShortcut();
	void tracklistShortcut();
	void muteShortcut();
	void playShortcut();
	void randomShortcut();
	void looptrackShortcut();
	void loopdiscShortcut();
	void volumeUpShortcut();
	void volumeDownShortcut();
	void quitShortcut();
	void minimizeShortcut();
	void actionButton(const QString&);
	void catchtime(qint64 pos);

	//void setNewSkin(QString);
	void unsetHourglass();
	void configureShortcuts();

	/**
	* Open the config window
	*/
	void optionsPreferences();
	void updateSettings();
	void configureKeys();
private:
	HWControler* devices;
	MBManager* m_MBManager;

	bool mute;
	bool play;
	bool random;
	bool looptrack;
	bool loopdisc;

	QMenu *contextMenu;


	KAction* m_configureShortcutsAction;
	KAction* m_configureAction;
	KAction* m_playPauseAction;
	KAction* m_stopAction;
	KAction* m_nextAction;
	KAction* m_previousAction;
	KAction* m_ejectAction;
	KAction* m_randomAction;
	KAction* m_looptrackAction;
	KAction* m_loopdiscAction;
	KAction* m_tracklistAction;
	KAction* m_muteAction;
	KAction* m_downloadAction;
	KAction* m_uploadAction;
	KAction* m_CDDBWindowAction;
	KAction* m_volumeUpAction;
	KAction* m_volumeDownAction;
	KAction* m_quitAction;
	KAction* m_minimizeAction;

	KActionCollection * m_actions;
	void setHourglass();

	// Settings.
	Ui::generalSettings ui_general;
	Ui::interfaceSettings ui_interface;

	/**
	 * Load the last settings
	 */
	void loadSettings();

};

#endif
