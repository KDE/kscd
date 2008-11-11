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
#include <QBitmap>
#include <QPainter>
#include <klocalizedstring.h>

#include "panel.h"

using namespace Phonon;

KscdWindow::KscdWindow(QWidget *parent):QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);

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
// 	m_slider = new SeekSlider(this);
	m_bar = new SeekBar(this);
	m_panel = new Panel(this);
	m_stateTrackDialog = false;
	m_trackDlgCreated = false;
 	m_trackDlg = new TrackListDlg(parent);

// 	m_prefB = new ConfigButton(this);

 	m_move = false;
// 	createTrackWindow();

	setMask( m_backG->getPix().mask() );


	connect(m_stopB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_playB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_prevB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_nextB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_ejectB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_muteB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_randB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_loopB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_trackB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_volumeB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_volumeB,SIGNAL(volumeChange(qreal)),this,SIGNAL(actionVolume(qreal)));
	connect(m_trackDlg,SIGNAL(itemClicked(int)),this,SLOT(doubleClickedEvent(int)));
        connect( m_trackDlg, SIGNAL( trackListClosed() ), this, SLOT( closeTrackDialog() ) );
        connect(m_miniB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_closeB,SIGNAL(buttonClicked(const QString&)),this,SIGNAL(actionClicked(const QString&)));
	connect(m_volumeB,SIGNAL(volumeChange(qreal)),m_panel,SLOT(setVolumeDisplay(qreal)));
//	connect(m_prefB,SIGNAL(buttonClicked(QString)),this,SIGNAL(actionClicked(QString)));
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
// 	delete m_slider;
 	delete m_volumeB;
	delete m_loopB;
	delete m_randB;
// 	delete /*m_popUp*/;

	delete m_trackDlg;
}

void KscdWindow::closeTrackDialog()
{
	kDebug()<<"Close Track Dialog";
	m_stateTrackDialog = false;
	m_trackDlg->hide();
}

// void KscdWindow::clearTracklist()

void KscdWindow::createTrackDialog(QList<MBTrackInfo> & trackList,const QString  & albumTitle)
{
	QList<MBTrackInfo>::const_iterator it;
        QList<MBTrackInfo>::const_iterator end = trackList.constEnd();
	m_trackDlg->removeRowsTrackTable(trackList.size());

	m_stateTrackDialog = true;
	m_trackDlg->setAlbumLbl(albumTitle);
 	int trackNumber = 0;
	for(it = trackList.constBegin();it != end;++it)
	{
		m_trackDlg->addRowTrackTable(trackNumber);
		m_trackDlg->addItemTrackTable(trackNumber,0,QString::number(trackNumber+1));
		m_trackDlg->addItemTrackTable(trackNumber,1,(*it).Title);
		m_trackDlg->addItemTrackTable(trackNumber,2,(*it).Duration);
//		m_trackDlg->setYearLbl((*it).Year);
		trackNumber++;
	}
	m_trackDlg->moveTrackDialog(x(),y()+frameGeometry().height());
	m_trackDlg->show();
}

//Apply changes on kscdwidgets with new skin
void KscdWindow::setNewSkin(QString & newS){

	kDebug () << "make change with new skin :"<<newS;
	Prefs::setUrl(newS);
	Prefs::self()->writeConfig();
	kDebug () << "**** " << Prefs::url() << " ****";

	QSvgRenderer* rend = new QSvgRenderer(newS,this);
	this->resize(rend->boundsOnElement("kscdBack_default").width(),
			rend->boundsOnElement("kscdBack_default").height());

	m_backG->loadSkin(newS);
	m_stopB->loadSkin(newS);
	m_playB->loadSkin(newS);
	m_prevB->loadSkin(newS);
	m_nextB->loadSkin(newS);
	m_ejectB->loadSkin(newS);
	m_muteB->loadSkin(newS);
	m_randB->loadSkin(newS);
	m_loopB->loadSkin(newS);
	m_trackB->loadSkin(newS);
	m_volumeB->loadSkin(newS);
	m_closeB->loadSkin(newS);
	m_miniB->loadSkin(newS);
	m_panel->loadSkin(newS);
	sslider->move(m_bar->x(),m_bar->y()-5);
	sslider->setMaximumWidth(m_bar->width());

	sslider->setMinimumWidth(m_bar->width());

//  	(m_slider->cursor())->changeSkin(newS);
// 	(m_slider->bar())->changeSkin(newS);

//m_popUp->changeSkin(newS);;
	QRectF rect = rend->boundsOnElement("kscdBack_default");
	QPixmap pix(rect.toRect().size());
	pix.fill(QColor(Qt::transparent));
	QPainter p(&pix);
	rend->render(&p,"kscdBack_default",rect);
	setMask(pix.mask());
	delete rend;
}


void KscdWindow::doubleClickedEvent(int pos)
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
void KscdWindow::catchButton(QString & name)
{
	kDebug()<<"Catch :" << name;
	emit(actionClicked(name));
}

void KscdWindow::catchVolume(qreal value)
{
	emit(actionVolume(value));
}

void KscdWindow::changePicture(const QString & name,const QString & state)
{
	QString result;
	QString def = "default";
	if(name == "play")
	{
		result = "pause";
		m_playB->loadPicture(result,state);
		m_playB->setName(result);
	}
	else if(name == "pause")
	{
		result = "play";
		m_playB->loadPicture(result,state);
		m_playB->setName(result);
	}
	else if(name == "stop")
	{
		result = "play";
		m_stopB->loadPicture(name,state);
		m_playB->setName(result);
		QString tmp = m_playB->getName();
		m_playB->loadPicture(tmp,def);
	}
	else if(name == "eject")
	{
		result = "play";
		m_playB->setName(result);
		QString tmp = m_playB->getName();
		m_playB->loadPicture(tmp,def);
	}
	else if(name == "next")
	{
		m_nextB->loadPicture(name,state);
	}
	else if(name == "previous")
	{
		m_prevB->loadPicture(name,state);
	}
	else if(name == "mute")
	{
		m_muteB->loadPicture(name,state);
	}
	else if(name == "unmute")
	{
		m_muteB->loadPicture(name,state);
	}
	else if(name == "random")
	{
		m_randB->loadPicture(name,state);
	}
	else if(name == "p_random")
	{
		result = "random";
		def = "pressed";
		m_randB->loadPicture(result,def);
	}
	else if(name == "loop")
	{
		m_loopB->loadPicture(name,state);
	}
	else if(name == "looptrack")
	{
		m_loopB->loadPicture(name,state);
	}
	else if(name == "loopdisc")
	{
		m_loopB->loadPicture(name,state);
	}
	else if(name == "tracklist")
	{
		m_trackB->loadPicture(name,state);
	}
	else if(name == "close")
	{
		m_closeB->loadPicture(name,state);
	}
	else if(name == "minimize")
	{
		m_miniB->loadPicture(name,state);
	}
}
KscdWidget & KscdWindow::getPanel() const{
	return *m_panel;
}


//void  KscdWindow::paintEvent(QPaintEvent *event)
//{
//	QPainter painter(this);
//	painter.setBackgroundMode(Qt::TransparentMode);
//}


/**
 * Manages the Trackinfo Label
 */
void KscdWindow::showTrackinfoLabel(QString & infoStatus)
{
	m_panel->setTitle(infoStatus);
}

/**
 * Manages the Artist label
 */
void KscdWindow::showArtistLabel(QString  & infoStatus)
{
	m_panel->setAuthor(infoStatus);
}
void KscdWindow::showArtistAlbum(QString & infoStatus)
{
	m_panel->setAlbum(infoStatus);
}

void KscdWindow::setTime(qint64 pos){
	m_panel->setTime(pos);
// 	m_slider->setTime(pos);
}
void KscdWindow::panelInfo(const QString & mess)
{
	QString informationDisplay;
	if(mess == "loop")
	{
		m_panel->setLoop("");
        }
	if(mess == "looptrack")
	{
		m_panel->setLoop(i18n( "loop track  " ));
		//informationDisplay = "loop track  ";
	}
	if(mess == "loopdisc")
	{
		//informationDisplay = "loop disc  ";
		m_panel->setLoop(i18n( "loop disc  " ));
	}
	if(mess == "random")
	{
		m_panel->setRandom("");
	}
	if(mess == "p_random")
	{
		//informationDisplay += "random";
		m_panel->setRandom(i18nc( "This action allow the user to listen a random track","random"));
	}
	m_panel->displayInfo(m_panel->getLoop(),m_panel->getRandom());
}

void KscdWindow::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		event->accept();
		mousePosition = event->pos();
		m_move =true;
		grabMouse(Qt::SizeAllCursor);
	}
	else
	{
		event->ignore();
	}
}

void KscdWindow::mouseReleaseEvent(QMouseEvent *event)
{
	releaseMouse();
	m_move = false;
}

void KscdWindow::mouseMoveEvent(QMouseEvent * event)
{
	if(m_move == true)
	{
		event->accept();
		move(event->globalPos() - mousePosition);
		//emit(moveValue(event->globalPos() - mousePosition));
	}
	else
	{
		event->ignore();
	}
}

