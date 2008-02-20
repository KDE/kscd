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
#include "configwindow.h"
#include <kconfigdialog.h>

ConfigWindow::ConfigWindow(QWidget * parent):QMainWindow(parent)
{

	confPage = new QWidget(this);
	lConfPage = new QVBoxLayout(this);

	confPage->setLayout(lConfPage);

	tab = new QTabWidget(this);


	wButtons= new QWidget(this) ;
	lButtons= new QHBoxLayout(this);
	wButtons->setLayout(lButtons);

	lConfPage->addWidget(tab);
	lConfPage->addWidget(wButtons);


	resize(500,600);
	hwPage = new QWidget(tab);
	panelPage = new QWidget(tab);
	scPage = new QWidget(tab);

	setHardConfig();
	setPanelConf();
	setSCConfig();

	tab->addTab(hwPage,"Hardware");
	tab->addTab(panelPage,"Panel");
	tab->addTab(scPage,"Shortcuts");

	setCentralWidget(confPage);



	bOk = new QPushButton("Ok",this);
	bApply = new QPushButton("Apply",this);
	bCancel = new QPushButton("Cancel",this);

	lButtons->addWidget(bOk);
	connect(bOk,SIGNAL(clicked()),this,SLOT(ok()));
	lButtons->addWidget(bApply);
	connect(bApply,SIGNAL(clicked()),this,SLOT(apply()));
	lButtons->addWidget(bCancel);

	

}

ConfigWindow::~ConfigWindow(){
	delete confPage;
	delete lConfPage;
	delete tab;
	delete hwPage;
	delete panelPage;
	delete scPage;
	delete hwGrid;
	delete panelGrid;
	delete scGrid;
	delete wButtons;
	delete lButtons;
	delete bOk;
	delete bApply;
	delete bCancel;
	delete cbEject;
	delete lEject;
	delete lPanelColor;
	delete cbPanel;
	delete lTextColor;
	delete cbText;
}

void ConfigWindow::setPanelConf(){
	panelGrid = new QGridLayout(panelPage);
	panelPage->setLayout(panelGrid);

	cbPanel = new KColorButton(QColor(Qt::black),this);
	lPanelColor = new QLabel("Choose Panel Color : ",this);

	panelGrid->addWidget(lPanelColor, 0, 0);
	panelGrid->addWidget(cbPanel, 0, 1);

	cbText = new KColorButton(QColor(Qt::white),this);
	lTextColor = new QLabel("Choose Text Color : ",this);

	panelGrid->addWidget(lTextColor, 1, 0);
	panelGrid->addWidget(cbText, 1, 1);

	connect(cbPanel,SIGNAL(changed ( const QColor )),this,SLOT(catchPanelColor()));
	connect(cbText,SIGNAL(changed ( const QColor )),this,SLOT(catchTextColor()));

}
void ConfigWindow::setHardConfig(){
	hwGrid = new QGridLayout(hwPage);
	hwPage->setLayout(hwGrid);

	cbEject = new QCheckBox(this);
	lEject = new QLabel("Eject the CD at the end of the disc",this);

	hwGrid->addWidget(cbEject, 0, 0);
	hwGrid->addWidget(lEject, 0, 1);

	connect(cbEject,SIGNAL(stateChanged ( int )),this,SLOT(catchCBEject()));
	

}
void ConfigWindow::setSCConfig(){
	scGrid = new QGridLayout(scPage);
	scPage->setLayout(scGrid);
	
	playShortcut = new QLineEdit;
	stopShortcut = new QLineEdit;
	ejectShortcut = new QLineEdit;
	nextShortcut = new QLineEdit;
	previousShortcut = new QLineEdit;
	volumeUpShortcut = new QLineEdit;
	volumeDownShortcut = new QLineEdit;
	randomShortcut = new QLineEdit;
	loopTrackShortcut = new QLineEdit;
	loopDiscShortcut = new QLineEdit;
	trackListShortcut = new QLineEdit;
	cddbWindowShortcut = new QLineEdit;
	downloadInfoShortcut = new QLineEdit;
	muteShortcut = new QLineEdit;
	configureShortcut = new QLineEdit;

	playLabel = new QLabel("Play/Pause",this);
	stopLabel = new QLabel("Stop",this);
	ejectLabel = new QLabel("Eject",this);
	nextLabel = new QLabel("Next Track",this);
	previousLabel = new QLabel("Previous Track",this);
	volumeUpLabel = new QLabel("Volume Up",this);
	volumeDownLabel = new QLabel("Volume Down",this);
	randomLabel = new QLabel("Random",this);
	loopTrackLabel = new QLabel("Loop Track",this);
	loopDiscLabel = new QLabel("Loop Disc",this);
	trackListLabel = new QLabel("Tracklist",this);
	cddbWindowLabel = new QLabel("CDDB Window",this);
	downloadInfoLabel = new QLabel("Downlod Info",this);
	muteLabel = new QLabel("Mute",this);
	configureLabel = new QLabel("Configure KsCD",this);
	
	scGrid->addWidget(playLabel, 0, 0);
	scGrid->addWidget(playShortcut, 0, 1);
	scGrid->addWidget(stopLabel, 1, 0);
	scGrid->addWidget(stopShortcut, 1, 1);
	scGrid->addWidget(ejectLabel, 2, 0);
	scGrid->addWidget(ejectShortcut, 2, 1);
	scGrid->addWidget(nextLabel, 3, 0);
	scGrid->addWidget(nextShortcut, 3, 1);
	scGrid->addWidget(previousLabel, 4, 0);
	scGrid->addWidget(previousShortcut, 4, 1);
	scGrid->addWidget(volumeUpLabel, 5, 0);
	scGrid->addWidget(volumeUpShortcut, 5, 1);
	scGrid->addWidget(volumeDownLabel, 6, 0);
	scGrid->addWidget(volumeDownShortcut, 6, 1);
	scGrid->addWidget(randomLabel, 7, 0);
	scGrid->addWidget(randomShortcut, 7, 1);
	scGrid->addWidget(loopTrackLabel, 8, 0);
	scGrid->addWidget(loopTrackShortcut, 8, 1);
	scGrid->addWidget(loopDiscLabel, 9, 0);
	scGrid->addWidget(loopDiscShortcut, 9, 1);
	scGrid->addWidget(trackListLabel, 10, 0);
	scGrid->addWidget(trackListShortcut, 10, 1);
	scGrid->addWidget(cddbWindowLabel, 11, 0);
	scGrid->addWidget(cddbWindowShortcut, 11, 1);
	scGrid->addWidget(downloadInfoLabel, 12, 0);
	scGrid->addWidget(downloadInfoShortcut, 12, 1);
	scGrid->addWidget(muteLabel, 13, 0);
	scGrid->addWidget(muteShortcut, 13, 1);
	scGrid->addWidget(configureLabel, 14, 0);
	scGrid->addWidget(configureShortcut, 14, 1);

	connect (playShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchPlayShortcut()));
	connect (stopShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchStopShortcut()));
	connect (ejectShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchEjectShortcut()));
	connect (nextShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchNextShortcut()));
	connect (previousShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchPreviousShortcut()));
	connect (volumeUpShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchVolumeUpShortcut()));
	connect (volumeDownShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchVolumeDownShortcut()));
	connect (randomShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchRandomShortcut()));
	connect (loopTrackShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchLoopTrackShortcut()));
	connect (loopDiscShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchLoopDiscShortcut()));
	connect (trackListShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchTrackListShortcut()));
	connect (cddbWindowShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchCddbWindowShortcut()));
	connect (downloadInfoShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchDownloadInfoShortcut()));
	connect (muteShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchMuteShortcut()));
	connect (configureShortcut, SIGNAL(textChanged ( const QString & )),this,SLOT(catchConfigureShortcut()));
	
}
void ConfigWindow::apply(){
	for(int index = 0; index < actionsCalled.size(); index++){
		applyAction((actions)actionsCalled[index]);

	}
	actionsCalled.clear();
}
void ConfigWindow::applyAction(actions a){
	switch(a){
		case Eject:
			emit(ejectChanged(cbEject->isChecked()));
			break;
		case PanelColor:
			emit(panelColorChanged(cbPanel->color()));
			break;
		case TextColor:
			emit(textColorChanged(cbText->color()));
			break;
		case PlayShortcut:
			kDebug()<<"EMIT play!";
			emit(ShortcutChanged(playLabel->text(),playShortcut->text()));
			break;
		case StopShortcut:
			kDebug()<<"EMIT stop!";
			emit(ShortcutChanged(stopLabel->text(),stopShortcut->text()));
			break;
		case EjectShortcut:
			kDebug()<<"EMIT eject!";
			emit(ShortcutChanged(ejectLabel->text(),ejectShortcut->text()));
			break;
		case NextShortcut:
			kDebug()<<"EMIT next!";
			emit(ShortcutChanged(nextLabel->text(),nextShortcut->text()));
			break;
		case PreviousShortcut:
			kDebug()<<"EMIT previous!";
			emit(ShortcutChanged(previousLabel->text(),previousShortcut->text()));
			break;
		case VolumeUpShortcut:
			kDebug()<<"EMIT volumeUp!";
			emit(ShortcutChanged(volumeUpLabel->text(),volumeUpShortcut->text()));
			break;
		case VolumeDownShortcut:
			kDebug()<<"EMIT volumeDown!";
			emit(ShortcutChanged(volumeDownLabel->text(),volumeDownShortcut->text()));
			break;
		case RandomShortcut:
			kDebug()<<"EMIT random!";
			emit(ShortcutChanged(randomLabel->text(),randomShortcut->text()));
			break;
		case LoopTrackShortcut:
			kDebug()<<"EMIT loopTrack!";
			emit(ShortcutChanged(loopTrackLabel->text(),loopTrackShortcut->text()));
			break;
		case LoopDiscShortcut:
			kDebug()<<"EMIT loopDisc!";
			emit(ShortcutChanged(loopDiscLabel->text(),loopDiscShortcut->text()));
			break;
		case TrackListShortcut:
			kDebug()<<"EMIT trackList!";
			emit(ShortcutChanged(trackListLabel->text(),trackListShortcut->text()));
			break;
		case CDDBWindowShortcut:
			kDebug()<<"EMIT cddbWindow!";
			emit(ShortcutChanged(cddbWindowLabel->text(),cddbWindowShortcut->text()));
			break;
		case DownloadInfoShortcut:
			kDebug()<<"EMIT downloadInfo!";
			emit(ShortcutChanged(downloadInfoLabel->text(),downloadInfoShortcut->text()));
			break;
		case MuteShortcut:
			kDebug()<<"EMIT mute!";
			emit(ShortcutChanged(muteLabel->text(),muteShortcut->text()));
			break;
		case ConfigureShortcut:
			kDebug()<<"EMIT configure!";
			emit(ShortcutChanged(configureLabel->text(),configureShortcut->text()));
			break;
		
	}

}
void ConfigWindow::catchCBEject(){
	actionsCalled.append(Eject);
	kDebug()<<"check baby check!";
}
void ConfigWindow::ok(){
	apply();
	hide();
}
void ConfigWindow::catchPanelColor(){
	actionsCalled.append(PanelColor);
	kDebug()<<"user has chosen a new panel color";
}
void ConfigWindow::catchTextColor(){
	actionsCalled.append(TextColor);
	kDebug()<<"user has chosen a new text color";
}
void ConfigWindow::catchPlayShortcut(){
	actionsCalled.append(PlayShortcut);
	kDebug()<<"Play/Pause Shortcut catched";
}
void ConfigWindow::catchStopShortcut(){
	actionsCalled.append(StopShortcut);
	kDebug()<<"Stop Shortcut catched";
}
void ConfigWindow::catchEjectShortcut(){
	actionsCalled.append(EjectShortcut);
	kDebug()<<"Eject Shortcut catched";
}
void ConfigWindow::catchNextShortcut(){
	actionsCalled.append(NextShortcut);
	kDebug()<<"Next Shortcut catched";
}
void ConfigWindow::catchPreviousShortcut(){
	actionsCalled.append(PreviousShortcut);
	kDebug()<<"Previous Shortcut catched";
}
void ConfigWindow::catchVolumeUpShortcut(){
	actionsCalled.append(VolumeUpShortcut);
	kDebug()<<"VolumeUp Shortcut catched";
}
void ConfigWindow::catchVolumeDownShortcut(){
	actionsCalled.append(VolumeDownShortcut);
	kDebug()<<"VolumeDown Shortcut catched";
}
void ConfigWindow::catchRandomShortcut(){
	actionsCalled.append(RandomShortcut);
	kDebug()<<"Random Shortcut catched";
}
void ConfigWindow::catchLoopTrackShortcut(){
	actionsCalled.append(LoopTrackShortcut);
	kDebug()<<"LoopTrack Shortcut catched";
}
void ConfigWindow::catchLoopDiscShortcut(){
	actionsCalled.append(LoopDiscShortcut);
	kDebug()<<"LoopDisc Shortcut catched";
}
void ConfigWindow::catchTrackListShortcut(){
	actionsCalled.append(TrackListShortcut);
	kDebug()<<"TrackList Shortcut catched";
}
void ConfigWindow::catchCddbWindowShortcut(){
	actionsCalled.append(CDDBWindowShortcut);
	kDebug()<<"CDDBWindow Shortcut catched";
}
void ConfigWindow::catchDownloadInfoShortcut(){
	actionsCalled.append(DownloadInfoShortcut);
	kDebug()<<"DownloadInfo Shortcut catched";
}
void ConfigWindow::catchMuteShortcut(){
	actionsCalled.append(MuteShortcut);
	kDebug()<<"Mute Shortcut catched";
}
void ConfigWindow::catchConfigureShortcut(){
	actionsCalled.append(ConfigureShortcut);
	kDebug()<<"Configure Shortcut catched";
}
