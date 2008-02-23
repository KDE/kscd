/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
 * Copyright (c) 2003-2006 Richard Lärkäng <nouseforaname@home.se>
 *
 * --------------
 * ISI KsCD Team :
 * --------------
 * Stanislas KRZYWDA <stanislas.krzywda@gmail.com>
 * Sovanramy Var <mastasushi@gmail.com>
 * Bouchikhi Mohamed-Amine <bouchikhi.amine@gmail.com>
 * Gastellu Sylvain<sylvain.gastellu@gmail.com>
 * -----------------------------------------------------------------------------
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef CONFIGWINDOW_H_
#define CONFIGWINDOW_H_

#include <QWidget>
#include <QMainWindow>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QList>
#include <QColor>
#include <QLineEdit>
#include <QComboBox>

#include <kcolorbutton.h>
#include <kdebug.h>

#include "../kscd.h"


class ConfigWindow:public QMainWindow
{
	Q_OBJECT

	KSCD * player;

	QWidget * confPage;
	QVBoxLayout * lConfPage;

	QTabWidget * tab;
	QWidget * hwPage;
	QWidget * panelPage;
	QWidget * scPage;

	QGridLayout * hwGrid;
	QGridLayout * panelGrid;
	QGridLayout * scGrid;

	QWidget * wButtons;
	QHBoxLayout * lButtons;

	QPushButton * bOk;
	QPushButton * bApply;
	QPushButton * bCancel;

	// Close your eyes XD
	KSCD * kscd;
	// You can open your eyes!

// Hardware Configuration

	QCheckBox * cbEject;
	QLabel * lEject;
	QComboBox * cbDriver;
	QLabel * lDriver;

// Panel Configuration
	QLabel * lPanelColor;
	KColorButton * cbPanel;

	QLabel * lTextColor;
	KColorButton * cbText;

// Shortcuts conf
	QLineEdit* playShortcut;
	QLineEdit* stopShortcut;
	QLineEdit* ejectShortcut;
	QLineEdit* nextShortcut;
	QLineEdit* previousShortcut;
	QLineEdit* volumeUpShortcut;
	QLineEdit* volumeDownShortcut;
	QLineEdit* randomShortcut;
	QLineEdit* loopTrackShortcut;
	QLineEdit* loopDiscShortcut;
	QLineEdit* trackListShortcut;
	QLineEdit* cddbWindowShortcut;
	QLineEdit* downloadInfoShortcut;
	QLineEdit* muteShortcut;
	QLineEdit* configureShortcut;
	
	QLabel* playLabel;
	QLabel* stopLabel;
	QLabel* ejectLabel;
	QLabel* nextLabel;
	QLabel* previousLabel;
	QLabel* volumeUpLabel;
	QLabel* volumeDownLabel;
	QLabel* randomLabel;
	QLabel* loopTrackLabel;
	QLabel* loopDiscLabel;
	QLabel* trackListLabel;
	QLabel* cddbWindowLabel;
	QLabel* downloadInfoLabel;
	QLabel* muteLabel;
	QLabel* configureLabel;
	
//------------------------------------


	QList<int> actionsCalled;

enum actions{
	Eject = 1,
	PanelColor = 2,
	TextColor = 3,
	PlayShortcut = 4,
	StopShortcut = 5,
	EjectShortcut = 6,
	NextShortcut = 7,
	PreviousShortcut = 8,
	VolumeUpShortcut = 9,
	VolumeDownShortcut = 10,
	RandomShortcut = 11,
	LoopTrackShortcut = 12,
	LoopDiscShortcut = 13,
	TrackListShortcut = 14,
	CDDBWindowShortcut = 15,
	DownloadInfoShortcut = 16,
	MuteShortcut = 17,
	ConfigureShortcut = 18,
	DriverChanged = 19
};


public:
	ConfigWindow(KSCD *parent);
	virtual ~ConfigWindow();

private:
	void setPanelConf();
	void setHardConfig();
	void setSCConfig();
	void applyAction(actions a);

signals:
	void ejectChanged(bool b);
	void panelColorChanged(QColor c);
	void textColorChanged(QColor c);
	void ShortcutChanged(QString name, QString key);

private slots:
	void apply();
	void ok();
	void cancel();
	void catchCBEject();
	void catchCBDriver();
	void catchPanelColor();
	void catchTextColor();
	void catchPlayShortcut();
	void catchStopShortcut();
	void catchEjectShortcut();
	void catchNextShortcut();
	void catchPreviousShortcut();
	void catchVolumeUpShortcut();
	void catchVolumeDownShortcut();
	void catchRandomShortcut();
	void catchLoopTrackShortcut();
	void catchLoopDiscShortcut();
	void catchTrackListShortcut();
	void catchCddbWindowShortcut();
	void catchDownloadInfoShortcut();
	void catchMuteShortcut();
	void catchConfigureShortcut();
		
};

#endif /*EJECTBUTTON_H_*/
