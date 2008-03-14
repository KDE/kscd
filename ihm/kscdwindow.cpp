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
#include "kscdwindow.h"
#include <QString>
#include <QTextEdit>
#include <klocalizedstring.h>

#include "panel.h"
#include "configwindow.h"

KscdWindow::KscdWindow(QWidget *parent):QWidget(parent)
{	
	setWindowFlags(Qt::FramelessWindowHint);
	setAutoFillBackground(false);

	m_backG = new BackGround(this);
	m_stopB = new StopButton(this);
	m_playB = new PlayButton(this);
	m_nextB = new NextButton(this);
	m_prevB = new PreviousButton(this);
	m_ejectB = new EjectButton(this);
	m_muteB = new MuteButton(this);
	m_randB = new RandomButton(this);
	m_loopB = new LoopButton(this);
	m_trackB = new TrackListButton(this);
	m_volumeB = new VolumeButton(this);
	m_closeB = new CloseButton(this);
	m_miniB = new MinimizeButton(this);
	m_slider = new SeekSlider(this);
	m_panel = new Panel(this);
	m_stateTrackDialog = false;
	m_trackDlgCreated = false;
 	m_trackDlg = new TrackListDlg(parent);
	
// 	createTrackWindow();

// 	setMask(*m_backG->bitmap());


	m_finderSkin= new FinderSkin(this); //New finder skin dialog created at the begining

	// Configuration windows
	//ConfigWindow *m_config = new ConfigWindow(this);

// 	QGridLayout* panelLayout = new QGridLayout;
// 	m_layout->addLayout(panelLayout, 0, 3, 2, 1);

	connect(m_stopB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_playB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_prevB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_nextB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_ejectB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_muteB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_randB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_loopB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_trackB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_volumeB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_volumeB,SIGNAL(volumeChange(qreal)),SLOT(catchVolume(qreal)));
	connect(m_trackDlg,SIGNAL(itemClicked(int)),this,SLOT(doubleClickedEvent(int)));
	connect(m_miniB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_closeB,SIGNAL(buttonClicked(QString)),SLOT(catchButton(QString)));
	connect(m_backG,SIGNAL(moveValue(QPoint)),this,SLOT(moveWindow(QPoint)));
	connect(m_volumeB,SIGNAL(volumeChange(qreal)),m_panel,SLOT(setVolumeDisplay(qreal)));
	show();
}

KscdWindow::~KscdWindow()
{

	delete m_playB;
	delete m_stopB;
	delete m_nextB;
	delete m_prevB;
	delete m_ejectB;
	delete m_muteB;
	delete m_trackB;
	delete m_panel;
	delete m_closeB;
	delete m_backG;
	delete m_miniB;
	delete m_slider;
	delete m_miniB;
 	delete m_volumeB;
	delete m_loopB;
	delete m_randB;
// 	delete /*m_popUp*/;

	delete m_trackDlg;
	delete m_finderSkin;
}

// void KscdWindow :: paintEvent(QPaintEvent* event)
// {
// 	QPainter painter(this);
// 	painter.setPen(Qt::NoPen);
// 	painter.setBrush(Qt::transparent);
// 
// }

void KscdWindow :: closeTrackDialog()
{
	kDebug()<<"Close Track Dialog";
	m_stateTrackDialog = false;
	m_trackDlg->hide();
}

void KscdWindow :: createTrackDialog(QList<MBTrackInfo> trackList,QString albumTitle)
{
	QList<MBTrackInfo>::iterator it;
	m_trackDlg->removeRowsTrackTable(trackList.size());

	m_stateTrackDialog = true;
	m_trackDlg->setAlbumLbl(albumTitle);
 	int trackNumber = 1;
	for(it = trackList.begin();it != trackList.end();it++)
	{
		m_trackDlg->addRowTrackTable(trackNumber-1);
		m_trackDlg->addItemTrackTable(trackNumber-1,0,QString::number(trackNumber));
		m_trackDlg->addItemTrackTable(trackNumber-1,1,(*it).Title);
		m_trackDlg->addItemTrackTable(trackNumber-1,2,(*it).Duration);
//		m_trackDlg->setYearLbl((*it).Year);
		trackNumber++;
	}
	m_trackDlg->moveTrackDialog(x(),y()+frameGeometry().height());
	m_trackDlg->show();
}

void KscdWindow :: makeFinderSkinDialog()
{
	kDebug()<<"kscdWindow:createFinderSkinDialog";
 	m_finderSkin->show();
}

//Apply changes on kscdwidgets with new skin
void KscdWindow::setNewSkin(QString newS){
	kDebug () << "make change with new skin :"<<newS;
	
	QSvgRenderer* rend = new QSvgRenderer(newS,this);
	this->resize(rend->boundsOnElement("kscd_default").width(),
			rend->boundsOnElement("kscd_default").height());

	m_backG->changeSkin(newS);
	m_stopB->changeSkin(newS);
	kDebug()<<"name play1:"<<m_playB->getName();
	kDebug()<<"name id1:"<<m_playB->getId();
	m_playB->changeSkin(newS);	
	m_prevB->changeSkin(newS);
	m_nextB->changeSkin(newS);
	m_ejectB->changeSkin(newS);
	m_muteB->changeSkin(newS);
	m_randB->changeSkin(newS);
	m_loopB->changeSkin(newS);
	m_trackB->changeSkin(newS);
	m_volumeB->changeSkin(newS);
	m_closeB->changeSkin(newS);
	m_miniB->changeSkin(newS);
	m_panel->changeSkin(newS);
 	(m_slider->cursor())->changeSkin(newS);
	(m_slider->bar())->changeSkin(newS);	

//m_popUp->changeSkin(newS);;
		
}


void KscdWindow :: doubleClickedEvent(int pos)
{
	kDebug()<<"signal recu\n"<<"pos clicked:"<<pos;
 	emit(trackClicked(pos));
}

// void KscdWindow::addSeekSlider(Phonon::SeekSlider *ss)
// {
// 	m_layout->addWidget((QWidget*)ss, 2, 3,1,2);
// }


/**
* hide the title popUp
*/
// void KscdWindow::hideTitlePopUp()
// {
// 	if(m_titlePopUp != NULL)
// 	{
// 		m_titlePopUp->hide();
// 	}
// }

/**
 * Links treatments with the UI
 */
void KscdWindow :: catchButton(QString name)
{
	kDebug()<<"Catch :" << name;
	emit(actionClicked(name));
}

void KscdWindow :: catchVolume(qreal value)
{
	emit(actionVolume(value));
}

void KscdWindow :: moveWindow(QPoint value)
{
	move(value);
}

void KscdWindow::changePicture(QString name,QString state)
{
	if(name == "play")
	{
		m_playB->loadPicture("pause",state);
		m_playB->setName("pause");
	}
	if(name == "pause")
	{
		m_playB->loadPicture("play",state);
		m_playB->setName("play");
	}
	if(name == "stop")
	{
		m_stopB->loadPicture(name,state);
		m_playB->setName("play");
		m_playB->loadPicture(m_playB->getName(),"default");
	}
	if(name == "eject")
	{
		m_ejectB->loadPicture(name,state);
		m_playB->setName("play");
		m_playB->loadPicture(m_playB->getName(),"default");
	}
	if(name == "next")
	{
		m_nextB->loadPicture(name,state);
	}
	if(name == "previous")
	{
		m_prevB->loadPicture(name,state);
	}
	if(name == "mute")
	{
		m_muteB->loadPicture(name,state);
	}
	if(name == "unmute")
	{
		m_muteB->loadPicture(name,state);
	}
	if(name == "random")
	{
		m_randB->loadPicture(name,state);
	}
	if(name == "p_random")
	{
		m_randB->loadPicture("random","pressed");	
	}
	if(name == "loop")
	{
		m_loopB->loadPicture(name,state);
	}
	if(name == "looptrack")
	{
		m_loopB->loadPicture(name,state);
	}
	if(name == "loopdisc")
	{
		m_loopB->loadPicture(name,state);
	}
	if(name == "tracklist")
	{
		m_trackB->loadPicture(name,state);
	}
	if(name == "close")
	{
		m_closeB->loadPicture(name,state);
	}
	if(name == "minimize")
	{
		m_miniB->loadPicture(name,state);
	}
}
KscdWidget * KscdWindow::getPanel(){
	return m_panel;
}


void  KscdWindow::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setBackgroundMode(Qt::TransparentMode);
}


/**
 * Manages the Trackinfo Label
 */
void KscdWindow::showTrackinfoLabel(QString infoStatus)
{
	m_panel->setTitle(&infoStatus);
}

/**
 * Manages the Artist label
 */
void KscdWindow::showArtistLabel(QString infoStatus)
{
	m_panel->setAuthor(&infoStatus);
}
void KscdWindow::showArtistAlbum(QString infoStatus)
{
	m_panel->setAlbum(&infoStatus);
}

void KscdWindow::setTime(qint64 pos){
	m_panel->setTime(pos);
}
void KscdWindow::panelInfo(QString mess)
{
	QString informationDisplay;
	if(mess == "loop")
	{
		m_panel->setLoop("");
		//m_panel->setLoop("");
	}
	if(mess == "looptrack")
	{
		m_panel->setLoop("loop track  ");
		//m_panel->setLoop("loop track  ");
		//informationDisplay = "loop track  ";
	}
	if(mess == "loopdisc")
	{
		//m_panel->setLoop("loop disc  ");
		//informationDisplay = "loop disc  ";
		m_panel->setLoop("loop disc  ");
	}	
	if(mess == "random")
	{
		//m_panel->setRandom("");
		m_panel->setRandom("");
	}
	if(mess == "p_random")
	{
		//m_panel->setRandom("random");
		//informationDisplay += "random";
		m_panel->setRandom("random");
	}
	m_panel->displayInfo(m_panel->getLoop(),m_panel->getRandom());
}

