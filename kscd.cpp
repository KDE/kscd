/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
 * Copyright (c) 2003-2006 Richard Lärkäng <nouseforaname@home.se>
 * Copyright (c) 2008 Amine Bouchikhi <bouchikhi.amine@gmail.com>
 * Copyright (c) 2008 Laurent Montel <montel@kde.org>
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
#include "dbus/PlayerDBusHandler.h"
#include "dbus/RootDBusHandler.h"
#include "dbus/TracklistDBusHandler.h"
#include <QSplashScreen>
#include <QPixmap>
#include <QStringList>
#include <QDir>
#include <QMenu>
#include <QDBusConnection>
#include <QDBusInterface>
#include "cdplayeradaptor.h"

using namespace Phonon;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;

KSCD::KSCD( QWidget *parent ) : KscdWindow(parent)
{
	/** Hourglass */
	setHourglass();

        new CDPlayerAdaptor( this );

	QDBusConnection::sessionBus().registerObject("/CDPlayer", this);


	devices = new HWControler();

	KsCD::PlayerDBusHandler * pdbh = new KsCD::PlayerDBusHandler(this);
	KsCD::RootDBusHandler * rdbh = new KsCD::RootDBusHandler(this);
	KsCD::TracklistDBusHandler * tdbh = new KsCD::TracklistDBusHandler(this);

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
}

void KSCD::setupActions()
{
	m_actions = new KActionCollection(this);
	m_actions->setConfigGroup("Configuration");

	m_configureShortcutsAction = m_actions->addAction(i18n("Configure Shortcuts..."));
	m_configureShortcutsAction->setText(i18n("Configure Shortcuts..."));
	addAction(m_configureShortcutsAction);
	//m_configureShortcutsAction->setShortcut(Qt::Key_C);
	connect(m_configureShortcutsAction, SIGNAL(triggered()), this, SLOT(configureShortcuts()));





	m_configureAction = m_actions->addAction(i18n("Configure..."));
	m_configureAction->setText(i18n("Configure..."));
	addAction(m_configureAction);
	connect(m_configureAction, SIGNAL(triggered()), this, SLOT(optionsPreferences()));

	//download info
	m_downloadAction = m_actions->addAction(i18n("Download Info"));
	m_downloadAction->setText(i18n("Download Info"));
	addAction(m_downloadAction);
	connect(m_downloadAction, SIGNAL(triggered()), m_MBManager, SLOT(discLookup()));

	//upload info
	m_uploadAction = m_actions->addAction("Upload Info");
	m_uploadAction->setText(i18n("Upload Info"));
	addAction(m_uploadAction);
	connect(m_uploadAction, SIGNAL(triggered()), m_MBManager, SLOT(discUpload()));

	//play/pause
	m_playPauseAction = m_actions->addAction("Play/Pause");
	m_playPauseAction->setText(i18n("Play/Pause"));
	m_playPauseAction->setShortcut(Qt::Key_Space);
	connect(m_playPauseAction, SIGNAL(triggered()), this, SLOT(playShortcut()));
	addAction(m_playPauseAction);

	//stop
	m_stopAction = m_actions->addAction("Stop");
	m_stopAction->setText(i18n("Stop"));
	addAction(m_stopAction);
	m_stopAction->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_V);
	connect(m_stopAction, SIGNAL(triggered()), devices, SLOT(stop()));

	//next
	m_nextAction = m_actions->addAction("next");
	m_nextAction->setText(i18nc( "This action allow user to pass to the following track","Next" ));
	addAction(m_nextAction);
	m_nextAction->setShortcut(Qt::Key_Right);
	connect(m_nextAction, SIGNAL(triggered()), devices, SLOT(nextTrack()));

	//previous
	m_previousAction = m_actions->addAction("previous");
	m_previousAction->setText(i18nc( "This action allow the user to pass to the preceding track", "Previous" ) );
	addAction(m_previousAction);
	m_previousAction->setShortcut(Qt::Key_Left);
	connect(m_previousAction, SIGNAL(triggered()), devices, SLOT(prevTrack()));

	//eject
	m_ejectAction = m_actions->addAction("eject");
	m_ejectAction->setText(i18nc( " This action allow to eject the inserted disc", "Eject"));
	addAction(m_ejectAction);
	m_ejectAction->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_E);
	connect(m_ejectAction, SIGNAL(triggered()), this, SLOT(ejectShortcut()));

	//volume up
	m_volumeUpAction = m_actions->addAction("volume_up");
	m_volumeUpAction->setText(i18n("Volume Up"));
	addAction(m_volumeUpAction);
	m_volumeUpAction->setShortcut(Qt::Key_Up);
	connect(m_volumeUpAction, SIGNAL(triggered()), this, SLOT(volumeUpShortcut()));

	//volume down
	m_volumeDownAction = m_actions->addAction("volume_down");
	m_volumeDownAction->setText(i18n("Volume Down"));
	addAction(m_volumeDownAction);
	m_volumeDownAction->setShortcut(Qt::Key_Down);
	connect(m_volumeDownAction, SIGNAL(triggered()), this, SLOT(volumeDownShortcut()));

	//random
	m_randomAction = m_actions->addAction("random");
	m_randomAction->setText(i18nc("This action allow the user to listen a random track list","Random"));
	addAction(m_randomAction);
	m_randomAction->setShortcut(Qt::CTRL + Qt:: Key_H);
	connect(m_randomAction, SIGNAL(triggered()), this, SLOT(randomShortcut()));

	//looptrack
	m_looptrackAction = m_actions->addAction("looptrack");
	m_looptrackAction->setText(i18n("Repeat Track"));
	addAction(m_looptrackAction);
	m_looptrackAction->setShortcut(Qt::CTRL + Qt::Key_T);
	connect(m_looptrackAction, SIGNAL(triggered()), this, SLOT(looptrackShortcut()));

	//loopdisc
	m_loopdiscAction = m_actions->addAction("loopdisc");
	m_loopdiscAction->setText(i18n("Repeat Album"));
	addAction(m_loopdiscAction);
	m_loopdiscAction->setShortcut(Qt::CTRL + Qt::Key_D);
	connect(m_loopdiscAction, SIGNAL(triggered()), this, SLOT(loopdiscShortcut()));

	//tracklist
	m_tracklistAction = m_actions->addAction("tracklist");
	m_tracklistAction->setText(i18n("Show Tracklist"));
	addAction(m_tracklistAction);
	connect(m_tracklistAction, SIGNAL(triggered()), this, SLOT(tracklistShortcut()));

	//mute
	m_muteAction = m_actions->addAction("mute");
	m_muteAction->setText(i18n("Mute/Unmute"));
	addAction(m_muteAction);
	connect(m_muteAction, SIGNAL(triggered()), this, SLOT(muteShortcut()));

	//minimize
	m_minimizeAction = m_actions->addAction("Minimize");
	m_minimizeAction->setText(i18n("Minimize"));
	addAction(m_minimizeAction);
	connect(m_minimizeAction, SIGNAL(triggered()), this, SLOT(minimizeShortcut()));

	//quit
	m_quitAction = KStandardAction::quit(this,SLOT(quitShortcut()),this);

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
	connect(this,SIGNAL(actionClicked(const QString&)), this, SLOT(actionButton(const QString&)));
	connect(this,SIGNAL(picture(const QString&,const QString&)), this, SLOT(changePicture(const QString&,const QString&)));

	// General connects
	connect(this,SIGNAL(trackClicked(int)), this, SLOT(playTrack(int)));
	connect(this,SIGNAL(actionVolume(qreal)), this, SLOT(changeVolume(qreal)));
	connect(devices,SIGNAL(currentTime(qint64)),this,SLOT(catchtime(qint64)));
	connect(this,SIGNAL(infoPanel(const QString&)),this,SLOT(panelInfo(const QString&)));

	// MB
	connect(m_MBManager, SIGNAL(showArtistLabel(QString&)), this, SLOT(showArtistLabel(QString&)));
	connect(m_MBManager, SIGNAL(showTrackinfoLabel(QString&)), this, SLOT(showTrackinfoLabel(QString&)));

	connect(devices,SIGNAL(trackChanged()),this,SLOT(restoreTrackinfoLabel()));
	connect(devices,SIGNAL(cdLoaded()),m_MBManager,SLOT(discLookup()));

	connect( this , SIGNAL( customContextMenuRequested( const QPoint &) ) , SLOT( showContextMenu( const QPoint &) ) );
}

void KSCD::setupContextMenu()
{
	contextMenu = new QMenu( this );
	contextMenu->addAction(m_configureShortcutsAction);
	contextMenu->addAction(m_configureAction);
	contextMenu->addSeparator();
	contextMenu->addAction(m_minimizeAction);
	contextMenu->addAction(m_quitAction);
}

void KSCD::showContextMenu( const QPoint &p)
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
		showArtistLabel(artist);
		showArtistAlbum(title);
	}
	else
	{
		QString empty = "";
		showArtistLabel(empty);
	}

}

void KSCD::restoreTrackinfoLabel()
{
	QString title, length ;
/*
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
	}*/
}
void KSCD::changeVolume(qreal value)
{
	//kDebug()<<"changeVolume enter "<<value;
	devices->setVolume(value);
}

void KSCD::configureShortcuts()
{
	KShortcutsDialog::configure(m_actions, KShortcutsEditor::LetterShortcutsAllowed, this, true);
}

void KSCD::ejectShortcut()
{
	QString result = "eject";
	actionButton(result);
}

void KSCD::quitShortcut()
{
	QString result = "close";
	actionButton(result);
}

void KSCD::minimizeShortcut()
{
	QString result = "minimize";
	actionButton(result);
}

void KSCD::tracklistShortcut()
{
	QString result = "tracklist";
	actionButton(result);
}

void KSCD::muteShortcut()
{
	QString def = "default";
	if (!mute)
	{
		QString result = "unmute";
		actionButton(result);
		emit(picture(result,def));
		//mute = !mute;
	}
	else
	{
		QString result = "mute";
		actionButton(result);
		emit(picture(result,def));
		//mute = !mute;
	}
}

void KSCD::playShortcut()
{
	QString def = "default";
	if (!play)
	{
		QString result = "play";
		actionButton(result);
		emit(picture(result,def));
		//play = !play;
	}
	else
	{
		QString result = "pause";
		actionButton(result);
		emit(picture(result,def));
		//play = !play;
	}
}

void KSCD::randomShortcut()
{
	QString def = "default";
	if (!random)
	{
		QString result = "p_random";
		actionButton(result);
		emit(picture(result,def));
		emit(infoPanel(result));

		//random = !random;
	}
	else
	{
		QString result = "random";
		actionButton(result);
		emit(picture(result,def));
		emit(infoPanel(result));
		//random = !random;
	}
}

void KSCD::looptrackShortcut()
{
	QString def = "default";
	if (!looptrack)
	{
		QString result = "looptrack";
		actionButton(result);
		emit(picture(result,def));
		emit(infoPanel(result));

		//looptrack = !looptrack;
	}
	else
	{
		QString result = "loop";
		actionButton(result);
		emit(picture(result,def));
		emit(infoPanel(result));

		//looptrack = !looptrack;
	}
}

void KSCD::loopdiscShortcut()
{
	QString def = "default";
	if (!loopdisc)
	{
		QString result = "loopdisc";
		actionButton(result);
		emit(picture(result,def));
		emit(infoPanel(result));

		//loopdisc = !loopdisc;
	}
	else
	{
		QString result = "loop";
		actionButton(result);
		emit(picture(result,def));
		emit(infoPanel(result));

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
	QString result = "play";
	QString def = "default";
	kDebug()<<"playtrack enter "<<track;
	devices->play(track);
	emit(picture(result,def));
}

/**
 * Link IHM with actions
 */
void KSCD::actionButton(const QString & name)
{

	QString state = "over";
	QString result;
	if(name=="play")
	{
		if( !devices->isDiscValid() || !devices->getCD()->isCdInserted())
		{
			QString result;
			if(!devices->getCD()->isCdInserted()){
				result = i18n("No disc");
				showArtistLabel(result);
			}
			else{
				result = i18n("Invalid disc");
				showArtistLabel(result);
			}
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
			QString result;
			if(!devices->getCD()->isCdInserted()){
				result = i18n("No disc");
				showArtistLabel(result);
			}
			else{
				result = i18n("Invalid disc");
				showArtistLabel(result);
			}
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
			QString result;
			if(!devices->getCD()->isCdInserted()){
				result = i18n("No disc");
				showArtistLabel(result);
			}
			else{
				result = i18n("Invalid disc");
				showArtistLabel(result);
			}
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
		result = "random";
		emit(infoPanel(result));

		random = !random;
	}
	if(name == "p_random")
	{
		devices->setRandom(true);
		emit(picture(name,state));
		result = "p_random";
		emit(infoPanel(result));

		random = !random;
	}
	if(name == "loop")
	{
		devices->setLoopMode(NoLoop);
		emit(picture(name,state));
		result = "loop";
		emit(infoPanel(result));

		looptrack = false;
		loopdisc = false;
	}
	if(name == "looptrack")
	{
		devices->setLoopMode(LoopOne);
		emit(picture(name,state));
		result = "looptrack";
		emit(infoPanel(result));

		looptrack = true;
		loopdisc = false;
	}
	if(name == "loopdisc")
	{
		devices->setLoopMode(LoopAll);
		emit(picture(name,state));
		emit(infoPanel(name));

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
			QList<MBTrackInfo> list = m_MBManager->getTrackList();
			QString title(m_MBManager->getDiscInfo().Title);
			createTrackDialog(list,title);
			kDebug()<<"open track window";
		}
		QString def = "default";
		emit(picture(name,def));
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
HWControler* KSCD::getDevices() const
{
	return devices;
}

/**
 * Save state on session termination
 */
bool KSCD::saveState(QSessionManager& /*sm*/)
{
	writeSettings();
	KConfigGroup config(KApplication::kApplication()->sessionConfig(), i18nc("General option in the configuration dialog","General"));
	//config.writeEntry(i18n("Show"), isVisible());
	return true;
}

void KSCD::optionsPreferences()
{
	if ( KConfigDialog::showDialog( i18n("Settings") ) )  {
	        return;
	}

	//KConfigDialog didn't find an instance of this dialog, so lets create it :
	KConfigDialog* dialog = new KConfigDialog( this, i18n("Settings"),  Prefs::self() );
	// Add the General Settings page
	QWidget *generalSettingsDlg = new QWidget;
	ui_general.setupUi(generalSettingsDlg);

	dialog->addPage(generalSettingsDlg, i18nc("General option in the configuration dialog","General"), "kscd");

	QWidget *interfaceSettingsDlg = new QWidget;
	ui_interface.setupUi(interfaceSettingsDlg);

	//Filter on the skin url combo box
	QString pathSkins=KStandardDirs::installPath("data") + "/kscd/skin/";
	QDir directory(pathSkins);
	QStringList filter;
	filter << "*.svg";
	directory.setNameFilters(filter);
	QStringList list = directory.entryList();
	ui_interface.kcfg_url->addItems(list);

	dialog->addPage(interfaceSettingsDlg, i18n("Appearance"), "fill-color");

	connect(dialog, SIGNAL(settingsChanged( const QString &)), this, SLOT(updateSettings()));
	dialog->setAttribute( Qt::WA_DeleteOnClose );
	dialog->setHelp(QString(),"kscd");
	dialog->show();
}

void KSCD::updateSettings()
{
	m_panel->setTextColor(Prefs::textColor());
	//kDebug()<<"color config:"<<Prefs::textColor();
	m_panel->setTextSizeFont(Prefs::textFont());
	//kDebug()<<"font config:"<<Prefs::textFont();
	devices->setEjectActivated(Prefs::ejectOnFinish());
	//kDebug()<<"eject setting:"<<Prefs::ejectOnFinish();
	m_panel->setEjectAct( Prefs::ejectOnFinish() );
        QString skin;
        if(Prefs::url().startsWith('/'))
            skin = Prefs::url();
        else
	    skin = KStandardDirs::installPath("data") + "kscd/skin/" + Prefs::url();
	setNewSkin( skin );
}

void KSCD::loadSettings()
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
	aboutData.addCredit(ki18n("Amine Bouchikhi"), ki18n("Current maintainer, Solid/Phonon Upgrade, QDBus connection"),"bouchikhi.amine@gmail.com");
	aboutData.addAuthor(ki18n("Aaron J. Seigo"), ki18n("Previous maintainer"), "aseigo@kde.org");
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
	KCmdLineArgs::addCmdLineOptions(options);
	KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	if (!KUniqueApplication::start())
	{
            fprintf(stderr, "kscd is already running\n");
            if (args->count() > 0 || args->isSet("start"))
            {
                QDBusInterface kscd("org.kde.kscd", "/CDPlayer", "org.kde.kscd.CDPlayer");
                if(kscd.isValid())
                {
                    // Forward the command line args to the running instance.
                    if (args->isSet("start"))
                    {
                        kscd.call("play");
                    }
                }
                args->clear();
            }
            exit(0);
	}
	KUniqueApplication a;
	KSCD *k = new KSCD();
	a.setTopWidget( k );

	k->setWindowTitle(KGlobal::caption());

	if (kapp->isSessionRestored())
	{
		// The user has no way to show it if it's hidden - so why start it hidden?
#if 0
		KConfigGroup group(KApplication::kApplication()->sessionConfig(), "General");
		if (group.readEntry("Show", false))
#endif
                {
			k->show();
		}
	}
	else
	{
            k->show();
	}
	args->clear();
	return a.exec();
}


#include "kscd.moc"
