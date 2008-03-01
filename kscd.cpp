/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
 * Copyright (c) 2003-2006 Richard Lärkäng <nouseforaname@home.se>
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
#include "kscd.h"
#include "ihm/configwindow.h"

using namespace Phonon;
using namespace KCDDB;
class CDDB;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;

KSCD::KSCD( QWidget *parent ) : KscdWindow(parent)
{
/**
 * Hourglass
 */
	setHourglass();
/**
   *************************/	


	QString commande = "cp "+KStandardDirs::installPath("data") + "kscd/ihm/skin/*.TTF ~/.fonts/";
	commande.replace("/ihm/","/");
	char * chemin = (char *)malloc(1024 * sizeof(char));
	kDebug () << commande;
	strcpy(chemin, commande.toAscii().data());
	system(chemin);
	//system("cp "+KStandardDirs::installPath("data") + "/kscd/ihm/skin/TRANGA__.TTF ~/.fonts/TRANGA__.TTF");
  
	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);

/**
	 * General
 */
	connect(this,SIGNAL(actionClicked(QString)), this, SLOT(actionButton(QString)));
	connect(this,SIGNAL(picture(QString,QString)), this, SLOT(changePicture(QString,QString)));
	
	connect(this,SIGNAL(trackClicked(int)), this, SLOT(playTrack(int)));
	connect(this,SIGNAL(actionVolume(qreal)), this, SLOT(changeVolume(qreal)));

	devices = new HWControler();
	connect(devices,SIGNAL(currentTime(qint64)),this,SLOT(catchtime(qint64)));
	connect(this,SIGNAL(infoPanel(QString)),this,SLOT(panelInfo(QString)));
//  	addSeekSlider(new Phonon::SeekSlider(devices->getMedia()));
// 	Phonon::VolumeSlider * vs = new Phonon::VolumeSlider(devices->getAudioOutPut());
// 	vs->setOrientation(Qt::Vertical);
// 	vs->setMuteVisible(false);
// 	addVolumeSlider(vs);


/**
	 * CDDB
 */
// // TODO kept for CDDB compatibility
// // TODO deactivate CDDB options if no disc
//  	m_cd = new KCompactDisc();
//  	m_cd->setDevice(Prefs::cdDevice(), 50, false, QString("phonon"), Prefs::audioDevice());
	// 
// 	// CDDB Initialization
// 	m_cddbManager = new CDDBManager(this);
	
	// Music Brainz initialisation
	m_MBManager = new MBManager();
	// TODO move lookup to cd detected
	m_MBManager->discLookup();
	
	connect(m_MBManager, SIGNAL(showArtistLabel(QString)), this, SLOT(showArtistLabel(QString)));
	connect(m_MBManager, SIGNAL(showTrackinfoLabel(QString)), this, SLOT(showTrackinfoLabel(QString)));
	
// 	connect(m_cddbManager, SIGNAL(showArtistLabel(QString)), this, SLOT(showArtistLabel(QString)));
// 	connect(m_cddbManager, SIGNAL(showTrackinfoLabel(QString)), this, SLOT(showTrackinfoLabel(QString)));
// 	connect(m_cddbManager, SIGNAL(restoreArtistLabel()), this, SLOT(restoreArtistLabel()));
// 	connect(m_cddbManager, SIGNAL(restoreTrackinfoLabel()), this, SLOT(restoreTrackinfoLabel()));
	
	connect(devices,SIGNAL(trackChanged()),this,SLOT(restoreTrackinfoLabel()));
// 	connect(devices,SIGNAL(cdLoaded()),m_cddbManager,SLOT(refreshCDDB()));
	connect(devices,SIGNAL(cdLoaded()),m_MBManager,SLOT(discLookup()));

/**
	 * Contextual Menu
 */
	// Set context menu policy to ActionsContextMenu
	setContextMenuPolicy(Qt::ActionsContextMenu);

// 	CDDBWindowAction = new QAction(i18n("CDDB..."), this);
// 	addAction(CDDBWindowAction);
// 	connect(CDDBWindowAction, SIGNAL(triggered()), m_cddbManager, SLOT(CDDialogSelected()));
// 	//shortcut
// 	CDDBWindowAction->setShortcut(tr("w"));

	DownloadAction = new QAction(i18n("Download Information"), this);
	addAction(DownloadAction);
	connect(DownloadAction, SIGNAL(triggered()), m_MBManager, SLOT(discLookup()));
	//shortcut
	DownloadAction->setShortcut(tr("d"));
	
	UploadAction = new QAction(i18n("Upload Information"), this);
	addAction(UploadAction);
	connect(UploadAction, SIGNAL(triggered()), m_MBManager, SLOT(infoDisplay()));
	//shortcut
	UploadAction->setShortcut(tr("u"));

	ConfigWindow * conf = new ConfigWindow(this);

	connect(conf,SIGNAL(ejectChanged(bool)),devices,SLOT(setEjectActivated(bool)));
	connect(conf,SIGNAL(textSizeChanged(QString)),getPanel(),SLOT(setTextSize(QString)));
	connect(conf,SIGNAL(textColorChanged(QColor)),getPanel(),SLOT(setTextColor(QColor)));
	//Find skin --> Two ways of change
	connect(conf, SIGNAL(pathSkinChanged(QString)),this,SLOT(setNewSkin(QString)));
	connect(m_finderSkin,SIGNAL(pathSkinChanged(QString)),this,SLOT(setNewSkin(QString)));

	configure = new QAction(i18n("Configure..."), this);
	addAction(configure);
	configure->setShortcut(tr("c"));
	connect(configure, SIGNAL(triggered()), conf, SLOT(show()));

// Experimental Function
	QAction* test = new QAction(i18n("test"), this);
	addAction(test);
	connect(test, SIGNAL(triggered()), this, SLOT(test()));

//Find out skin
	QAction* findS = new QAction(i18n("find out skin"), this);
	addAction(findS);
	connect(findS, SIGNAL(triggered()), this, SLOT(showFinderSkin()));

//////////Set Shortcuts
	setDefaultShortcuts();
	mute = false;
	play = false;
	random = false;
	looptrack = false;
	loopdisc = false;
	//For User Shortcuts Configuration
	connect(conf,SIGNAL(ShortcutChanged(QString,QString)),this,SLOT(setShortcut(QString,QString)));
}

KSCD::~KSCD()
{
	delete devices;
// 	delete m_cd;
// 	delete m_cddbManager;
//	delete w_titlePopUp;	//deleting of the title popup

/*	delete m_cddialog;
	delete m_cddb;*/
}

//Apply changes on kscdwidgets with new skin
void KSCD::setNewSkin(QString newS){
	kDebug () << "make change with new skin :"<<newS;
	
	QSvgRenderer* rend = new QSvgRenderer(newS,this);
	this->resize(rend->boundsOnElement("kscd_default").width(),
				 rend->boundsOnElement("kscd_default").height());

	m_backG->changeSkin(newS);
	m_stopB->changeSkin(newS);
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
// 	m_cursor->changeSkin(newS);
	//m_popUp->changeSkin(newS);;
		
}


void KSCD::setContextualMenu()
{
	// TODO move from Kscd() to here
	
}

void KSCD::test()
{
	//kDebug () << "total time : " << devices->getTotalTime() ;

	m_MBManager->infoDisplay();
}

/**
 * Find out new skin
 */
void KSCD::showFinderSkin()
{
	kDebug () << "Find out a new skin : begining";
	makeFinderSkinDialog();
}



/**
 * CDDB Management
 */

void KSCD::restoreArtistLabel()
{
// 	kDebug() << "NbTracks CDDB = " << m_cddbManager->getCddbInfo().numberOfTracks();
// 	kDebug() << "NbTracks Devices = " << devices->getTotalTrack();

	if( devices->getCD()->isCdInserted() && devices->isDiscValid() )
	{
		QString artist, title;

// 		if (m_cddbManager->getCddbInfo().isValid()/* && m_cddbManager->getCddbInfo().numberOfTracks() == devices->getTotalTrack()*/) {
// 			artist = m_cddbManager->getCddbInfo().get(KCDDB::Artist).toString();
// 			title = m_cddbManager->getCddbInfo().get(KCDDB::Title).toString();
// 		}
// 		else
// 		{
// 			artist = i18n("Unknown artist");
// 			title = i18n("Unknown album");
// 		}
		artist = m_MBManager->getDiscInfo().Artist;
		title = m_MBManager->getDiscInfo().Title;
		showArtistLabel(QString("%1").arg(artist));
		showArtistAlbum(QString("%1").arg(title));
	}
	else
	{
		showArtistLabel(i18n(""));
	}

}

void KSCD::restoreTrackinfoLabel()
{
	QString title, length ;

	// If disc is inserted
	if (devices->getCD()->isCdInserted())
	{
		title = QString("%1 - ").arg(devices->getCurrentTrack(), 2, 10, QLatin1Char('0')) ;

// 		if (m_cddbManager->getCddbInfo().isValid()/* && m_cddbManager->getCddbInfo().numberOfTracks() == devices->getTotalTrack()*/)
// 		{
// 			title.append(m_cddbManager->getCddbInfo().track(devices->getCurrentTrack()-1).get(KCDDB::Title).toString());
// 			length.append(m_cddbManager->getCddbInfo().track(devices->getCurrentTrack()-1).get(KCDDB::Length).toString());
// 		}
// 		else
// 		{
// 			title.append(i18n("unknown"));
// 			length.append(i18n("duration"));
// 		}
		title.append(m_MBManager->getTrackList()[devices->getCurrentTrack()-1].Title);
		length.append(m_MBManager->getTrackList()[devices->getCurrentTrack()-1].Duration);
		
		showTrackinfoLabel(title);

		//shows the title popup with title info and title lenght
// 		m_titlePopUp->showTitlePopUp(title, length);
	  //programming title popup hiding
// 		QTimer::singleShot(5000, this, SLOT(hideTitlePopUp()));
	}
	else
	{
		showTrackinfoLabel(title);
	}
}
void KSCD::changeVolume(qreal value)
{
	kDebug()<<"changeVolume enter "<<value;
	devices->setVolume(value);
}

void KSCD::setDefaultShortcuts()
{
	//play/pause
	play_pause_shortcut = new QAction(i18n("play"), this);
	addAction(play_pause_shortcut);
	play_pause_shortcut->setShortcut(tr("Space"));
	connect(play_pause_shortcut, SIGNAL(triggered()), this, SLOT(playShortcut()));
	
	//stop
	stop_shortcut = new QAction(i18n("stop"), this);
	addAction(stop_shortcut);
	stop_shortcut->setShortcut(tr("s"));
	connect(stop_shortcut, SIGNAL(triggered()), devices, SLOT(stop()));

	//next
	next_shortcut = new QAction(i18n("next"), this);
	addAction(next_shortcut);
	next_shortcut->setShortcut(tr("Right"));
	connect(next_shortcut, SIGNAL(triggered()), devices, SLOT(nextTrack()));

	//previous
	previous_shortcut = new QAction(i18n("previous"), this);
	addAction(previous_shortcut);
	previous_shortcut->setShortcut(tr("Left"));
	connect(previous_shortcut, SIGNAL(triggered()), devices, SLOT(prevTrack()));

	//eject
	eject_shortcut = new QAction(i18n("eject"), this);
	addAction(eject_shortcut);
	eject_shortcut->setShortcut(tr("e"));
	connect(eject_shortcut, SIGNAL(triggered()), devices, SLOT(eject()));

	//volume up
	volume_up_shortcut = new QAction(i18n("volume_up"), this);
	addAction(volume_up_shortcut);
	volume_up_shortcut->setShortcut(tr("Up"));
	connect(volume_up_shortcut, SIGNAL(triggered()), this, SLOT(volumeUpShortcut()));

	//volume down
	volume_down_shortcut = new QAction(i18n("volume_down"), this);
	addAction(volume_down_shortcut);
	volume_down_shortcut->setShortcut(tr("Down"));
	connect(volume_down_shortcut, SIGNAL(triggered()), this, SLOT(volumeDownShortcut()));

	//random
	random_shortcut = new QAction(i18n("random"), this);
	addAction(random_shortcut);
	random_shortcut->setShortcut(tr("r"));
	connect(random_shortcut, SIGNAL(triggered()), this, SLOT(randomShortcut()));

	//looptrack
	looptrack_shortcut = new QAction(i18n("looptrack"), this);
	addAction(looptrack_shortcut);
	looptrack_shortcut->setShortcut(tr("l"));
	connect(looptrack_shortcut, SIGNAL(triggered()), this, SLOT(looptrackShortcut()));

	//loopdisc
	loopdisc_shortcut = new QAction(i18n("loopdisc"), this);
	addAction(loopdisc_shortcut);
	loopdisc_shortcut->setShortcut(tr("Ctrl+l"));
	connect(loopdisc_shortcut, SIGNAL(triggered()), this, SLOT(loopdiscShortcut()));

	//download info
	//Done in constructor

	//cddb window
	//Done in constructor

	//Configure KsCD
	//Done in constructor	

	//tracklist
	tracklist_shortcut = new QAction(i18n("tracklist"), this);
	addAction(tracklist_shortcut);
	tracklist_shortcut->setShortcut(tr("t"));
	connect(tracklist_shortcut, SIGNAL(triggered()), this, SLOT(tracklistShortcut()));

	//mute
	mute_shortcut = new QAction(i18n("mute"), this);
	addAction(mute_shortcut);
	mute_shortcut->setShortcut(tr("m"));
	connect(mute_shortcut, SIGNAL(triggered()), this, SLOT(muteShortcut()));

}

void KSCD::setShortcut(QString name, QString key)
{
	kDebug()<<"SetShortcut, "<<name<<" : "<<key;
	if (name == "Play/Pause")
	{
		play_pause_shortcut->setShortcut(key);
	}

	if (name == "Stop")
	{
		stop_shortcut->setShortcut(key);
	}

	if (name == "Next Track")
	{
		next_shortcut->setShortcut(key);
	}

	if (name == "Previous Track")
	{
		previous_shortcut->setShortcut(key);
	}

	if (name == "Eject")
	{
		eject_shortcut->setShortcut(key);
	}

	if (name == "Random")
	{
		random_shortcut->setShortcut(key);
	}

	if (name == "Loop Track")
	{
		looptrack_shortcut->setShortcut(key);
	}

	if (name == "Loop Disc")
	{
		loopdisc_shortcut->setShortcut(key);
	}

	if (name == "Tracklist")
	{
		tracklist_shortcut->setShortcut(key);
	}

	if (name == "Mute")
	{
		mute_shortcut->setShortcut(key);
	}

	if (name == "Download Info")
	{
		DownloadAction->setShortcut(key);
	}

	if (name == "CDDB Window")
	{
		CDDBWindowAction->setShortcut(key);
	}
	
	if (name == "Volume Up")
	{
		volume_up_shortcut->setShortcut(key);
	}
	
	if (name == "Volume Down")
	{
		volume_down_shortcut->setShortcut(key);
	}

	if (name == "Configure KsCD")
	{
		configure->setShortcut(key);
	}

}

void KSCD::tracklistShortcut()
{
	actionButton("tracklist");
}

void KSCD::muteShortcut()
{
	if (!mute)
	{
		actionButton("unmute");
		emit(picture("unmute","default"));
		//mute = !mute;
	}
	else
	{
		actionButton("mute");
		emit(picture("mute","default"));
		//mute = !mute;
	}
}

void KSCD::playShortcut()
{
	if (!play)
	{
		actionButton("play");
		emit(picture("play","default"));
		//play = !play;
	}
	else
	{
		actionButton("pause");
		emit(picture("pause","default"));
		//play = !play;
	}
}

void KSCD::randomShortcut()
{
	if (!random)
	{
		actionButton("p_random");
		emit(picture("p_random","default"));
		emit(infoPanel("p_random"));

		//random = !random;
	}
	else
	{
		actionButton("random");
		emit(picture("random","default"));
		emit(infoPanel("random"));
		//random = !random;
	}
}

void KSCD::looptrackShortcut()
{
	if (!looptrack)
	{
		actionButton("looptrack");
		emit(picture("looptrack","default"));
		emit(infoPanel("looptrack"));

		//looptrack = !looptrack;
	}
	else
	{
		actionButton("loop");
		emit(picture("loop","default"));
		emit(infoPanel("loop"));

		//looptrack = !looptrack;
	}
}

void KSCD::loopdiscShortcut()
{
	if (!loopdisc)
	{
		actionButton("loopdisc");
		emit(picture("loopdisc","default"));
		emit(infoPanel("loopdisc"));

		//loopdisc = !loopdisc;
	}
	else
	{
		actionButton("loop");
		emit(picture("loop","default"));
		emit(infoPanel("loop"));

		//loopdisc = !loopdisc;
	}
}

void KSCD::volumeUpShortcut()
{
	if (devices->getVolume()<=0.99)
		this->changeVolume(devices->getVolume()*100+1);
}

void KSCD::volumeDownShortcut()
{
	if (devices->getVolume()>=0.01)
		this->changeVolume(devices->getVolume()*100-1);
}


void KSCD::playTrack(int track)
{
	kDebug()<<"playtrack enter "<<track;
	devices->play(track);
	emit(picture("play","default"));
}

/**
 * Link IHM with actions
 */
void KSCD::actionButton(QString name)
{
	QString state = "over";
	if(name=="play")
	{
		if( !devices->isDiscValid() || !devices->getCD()->isCdInserted())
		{
			if(!devices->getCD()->isCdInserted())
				showArtistLabel(i18n("No disc"));
			else
				showArtistLabel(i18n("Invalid disc"));
			QTimer::singleShot(2000, this, SLOT(restoreArtistLabel()));
		}
		else
		{
			if((devices->getState() == StoppedState) || (devices->getState()) == PausedState)
			{
				devices->play();
				restoreTrackinfoLabel();
				restoreArtistLabel();
			}
		}
		emit(picture(name,state));
		play = !play;
	}
	if(name=="pause")
	{
		if( !devices->isDiscValid() || !devices->getCD()->isCdInserted())
		{
			if(!devices->getCD()->isCdInserted())
				showArtistLabel(i18n("No disc"));
			else
				showArtistLabel(i18n("Invalid disc"));
			QTimer::singleShot(2000, this, SLOT(restoreArtistLabel()));
		}
		else{
			if(devices->getState() == PlayingState)
			{
				devices->pause();
			}
		}
		emit(picture(name,state));
		play = !play;
	}
	if(name=="next")
	{
		if( !devices->isDiscValid() || !devices->getCD()->isCdInserted())
		{
			if(!devices->getCD()->isCdInserted())
				showArtistLabel(i18n("No disc"));
			else
				showArtistLabel(i18n("Invalid disc"));
			QTimer::singleShot(2000, this, SLOT(restoreArtistLabel()));
		}
		else
		{
			devices->nextTrack();
			restoreTrackinfoLabel();
			if((devices->getState() == StoppedState) || (devices->getState() == PausedState))
			{
				devices->stop(false);
			}
			if ((devices->getState() == PlayingState))
			{
				devices->play();
				
			}
		}
		emit(picture(name,state));
	}
	if(name=="previous")
	{
		if( !devices->isDiscValid() || !devices->getCD()->isCdInserted()) 
		{
			if(!devices->getCD()->isCdInserted())
				showArtistLabel(i18n("No disc"));
			else
				showArtistLabel(i18n("Invalid disc"));
			QTimer::singleShot(2000, this, SLOT(restoreArtistLabel()));
		}
		else
		{
			devices->prevTrack();
			restoreTrackinfoLabel();
			
			if((devices->getState() == StoppedState) || (devices->getState() == PausedState))
			{
				devices->stop(false);
			}
			if ((devices->getState() == PlayingState))
			{
				devices->play();

			}
		}
		emit(picture(name,state));
	}
	if(name=="stop")
	{
		if ((devices->getState() == PlayingState)|| (devices->getState() == PausedState))
		{
			devices->stop();
		}
		emit(picture(name,state));
	}
	if(name=="eject")
	{
		devices->eject();
		emit(picture(name,state));
		if ((devices->getState() == PlayingState)|| (devices->getState() == PausedState))
		{
			devices->stop();
		}
	}
	if(name=="mute")
	{
		devices->mute(false);
		emit(picture(name,state));
		mute = !mute;
	}
	if(name=="unmute")
	{
		devices->mute(true);
		emit(picture(name,state));
		mute = !mute;
	}
	if(name == "random")
	{
		kDebug() << 5;
		devices->setRandom(false);
		emit(picture(name,state));
		emit(infoPanel("random"));

		random = !random;
	}
	if(name == "p_random")
	{
		kDebug() << 6;
		devices->setRandom(true);
		emit(picture(name,state));
		emit(infoPanel("p_random"));

		random = !random;
	}
	if(name == "loop")
	{
		devices->setLoopMode(NoLoop);
		emit(picture(name,state));
		emit(infoPanel("loop"));

		looptrack = false;
		loopdisc = false;
	}
	if(name == "looptrack")
	{
		devices->setLoopMode(LoopOne);
		emit(picture(name,state));
		emit(infoPanel("looptrack"));

		looptrack = true;
		loopdisc = false;
	}
	if(name == "loopdisc")
	{
		devices->setLoopMode(LoopAll);
		emit(picture(name,state));
		emit(infoPanel("loopdisc"));

		loopdisc = true;
		looptrack = false;
	}
	if(name=="minimize")
	{
		showMinimized ();
		emit(picture(name,state));
	}
	if(name=="close")
	{
		close();
		emit(picture(name,state));
	}
	if(name == "tracklist")
	{
		if(m_stateTrackDialog == true)
		{
			kDebug()<<"close track window";
			closeTrackDialog();
		}
		else
		{
			//createTrackDialog(m_cddbManager->getTrackList(),m_cddbManager->getDiscTitle());
			createTrackDialog(m_MBManager->getTrackList(),m_MBManager->getDiscInfo().Title);
			kDebug()<<"open track window";
		}
		emit(picture(name,"default"));
	}
}

/**
 * Hourglass
 */
void KSCD::setHourglass()
{
	this->setCursor(Qt::WaitCursor);
	QTimer::singleShot(8000, this, SLOT(unsetHourglass()));
}
void KSCD::unsetHourglass()
{
	this->unsetCursor();
}

/**
 * Configuration
 */
void KSCD::writeSettings()
{
	Prefs::self()->writeConfig();
}


/**
 * Accessors
 */
HWControler* KSCD::getDevices()
{
	return devices;
}

// KCompactDisc* KSCD::getCd()
// {
// // 	return m_cd;
// }

/**
 * Save state on session termination
 */
bool KSCD::saveState(QSessionManager& /*sm*/)
{
	writeSettings();
	KConfigGroup config(KApplication::kApplication()->sessionConfig(), "General");
	config.writeEntry("Show", isVisible());
	return true;
}

/**
 * main()
 */
int main( int argc, char *argv[] )
{
	KAboutData aboutData("kscd", 0, ki18n("KsCD"),
						 "1.5", ki18n(description),
									  KAboutData::License_GPL,
		   ki18n("(c) 2001, Dirk Försterling\n(c) 2003, Aaron J. Seigo"));
	aboutData.addAuthor(ki18n("Aaron J. Seigo"), ki18n("Current maintainer"), "aseigo@kde.org");
	aboutData.addAuthor(ki18n("Alexander Kern"),ki18n("Workman library update, CDTEXT, CDDA"), "kernalex@kde.org");
	aboutData.addAuthor(ki18n("Bernd Johannes Wuebben"),KLocalizedString(), "wuebben@kde.org");
	aboutData.addAuthor(ki18n("Dirk Försterling"), ki18n("Workman library, previous maintainer"), "milliByte@gmx.net");
	aboutData.addCredit(ki18n("Wilfried Huss"), ki18n("Patches galore"));
	aboutData.addCredit(ki18n("Steven Grimm"), ki18n("Workman library"));
	aboutData.addCredit(ki18n("Sven Lueppken"), ki18n("UI Work"));
	aboutData.addCredit(ki18n("freedb.org"), ki18n("Special thanks to freedb.org for providing a free CDDB-like CD database"), 0, "http://freedb.org");
    
	KCmdLineArgs::init( argc, argv, &aboutData );

	KCmdLineOptions options;
	options.add("s");
	options.add("start", ki18n("Start playing"));
	options.add("+[device]", ki18n("CD device, can be a path or a media:/ URL"));
	KCmdLineArgs::addCmdLineOptions(options);
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	if (!KUniqueApplication::start())
	{
		fprintf(stderr, "kscd is already running\n");
		if (args->count() > 0 || args->isSet("start"))
		{
			QDBusInterface kscd("org.kde.kscd", "/CDPlayer", "org.kde.kscd.CDPlayer");
            // Forward the command line args to the running instance.
			if (args->count() > 0)
			{
				kscd.call( "setDevice",  QString(args->arg(0)));
			}
			if (args->isSet("start"))
			{
				kscd.call("play");
			}
		}
		exit(0);
	}
	KUniqueApplication a;
	KSCD *k = new KSCD();
	a.setTopWidget( k );
//   a.setMainWidget(k);

	k->setWindowTitle(KGlobal::caption());

	if (kapp->isSessionRestored())
	{
		KConfigGroup group(KApplication::kApplication()->sessionConfig(), "General");
		if (group.readEntry("Show", false))
			k->show();
	}
	else
	{
		k->show();
	}

	if (args->count() > 0)
		Prefs::self()->setCdDevice(args->arg(0));

	return a.exec();
}

void KSCD::catchtime(qint64 pos){
	setTime(pos);
}


#include "kscd.moc"
