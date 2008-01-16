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
#include "docking.h"
#include "kscd.h"
#include "prefs.h"
#include "cddbdlg.h"
#include "configWidget.h"
#include "kcompactdisc.h"

#include <config-alsa.h>


#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QMenu>
#include <QtDBus>

#include <kdebug.h>
#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kcmdlineargs.h>
#include <khelpmenu.h>
#include <kshortcutsdialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <kuniqueapplication.h>
#include <kcmoduleloader.h>
#include <ktoolinvocation.h>

#include <phonon/phononnamespace.h>
#include <phonon/seekslider.h>

#include "ihm/kscdwindow.h"

using namespace KCDDB;
using namespace Phonon;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;

KSCD::KSCD( QWidget *parent ) : KscdWindow()
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


	/* CDDB initialization */
	m_cddbInfo.clear(); // The first freedb revision is "0"
	m_cddb = new KCDDB::Client();
	m_cddialog = 0L;
	connect(m_cddb, SIGNAL(finished(KCDDB::Result)), this, SLOT(lookupCDDBDone(KCDDB::Result)));


// TODO kept for CDDB compatibility
	m_cd = new KCompactDisc();
	m_cd->setDevice(Prefs::cdDevice(), 50, Prefs::digitalPlayback(), QString("cdin"), Prefs::audioDevice());

// TODO inactivate CDDB options if no disc
	// Adds CDDB to the context menu
	QAction* CDDBWindowAction = new QAction(i18n("CDDB..."), this);
	addAction(CDDBWindowAction);
	connect(CDDBWindowAction, SIGNAL(triggered()), this, SLOT(CDDialogSelected()));

	// Adds download CDDB information
	QAction* downloadCDDBAction = new QAction(i18n("Download CDDB"), this);
	addAction(downloadCDDBAction);

	connect(downloadCDDBAction, SIGNAL(triggered()), this, SLOT(lookupCDDB()));
	
}

KSCD::~KSCD()
{
	delete devices;
	delete m_cd;
	delete m_cddialog;
	delete m_cddb;
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
		emit(picture(name,state));
	}
}

/**
 * CDDB
 */
void KSCD::CDDialogSelected()
{

	if (!m_cddialog)
	{
		kDebug() << "Create a CDDB Dialog" ;
		/* CDDB Dialog initialization */
		m_cddialog = new CDDBDlg(this);
		connect(m_cddialog,SIGNAL(newCDInfoStored(KCDDB::CDInfo)), SLOT(setCDInfo(KCDDB::CDInfo)));
 		connect(m_cddialog,SIGNAL(finished()),SLOT(CDDialogDone()));
// 		connect(cddialog,SIGNAL(play(int)),cddialog,SLOT(slotTrackSelected(int)));
	}

	m_cddialog->show();
//	m_cddialog->raise(); // Puts the window on top

}

void KSCD::CDDialogDone()
{
	m_cddialog->delayedDestruct();
	m_cddialog = 0L;
}

void KSCD::lookupCDDB()
{
	if (devices->getCD().isCdInserted())
	{
		showArtistLabel(i18n("Start freedb lookup."));
	
		m_cddb->config().reparse();
		m_cddb->setBlockingMode(false);

//TODO get CD signature through Solid
		m_cddb->lookup(m_cd->discSignature());
	}
	else{
		showArtistLabel(i18n("No Disc"));
		QTimer::singleShot(3000, this, SLOT(restoreArtistLabel()));
	}
}

void KSCD::showArtistLabel(QString infoStatus)
{
	setArtistLabel(infoStatus);
}

void KSCD::lookupCDDBDone(Result result)
{
	if (result != KCDDB::Success)
	{
		if(result == NoRecordFound)
			showArtistLabel(i18n("No matching freedb entry found."));
		else
			showArtistLabel(i18n("Error getting freedb entry."));

		QTimer::singleShot(3000, this, SLOT(restoreArtistLabel()));
		return;
	}

	KCDDB::CDInfo info = m_cddb->lookupResponse().first();

	// If there is more than 1 result display a choice window
	if(m_cddb->lookupResponse().count() > 1) {
		CDInfoList cddb_info = m_cddb->lookupResponse();
		QStringList list;
		CDInfoList::iterator it;
		for ( it = cddb_info.begin(); it != cddb_info.end(); ++it  )
		{
			list.append( QString("%1, %2, %3, %4").arg((*it).get(Artist).toString()).arg((*it).get(Title).toString()).arg((*it).get(Genre).toString()).arg((*it).get(KCDDB::Category).toString()));
		}
	
		bool ok(false);
		QString res = KInputDialog::getItem(
				i18n("Select CDDB Entry"),
				i18n("Select a CDDB entry:"), list, 0, false, &ok,
				this );
		if(ok)
		{
			// The user selected and item and pressed OK
			int c = 0;
			for(QStringList::Iterator it = list.begin(); it != list.end(); ++it )
			{
				if( *it == res) break;
				c++;
			}
			// Ensure that the selected item is within the range of the number of CDDB entries found
			if( c < cddb_info.size() )
			{
				info = cddb_info[c];
			}
		}
		else
		{
			return;
			// user pressed Cancel
		}
	}

	setCDInfo(info);
}

void KSCD::setCDInfo(KCDDB::CDInfo info)
{
	// Some sanity provisions to ensure that the number of records matches what
	// the CD actually contains.
	// TODO Q_ASSERT(info.numberOfTracks() == cd->tracks());

	m_cddbInfo = info;
	populateSongList();
	restoreArtistLabel();
}

void KSCD::restoreArtistLabel()
{
	if (devices->getCD().isCdInserted())
	{
		QString artist, title;


		if (m_cddbInfo.isValid()/* && cddbInfo.numberOfTracks() == m_cd->tracks()*/) {
			artist = m_cddbInfo.get(KCDDB::Artist).toString();
			title = m_cddbInfo.get(KCDDB::Title).toString();
			kDebug() << "!!!!!!!!!!!!!!!restore artist label!!!!!!!!!!!!" ;
		}
		else{
			artist = m_cd->discArtist();
			title = m_cd->discTitle();
		}
		showArtistLabel(QString("%1 - %2").arg(artist, title));

	} else {
		showArtistLabel(i18n("Welcome to KsCD !"));
	}

}

void KSCD::populateSongList()
{
/*
    songListCB->clear();
    for (uint i = 1; i <= m_cd->tracks(); ++i)
    {
        unsigned tmp = m_cd->trackLength(i);
        unsigned mymin;
        unsigned mysec;
        mymin = tmp / 60;
        mysec = (tmp % 60);
        QString str1;
        str1.sprintf("%02u: ", i);

        QString str2;
        str2.sprintf(" (%02u:%02u) ", mymin,  mysec);
	// see comment before restoreArtistLabel()
        if (cddbInfo.isValid() && cddbInfo.numberOfTracks() == m_cd->tracks()) {
            const KCDDB::TrackInfo& t = cddbInfo.track(i - 1);

            if (cddbInfo.get(KCDDB::Artist).toString() != t.get(KCDDB::Artist).toString())
                str1.append(t.get(KCDDB::Artist).toString()).append(" - ");
            str1.append(t.get(KCDDB::Title).toString());
        } else {
            if (m_cd->discArtist() != m_cd->trackArtist(i))
                str1.append(m_cd->trackArtist(i)).append(" - ");
            str1.append(m_cd->trackTitle(i));
        }
        str1.append(str2);
        songListCB->addItem(str1);
    }
*/
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
    a.setMainWidget( k );

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
