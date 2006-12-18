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
#include <config.h>
#include <config-alsa.h>
#include <QDir>
#include <QRegExp>
#include <qtextstream.h>
#include <QLayout>
#include <q3hbox.h>
#include <q3vbox.h>
#include <qapplication.h>
#include <q3groupbox.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QMenu>
#include <stdlib.h>

#include <kaboutdata.h>
#include <kactioncollection.h>

#include <kaction.h>
#include <kcharsets.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kemailsettings.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kkeydialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kprotocolmanager.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <kuniqueapplication.h>
#include <kglobalsettings.h>
#include <kcmoduleloader.h>
#include <kconfigdialog.h>
#include <QtDBus>
#include "docking.h"
#include "kscd.h"
#include "version.h"
#include "prefs.h"

#include <kwin.h>
#include <netwm.h>

#include <config.h>

#include "cddbdlg.h"
#include "configWidget.h"
#include <qtextcodec.h>
#include "kcompactdisc.h"
#include <fixx11h.h>
#include <ktoolinvocation.h>

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = false;


/****************************************************************************
                  The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name )
  : QWidget( parent ),
    kscdPanelDlg( ),
    configDialog(0L),
    cddialog(0L),  //!!!!
    jumpToTrack(0L),
    updateTime(true),
    m_dockWidget(0)
{
    QDBusConnection::sessionBus().registerObject("/CDPlayer", this);
    setupUi(this);
  m_cd = new KCompactDisc();
  cddbInfo.clear(); // The first freedb revision is "0" //!!!!
  random_current      = random_list.begin();

  cddb = new KCDDB::Client();
  connect(cddb, SIGNAL(finished(CDDB::Result)), this, SLOT(lookupCDDBDone(CDDB::Result)));

  audio_systems_list
                    << "phonon"
#ifdef USE_ARTS
                     << "arts"
#endif
#if defined(HAVE_LIBASOUND2)
                     << "alsa"
#endif
#if defined(sun) || defined(__sun__)
                     << "sun"
#endif
  ;

  readSettings();
  initFont();
  drawPanel();
  setColors();

  // the time slider
  timeIcon->setPixmap(SmallIcon("player_time"));
  connect(timeSlider, SIGNAL(sliderPressed()), SLOT(timeSliderPressed()));
  connect(timeSlider, SIGNAL(sliderReleased()), SLOT(timeSliderReleased()));
  connect(timeSlider, SIGNAL(sliderMoved(int)), SLOT(timeSliderMoved(int)));
  connect(timeSlider, SIGNAL(valueChanged(int)), SLOT(jumpToTime(int)));

  // the volume slider
  volumeIcon->setPixmap(SmallIcon("player_volume"));
  volumeSlider->setValue(Prefs::volume());
  QString str;
  str = ki18n("Vol: %1%").subs(Prefs::volume(), 2).toString();
  volumelabel->setText(str);
  connect(volumeSlider, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));
  connect(m_cd, SIGNAL(trackPlaying(unsigned, unsigned)), this, SLOT(trackUpdate(unsigned, unsigned)));
  connect(m_cd, SIGNAL(trackPaused(unsigned, unsigned)), this, SLOT(trackUpdate(unsigned, unsigned)));
  connect(m_cd, SIGNAL(trackChanged(unsigned, unsigned)), this, SLOT(trackChanged(unsigned, unsigned)));
  connect(m_cd, SIGNAL(discStopped()), this, SLOT(discStopped()));
  connect(m_cd, SIGNAL(discChanged(unsigned)), this, SLOT(discChanged(unsigned)));
  connect( &queryledtimer, SIGNAL(timeout()),  SLOT(togglequeryled()) );
  connect( &titlelabeltimer, SIGNAL(timeout()),  SLOT(titlelabeltimeout()) );
  connect( &cycletimer, SIGNAL(timeout()),  SLOT(cycletimeout()) );
  connect( &jumpTrackTimer, SIGNAL(timeout()),  SLOT(jumpTracks()) );
  connect( playPB, SIGNAL(clicked()), SLOT(playClicked()) );
  connect( nextPB, SIGNAL(clicked()), SLOT(nextClicked()) );
  connect( prevPB, SIGNAL(clicked()), SLOT(prevClicked()) );
  connect( stopPB, SIGNAL(clicked()), SLOT(stopClicked()) );
  connect( ejectPB, SIGNAL(clicked()), SLOT(ejectClicked()) );
  connect( repeatPB, SIGNAL(clicked()), SLOT(loopClicked()) );
  connect( songListCB, SIGNAL(activated(int)), SLOT(trackSelected(int)));
  connect( shufflePB, SIGNAL(clicked()), SLOT(randomSelected()));
  connect( cddbPB, SIGNAL(clicked()), SLOT(CDDialogSelected()));
  connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(setColors()));
  connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)), this, SLOT(setIcons()));
  QToolTip::remove(songListCB);
  songListCB->setToolTip( i18n("Track list"));


  // set up the actions and keyboard accels
  m_actions = new KActionCollection(this);
  m_actions->setConfigGroup("Shortcuts");

  KAction* action;
  action = new KAction(i18n("Play/Pause"), m_actions, "Play/Pause");
  connect(action, SIGNAL(triggered(bool) ), SLOT(playClicked()));
  action->setShortcut(Qt::Key_P);
  action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_P));

  action = new KAction(i18n("Stop"), m_actions, "Stop");
  connect(action, SIGNAL(triggered(bool) ), SLOT(stopClicked()));
  action->setShortcut(Qt::Key_S);
  action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_S));

  action = new KAction(i18n("Previous"), m_actions, "Previous");
  connect(action, SIGNAL(triggered(bool) ), SLOT(prevClicked()));
  action->setShortcut(Qt::Key_B);
  //NOTE: WIN+B collidates with amarok's default global shortcut.
  action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_B));

  action = new KAction(i18n("Next"), m_actions, "Next");
  connect(action, SIGNAL(triggered(bool) ), SLOT(nextClicked()));
  action->setShortcut(Qt::Key_N);
  action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_N));

  action = KStandardAction::quit(this, SLOT(quitClicked()), m_actions);
  action = KStandardAction::keyBindings(this, SLOT(configureKeys()), m_actions, "options_configure_shortcuts");
  action = KStandardAction::preferences(this, SLOT(showConfig()), m_actions);
  action = new KAction(i18n("Loop"), m_actions, "Loop");
  connect(action, SIGNAL(triggered(bool) ), SLOT(loopClicked()));
  action->setShortcut(Qt::Key_L);
  action = new KAction(i18n("Eject"), m_actions, "Eject");
  connect(action, SIGNAL(triggered(bool) ), SLOT(ejectClicked()));
  action->setShortcut(Qt::CTRL + Qt::Key_E);
  action = new KAction(i18n("Increase Volume"), m_actions, "IncVolume");
  connect(action, SIGNAL(triggered(bool) ), SLOT(incVolume()));
  KShortcut increaseVolume(QKeySequence(Qt::Key_Plus), QKeySequence(Qt::Key_Equal));
  action->setShortcut( increaseVolume );
  action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Plus));

  action = new KAction(i18n("Decrease Volume"), m_actions, "DecVolume");
  connect(action, SIGNAL(triggered(bool) ), SLOT(decVolume()));
  action->setShortcut(Qt::Key_Minus);
  action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Minus));

  action = new KAction(i18n("Options"), m_actions, "Options");
  connect(action, SIGNAL(triggered(bool) ), SLOT(showConfig()));
  action->setShortcut(Qt::CTRL + Qt::Key_T);
  action = new KAction(i18n("Shuffle"), m_actions, "Shuffle");
  connect(action, SIGNAL(triggered(bool) ), SLOT(randomSelected()));
  action->setShortcut(Qt::Key_R);
  action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_R));

  action = new KAction(i18n("CDDB"), m_actions, "CDDB");
  connect(action, SIGNAL(triggered(bool) ), SLOT(CDDialogSelected()));
  action->setShortcut(Qt::CTRL + Qt::Key_D);
  m_actions->readSettings();

  initGlobalShortcuts();

  setupPopups();

  if (Prefs::looping())
  {
    loopled->on();
    loopled->show();
    repeatPB->setOn(true);
  }

  setDocking(Prefs::docking());

  setFocusPolicy(Qt::NoFocus);

  songListCB->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  adjustSize();
  setFixedHeight(this->height());

/* FIXME check for return value */
  setDevicePaths();
} // KSCD


KSCD::~KSCD()
{
    delete cddb;
    delete m_cd;
} // ~KSCD


void KSCD::initGlobalShortcuts() {
  KGlobalAccel::self()->setConfigGroup( "GlobalShortcuts" );
  KGlobalAccel::self()->readSettings();
}

bool KSCD::digitalPlayback() {
        return !(Prefs::audioSystem().isEmpty());
}

void KSCD::setVolume(int v)
{
    volChanged(v);
    volumeSlider->setValue(v);
}

void KSCD::setDevice(const QString& dev)
{
    Prefs::setCdDevice(dev);
    setDevicePaths();
}

/**
 * Initialize smallfont which fits into the 13 and 14 pixel widgets.
 */
void KSCD::initFont()
{
/*  int theSmallPtSize = 10;

  // Find a font that fits the 13 and 14 pixel widgets
  QFont fn( KGlobalSettings::generalFont().family(), theSmallPtSize, QFont::Bold );
  bool fits = false;
  while (!fits && theSmallPtSize > 1)
  {
      QFontMetrics metrics(fn);
      if(metrics.height() > 13)
      {
          --theSmallPtSize;
          fn.setPointSize(theSmallPtSize);
      } else {
          fits = true;
      }
  }
  smallfont = QFont(KGlobalSettings::generalFont().family(), theSmallPtSize, QFont::Bold);
*/
} // initFont()

/**
 * drawPanel() constructs KSCD's little black LED area
 * all settings are made via panel.ui
 */
void KSCD::drawPanel()
{
  setIcons();
  adjustSize();

  const int D = 6;
  for (int u = 0; u < 5; u++) {
     trackTimeLED[u] = new BW_LED_Number(frameleds);
     trackTimeLED[u]->setLEDoffColor(Prefs::backColor());
     trackTimeLED[u]->setLEDColor(Prefs::ledColor(), Prefs::backColor());
     trackTimeLED[u]->setGeometry(2 + u * 18, D, 23,  30);
     connect(trackTimeLED[u], SIGNAL(clicked()), this, SLOT(cycleplaytimemode()));
  }

  setLEDs(-1);

  queryled = new LedLamp(symbols);
  queryled->move(+10, D + 1);
  queryled->off();
  queryled->hide();

  loopled = new LedLamp(symbols, LedLamp::Loop);
  loopled->move(+10, D + 18);
  loopled->off();

  totaltimelabel->hide();
} // drawPanel

void KSCD::setIcons()
{
  playPB->setIcon(SmallIconSet("player_play"));
  stopPB->setIcon(SmallIconSet("player_stop"));
  ejectPB->setIcon(SmallIconSet("player_eject"));
  prevPB->setIcon(SmallIconSet("player_start"));
  nextPB->setIcon(SmallIconSet("player_end"));
  cddbPB->setIcon(SmallIconSet("view_text"));
  infoPB->setIcon(SmallIconSet("run"));
}

void KSCD::setupPopups()
{
    QMenu* mainPopup   = new QMenu(this);
    infoPB->setMenu(mainPopup);
    infoPopup   = new QMenu (this);


    infoPopup->insertItem("MusicMoz", 0);
    infoPopup->insertItem("Ultimate Bandlist", 1);
    infoPopup->insertItem("CD Universe", 2);
    infoPopup->addSeparator();
    infoPopup->insertItem("AlltheWeb", 3);
    infoPopup->insertItem("Altavista", 4);
    infoPopup->insertItem("Excite", 5);
    infoPopup->insertItem("Google", 6);
    infoPopup->insertItem("Google Groups", 7);
    infoPopup->insertItem("HotBot", 8);
    infoPopup->insertItem("Lycos", 9);
    infoPopup->insertItem("Open Directory", 10);
    infoPopup->insertItem("Yahoo!", 11);

    mainPopup->addAction( m_actions->action(KStandardAction::name(KStandardAction::Preferences)) );
    //NEW add the shortcut dialogs
    mainPopup->addAction( m_actions->action("options_configure_globals") );
    mainPopup->addAction( m_actions->action("options_configure_shortcuts") );
    mainPopup->addSeparator();

    mainPopup->insertItem(i18n("Artist Information"), infoPopup);

    connect( infoPopup, SIGNAL(activated(int)), SLOT(information(int)) );

    KHelpMenu* helpMenu = new KHelpMenu(this, KGlobal::instance()->aboutData(), false);
    mainPopup->insertItem(SmallIcon("help"),i18n("&Help"), helpMenu->menu());
    mainPopup->addSeparator();
    mainPopup->addAction( m_actions->action(KStandardAction::name(KStandardAction::Quit)) );
} // setupPopups

void KSCD::setPlayStatus(void)
{
    // Update UI to allow a subsequent pause.
    statuslabel->setText(i18n("Play"));
    playPB->setIcon(SmallIconSet("player_pause"));
    playPB->setText(i18n("Pause"));
}

void KSCD::playClicked()
{
    if (m_cd->discId() == KCompactDisc::missingDisc)
        return;

    kapp->processEvents();
    kapp->flush();

    if (!m_cd->isPlaying())
    {
        kapp->processEvents();
        kapp->flush();

        if (m_cd->isPaused())
        {
            // Unpause (!!).
            m_cd->pause();
        }
        else
        {
            setLEDs(0);
            resetTimeSlider(true);

            if(Prefs::randomPlay())
            {
                make_random_list();
                // next clicked handles updating the play button, etc.
                nextClicked();
            }
            else
            {
                m_cd->play(0, 0, playlist.isEmpty() ? 0 : 1);
            }
        }

        setPlayStatus();
    }
    else
    {
        m_cd->pause();

        // Update UI to allow a subsequent play.
        statuslabel->setText(i18n("Pause"));
        playPB->setIcon(SmallIconSet("player_play"));
        playPB->setText(i18n("Play"));
    }

    kapp->processEvents();
    kapp->flush();
} // playClicked()

void KSCD::setShuffle(int shuffle)
{
    if (shuffle == 2) {
        if(Prefs::randomPlay() && m_cd->tracks() > 0) {
            shufflePB->blockSignals(true);
            shufflePB->setOn(true);
            shufflePB->blockSignals(false);
            make_random_list(); /* koz: Build a unique, once, random list */
            if(m_cd->isPlaying())
                nextClicked();
        }

        return;
    }

    Prefs::setRandomPlay(shuffle);
    shufflePB->blockSignals(true);
    shufflePB->setOn(shuffle);
    shufflePB->blockSignals(false);

    if (Prefs::randomPlay() && m_cd->tracks() > 0) {
        make_random_list(); /* koz: Build a unique, once, random list */
        if(m_cd->isPlaying())
            nextClicked();
    }
}

void KSCD::stopClicked()
{
    stoppedByUser = true;

    kapp->processEvents();
    kapp->flush();
    m_cd->stop();
} // stopClicked()

void KSCD::prevClicked()
{
    int track = m_cd->track();

    if (Prefs::randomPlay()) {
        track = prev_randomtrack();
        if (track == -1) {
            return;
        }
    } else {
        if (track <= 1) {
            if (Prefs::looping()) {
                track = m_cd->tracks();
            } else {
                return;
            }
        } else {
            track--;
        }
    }

    kapp->processEvents();
    kapp->flush();
    m_cd->play(track, 0, playlist.isEmpty() ? 0 : track);

    setPlayStatus();

} // prevClicked()

bool KSCD::nextClicked()
{
    unsigned track = m_cd->track();

    if (Prefs::randomPlay()) {
        track = next_randomtrack();
        if(track == 0) {
            return false;
        }
    } else {
        if(track < 1) {
            track = 1;
        } else if (track >= m_cd->tracks()) {
            if (Prefs::looping()) {
                track = 1;
            } else {
                return true;
            }
        } else {
            track++;
        }
    }

    kapp->processEvents();
    kapp->flush();
    m_cd->play(track, 0, Prefs::randomPlay() || !playlist.isEmpty() ? track + 1 : 0);

    setPlayStatus();

    return true;
} // nextClicked()

void KSCD::trackChanged(unsigned track, unsigned trackLength)
{
    QString tooltip = artistlabel->text();
    if (track < 1)
    {
        setLEDs(-1);
        resetTimeSlider(true);
        tracklabel->setText("--/--");
        titlelabel->clear();
    }
    else
    {
//             if (!nextClicked())
//             {
//                 statuslabel->setText(i18n("Disc Finished"));
//                 m_cd->stop();
//             }
//            break;

        if (songListCB->count())
        {
            songListCB->setCurrentIndex(track - 1);
            // drop the number.
            // for Mahlah, a picky though otherwise wonderful person - AJS
            QString justTheName = songListCB->currentText();
            justTheName = justTheName.right(justTheName.length() - 4);

            songListCB->setToolTip( i18n("Current track: %1", justTheName));
        }
        timeSlider->blockSignals(true);
        timeSlider->setRange(0, trackLength ? trackLength - 1 : 0);
        timeSlider->blockSignals(false);
        QString str;
        str.sprintf("%02d/%02d", track, m_cd->tracks());
        tracklabel->setText(str);

	QString title;
	if (cddbInfo.track(track-1).get(Artist) != cddbInfo.get(Artist))
	  title.append(cddbInfo.track(track-1).get(Artist).toString()).append(" - ");
        title.append(cddbInfo.track(track-1).get(Title).toString());
        titlelabel->setText(title);
        tooltip += '/' + KStringHandler::rsqueeze(title, 30);
    }
    emit trackChanged(tooltip);
} //trackChanged(int track)


void KSCD::jumpToTime(int ms, bool forcePlay)
{
    kapp->processEvents();
    kapp->flush();

    int track = m_cd->track();
    if ((m_cd->isPlaying() || forcePlay) &&
        ms < (int)m_cd->trackLength())
    {
        if(Prefs::randomPlay() || !playlist.isEmpty())
        {
            m_cd->play(track, ms, track + 1);
        }
        else
        {
            m_cd->play(track, ms);
        }

        setPlayStatus();
    }
} // jumpToTime(int ms)

void KSCD::timeSliderPressed()
{
    updateTime = false;
} // timeSliderPressed()

void KSCD::timeSliderMoved(int milliseconds)
{
    setLEDs(milliseconds);
} // timeSliderMoved(int seconds)

void KSCD::timeSliderReleased()
{
    updateTime = true;
} // timeSliderReleased()

void KSCD::quitClicked()
{
    // ensure nothing else starts happening
    queryledtimer.stop();
    titlelabeltimer.stop();
    cycletimer.stop();
    jumpTrackTimer.stop();

    writeSettings();
    //setShuffle(0);
    statuslabel->clear();
    setLEDs(-1);

    // Good GOD this is evil
    kapp->processEvents();
    kapp->flush();

    if(Prefs::stopExit())
        m_cd->stop();

    delete m_cd;

    kapp->quit();
} // quitClicked()

bool KSCD::event( QEvent *e )
{
    return QWidget::event(e);
} // event


void KSCD::loopOn()
{
    Prefs::setLooping(true);
    loopled->on();
    loopled->show();
    kapp->processEvents();
    kapp->flush();
} // loopOn;

void KSCD::loopOff()
{
    Prefs::setLooping(false);
    loopled->off();
    loopled->show();
    kapp->processEvents();
    kapp->flush();
} // loopOff;

void KSCD::loopClicked()
{
    if(Prefs::looping())
    {
        loopOff();
    }
    else
    {
        loopOn();
    }
} // loopClicked

/**
 * Do everything needed if the user requested to eject the disc.
 *
 */
void KSCD::ejectClicked()
{
    m_cd->eject();
} // ejectClicked

void KSCD::closeEvent(QCloseEvent *e)
{
    if (Prefs::docking() && !kapp->sessionSaving())
    {
        hide();
        e->ignore();
        return;
    }
    e->accept();
}

void KSCD::randomSelected()
{
    setShuffle(Prefs::randomPlay()?0:1);

    /* FIXME this helps us to display "Random" in Status line
       should it maybe to be replaced with symbol "RAND" or something others */
    statuslabel->setText(Prefs::randomPlay()?i18n("Random"):i18n("Play"));
} // randomSelected

/**
 * A Track was selected for playback from the drop down box.
 *
 */
void KSCD::trackSelected( int cb_index )
{
    if (cb_index < 0)
        return;

    unsigned int track = cb_index + 1;
    setShuffle(0);

    m_cd->play(track, 0);

    setPlayStatus();
} // trackSelected

void KSCD::updateConfigDialog(configWidget* widget)
{
    if(!widget)
        return;

    static QString originalTitleOfGroupBox = widget->groupBox3->title();
    if(m_cd->isPlaying()) {
        widget->groupBox3->setEnabled(false);
        widget->groupBox3->setTitle( i18n( "CD Drive (you must stop playing to change this)" ) );
    } else {
        widget->groupBox3->setEnabled(true);
        widget->groupBox3->setTitle(originalTitleOfGroupBox);
    }
}

void KSCD::showConfig()
{
    static configWidget* confWidget = 0;

    if (KConfigDialog::showDialog("settings")) {
        updateConfigDialog(confWidget);
        return;
    }

    configDialog = new KConfigDialog(this, "settings", Prefs::self());

    configDialog->setHelp(QString::null);

    confWidget = new configWidget(this, 0);

    // kscd config page
    configDialog->addPage(confWidget, i18n("CD Player"), "kscd", i18n("Settings & Behavior"));
    connect(configDialog, SIGNAL(okClicked()), confWidget, SLOT(save()));
    connect(configDialog, SIGNAL(applyClicked()), confWidget, SLOT(save()));
    connect(configDialog, SIGNAL(defaultClicked()), confWidget, SLOT(defaults()));

    // libkcddb page
    KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
    if (libkcddb && libkcddb->isValid())
    {
        KCModuleInfo info(libkcddb->desktopEntryPath());
        if (info.service()->isValid())
        {
            KCModule *m = KCModuleLoader::loadModule(info, KCModuleLoader::Inline);
            if (m)
            {
                m->load();
                KCDDB::Config* cfg = new KCDDB::Config();
                cfg->readConfig();
                configDialog -> addPage(m, cfg, QString("CDDB"), "cdtrack", i18n("Configure Fetching Items"));

                connect(configDialog, SIGNAL(okClicked()), m, SLOT(save()));
                connect(configDialog, SIGNAL(applyClicked()), m, SLOT(save()));
                connect(configDialog, SIGNAL(defaultClicked()), m, SLOT(defaults()));
            }
        }
    }

    updateConfigDialog(confWidget);

    connect(configDialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(configDone()));
    configDialog -> show();
} // showConfig()

void KSCD::configDone()
{
    setColors();
    setDocking(Prefs::docking());

    setDevicePaths();
    // dialog deletes itself
    configDialog = 0L;
}

void KSCD::configureKeys()
{
    KKeyDialog::configure(m_actions, KKeyChooser::LetterShortcutsAllowed, this);
}

void KSCD::setDevicePaths()
{
    if (!m_cd->setDevice(Prefs::cdDevice(), Prefs::volume(), Prefs::digitalPlayback(),
         Prefs::audioSystem(), Prefs::audioDevice()))
    {
        // This device did not seem usable.
        QString str = i18n("CD-ROM read or access error (or no audio disc in drive).\n"\
                            "Please make sure you have access permissions to:\n%1",
                            KCompactDisc::urlToDevice(Prefs::cdDevice()));
        KMessageBox::error(this, str, i18n("Error"));
    }
} // setDevicePath()

void KSCD::setDocking(bool dock)
{
    Prefs::setDocking(dock);
    if (Prefs::docking())
    {
        if (!m_dockWidget)
        {
            m_dockWidget = new DockWidget(this, "dockw");
            connect(m_dockWidget, SIGNAL(quitSelected()), this, SLOT(quitClicked()));
        }

        m_dockWidget->show();
        connect(this, SIGNAL(trackChanged(const QString&)),
                m_dockWidget, SLOT(setToolTip(const QString&)));
        connect(this, SIGNAL(trackChanged(const QString&)),
                m_dockWidget, SLOT(createPopup(const QString&)));
    }
    else
    {
        show();
        delete m_dockWidget;
        m_dockWidget = 0;
    }
}

void KSCD::incVolume()
{
   int v = Prefs::volume() + 5;

   if (v > 100)
   {
       v = 100;
   }

   volChanged(v);
   volumeSlider->setValue(v);
} // incVolume

void KSCD::decVolume()
{
   int v = Prefs::volume() - 5;

   if (v < 0)
   {
       v = 0;
   }

   volChanged(v);
   volumeSlider->setValue(v);
} // decVolume

void KSCD::volChanged( int vol )
{
    QString str;
    str = ki18n("Vol: %1%").subs(vol, 2).toString();
    volumelabel->setText(str);
    m_cd->setVolume(vol);
    Prefs::setVolume(vol);
} // volChanged

void KSCD::make_random_list()
{
    /* koz: 15/01/00. I want a random list that does not repeat tracks. Ie, */
    /* a list is created in which each track is listed only once. The tracks */
    /* are picked off one by one until the end of the list */

    int selected = 0;
    bool rejected = false;

    //kDebug(67000) << "Playlist has " << size << " entries\n" << endl;
    random_list.clear();
    for(unsigned i = 0; i < m_cd->tracks(); i++)
    {
        do {
            selected = 1 + (int) randSequence.getLong(m_cd->tracks());
            rejected = (random_list.find(selected) != random_list.end());
        } while(rejected == true);
        random_list.append(selected);
    }
/*
    printf("debug: dump random list\n");
    RandomList::iterator it;
    for(it = random_list.begin(); it != random_list.end(); it++) {
        printf("%i ", *it);
    }
    printf("\n");
*/
    random_current = random_list.end();
} // make_random_list()

int KSCD::next_randomtrack()
{
    /* Check to see if we are at invalid state */
    if(random_current == random_list.end())
    {
        random_current = random_list.begin();
    }
    else if(!random_list.empty() && random_current == --random_list.end())
    {
        if(!Prefs::looping())
        {
            m_cd->stop();
            return 0;
        }
        else
        {
            // playing the same random list isn't very random, is it?
            make_random_list();
            return next_randomtrack();
        }
    }
    else
    {
        ++random_current;
    }

    return *random_current;
} // next_randomtrack

int KSCD::prev_randomtrack()
{
    /* Check to see if we are at invalid state */
    if(random_current == random_list.end())
    {
        random_current = --random_list.end();
    }
    else if(random_current == random_list.begin())
    {
        if(!Prefs::looping())
        {
            return -1;
        }
        else
        {
            // playing the same random list isn't very random, is it?
            make_random_list();
            return prev_randomtrack();
        }
    }
    else
    {
        --random_current;
    }

    return *random_current;
} // prev_randomtrack

void KSCD::discChanged(unsigned discId)
{
    cddbInfo.clear();
    if (discId == KCompactDisc::missingDisc)
    {
        statuslabel->setText(i18n("No disc"));
    }
    else
    {
        cddbInfo.set("discid", QString::number(discId, 16).rightJustified(8,'0'));
        cddbInfo.set(Length, m_cd->discLength() / 1000);
        cddbInfo.set(Artist, m_cd->discArtist());
        cddbInfo.set(Title, m_cd->discTitle());

        // If it's a sampler, we'll do artist/title.
        bool isSampler = (cddbInfo.get(Title).toString().compare("Various") == 0);
        for (unsigned i = 1; i <= m_cd->tracks(); i++)
        {
	    KCDDB::TrackInfo track = cddbInfo.track(i-1);
            if (isSampler)
            {
	        QString title = m_cd->trackArtist(i);
		title.append("/");
		title.append(m_cd->trackTitle(i));
                track.set(Title, title);
            }
            else
            {
                track.set(Title, m_cd->trackTitle(i));
            }

            // FIXME: KDE4
            // track.length = cd->trk[i - 1].length;
        }
    }

    // Set the total time.
    QTime dml;
    dml = dml.addSecs(m_cd->discLength() / 1000);

    QString fmt;
    if(dml.hour() > 0)
        fmt.sprintf("%02d:%02d:%02d",dml.hour(),dml.minute(),dml.second());
    else
        fmt.sprintf("%02d:%02d",dml.minute(),dml.second());
    totaltimelabel->setText(fmt);

    trackChanged(0, 0);
    populateSongList("");
    //totaltimelabel->clear();
    totaltimelabel->lower();

    if ((Prefs::autoplay() || KCmdLineArgs::parsedArgs()->isSet("start"))
        && !m_cd->isPlaying())
    {
        playClicked();
    }

    // We just populated the GUI with what we got from the CD. Now look for
    // more from the Internet...
    lookupCDDB();
}

void KSCD::discStopped()
{
    if (Prefs::ejectOnFinish() && !stoppedByUser && (m_cd->discId() != KCompactDisc::missingDisc))
    {
        ejectClicked();
    }

    if (!stoppedByUser)
    {
      if (Prefs::randomPlay())
      {
        // If nextClicked returns false, it was the last
        // random track, and the player should be stopped
        if (nextClicked())
          return;
      }
      else if (Prefs::looping())
      {
        playClicked();
        return;
      }
    }

    statuslabel->setText(i18n("Stopped"));
    playPB->setText(i18n("Play"));
    playPB->setIcon(SmallIconSet("player_play"));

    /* reset to initial value, only stopclicked() sets this to true */
    stoppedByUser = false;

    trackChanged(0, 0);
    populateSongList("");
    totaltimelabel->clear();
    totaltimelabel->lower();
}

void KSCD::setLEDs(int milliseconds)
{
    QString symbols;

    if (milliseconds < 0)
    {
        symbols = "--:--";
    }
    else
    {
        unsigned mymin;
        unsigned mysec;
        mymin = milliseconds / 60000;
        mysec = (milliseconds % 60000) / 1000;
        symbols.sprintf("%02d:%02d", mymin, mysec);
    }

    for (int i = 0; i < 5; i++)
    {
        trackTimeLED[i]->display((char)symbols.local8Bit().at(i));
    }
}

void KSCD::resetTimeSlider(bool enabled)
{
    timeSlider->setEnabled(enabled);
    timeSlider->blockSignals(true);
    timeSlider->setValue(0);
    timeSlider->blockSignals(false);
} // resetTimeSlider(bool enabled);

void KSCD::setColors()
{
    QColor led_color = Prefs::ledColor();
    QColor background_color = Prefs::backColor();

    backdrop->setBackgroundColor(background_color);

    QPalette pal( led_color, background_color, led_color,led_color , led_color,
                        led_color, Qt::white );

    titlelabel ->setPalette( pal );
    artistlabel->setPalette( pal );
    volumelabel->setPalette( pal );
    statuslabel->setPalette( pal );
    tracklabel ->setPalette( pal );
    totaltimelabel->setPalette( pal );

    queryled->setPalette( pal );
    loopled->setPalette( pal );

    for (int u = 0; u< 5;u++){
        trackTimeLED[u]->setLEDoffColor(background_color);
        trackTimeLED[u]->setLEDColor(led_color,background_color);
    }

    titlelabel ->setFont( Prefs::ledFont() );
    artistlabel->setFont( Prefs::ledFont() );
    volumelabel->setFont( Prefs::ledFont() );
    statuslabel->setFont( Prefs::ledFont() );
    tracklabel ->setFont( Prefs::ledFont() );
    totaltimelabel->setFont( Prefs::ledFont() );
}

void KSCD::readSettings()
{
/*
        time_display_mode = config->readNumEntry("TimeDisplay", TRACK_SEC);
*/

    if (Prefs::cdDevice().isEmpty())
    {
        Prefs::setCdDevice(KCompactDisc::defaultDevice);
    }
}

/**
 * Write KSCD's Configuration into the kderc file.
 *
 */
void KSCD::writeSettings()
{
    Prefs::writeConfig();
} // writeSettings()

void KSCD::CDDialogSelected()
{
    if (!cddialog)
    {
        cddialog = new CDDBDlg(this);

        cddialog->setData(cddbInfo, m_cd->discSignature(), playlist);
        connect(cddialog,SIGNAL(cddbQuery()),SLOT(lookupCDDB()));
        connect(cddialog,SIGNAL(newCDInfoStored(KCDDB::CDInfo)),
            SLOT(setCDInfo(KCDDB::CDInfo)));
        connect(cddialog,SIGNAL(finished()),SLOT(CDDialogDone()));
        connect(cddialog,SIGNAL(play(int)),SLOT(trackSelected(int)));
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
    if (m_cd->discId() == KCompactDisc::missingDisc)
        return;
    kDebug(67000) << "lookupCDDB() called" << endl;

    populateSongList(i18n("Start freedb lookup."));

    setShuffle(2);

    led_on();

    cddb->config().reparse();
    cddb->setBlockingMode(false);
    cddb->lookup(m_cd->discSignature());
} // lookupCDDB

void KSCD::lookupCDDBDone(CDDB::Result result)
{
    led_off();
    if ((result != KCDDB::CDDB::Success) &&
        (result != KCDDB::CDDB::MultipleRecordFound))
    {
        populateSongList(result == CDDB::NoRecordFound ? i18n("No matching freedb entry found.") : i18n("Error getting freedb entry."));
        return;
    }

    // The intent of the original code here seems to have been to perform the
    // lookup, and then to convert all the string data within the CDDB response
    // using the use Prefs::selectedEncoding() and a QTextCodec. However, that
    // seems to be irrelevant these days.
    KCDDB::CDInfo info = cddb->lookupResponse().first();
    // TODO Why doesn't libcddb not return MultipleRecordFound?
    //if( result == KCDDB::CDDB::MultipleRecordFound ) {
    if( cddb->lookupResponse().count() > 1 ) {
      CDInfoList cddb_info = cddb->lookupResponse();
      CDInfoList::iterator it;
      QStringList list;
      for ( it = cddb_info.begin(); it != cddb_info.end(); ++it  ) {
        list.append( QString("%1, %2, %3").arg((*it).get(Artist).toString())
	    .arg((*it).get(Title).toString()).arg((*it).get(Genre).toString()));
      }

      bool ok(false);
      QString res = KInputDialog::getItem(
              i18n("Select CDDB Entry"),
              i18n("Select a CDDB entry:"), list, 0, false, &ok,
              this );
      if ( ok ) {
        // The user selected and item and pressed OK
        int c = 0;
        for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
          if( *it == res)  break;
          c++;
        }
        if( c < cddb_info.size() )
          info = cddb_info[c];
      } else {
        return;
        // user pressed Cancel
      }
    }

    setCDInfo(info);

    // In case the cddb dialog is open, update it
    if (cddialog)
      cddialog->setData(cddbInfo, m_cd->discSignature(), playlist);
} // lookupCDDBDone

void KSCD::setCDInfo(KCDDB::CDInfo info)
{
    // Some sanity provisions to ensure that the number of records matches what
    // the CD actually contains.
    Q_ASSERT(info.numberOfTracks() == cddbInfo.numberOfTracks());
    cddbInfo = info;
    populateSongList("");
}

void KSCD::led_off()
{
    queryledtimer.stop();
    queryled->off();
    queryled->hide();
    totaltimelabel->raise();
    totaltimelabel->show();
} // led_off

void KSCD::led_on()
{
    totaltimelabel->hide();
    totaltimelabel->lower();
    queryledtimer.start(800);
    queryled->off();
    queryled->show();
    kapp->processEvents();
    kapp->flush();
} // led_on

void KSCD::togglequeryled()
{
    queryled->show();
    queryled->toggle();
} // togglequeryled

void KSCD::titlelabeltimeout()
{
    // clear the cddb error message on the title label.
    titlelabeltimer.stop();
    titlelabel->clear();

} // titlelabeltimeout

void KSCD::trayOpening()
{
    statuslabel->setText(i18n("Ejected"));
    trackChanged(0, 0);
}

int KSCD::currentTrack()
{
    return m_cd->track();
}

int KSCD::currentTrackLength()
{
    return m_cd->trackLength();
}

int KSCD::currentPosition()
{
    return m_cd->trackPosition();
}

int KSCD::getStatus()
{
    if (m_cd->isPlaying())
      return 2;
    else if (m_cd->isPaused())
      return 4;
    else if (m_cd->discId() != KCompactDisc::missingDisc)
      return 5;
    else
      return 6;
}

bool KSCD::playing()
{
    return m_cd->isPlaying();
}

void KSCD::trackUpdate(unsigned /*track*/, unsigned trackPosition)
{
    unsigned tmp;

    switch (Prefs::timeDisplayMode())
    {
    case TRACK_REM:
        tmp = m_cd->trackLength() - trackPosition;
        break;

    case TOTAL_SEC:
        tmp = m_cd->discPosition();
        break;

    case TOTAL_REM:
        tmp = m_cd->discLength() - m_cd->discPosition();
        break;

    case TRACK_SEC:
    default:
        tmp = trackPosition;
        break;
    }
    if (updateTime)
    {
    setLEDs(tmp);
    timeSlider->blockSignals(true);
    timeSlider->setValue(trackPosition);
    timeSlider->blockSignals(false);
    }
}

void KSCD::cycleplaytimemode()
{
    cycletimer.stop();

    if (Prefs::timeDisplayMode() > 2) {
        Prefs::setTimeDisplayMode(TRACK_SEC);
    } else {
        Prefs::setTimeDisplayMode(Prefs::timeDisplayMode() + 1);
    }

    switch(Prefs::timeDisplayMode()) {

        case TRACK_REM:
            volumelabel->setText(i18n("Tra Rem"));
            break;

        case TOTAL_SEC:
            volumelabel->setText(i18n("Tot Sec"));
            break;

        case TOTAL_REM:
            volumelabel->setText(i18n("Tot Rem"));
            break;

        case TRACK_SEC:
        default:
            volumelabel->setText(i18n("Tra Sec"));
            break;
    }

    cycletimer.start(3000, true);
} // cycleplaymode

void KSCD::cycletimeout()
{
    cycletimer.stop();
    QString str;
    str = ki18n("Vol: %1%").subs(Prefs::volume(), 2).toString();
    volumelabel->setText(str);
} // cycletimeout


void KSCD::information(int i)
{
    //kDebug(67000) << "Information " << i << "\n" << endl;

    const QString artist = cddbInfo.get( Artist ).toString();
    if(artist.isEmpty())
        return;

    //QString encodedArtist = KUrl::encode_string_no_slash(cddbInfo.get(Artist).toString());

    KUrl url;

    switch(i)
    {
        case 0:
            url = KUrl("http://musicmoz.org/cgi-bin/ext.cgi");
            url.addQueryItem( "artist", artist );
            break;

         case 1:
            url = KUrl("http://ubl.artistdirect.com/cgi-bin/gx.cgi/AppLogic+Search?select=MusicArtist&searchtype=NormalSearch");
            url.addQueryItem( "searchstr", artist );
            break;

        case 2:
            url = KUrl( QString( "http://www.cduniverse.com/cgi-bin/cdubin.exe/rlinka/ean=%1" ).arg( QString::fromLatin1(QUrl::toPercentEncoding(artist)) ) );
            break;

        case 3:
            url = KUrl("http://www.alltheweb.com/search?cat=web");
            url.addQueryItem( "q", artist );
            break;

        case 4:
            url = KUrl("http://altavista.com/web/results?kgs=0&kls=1&avkw=xytx");
            url.addQueryItem( "q", artist );
            break;

        case 5:
            url = KUrl("http://msxml.excite.com/_1_2UDOUB70SVHVHR__info.xcite/dog/results?otmpl=dog/webresults.htm&qcat=web&qk=20&top=1&start=&ver=14060");
            url.addQueryItem( "qkw", artist );
            break;

        case 6:
            url = KUrl("http://www.google.com/search");
            url.addQueryItem( "q", artist );
            break;

        case 7:
            url = KUrl("http://groups.google.com/groups?oi=djq&num=20");
            url.addQueryItem( "as_q", artist );
            break;

        case 8:
            url = KUrl("http://www.hotbot.com/default.asp?prov=Inktomi&ps=&loc=searchbox&tab=web");
            url.addQueryItem( "query", artist );
            break;

        case 9:
            url = KUrl("http://search.lycos.com/default.asp?lpv=1&loc=searchhp&tab=web");
            url.addQueryItem( "query", artist );
            break;

         case 10:
            url = KUrl("http://search.dmoz.org/cgi-bin/search");
            url.addQueryItem( "search", artist );
            break;

         case 11:
            url = KUrl("http://search.yahoo.com/bin/search");
            url.addQueryItem( "p", artist );
            break;

         default:
            return;
            break;
    } // switch()

    KRun::runUrl( url, "text/html", 0L);
} // information

/**
 * Save state on session termination
 */
bool KSCD::saveState(QSessionManager& /*sm*/)
{
  writeSettings();
  KConfig* config = KApplication::kApplication()->sessionConfig();
  config->setGroup("General");
  config->writeEntry("Show", isVisible());
  return true;
} // saveState


/**
 * Allow the user to type in the number of the track
 */
void KSCD::keyPressEvent(QKeyEvent* e)
{
    bool isNum;
    int value = e->text().toInt(&isNum);

    if (e->key() == Qt::Key_F1)
    {
        KToolInvocation::invokeHelp();
    }
    else if (isNum)
    {
        value = (jumpToTrack * 10) + value;

        if (value <= cddbInfo.numberOfTracks())
        {
            jumpToTrack = value;
            jumpTrackTimer.stop();
            jumpTrackTimer.start(333);
        }
    }
    else
    {
      QWidget::keyPressEvent(e);
    }
} //keyPressEvent

void KSCD::jumpTracks()
{
    if (jumpToTrack > 0 && jumpToTrack <= cddbInfo.numberOfTracks())
    {
        m_cd->play(jumpToTrack, 0, jumpToTrack + 1);

        setPlayStatus();
    }

    jumpToTrack = 0;
} // jumpTracks

QString KSCD::currentTrackTitle()
{
    int track = m_cd->track();
    return (track > -1) ? cddbInfo.track(track-1).get(Title).toString() : QString();
}

QString KSCD::currentAlbum()
{
  return cddbInfo.get(Title).toString();
}

QString KSCD::currentArtist()
{
        return cddbInfo.get(Artist).toString();
}

QStringList KSCD::trackList()
{
  QStringList tracks;
  for (int i=0; i < cddbInfo.numberOfTracks(); i++)
    tracks << cddbInfo.track(i).get(Title).toString();

  return tracks;
}

void KSCD::populateSongList(QString infoStatus)
{
    // set the artist and title labels as well as the dock tooltip.
    if (!infoStatus.isEmpty())
        artistlabel->setText(infoStatus);
    else
        artistlabel->setText(QString("%1 - %2").arg(cddbInfo.get(Artist).toString(),
	                               cddbInfo.get(Title).toString()));

    songListCB->clear();
    for (int  i = 0; i < cddbInfo.numberOfTracks(); i++)
    {
        unsigned tmp = m_cd->trackLength(i + 1);
        unsigned mymin;
        unsigned mysec;
        mymin = tmp / 60000;
        mysec = (tmp % 60000) / 1000;
        QString str1;
        str1.sprintf("%02d: ", i + 1);
        QString str2;
        str2.sprintf(" (%02d:%02d) ", mymin,  mysec);
	if (cddbInfo.get(Artist) != cddbInfo.track(i).get(Artist))
	  str1.append(cddbInfo.track(i).get(Artist).toString()).append(" - ");
        str1.append(cddbInfo.track(i).get(Title).toString());
        str1.append(str2);
        songListCB->addItem(str1);
    }

    emit trackChanged(m_cd->track(), m_cd->trackLength());
}

static const KCmdLineOptions options[] =
{
    {"s",0,0},
    {"start",I18N_NOOP("Start playing"),0},
    {"+[device]",I18N_NOOP("CD device, can be a path or a media:/ URL"),0},
    KCmdLineLastOption
};


/**
 * main()
 */
int main( int argc, char *argv[] )
{
    KAboutData aboutData("kscd", I18N_NOOP("KsCD"),
                            KSCDVERSION, description,
                            KAboutData::License_GPL,
                            "(c) 2001, Dirk Försterling\n(c) 2003, Aaron J. Seigo");
    aboutData.addAuthor("Aaron J. Seigo", I18N_NOOP("Current maintainer"), "aseigo@kde.org");
    aboutData.addAuthor("Alexander Kern",I18N_NOOP("Workman library update, CDTEXT, CDDA"), "kernalex@kde.org");
    aboutData.addAuthor("Bernd Johannes Wuebben",0, "wuebben@kde.org");
    aboutData.addAuthor("Dirk Försterling", I18N_NOOP("Workman library, previous maintainer"), "milliByte@gmx.net");
    aboutData.addCredit("Wilfried Huss", I18N_NOOP("Patches galore"));
    aboutData.addCredit("Steven Grimm", I18N_NOOP("Workman library"));
    aboutData.addCredit("Sven Lueppken", I18N_NOOP("UI Work"));
    aboutData.addCredit("freedb.org", I18N_NOOP("Special thanks to freedb.org for providing a free CDDB-like CD database"), 0, "http://freedb.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication::addCmdLineOptions();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (!KUniqueApplication::start())
    {
        fprintf(stderr, "kscd is already running\n");
        if (args->count()>0 || args->isSet("start"))
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
    QDBusConnection::sessionBus().registerObject("/CDPlayer", k);

    a.setTopWidget( k );
    a.setMainWidget( k );

    k->setWindowTitle(a.caption());

    if (kapp->isSessionRestored())
    {
        KConfig* config = KApplication::kApplication()->sessionConfig();
        config->setGroup("General");
        if (config->readEntry("Show",false))
            k->show();
    }
    else
    {
        k->show();
    }

    if (args->count()>0) Prefs::self()->setCdDevice(args->arg(0));

    return a.exec();
}

#include "kscd.moc"
