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
// #include <QKeyEvent>
// #include <kactioncollection.h>
// #include <kshortcutsdialog.h>
#include "kscd.h"
#include "ihm/configwindow.h"

using namespace Phonon;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;

KSCD::KSCD( QWidget *parent ) : KscdWindow(parent)
{
	/** Hourglass */
	setHourglass();

	/** Application initialization */
// 	QString commande = "cp "+KStandardDirs::installPath("data") + "kscd/ihm/skin/*.TTF ~/.fonts/";
// 	commande.replace("/ihm/","/");
// 	char * chemin = (char *)malloc(1024 * sizeof(char));
// 	kDebug () << commande;
// 	strcpy(chemin, commande.toAscii().data());
// 	system(chemin);
	//system("cp "+KStandardDirs::installPath("data") + "/kscd/ihm/skin/TRANGA__.TTF ~/.fonts/TRANGA__.TTF");
  
	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);

	devices = new HWControler();
	
// 	addSeekSlider(new Phonon::SeekSlider(devices->getMedia()));
// 	Phonon::VolumeSlider * vs = new Phonon::VolumeSlider(devices->getAudioOutPut());
// 	vs->setOrientation(Qt::Vertical);
// 	vs->setMuteVisible(false);
// 	addVolumeSlider(vs);

/**
 *	SETTINGS
 */
	loadSettings();
	
	/** Music Brainz initialisation	*/
	m_MBManager = new MBManager();
	m_MBManager->discLookup();
	
	setupActions();
	
}

KSCD::~KSCD()
{
	delete devices;
	delete m_MBManager;
// 	delete m_cd;
// 	delete m_cddbManager;
//	delete w_titlePopUp;	//deleting of the title popup

/*	delete m_cddialog;
	delete m_cddb;*/
}

void KSCD::setupActions()
{	
	/**
	 * General
	 */
	// Connects UI with actions triggering
	connect(this,SIGNAL(actionClicked(QString)), this, SLOT(actionButton(QString)));
	connect(this,SIGNAL(picture(QString,QString)), this, SLOT(changePicture(QString,QString)));
	
	// General connects
	connect(this,SIGNAL(trackClicked(int)), this, SLOT(playTrack(int)));
	connect(this,SIGNAL(actionVolume(qreal)), this, SLOT(changeVolume(qreal)));
	connect(devices,SIGNAL(currentTime(qint64)),this,SLOT(catchtime(qint64)));
	connect(this,SIGNAL(infoPanel(QString)),this,SLOT(panelInfo(QString)));
	
	// MB
	connect(m_MBManager, SIGNAL(showArtistLabel(QString)), this, SLOT(showArtistLabel(QString)));
	connect(m_MBManager, SIGNAL(showTrackinfoLabel(QString)), this, SLOT(showTrackinfoLabel(QString)));
	
	connect(devices,SIGNAL(trackChanged()),this,SLOT(restoreTrackinfoLabel()));
//	connect(devices,SIGNAL(trackChanged()),m_popUp,SLOT(showTitlePopUp()));
	connect(devices,SIGNAL(cdLoaded()),m_MBManager,SLOT(discLookup()));

/**
 * Contextual Menu
 */
	// Set context menu policy to ActionsContextMenu
	setContextMenuPolicy(Qt::ActionsContextMenu);
	
	m_actions = new KActionCollection(this);
	m_actions->setConfigGroup("Configuration");

//	configure_shortcuts = new QAction(i18n("Configure Shortcuts..."), this);
	configure_shortcuts = m_actions->addAction(i18n("Configure Shortcuts..."));
	configure_shortcuts->setShortcut(i18n("Configure Shortcuts..."));
	configure_shortcuts->setText(i18n("Configure Shortcuts..."));
	addAction(configure_shortcuts);
	connect(configure_shortcuts, SIGNAL(triggered()), this, SLOT(configureShortcuts()));

	configure = m_actions->addAction(i18n("Configure..."));
	configure->setShortcut(i18n("Configure..."));
	configure->setText(i18n("Configure..."));
	addAction(configure);
	configure->setShortcut(tr("c"));
	connect(configure, SIGNAL(triggered()), this, SLOT(optionsPreferences()));

/*	QAction* test = new QAction(i18n("test"), this);
	addAction(test);
	connect(test, SIGNAL(triggered()), this, SLOT(test()));
	test->setShortcut(Qt::CTRL + Qt::Key_T);
	*/
	
//Find out skin
//	QAction* findS = new QAction(i18n("Skin..."), this);
	QAction* findS = m_actions->addAction(i18n("Skin..."));
	findS->setShortcut(i18n("Skin..."));
	findS->setText(i18n("Skin..."));
	addAction(findS);
	connect(findS, SIGNAL(triggered()), this, SLOT(makeFinderSkinDialog()));
	connect(m_finderSkin,SIGNAL(pathSkinChanged(QString)),this,SLOT(setNewSkin(QString)));
	
	
//////////Set Shortcuts
	setDefaultShortcuts();
	mute = false;
	play = false;
	random = false;
	looptrack = false;
	loopdisc = false;

}

void KSCD::test()
{
	//kDebug () << "total time : " << devices->getTotalTime() ;
	//kDebug () << "CDDB disc ID : " << devices->getMedia()->metaData("MUSICBRAINZ_DISCID") ;
	configureKeys();
}

/**
 * CDDB Management
 */

void KSCD::restoreArtistLabel()
{
	if( devices->getCD()->isCdInserted() && devices->isDiscValid() )
	{
		QString artist, title;
		artist = m_MBManager->getDiscInfo().Artist;
		title = m_MBManager->getDiscInfo().Title;
		showArtistLabel(QString("%1").arg(artist));
		showArtistAlbum(QString("%1").arg(title));
	}
	else
	{
		showArtistLabel("");
	}

}

void KSCD::restoreTrackinfoLabel()
{
	QString title, length ;

	// If disc is inserted
	if (devices->getCD()->isCdInserted())
	{
		title = QString("%1 - ").arg(devices->getCurrentTrack(), 2, 10, QLatin1Char('0')) ;
		title.append(m_MBManager->getTrackList()[devices->getCurrentTrack()-1].Title);
		length.append(m_MBManager->getTrackList()[devices->getCurrentTrack()-1].Duration);
		
		showTrackinfoLabel(title);
		m_popup = new TitlePopUp(this, "popup");
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
	//download info
	DownloadAction = new QAction(i18n("Download Information"), this);
	DownloadAction = m_actions->addAction("Download Info");
	DownloadAction->setText("Download Info");
	addAction(DownloadAction);
	DownloadAction->setShortcut(tr("d"));
	connect(DownloadAction, SIGNAL(triggered()), m_MBManager, SLOT(discLookup()));

	//upload info
	UploadAction = new QAction(i18n("Upload Information"), this);
	UploadAction = m_actions->addAction("Upload Info");
	UploadAction->setText("Upload Info");
	addAction(UploadAction);
	UploadAction->setShortcut(tr("u"));
	connect(UploadAction, SIGNAL(triggered()), m_MBManager, SLOT(discUpload()));

	//quit
	quit_shortcut = new QAction(i18n("quit"), this);
	quit_shortcut = m_actions->addAction("Quit");
	quit_shortcut->setText("Quit");
	addAction(quit_shortcut);
	quit_shortcut->setShortcut(tr("Escape"));
	connect(quit_shortcut, SIGNAL(triggered()), this, SLOT(quitShortcut()));

	//minimize
	minimize_shortcut = new QAction(i18n("minimize"), this);
	minimize_shortcut = m_actions->addAction("Minimize");
	minimize_shortcut->setText("Minimize");
	addAction(minimize_shortcut);
	minimize_shortcut->setShortcut(tr("Alt+Escape"));
	connect(minimize_shortcut, SIGNAL(triggered()), this, SLOT(minimizeShortcut()));
	
	//play/pause
	play_pause_shortcut = new QAction(i18n("play"), this);
	play_pause_shortcut = m_actions->addAction("Play/Pause");
	play_pause_shortcut->setText("Play/Pause");
	addAction(play_pause_shortcut);
	play_pause_shortcut->setShortcut(Qt::Key_Space);
	connect(play_pause_shortcut, SIGNAL(triggered()), this, SLOT(playShortcut()));
	
	//stop
	stop_shortcut = new QAction(i18n("stop"), this);
	stop_shortcut = m_actions->addAction("Stop");
	stop_shortcut->setText("Stop");
	addAction(stop_shortcut);
	stop_shortcut->setShortcut(tr("s"));
	connect(stop_shortcut, SIGNAL(triggered()), devices, SLOT(stop()));

	//next
	next_shortcut = new QAction(i18n("next"), this);
	next_shortcut = m_actions->addAction("next");
	next_shortcut->setText("Next");
	addAction(next_shortcut);
	next_shortcut->setShortcut(tr("Right"));
	connect(next_shortcut, SIGNAL(triggered()), devices, SLOT(nextTrack()));

	//previous
	previous_shortcut = new QAction(i18n("previous"), this);
	previous_shortcut = m_actions->addAction("previous");
	previous_shortcut->setText("Previous");
	addAction(previous_shortcut);
	previous_shortcut->setShortcut(tr("Left"));
	connect(previous_shortcut, SIGNAL(triggered()), devices, SLOT(prevTrack()));

	//eject
	eject_shortcut = new QAction(i18n("eject"), this);
	eject_shortcut = m_actions->addAction("eject");
	eject_shortcut->setText("Eject");
	addAction(eject_shortcut);
	eject_shortcut->setShortcut(tr("e"));
	connect(eject_shortcut, SIGNAL(triggered()), devices, SLOT(eject()));

	//volume up
	volume_up_shortcut = new QAction(i18n("volume_up"), this);
	volume_up_shortcut = m_actions->addAction("volume_up");
	volume_up_shortcut->setText("Volume Up");
	addAction(volume_up_shortcut);
	volume_up_shortcut->setShortcut(tr("Up"));
	//connect(volume_up_shortcut, SIGNAL(triggered()), m_volumeB, SIGNAL(volumeupSignal()));
	connect(volume_up_shortcut, SIGNAL(triggered()), this, SLOT(volumeUpShortcut()));

	//volume down
	volume_down_shortcut = new QAction(i18n("volume_down"), this);
	volume_down_shortcut = m_actions->addAction("volume_down");
	volume_down_shortcut->setText("Volume Down");
	addAction(volume_down_shortcut);
	volume_down_shortcut->setShortcut(tr("Down"));
	connect(volume_down_shortcut, SIGNAL(triggered()), this, SLOT(volumeDownShortcut()));

	//random
	random_shortcut = new QAction(i18n("random"), this);
	random_shortcut = m_actions->addAction("random");
	random_shortcut->setText("Random");
	addAction(random_shortcut);
	random_shortcut->setShortcut(tr("r"));
	connect(random_shortcut, SIGNAL(triggered()), this, SLOT(randomShortcut()));

	//looptrack
	looptrack_shortcut = new QAction(i18n("looptrack"), this);
	looptrack_shortcut = m_actions->addAction("looptrack");
	looptrack_shortcut->setText("Repeat Track");
	addAction(looptrack_shortcut);
	looptrack_shortcut->setShortcut(tr("l"));
	connect(looptrack_shortcut, SIGNAL(triggered()), this, SLOT(looptrackShortcut()));

	//loopdisc
	loopdisc_shortcut = new QAction(i18n("loopdisc"), this);
	loopdisc_shortcut = m_actions->addAction("loopdisc");
	loopdisc_shortcut->setText("Repeat Album");
	addAction(loopdisc_shortcut);
	loopdisc_shortcut->setShortcut(tr("Ctrl+l"));
	connect(loopdisc_shortcut, SIGNAL(triggered()), this, SLOT(loopdiscShortcut()));

	//cddb window
	//Done in constructor

	//Configure KsCD
	//Done in constructor	

	//tracklist
	tracklist_shortcut = new QAction(i18n("tracklist"), this);
	tracklist_shortcut = m_actions->addAction("tracklist");
	tracklist_shortcut->setText("Show Tracklist");
	addAction(tracklist_shortcut);
	tracklist_shortcut->setShortcut(tr("t"));
	connect(tracklist_shortcut, SIGNAL(triggered()), this, SLOT(tracklistShortcut()));

	//mute
	mute_shortcut = new QAction(i18n("mute"), this);
	mute_shortcut = m_actions->addAction("mute");
	mute_shortcut->setText("Mute/Unmute");
	addAction(mute_shortcut);
	mute_shortcut->setShortcut(tr("m"));
	connect(mute_shortcut, SIGNAL(triggered()), this, SLOT(muteShortcut()));

	//Read saved settings
	m_actions->readSettings();
	//configureShortcuts();
}

void KSCD::configureShortcuts()
{
	KShortcutsDialog::configure(m_actions, KShortcutsEditor::LetterShortcutsAllowed, this, true);
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

void KSCD::quitShortcut()
{
	kDebug()<<"Quit Shortcut";
	actionButton("close");
}

void KSCD::minimizeShortcut()
{
	kDebug()<<"Minimize Shortcut";
	actionButton("minimize");
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
	if (devices->getVolume()<=0.96)
	{
		m_volumeB->volumeShortcut(5.0);
	}
}

void KSCD::volumeDownShortcut()
{
	if (devices->getVolume()>=0.05)
	{
		m_volumeB->volumeShortcut(-5.0);
	}
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
				kDebug()<<"time total"<<devices->getTotalTime();
				devices->play();
				m_slider->stop();
				m_slider->start(devices->getTotalTime());
				restoreTrackinfoLabel();
				restoreArtistLabel();
			}
		}
		emit(picture(name,state));
		play = !play;
	}
	if(name=="pause")
	{
		/*if( !devices->isDiscValid() || !devices->getCD()->isCdInserted())
		{
			if(!devices->getCD()->isCdInserted())
				showArtistLabel(i18n("No disc"));
			else
				showArtistLabel(i18n("Invalid disc"));
			QTimer::singleShot(2000, this, SLOT(restoreArtistLabel()));
		}
		else{*/
			if(devices->getState() == PlayingState)
			{
				devices->pause();
				m_slider->pause();
			}
		/*}*/
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
				m_slider->stop();
			}
			if ((devices->getState() == PlayingState))
			{
				m_slider->stop();
				m_slider->start(devices->getTotalTime());
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
				m_slider->stop();
			}
			if ((devices->getState() == PlayingState))
			{
				m_slider->stop();
				devices->play();
				m_slider->start(devices->getTotalTime());
			}
		}
		emit(picture(name,state));
	}
	if(name=="stop")
	{
		if ((devices->getState() == PlayingState)|| (devices->getState() == PausedState))
		{
			devices->stop();
			m_slider->stop();
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
			m_slider->stop();
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
		devices->setRandom(false);
		emit(picture(name,state));
		emit(infoPanel("random"));

		random = !random;
	}
	if(name == "p_random")
	{
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
// 	Prefs::self()->writeConfig();
}

void KSCD::configureKeys()
{
	KShortcutsDialog::configure(m_actions, KShortcutsEditor::LetterShortcutsAllowed, this, true);
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

void KSCD :: optionsPreferences()
{
	if ( KConfigDialog::showDialog( "settings" ) )  {
	        return;
	}

	//KConfigDialog didn't find an instance of this dialog, so lets create it :
	KConfigDialog* dialog = new KConfigDialog( this, "settings",  Prefs::self() );
	// Add the General Settings page
	QWidget *generalSettingsDlg = new QWidget;
	ui_general.setupUi(generalSettingsDlg);
	
	dialog->addPage(generalSettingsDlg, i18n("General"), "kscd");
  
	QWidget *interfaceSettingsDlg = new QWidget;
	ui_interface.setupUi(interfaceSettingsDlg);
	
	dialog->addPage(interfaceSettingsDlg, i18n("Interface"), "fill-color");
	connect(dialog, SIGNAL(settingsChanged( const QString &)), this, SLOT(updateSettings()));
	dialog->setAttribute( Qt::WA_DeleteOnClose );
	dialog->setHelp(QString(),"kscd");
	dialog->show();
}

void KSCD :: updateSettings()
{
	m_panel->setTextColor(Prefs::textColor());
	kDebug()<<"color config:"<<Prefs::textColor();
	m_panel->setTextSizeFont(Prefs::textFont());
	kDebug()<<"font config:"<<Prefs::textFont();
	devices->setEjectActivated(Prefs::ejectOnFinish());
	kDebug()<<"eject setting:"<<Prefs::ejectOnFinish();
	m_panel->setEjectAct(Prefs::ejectOnFinish());
}

void KSCD :: loadSettings()
{
	m_panel->setTextColor(Prefs::textColor());
	m_panel->setTextSizeFont(Prefs::textFont());
	m_panel->setEjectAct(Prefs::ejectOnFinish());
	devices->setEjectActivated(Prefs::ejectOnFinish());
}

void KSCD::catchtime(qint64 pos){
	setTime(pos);
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

// 	if (args->count() > 0)
// 		Prefs::self()->setCdDevice(args->arg(0));

	return a.exec();
}


#include "kscd.moc"
