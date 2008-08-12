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
#include <QSplashScreen>
#include <QPixmap>
#include <KUrl>
#include <QStringList>
#include <QDir>
#include <QCursor>
#include <QList>
#include <QMenu>

using namespace Phonon;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;

KSCD::KSCD( QWidget *parent ) : KscdWindow(parent)
{
	/** Hourglass */
	setHourglass();
  
	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);
	
	
	
	devices = new HWControler();
	

	sslider = new Phonon::SeekSlider(devices->getMedia(),this);
// 	sslider->setMediaObject(devices->getMedia());
	sslider->move(m_bar->x(),m_bar->y());
	sslider->setMaximumWidth(m_bar->width());
	sslider->setMinimumWidth(m_bar->width());
	sslider->show();

	loadSettings();
	
	/** Music Brainz initialisation	*/
	m_MBManager = new MBManager();
	m_MBManager->discLookup();
	
	setupActions();
	setupContextMenu();
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
	m_actions = new KActionCollection(this);
	m_actions->setConfigGroup("Configuration");
	
	m_configureShortcutsAction = m_actions->addAction(i18n("Configure Shortcuts..."));
	m_configureShortcutsAction->setText(i18n("Configure Shortcuts..."));
	addAction(m_configureShortcutsAction);
	m_configureShortcutsAction->setShortcut(tr("Ctrl+c"));
	connect(m_configureShortcutsAction, SIGNAL(triggered()), this, SLOT(configureShortcuts()));
	
	m_configureAction = m_actions->addAction(i18n("Configure..."));
	m_configureAction->setShortcut(i18n("Configure..."));
	m_configureAction->setText(i18n("Configure..."));
	addAction(m_configureAction);
	m_configureAction->setShortcut(tr("c"));
	connect(m_configureAction, SIGNAL(triggered()), this, SLOT(optionsPreferences()));

	//download info
	m_downloadAction = m_actions->addAction("Download Info");
	m_downloadAction->setText("Download Info");
	addAction(m_downloadAction);
	m_downloadAction->setShortcut(tr("d"));
	connect(m_downloadAction, SIGNAL(triggered()), m_MBManager, SLOT(discLookup()));
	
	//upload info
	m_uploadAction = m_actions->addAction("Upload Info");
	m_uploadAction->setText("Upload Info");
	addAction(m_uploadAction);
	m_uploadAction->setShortcut(tr("u"));
	connect(m_uploadAction, SIGNAL(triggered()), m_MBManager, SLOT(discUpload()));

	//play/pause
	m_playPauseAction = m_actions->addAction("Play/Pause");
	m_playPauseAction->setText("Play/Pause");
	m_playPauseAction->setShortcut(Qt::Key_Space);
	connect(m_playPauseAction, SIGNAL(triggered()), this, SLOT(playShortcut()));
	addAction(m_playPauseAction);

	//stop
	m_stopAction = m_actions->addAction("Stop");
	m_stopAction->setText("Stop");
	addAction(m_stopAction);
	m_stopAction->setShortcut(tr("s"));
	connect(m_stopAction, SIGNAL(triggered()), devices, SLOT(stop()));

	//next
	m_nextAction = m_actions->addAction("next");
	m_nextAction->setText("Next");
	addAction(m_nextAction);
	m_nextAction->setShortcut(tr("Right"));
	connect(m_nextAction, SIGNAL(triggered()), devices, SLOT(nextTrack()));

	//previous
	m_previousAction = m_actions->addAction("previous");
	m_previousAction->setText("Previous");
	addAction(m_previousAction);
	m_previousAction->setShortcut(tr("Left"));
	connect(m_previousAction, SIGNAL(triggered()), devices, SLOT(prevTrack()));

	//eject
	m_ejectAction = m_actions->addAction("eject");
	m_ejectAction->setText("Eject");
	addAction(m_ejectAction);
	m_ejectAction->setShortcut(tr("e"));
	connect(m_ejectAction, SIGNAL(triggered()), this, SLOT(ejectShortcut()));

	//volume up
	m_volumeUpAction = m_actions->addAction("volume_up");
	m_volumeUpAction->setText("Volume Up");
	addAction(m_volumeUpAction);
	m_volumeUpAction->setShortcut(tr("Up"));
	connect(m_volumeUpAction, SIGNAL(triggered()), this, SLOT(volumeUpShortcut()));

	//volume down
	m_volumeDownAction = m_actions->addAction("volume_down");
	m_volumeDownAction->setText("Volume Down");
	addAction(m_volumeDownAction);
	m_volumeDownAction->setShortcut(tr("Down"));
	connect(m_volumeDownAction, SIGNAL(triggered()), this, SLOT(volumeDownShortcut()));

	//random
	m_randomAction = m_actions->addAction("random");
	m_randomAction->setText("Random");
	addAction(m_randomAction);
	m_randomAction->setShortcut(tr("r"));
	connect(m_randomAction, SIGNAL(triggered()), this, SLOT(randomShortcut()));

	//looptrack
	m_looptrackAction = m_actions->addAction("looptrack");
	m_looptrackAction->setText("Repeat Track");
	addAction(m_looptrackAction);
	m_looptrackAction->setShortcut(tr("l"));
	connect(m_looptrackAction, SIGNAL(triggered()), this, SLOT(looptrackShortcut()));

	//loopdisc
	m_loopdiscAction = m_actions->addAction("loopdisc");
	m_loopdiscAction->setText("Repeat Album");
	addAction(m_loopdiscAction);
	m_loopdiscAction->setShortcut(tr("Ctrl+l"));
	connect(m_loopdiscAction, SIGNAL(triggered()), this, SLOT(loopdiscShortcut()));

	//tracklist
	m_tracklistAction = m_actions->addAction("tracklist");
	m_tracklistAction->setText("Show Tracklist");
	addAction(m_tracklistAction);
	m_tracklistAction->setShortcut(tr("t"));
	connect(m_tracklistAction, SIGNAL(triggered()), this, SLOT(tracklistShortcut()));

	//mute
	m_muteAction = m_actions->addAction("mute");
	m_muteAction->setText("Mute/Unmute");
	addAction(m_muteAction);
	m_muteAction->setShortcut(tr("m"));
	connect(m_muteAction, SIGNAL(triggered()), this, SLOT(muteShortcut()));
	
	//minimize
	m_minimizeAction = m_actions->addAction("Minimize");
	m_minimizeAction->setText("Minimize");
	addAction(m_minimizeAction);
	m_minimizeAction->setShortcut(tr("Ctrl+Escape"));
	connect(m_minimizeAction, SIGNAL(triggered()), this, SLOT(minimizeShortcut()));
		
	//quit
	m_quitAction = m_actions->addAction("Quit");
	m_quitAction->setText("Quit");
	addAction(m_quitAction);
	m_quitAction->setShortcut(tr("Escape"));
	connect(m_quitAction, SIGNAL(triggered()), this, SLOT(quitShortcut()));
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	
	//Read saved settings
	m_actions->readSettings();

	mute = false;
	play = false;
	random = false;
	looptrack = false;
	loopdisc = false;
	
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
	connect(devices,SIGNAL(cdLoaded()),m_MBManager,SLOT(discLookup()));
	
	connect( this , SIGNAL( customContextMenuRequested( const QPoint &) ) , SLOT( showContextMenu( const QPoint &) ) );
}

void KSCD :: setupContextMenu()
{
	contextMenu = new QMenu( this );
	contextMenu->addAction(m_configureShortcutsAction);
	contextMenu->addAction(m_configureAction);
	contextMenu->addSeparator();
	contextMenu->addAction(m_minimizeAction);
	contextMenu->addAction(m_quitAction);
}

void KSCD :: showContextMenu( const QPoint &p)
{
	contextMenu->popup( mapToGlobal ( p ) );
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
	int currentTrack = devices->getCurrentTrack();
	if (devices->getCD()->isCdInserted()  && currentTrack > 0 )
	{	
                
		title = QString("%1 - ").arg(currentTrack, 2, 10, QLatin1Char('0')) ;
		title.append(m_MBManager->getTrackList()[currentTrack-1].Title);
		length.append(m_MBManager->getTrackList()[currentTrack-1].Duration);
		
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

void KSCD::configureShortcuts()
{
	KShortcutsDialog::configure(m_actions, KShortcutsEditor::LetterShortcutsAllowed, this, true);
}

void KSCD::setShortcut(QString name, QString key)
{
	kDebug()<<"SetShortcut, "<<name<<" : "<<key;
	if (name == "Play/Pause")
	{
		m_playPauseAction->setShortcut(key);
	}

	if (name == "Stop")
	{
		m_stopAction->setShortcut(key);
	}

	if (name == "Next Track")
	{
		m_nextAction->setShortcut(key);
	}

	if (name == "Previous Track")
	{
		m_previousAction->setShortcut(key);
	}

	if (name == "Eject")
	{
		m_ejectAction->setShortcut(key);
	}

	if (name == "Random")
	{
		m_randomAction->setShortcut(key);
	}

	if (name == "Loop Track")
	{
		m_looptrackAction->setShortcut(key);
	}

	if (name == "Loop Disc")
	{
		m_loopdiscAction->setShortcut(key);
	}

	if (name == "Tracklist")
	{
		m_tracklistAction->setShortcut(key);
	}

	if (name == "Mute")
	{
		m_muteAction->setShortcut(key);
	}

	if (name == "Download Info")
	{
		m_downloadAction->setShortcut(key);
	}

	if (name == "CDDB Window")
	{
		m_CDDBWindowAction->setShortcut(key);
	}
	
	if (name == "Volume Up")
	{
		m_volumeUpAction->setShortcut(key);
	}
	
	if (name == "Volume Down")
	{
		m_volumeDownAction->setShortcut(key);
	}

	if (name == "Configure KsCD")
	{
		m_configureAction->setShortcut(key);
	}

}

void KSCD::ejectShortcut()
{
	actionButton("eject");
}

void KSCD::quitShortcut()
{
	actionButton("close");
}

void KSCD::minimizeShortcut()
{
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
	if (devices->getVolume()<=0.95)
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
// 				m_slider->stop();
// 				m_slider->setTotalTime(devices->getTotalTime());
// 				m_slider->setStep(devices->getTotalTime());
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
// 				m_slider->stop();
			}
			if ((devices->getState() == PlayingState))
			{
// 				m_slider->stop();
// 				m_slider->start(devices->getTotalTime());
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
// 				m_slider->stop();
			}
			if ((devices->getState() == PlayingState))
			{
// 				m_slider->stop();
				devices->play();
// 				m_slider->start(devices->getTotalTime());
			}
		}
		emit(picture(name,state));
	}
	if(name=="stop")
	{
		if ((devices->getState() == PlayingState)|| (devices->getState() == PausedState))
		{
			devices->stop();
// 			m_slider->stop();
		}
		emit(picture(name,state));
	}
	if(name=="eject")
	{
		m_trackDlg->removeRowsTrackTable(m_MBManager->getTrackList().size());
		devices->eject();
		emit(picture(name,state));
		if ((devices->getState() == PlayingState)|| (devices->getState() == PausedState))
		{
			devices->stop();
// 			m_slider->stop();
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
	if ( name == "configure")
	{
		optionsPreferences();
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
	
	//Filter on the skin url combo box
	QString pathSkins=KStandardDirs::installPath("data") + "/kscd/skin/";
	QDir *directory= new QDir(pathSkins);
	QStringList filter;
	filter << "*.svg";
	directory->setNameFilters(filter);
	QStringList list = directory->entryList();
	ui_interface.kcfg_url->addItems(list);
	
	dialog->addPage(interfaceSettingsDlg, i18n("Appearance"), "fill-color");

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
	m_panel->setEjectAct( Prefs::ejectOnFinish() );
	setNewSkin( KStandardDirs::installPath("data") + "kscd/skin/" + Prefs::url() );
}

void KSCD :: loadSettings()
{
	//setNewSkin( KStandardDirs::installPath("data") + "kscd/skin/" + Prefs::url() );
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
//	QPixmap pixM("/home/stanislas/isi-kscd/kdemultimedia/kscd/splash2.png");
//	QSplashScreen splash( pixM );
//	splash.show();
	KSCD *k = new KSCD();
	a.setTopWidget( k );
//   a.setMainWidget(k);

	k->setWindowTitle(KGlobal::caption());

	if (kapp->isSessionRestored())
	{
		KConfigGroup group(KApplication::kApplication()->sessionConfig(), "General");
		if (group.readEntry("Show", false)){
//			splash.finish(k);
			k->show();
		}
	}
	else
	{
//		splash.finish(k);
		k->show();
	}

// 	if (args->count() > 0)
// 		Prefs::self()->setCdDevice(args->arg(0));

	return a.exec();
}


#include "kscd.moc"
