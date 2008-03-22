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

#include <QCursor>

// Phonon libs
#include <phonon/phononnamespace.h>
#include <phonon/seekslider.h>

#include "ihm/kscdwindow.h"
#include "hwcontroler.h"
#include "mbmanager.h"
#include "prefs.h"
// #include "configWidget.h"
#include <config-alsa.h>
#include <QLCDNumber>

#include <kapplication.h>
#include <kconfigdialog.h>
#include <k3process.h>
#include <krandomsequence.h>
#include <kglobalaccel.h>
#include <ksessionmanager.h>
// #include <kcompactdisc.h>
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
#include "panel.h"

#include "prefs.h"
#include "ui_generalSettings.h"
#include "ui_interfaceSettings.h"
// class CDDBDlg;
// class DockWidget;
// class QGridLayout;
// class KActionCollection;
// class KToggleAction;
#include <kshortcutseditor.h>

// enum  	ActionType {
// 		WidgetAction = Qt::WidgetShortcut, 
// 		WindowAction = Qt::WindowShortcut, 
// 		ApplicationAction = Qt::ApplicationShortcut, 
// 		GlobalAction = 4, 
// 		AllActions = 0xffffffff
// 	}
// enum  	LetterShortcuts {
// 		LetterShortcutsDisallowed = 0, 
// 		LetterShortcutsAllowed
// 	}



class KSCD : public KscdWindow, public KSessionManager {

	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KSCD")

private:
	HWControler* devices;	
	MBManager* m_MBManager;
	
	bool mute;
	bool play;
	bool random;
	bool looptrack;
	bool loopdisc;
	
	QAction* configure_shortcuts;
	QAction* configure;
	QAction* play_pause_shortcut;
	QAction* stop_shortcut;
	QAction* next_shortcut;
	QAction* previous_shortcut;
	QAction* eject_shortcut;
	QAction* random_shortcut;
	QAction* looptrack_shortcut;
	QAction* loopdisc_shortcut;
	QAction* tracklist_shortcut;
	QAction* mute_shortcut;
	QAction* DownloadAction;
	QAction* UploadAction;
	QAction* CDDBWindowAction;
	QAction* volume_up_shortcut;
	QAction* volume_down_shortcut;
	QAction* quit_shortcut;
	QAction* minimize_shortcut;
	KActionCollection * m_actions;
	QMenu * rightClick;

	void setHourglass();

	// Settings.
	Ui::generalSettings ui_general;
	Ui::interfaceSettings ui_interface;

	/**
	 * Load the last settings 
	 */
	void loadSettings();

public:
	explicit KSCD(QWidget *parent = 0);
	~KSCD();
	
	virtual bool saveState(QSessionManager& sm);
	void writeSettings();
	HWControler * getDevices();
// 	KCompactDisc* getCd(); // kept for CDDB compatibility
protected:
	void setupActions();
	void setDefaultShortcuts();

signals:
	void picture(QString,QString);
	void infoPanel(QString);
	/* Popup signals */
// 	void showTitlePopUp(QString, QString);
// 	void hideTitlePopUp();

public slots:

	void test();
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
	void actionButton(QString);
	void setShortcut(QString, QString);
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
};

#endif
