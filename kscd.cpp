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
#include "ihm/titlePopUp.h"

using namespace Phonon;
using namespace KCDDB;
class CDDB;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;

KSCD::KSCD( QWidget *parent ) : KscdWindow(parent)
{
	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);

/**
 * General
 */
	connect(this,SIGNAL(actionClicked(QString)), this, SLOT(actionButton(QString)));
	connect(this,SIGNAL(picture(QString,QString)), this, SLOT(changePicture(QString,QString)));

	connect(this,SIGNAL(trackClicked(int)), this, SLOT(playTrack(int)));
	connect(this,SIGNAL(actionVolume(qreal)), this, SLOT(changeVolume(qreal)));

	devices = new HWControler();
	connect(devices,SIGNAL(currentTime(qint64)),this,SLOT(setTime(qint64)));
 	addSeekSlider(new Phonon::SeekSlider(devices->getMedia()));
// 	Phonon::VolumeSlider * vs = new Phonon::VolumeSlider(devices->getAudioOutPut());
// 	vs->setOrientation(Qt::Vertical);
// 	vs->setMuteVisible(false);
// 	addVolumeSlider(vs);

/**
 * CDDB
 */
// TODO kept for CDDB compatibility
// TODO deactivate CDDB options if no disc
 	m_cd = new KCompactDisc();
 	m_cd->setDevice(Prefs::cdDevice(), 50, false, QString("phonon"), Prefs::audioDevice());

	// CDDB Initialization
	m_cddbManager = new CDDBManager(this);
	

	connect(m_cddbManager, SIGNAL(showArtistLabel(QString)), this, SLOT(showArtistLabel(QString)));
	connect(m_cddbManager, SIGNAL(showTrackinfoLabel(QString)), this, SLOT(showTrackinfoLabel(QString)));
	connect(m_cddbManager, SIGNAL(restoreArtistLabel()), this, SLOT(restoreArtistLabel()));
	connect(m_cddbManager, SIGNAL(restoreTrackinfoLabel()), this, SLOT(restoreTrackinfoLabel()));
	
	connect(devices,SIGNAL(trackChanged()),this,SLOT(restoreTrackinfoLabel()));
	connect(devices,SIGNAL(cdLoaded()),m_cddbManager,SLOT(refreshCDDB()));

/**
 * Contextual Menu
 */
	// Set context menu policy to ActionsContextMenu
	setContextMenuPolicy(Qt::ActionsContextMenu);

	QAction* CDDBWindowAction = new QAction(i18n("CDDB..."), this);
	addAction(CDDBWindowAction);
	connect(CDDBWindowAction, SIGNAL(triggered()), m_cddbManager, SLOT(CDDialogSelected()));

	QAction* CDDBDownloadAction = new QAction(i18n("Download Information"), this);
	addAction(CDDBDownloadAction);
	connect(CDDBDownloadAction, SIGNAL(triggered()), m_cddbManager, SLOT(lookupCDDB()));

	QAction* test = new QAction(i18n("test"), this);
	addAction(test);
	connect(test, SIGNAL(triggered()), this, SLOT(test()));

//  	instantiating of kscd title popup
	w_titlePopUp = new TitlePopUp(0);
}

KSCD::~KSCD()
{
	delete devices;
	delete m_cd;
	delete m_cddbManager;
	delete w_titlePopUp;	//deleting of the title popup

/*	delete m_cddialog;
	delete m_cddb;*/
}

void KSCD::test()
{
	kDebug () << "total time : " << devices->getTotalTime() ;
}

/**
 * CDDB Management
 */

void KSCD::restoreArtistLabel()
{
	kDebug() << "NbTracks CDDB = " << m_cddbManager->getCddbInfo().numberOfTracks();
	kDebug() << "NbTracks Devices = " << devices->getTotalTrack();

	if( devices->getCD()->isCdInserted() && devices->isDiscValid() )
	{
		QString artist, title;

		if (m_cddbManager->getCddbInfo().isValid()/* && m_cddbManager->getCddbInfo().numberOfTracks() == devices->getTotalTrack()*/) {
			artist = m_cddbManager->getCddbInfo().get(KCDDB::Artist).toString();
			title = m_cddbManager->getCddbInfo().get(KCDDB::Title).toString();
		}
		else
		{
			artist = i18n("Unknown artist");
			title = i18n("Unknown album");
		}
		showArtistLabel(QString("%1 - %2").arg(artist, title));
	}
	else
	{
		showArtistLabel(i18n("<b>Welcome to KsCD !</b>"));
	}

}

void KSCD::restoreTrackinfoLabel()
{
	QString title, length ;

	// If disc is inserted
	if (devices->getCD()->isCdInserted())
	{
		title = QString("%1 - ").arg(devices->getCurrentTrack(), 2, 10, QLatin1Char('0')) ;
		//setting length
		length = "";

		if (m_cddbManager->getCddbInfo().isValid()/* && m_cddbManager->getCddbInfo().numberOfTracks() == devices->getTotalTrack()*/)
		{
			title.append(m_cddbManager->getCddbInfo().track(devices->getCurrentTrack()-1).get(KCDDB::Title).toString());
			length.append(m_cddbManager->getCddbInfo().track(devices->getCurrentTrack()-1).get(KCDDB::Length).toString());
		}
		else
		{
			title.append(i18n("unknown"));
			length.append(i18n("unknown"));
		}
		showTrackinfoLabel(title);

//showing the titlt popup with title info and title lenght
		showTitlePopUp(title, length);
//programming title popup hiding
		QTimer::singleShot(5000, this, SLOT(hideTitlePopUp()));
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


void KSCD::playTrack(int track)
{
	kDebug()<<"playtrack enter "<<track;
	devices->play(track);
	emit(picture("play","default"));
}

/**
*show a popup containning curent track title an his length
*/
void KSCD::showTitlePopUp(QString trackTitle, QString trackLength){
		
	w_titlePopUp->lengthLbl->setText(trackLength);
	w_titlePopUp->titleLbl->setText(trackTitle);
	w_titlePopUp->show();
	
}

/**
*hide the title popUp
*/
void KSCD::hideTitlePopUp(){
	
	if (this->w_titlePopUp != NULL){
		//hiding the title popUp
		this->w_titlePopUp->hide();
	}

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
	}
	if(name=="unmute")
	{
		devices->mute(true);
		emit(picture(name,state));
	}
	if(name == "random")
	{
		kDebug() << 5;
		devices->setRandom(false);
		emit(picture(name,state));
	}
	if(name == "p_random")
	{
		kDebug() << 6;
		devices->setRandom(true);
		emit(picture(name,state));
	}
	if(name == "loop")
	{
		devices->setLoopMode(NoLoop);
		emit(picture(name,state));
	}
	if(name == "looptrack")
	{
		devices->setLoopMode(LoopOne);
		emit(picture(name,state));
	}
	if(name == "loopdisc")
	{
		devices->setLoopMode(LoopAll);
		emit(picture(name,state));
	}
	if(name == "tracklist")
	{
		kDebug()<<"state track window:"<<m_stateTrackWindow;
		if(m_stateTrackWindow == true)
		{
			kDebug()<<"close track window";
		}
		else
		{
			createTrackWindow(m_cddbManager->getTrackList(),m_cddbManager->getDiscTitle());
			kDebug()<<"open track window";
		}
		emit(picture(name,"default"));
	}
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

KCompactDisc* KSCD::getCd()
{
	return m_cd;
}

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

#include "kscd.moc"
