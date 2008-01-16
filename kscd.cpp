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

using namespace Phonon;
using namespace KCDDB;
class CDDB;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;

KSCD::KSCD( QWidget *parent ) : KscdWindow(parent)
{

	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);

	// Set context menu policy to ActionsContextMenu
	setContextMenuPolicy(Qt::ActionsContextMenu);

	connect(this,SIGNAL(actionClicked(QString)), this, SLOT(actionButton(QString)));
	connect(this,SIGNAL(picture(QString,QString)), this, SLOT(changePicture(QString,QString)));

	devices = new HWControler();
	connect(devices,SIGNAL(currentTime(qint64)),this,SLOT(setTime(qint64)));
	addSeekSlider(new Phonon::SeekSlider(devices->getMedia()));
	Phonon::VolumeSlider * vs = new Phonon::VolumeSlider(devices->getAudioOutPut());
	vs->setOrientation(Qt::Vertical);
	vs->setMuteVisible(false);
	addVolumeSlider(vs);


// TODO kept for CDDB compatibility
	m_cd = new KCompactDisc();
	m_cd->setDevice(Prefs::cdDevice(), 50, Prefs::digitalPlayback(), QString("phonon"), Prefs::audioDevice());

	// CDDB Initialization
	m_cddbManager = new CDDBManager();

// TODO inactivate CDDB options if no disc
	// Adds CDDB to the context menu
	QAction* CDDBWindowAction = new QAction(i18n("CDDB..."), this);
	addAction(CDDBWindowAction);
	connect(CDDBWindowAction, SIGNAL(triggered()), m_cddbManager, SLOT(CDDialogSelected()));

	QAction* CDDBDownloadAction = new QAction(i18n("Download Information"), this);
	addAction(CDDBDownloadAction);
	connect(CDDBDownloadAction, SIGNAL(triggered()), this, SLOT(lookupCDDB()));

	connect(m_cddbManager, SIGNAL(showArtistLabel(QString)), this, SLOT(showArtistLabel(QString)));
	connect(m_cddbManager, SIGNAL(showTrackinfoLabel(QString)), this, SLOT(showTrackinfoLabel(QString)));
	connect(m_cddbManager, SIGNAL(restoreTrackinfoLabel()), this, SLOT(restoreTrackinfoLabel()));
	currentTrack=0;

}

KSCD::~KSCD()
{
	delete devices;
	delete m_cd;
	delete m_cddbManager;

/*	delete m_cddialog;
	delete m_cddb;*/
}

/**
 * CDDB Management
 */
void KSCD::lookupCDDB()
{
	kDebug() << "Lookup CDDB" ;
	if (devices->getCD().isCdInserted())
	{
		showArtistLabel(i18n("Start freedb lookup."));
	
		m_cddbManager->getCddbClient()->config().reparse();
		m_cddbManager->getCddbClient()->setBlockingMode(false);
		
//TODO get CD signature through Solid
		m_cddbManager->getCddbClient()->lookup(m_cd->discSignature());

		kDebug() << m_cd->discSignature() ;
	}
	else{
		showArtistLabel(i18n("No Disc"));
		QTimer::singleShot(3000, m_cddbManager, SLOT(restoreArtistLabel()));
	}
}

void KSCD::restoreTrackinfoLabel()
{
	kDebug() << "!!!!!!!!restore Trackinfo label!!!!!!" ;
	QString num, title;

	if (m_cddbManager->getCddbInfo().isValid()/* && cddbInfo.numberOfTracks() == m_cd->tracks()*/)
	{
		title = m_cddbManager->getCddbInfo().track(currentTrack).get(KCDDB::Title).toString();
	}
	else{
		title = i18n("Unknown");
	}
	emit showTrackinfoLabel(QString("%1 - %2").arg(num, title));

}

/**
 * Link IHM with actions
 */
void KSCD::actionButton(QString name)
{
	QString state = "over";
	if(name=="play")
	{
		if((devices->getState() == StoppedState) || (devices->getState()) == PausedState)
		{
			devices->play();
			emit(picture(name,state));
		}
	}
	if(name=="pause")
	{
		if(devices->getState() == PlayingState)
		{
			devices->pause();
			emit(picture(name,state));
		}
	}
	if(name=="stop")
	{
		if ((devices->getState() == PlayingState)|| (devices->getState() == PausedState))
		{
			devices->stop();
			emit(picture(name,state));
		}
	}
	if(name=="eject")
	{
		devices->eject();
		emit(picture(name,state));
		if ((devices->getState() == PlayingState)|| (devices->getState() == PausedState))
		{
			devices->stop();
			emit(picture("stop","default"));
		}
	}
	if(name=="next")
	{
		devices->nextTrack();
		emit(picture(name,state));
		
		currentTrack++;
		kDebug() << "Track : "<<currentTrack ;
		showTrackinfoLabel(QString("%1 - %2").arg(QString(currentTrack), m_cddbManager->getCddbInfo().track(currentTrack).get(KCDDB::Title).toString())) ;
		if((devices->getState() == StoppedState) || (devices->getState() == PausedState))
		{
			devices->stop();
			emit(picture("stop","default"));
		}
		if ((devices->getState() == PlayingState))
		{
			devices->play();
		}
	}
	if(name=="previous")
	{
		devices->prevTrack();
		emit(picture(name,state));

		currentTrack--;
		kDebug() << "Track : "<<currentTrack ;
		showTrackinfoLabel(QString("%1 - %2").arg(QString(currentTrack), m_cddbManager->getCddbInfo().track(currentTrack).get(KCDDB::Title).toString())) ;
		
		if((devices->getState() == StoppedState) || (devices->getState() == PausedState))
		{
			devices->stop();
			emit(picture("stop","default"));
		}
		if ((devices->getState() == PlayingState))
		{
			devices->play();
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
		emit(picture(name,state));
	}
	if(name == "p_random")
	{
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
		//emit(CDDBClicked(m_cd->discSignature()));
		emit(picture(name,state));
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
 //   a.setMainWidget( k );

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
