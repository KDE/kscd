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


/****************************************************************************
                  The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent )
  : QWidget( parent )
{
	KscdWindow *window = new KscdWindow;

	window->show();
	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);
	setupUi(this);


	kDebug()<<"3";

	connect(window,SIGNAL(actionClicked(QString)),SLOT(actionButton(QString)));
	connect(this,SIGNAL(picture(QString,StateButton)),window,SLOT(changePicture(QString,StateButton)));
	devices = new HWControler();
	connect(devices,SIGNAL(currentTime(qint64)),window,SLOT(setTime(qint64)));
	window->addSeekSlider(new Phonon::SeekSlider(devices->getMedia()));

}

KSCD::~KSCD()
{
}


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
		emit(picture(name,state));
	}
}

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
