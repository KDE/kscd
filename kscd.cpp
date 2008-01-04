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
	//KscdWindow *window = new KscdWindow;
	//window->show();

	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);
//	setupUi(this);
	
	connect(this,SIGNAL(actionClicked(QString)), this, SLOT(actionButton(QString)));
	connect(this,SIGNAL(picture(QString,StateButton)), this, SLOT(changePicture(QString,StateButton)));
	
	devices = new HWControler();

	/* CDDB initialization */
	cddbInfo.clear(); // The first freedb revision is "0"
	cddb = new KCDDB::Client();
	cddialog = 0L;
	connect(cddb, SIGNAL(finished(KCDDB::Result)), this, SLOT(lookupCDDBDone(KCDDB::Result)));
	connect(this, SIGNAL(CDDBClicked()), SLOT(CDDialogSelected()));

// TODO kept for CDDB compatibility
	m_cd = new KCompactDisc();
	m_cd->setDevice(Prefs::cdDevice(), 50, Prefs::digitalPlayback(), QString("cdin"), Prefs::audioDevice());

	connect(this,SIGNAL(actionClicked(QString)),SLOT(actionButton(QString)));
	connect(this,SIGNAL(picture(QString,StateButton)),SLOT(changePicture(QString,StateButton)));
	devices = new HWControler();
	connect(devices,SIGNAL(currentTime(qint64)),this,SLOT(setTime(qint64)));
	addSeekSlider(new Phonon::SeekSlider(devices->getMedia()));

}

KSCD::~KSCD()
{
	delete devices;
	delete m_cd;
}

/**
 * Link IHM with actions
 */
void KSCD::actionButton(QString name)
{
	StateButton state = Released;
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
	}
	if(name=="next")
	{
		devices->nextTrack();
		emit(picture(name,state));
	}
	if(name=="previous")
	{
		devices->prevTrack();
		emit(picture(name,state));
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
		emit(CDDBClicked());
		emit(picture(name,state));
	}
}

/**
 * CDDB
 */
void KSCD::CDDialogSelected()
{
	if (!cddialog)
	{
		cddialog = new CDDBDlg(this);

		connect(cddialog,SIGNAL(cddbQuery()),SLOT(lookupCDDB()));
		connect(cddialog,SIGNAL(newCDInfoStored(KCDDB::CDInfo)), SLOT(setCDInfo(KCDDB::CDInfo)));
		connect(cddialog,SIGNAL(finished()),SLOT(CDDialogDone()));
		connect(cddialog,SIGNAL(play(int)),cddialog,SLOT(slotTrackSelected(int)));
	}

	
	cddialog->show();
	cddialog->raise();

}

void KSCD::CDDialogDone()
{
	cddialog->delayedDestruct();
	cddialog = 0L;
}

void KSCD::lookupCDDB()
{
	if (devices->getCD().isCdInserted())
	{
		kDebug() << "!!!!!! lookupCDDB() called !!!!!!";
		
		showArtistLabel(i18n("Start freedb lookup."));
	
		cddb->config().reparse();
		cddb->setBlockingMode(false);

//TODO get CD signature through Solid
		cddb->lookup(m_cd->discSignature());
	}
}

void KSCD::showArtistLabel(QString infoStatus)
{
	setArtistLabel(infoStatus);
}

void KSCD::lookupCDDBDone(Result result)
{
	kDebug() << "lookupcddbdone :" << result ;

	if ((result != KCDDB::Success) && (result != KCDDB::MultipleRecordFound))
	{
		if(result == NoRecordFound)
			showArtistLabel(i18n("No matching freedb entry found."));
		else
			showArtistLabel(i18n("Error getting freedb entry."));

		QTimer::singleShot(3000, this, SLOT(restoreArtistLabel()));
		return;
	}

	// The intent of the original code here seems to have been to perform the
	// lookup, and then to convert all the string data within the CDDB response
	// using the use Prefs::selectedEncoding() and a QTextCodec. However, that
	// seems to be irrelevant these days.
	KCDDB::CDInfo info = cddb->lookupResponse().first();


// TODO MULTIPLE INFO
	// TODO Why doesn't libcddb not return MultipleRecordFound?
	//if( result == KCDDB::MultipleRecordFound ) {
	if(cddb->lookupResponse().count() > 1)
	{
		CDInfoList cddb_info = cddb->lookupResponse();
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
				cddialog->setData(cddbInfo, cddialog->getTrackStartFrames());
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

	cddbInfo = info;
	//populateSongList();
	restoreArtistLabel();
}

void KSCD::restoreArtistLabel()
{
//TODO verifications
/*    if(m_cd->tracks()) {*/
        QString artist, title;

/*
        if (cddbInfo.isValid() && cddbInfo.numberOfTracks() == m_cd->tracks()) {*/
            artist = cddbInfo.get(KCDDB::Artist).toString();
            title = cddbInfo.get(KCDDB::Title).toString();
	kDebug() << "!!!!!!!!!!!!!!!restore artist label!!!!!!!!!!!!" ;

/*        } else {
            artist = m_cd->discArtist();
            title = m_cd->discTitle();
        }
*/

        showArtistLabel(QString("%1 - %2").arg(artist, title));
/*
    } else {
        showArtistLabel(i18n("NO DISC"));
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
                kscd.call( "play" );
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
