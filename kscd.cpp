/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qapplication.h>

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kaccel.h>
#include <kaction.h>
#include <kcharsets.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kemailsettings.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kkeydialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kprotocolmanager.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <kuniqueapplication.h>
#include <kglobalsettings.h>
#include <kcmoduleloader.h>

#include <kconfigdialog.h>

#include "docking.h"
#include "kscd.h"
#include "mgconfdlg.h"
#include "version.h"
#include "prefs.h"

#include <kwin.h>
#include <netwm.h>

#include <config.h>

extern "C" {
    // We don't have libWorkMan installed already, so get everything
    // from within our own directory
#include "libwm/include/wm_cdrom.h"
#include "libwm/include/wm_cdtext.h"
#include "libwm/include/wm_config.h"
}

#include "cddbdlg.h"
#include "configWidget.h"
#include "kvolumecontrol.h"

#define N_TRACK_FIRST   1
#define N_TRACK_STOP 0
#define N_TRACK_UNKNOW -1
#define N_TRACK_LAST   (wm_cd_getcountoftracks())

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = false;

static QString formatTrack(int d1, int d2)
{
  QString str = QString::fromLocal8Bit("%1/%2")
    .arg( (d1 < N_TRACK_FIRST)?("--"):(QString::number(d1).rightJustify(2, '0')) )
    .arg( (d2 < N_TRACK_FIRST)?("--"):(QString::number(d2).rightJustify(2, '0')) );
  return str;
}

/****************************************************************************
                  The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name )
  : DCOPObject("CDPlayer"),
    kscdPanelDlg( parent, name, Qt::WDestructiveClose ),
    configDialog(0L),
    cddialog(0L),  //!!!!
    jumpToTrack(0L),
    cddrive_is_ok(true),
    have_new_cd(true),
    //updateDialog(false), //!!!!
    updateTime(true),
    cddb(0),
    revision(0), // The first freedb revision is "0" //!!!!
    year(0),
    m_dockWidget(0)
{
  random_current      = random_list.begin();

#if defined(BUILD_CDDA)
  audio_systems_list
                     << "arts"
#if defined(HAVE_ARTS_LIBASOUND2)
                     << "alsa"
#endif
#ifdef USE_SUN_AUDIO
                     << "sun"
#endif
  ;
#endif

  readSettings();
  initFont();
  drawPanel();
  setColors();

  /* debug
   wm_cd_set_verbosity(0xffff);
  */

  // the time slider
  timeIcon->setPixmap(SmallIcon("player_time"));
  connect(timeSlider, SIGNAL(sliderPressed()), SLOT(timeSliderPressed()));
  connect(timeSlider, SIGNAL(sliderReleased()), SLOT(timeSliderReleased()));
  connect(timeSlider, SIGNAL(sliderMoved(int)), SLOT(timeSliderMoved(int)));
  connect(timeSlider, SIGNAL(valueChanged(int)), SLOT(jumpToTime(int)));

  // the volume slider
  volumeIcon->setPixmap(SmallIcon("player_volume"));
  volumeSlider->setValue(Prefs::volume());
  connect(volumeSlider, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));

  /* FIXME check for return value */
  setDevicePaths(/*Prefs::cdDevice(), Prefs::audioSystem(), Prefs::audioDevice()*/);

  connect( &queryledtimer, SIGNAL(timeout()),  SLOT(togglequeryled()) );
  connect( &titlelabeltimer, SIGNAL(timeout()),  SLOT(titlelabeltimeout()) );
  connect( &cycletimer, SIGNAL(timeout()),  SLOT(cycletimeout()) );
  connect( &timer, SIGNAL(timeout()), SLOT(cdMode()) );
  connect( &jumpTrackTimer, SIGNAL(timeout()),  SLOT(jumpTracks()) );
/*
  these are always connected in base class
  connect( playPB, SIGNAL(clicked()), SLOT(playClicked()) );
  connect( nextPB, SIGNAL(clicked()), SLOT(nextClicked()) );
  connect( prevPB, SIGNAL(clicked()), SLOT(prevClicked()) );
  connect( stopPB, SIGNAL(clicked()), SLOT(stopClicked()) );
  connect( ejectPB, SIGNAL(clicked()), SLOT(ejectClicked()) );
*/
  connect( repeatPB, SIGNAL(clicked()), SLOT(loopClicked()) );
  connect( songListCB, SIGNAL(activated(int)), SLOT(trackSelected(int)));
  connect( shufflePB, SIGNAL(clicked()), SLOT(randomSelected()));
  connect( cddbPB, SIGNAL(clicked()), SLOT(CDDialogSelected()));
  connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(setColors()));
  connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(setIcons()));

  // set up the actions and keyboard accels
  m_actions = new KActionCollection(this);

  KAction* action;
  action = new KAction(i18n("Play/Pause"), Key_P, this, SLOT(playClicked()), m_actions, "Play/Pause");
  action = new KAction(i18n("Stop"), Key_S, this, SLOT(stopClicked()), m_actions, "Stop");
  action = new KAction(i18n("Previous"), Key_B, this, SLOT(prevClicked()), m_actions, "Previous");
  action = new KAction(i18n("Next"), Key_N, this, SLOT(nextClicked()), m_actions, "Next");
  action = KStdAction::quit(this, SLOT(quitClicked()), m_actions);
  action = KStdAction::keyBindings(this, SLOT(configureKeys()), m_actions);
  action = KStdAction::preferences(this, SLOT(showConfig()), m_actions);
  action = new KAction(i18n("Loop"), Key_L, this, SLOT(loopClicked()), m_actions, "Loop");
  action = new KAction(i18n("Eject"), CTRL + Key_E, this, SLOT(ejectClicked()), m_actions, "Eject");
  action = new KAction(i18n("Increase Volume"), Key_Plus, this, SLOT(incVolume()), m_actions, "IncVolume");
  action = new KAction(i18n("Increase Volume"), Key_Equal, this, SLOT(incVolume()), m_actions, "IncVolume Alt");
  action = new KAction(i18n("Decrease Volume"), Key_Minus, this, SLOT(decVolume()), m_actions, "DecVolume");
  action = new KAction(i18n("Options"), CTRL + Key_T, this, SLOT(showConfig()), m_actions, "Options");
  action = new KAction(i18n("Shuffle"), Key_R, this, SLOT(randomSelected()), m_actions, "Shuffle");
  action = new KAction(i18n("CDDB"), CTRL + Key_D, this, SLOT(CDDialogSelected()), m_actions, "CDDB");

  setupPopups();

  if (Prefs::looping())
  {
    loopled->on();
    loopled->show();
    repeatPB->setOn(true);
  }

  setDocking(Prefs::docking());

  connectDCOPSignal(0, 0, "KDE_emailSettingsChanged()", "emailSettingsChanged()", false);

  setFocusPolicy(QWidget::NoFocus);

  songListCB->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  adjustSize();
  setFixedHeight(this->height());
} // KSCD


KSCD::~KSCD()
{
    if (cd->trk)
    {
        free(cd->trk);
    }

    signal (SIGINT, SIG_DFL);
    delete cddb;
} // ~KSCD

bool KSCD::digitalPlayback() {
#if defined(BUILD_CDDA)
        return !(Prefs::audioSystem().isEmpty());
#else
        return false;
#endif
}

/**
 * Init CD-ROM and display
 */
void KSCD::initCDROM()
{
  have_new_cd = true;
  cdMode();

  if(cddrive_is_ok)
    volChanged(Prefs::volume());

  int cur_cdmode = wm_cd_status();
  if (Prefs::autoplay() && cur_cdmode != WM_CDM_PLAYING) {
    playClicked();
  }

  timer.start(1000);
} // initCDROM

void KSCD::setVolume(int v)
{
    volChanged(v);
    volumeSlider->setValue(v);
}

/**
 * Initialize smallfont which fits into the 13 and 14 pixel widgets
 * and verysmallfont which is slightly smaller.
 */
void KSCD::initFont()
{
  int theSmallPtSize = 10;

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
  verysmallfont = QFont(KGlobalSettings::generalFont().family(),
                        (theSmallPtSize > 4) ? theSmallPtSize - 2 : 2,
                        QFont::Bold);
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

  QString zeros("--:--");
  setLEDs(zeros);
  
  queryled = new LedLamp(symbols);
  queryled->move(symbols->width()-20, D + 1);
  queryled->off();
  queryled->hide();

  loopled = new LedLamp(symbols, LedLamp::Loop);
  loopled->move(symbols->width()-20, D + 18);
  loopled->off();

  totaltimelabel->hide();
} // drawPanel

void KSCD::setIcons()
{
  playPB->setIconSet(SmallIconSet("player_play"));
  stopPB->setIconSet(SmallIconSet("player_stop"));
  ejectPB->setIconSet(SmallIconSet("player_eject"));
  prevPB->setIconSet(SmallIconSet("player_start"));
  nextPB->setIconSet(SmallIconSet("player_end"));
  cddbPB->setIconSet(SmallIconSet("view_text"));
  infoPB->setIconSet(SmallIconSet("run"));
}

void
KSCD::setupPopups()
{
    QPopupMenu* mainPopup   = new QPopupMenu(this);
    infoPB->setPopup(mainPopup);
    infoPopup   = new QPopupMenu (this);


    infoPopup->insertItem("MusicMoz", 0);
    infoPopup->insertItem("Ultimate Bandlist", 1);
    infoPopup->insertItem("CD Universe", 2);
    infoPopup->insertSeparator();
    infoPopup->insertItem("AlltheWeb", 3);
    infoPopup->insertItem("Altavista", 4);
    infoPopup->insertItem("Excite", 5);
    infoPopup->insertItem("Google", 6);
    infoPopup->insertItem("Google Groups", 7);
    infoPopup->insertItem("HotBot", 8);
    infoPopup->insertItem("Lycos", 9);
    infoPopup->insertItem("Open Directory", 10);
    infoPopup->insertItem("Yahoo!", 11);

    m_actions->action(KStdAction::name(KStdAction::Preferences))->plug(mainPopup);
    m_actions->action(KStdAction::name(KStdAction::KeyBindings))->plug(mainPopup);
    mainPopup->insertSeparator();

    mainPopup->insertItem(i18n("Artist Information"), infoPopup);

    connect( infoPopup, SIGNAL(activated(int)), SLOT(information(int)) );

    KHelpMenu* helpMenu = new KHelpMenu(this, KGlobal::instance()->aboutData(), false);
    mainPopup->insertItem(SmallIcon("help"),i18n("&Help"), helpMenu->menu());
    mainPopup->insertSeparator();
    m_actions->action(KStdAction::name(KStdAction::Quit))->plug(mainPopup);
} // setupPopups

void KSCD::playClicked()
{
    int cur_cdmode = wm_cd_status();

    if (!cddrive_is_ok || WM_CDS_NO_DISC(cur_cdmode))
        return;

    kapp->processEvents();
    kapp->flushX();

#ifdef NEW_BSD_PLAYCLICKED
    if(cur_cdmode == WM_CDM_STOPPED ||
       cur_cdmode == WM_CDM_UNKNOWN ||
       cur_cdmode == WM_CDM_BACK)
#else
    if(cur_cdmode == WM_CDM_STOPPED ||
       cur_cdmode == WM_CDM_UNKNOWN)
#endif
    {
        setLEDs( "00:00" );
        resetTimeSlider(true);
        kapp->processEvents();
        kapp->flushX();

        if(Prefs::randomPlay())
        {
            make_random_list();
            // next clicked handles updating the play button, etc.
            nextClicked();
            return;
        }
        else
        {
            wm_cd_play(N_TRACK_FIRST, 0, playlist.isEmpty() ? WM_ENDTRACK : N_TRACK_FIRST + 1);
        }
    }
    else if (cur_cdmode == WM_CDM_PLAYING)
    {
        wm_cd_pause();
    }
    else if (cur_cdmode == WM_CDM_PAUSED)
    {
        jumpToTime(cur_pos_rel, true);
    }

    kapp->processEvents();
    kapp->flushX();
} // playClicked()

void KSCD::updatePlayPB(bool playing)
{
    if (playing)
    {
        playPB->setIconSet(SmallIconSet("player_pause"));
        playPB->setText(i18n("Pause"));
    }
    else
    {
        playPB->setIconSet(SmallIconSet("player_play"));
        playPB->setText(i18n("Play"));
    }
}

void KSCD::setShuffle(int shuffle)
{
    if (shuffle == 2) {
        if(Prefs::randomPlay() && cd && wm_cd_getcountoftracks() > 0) {
            make_random_list(); /* koz: Build a unique, once, random list */
            if(WM_CDM_PLAYING == wm_cd_status())
                nextClicked();
        }

        return;
    }

    Prefs::setRandomPlay(shuffle);
    shufflePB->blockSignals(true);
    shufflePB->setOn(shuffle);
    shufflePB->blockSignals(false);

    if (Prefs::randomPlay() && cd && wm_cd_getcountoftracks() > 0) {
        make_random_list(); /* koz: Build a unique, once, random list */
        if(WM_CDM_PLAYING == wm_cd_status())
            nextClicked();
    }
    /* FIXME this helps us to display "Random" in Status line
       should it maybe to be replaced with symbol "RAND" or something others */
    setRandomLabel(wm_cd_status());
}

void KSCD::stopClicked()
{
    stoppedByUser = true;

    kapp->processEvents();
    kapp->flushX();
    wm_cd_stop(); // must be first, apparently
} // stopClicked()

void KSCD::prevClicked()
{
    int track = currentTrack();

    if (Prefs::randomPlay()) {
        track = prev_randomtrack();
        if (track == N_TRACK_UNKNOW) {
            return;
        }
    } else {
        if (track <= N_TRACK_FIRST) {
            if (Prefs::looping()) {
                track = N_TRACK_LAST;
            } else {
                return;
            }
        } else {
            track--;
        }
    }

    kapp->processEvents();
    kapp->flushX();
    wm_cd_play(track, 0, playlist.isEmpty() ? WM_ENDTRACK : track);
} // prevClicked()

bool KSCD::nextClicked()
{
    int track = currentTrack();

    if (Prefs::randomPlay()) {
        track = next_randomtrack();
        if(track == N_TRACK_UNKNOW) {
            return false;
        }
    } else {
        if(track < N_TRACK_FIRST) {
            track = N_TRACK_FIRST;
        } else if (track >= N_TRACK_LAST) {
            if (Prefs::looping()) {
                track = N_TRACK_FIRST;
            } else {
                return true;
            }
        } else {
            track++;
        }
    }

    kapp->processEvents();
    kapp->flushX();
    wm_cd_play(track, 0, playlist.isEmpty() ? WM_ENDTRACK : track + 1);
    return true;
} // nextClicked()

void KSCD::updateDisplayedTrack(int track)
{
    if(track < N_TRACK_FIRST) {
        setLEDs("--:--");
        resetTimeSlider(true);
    } else {
        setSongListTo(track - 1);
        timeSlider->setRange(0, cd->trk[track - 1].length - 1);
    }

    tracklabel->setText(formatTrack(track, N_TRACK_LAST));
    setTitle(track);
} //updateDisplayedTrack(int track)


void KSCD::jumpToTime(int seconds, bool forcePlay)
{
    kapp->processEvents();
    kapp->flushX();

    int track = currentTrack();
    if ((wm_cd_status() == WM_CDM_PLAYING || forcePlay) &&
        seconds < cd->trk[track - 1].length)
    {
        if(Prefs::randomPlay() || !playlist.isEmpty())
        {
            wm_cd_play (track, seconds, track + 1);
        }
        else
        {
            wm_cd_play (track, seconds, WM_ENDTRACK);
        }
    }
    playtime(seconds);
} // jumpToTime(int seconds)

void KSCD::timeSliderPressed()
{
    updateTime = false;
} // timeSliderPressed()

void KSCD::timeSliderMoved(int seconds)
{
    setLEDs(calculateDisplayedTime(seconds));
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
    timer.stop();
    jumpTrackTimer.stop();

    writeSettings();
    //setShuffle(0);
    statuslabel->clear();
    setLEDs( "--:--" );

    // Good GOD this is evil
    kapp->processEvents();
    kapp->flushX();

    if(Prefs::stopExit())
        wm_cd_stop();

    wm_cd_destroy();

    kapp->quit();
} // quitClicked()


void KSCD::closeEvent( QCloseEvent *e )
{
    // we need to figure out if we were called by the system tray
    // to decide whether or not to actually quit or not =/
    // this behaviour of ksystemtray is, IMHO, very silly
    const QObject* caller = sender();
    while (m_dockWidget && caller)
    {
        if (caller == m_dockWidget)
        {
            break;
        }
        caller = caller->parent();
    }

    if (Prefs::docking() && !caller && !kapp->sessionSaving())
    {
        hide();
        e->ignore();
        return;
    }

    /* Stop playing the CD */
    int cur_cdmode = wm_cd_status();
    if ( Prefs::stopExit() && cur_cdmode == WM_CDM_PLAYING )
        wm_cd_stop();

    writeSettings();
    //setShuffle(0);

    statuslabel->clear();

    setLEDs( "--:--" );

    kapp->processEvents();
    kapp->flushX();

    // TODO: is this really necessary given the stopClicked() above?
    if (Prefs::stopExit())
        wm_cd_stop();

     e->accept();
} // closeEvent

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
    kapp->flushX();
} // loopOn;

void KSCD::loopOff()
{
    Prefs::setLooping(false);
    loopled->off();
    loopled->show();
    kapp->processEvents();
    kapp->flushX();
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
    if(!cddrive_is_ok)
        return;


    if (wm_cd_status() == WM_CDM_EJECTED)
    {
      statuslabel->setText(i18n("Closing"));
      kapp->processEvents();
      kapp->flushX();
      wm_cd_closetray();
    }
    else
    {
      statuslabel->setText(i18n("Ejecting"));
      kapp->processEvents();
      kapp->flushX();
      setArtist("");
      setTitle(0);
      tracktitlelist.clear();
      extlist.clear();
      category = "";
      genre = "";

      wm_cd_stop();
      wm_cd_eject();
    }
} // ejectClicked

void KSCD::randomSelected()
{
    setShuffle(Prefs::randomPlay()?0:1);
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

    wm_cd_play(track, 0, WM_ENDTRACK);
} // trackSelected

void KSCD::updateConfigDialog(configWidget* widget)
{
    if(!widget)
        return;

    static QString originalTitleOfGroupBox = widget->groupBox3->title();
    int status = wm_cd_status();
    if(WM_CDS_DISC_PLAYING(status)) {
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

    confWidget = new configWidget(this, 0, "Kscd");

    // kscd config page
    configDialog -> addPage(confWidget, i18n("CD Player"), "kscd", i18n("Settings & Behavior"));

    // libkcddb page
    KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
    if (libkcddb && libkcddb->isValid())
    {
        KCModuleInfo info(libkcddb->desktopEntryPath());
        if (info.service()->isValid())
        {
            KCModule *m = KCModuleLoader::loadModule(info);
            if (m)
            {
                m->load();
                configDialog -> addPage(m, QString("freedb"), "cdtrack", i18n("Configure Fetching Items"));
                connect(configDialog, SIGNAL(okClicked()), m, SLOT(save()));
                connect(configDialog, SIGNAL(applyClicked()), m, SLOT(save()));
                connect(configDialog, SIGNAL(defaultClicked()), m, SLOT(defaults()));
            }
        }
    }

    updateConfigDialog(confWidget);

    connect(configDialog, SIGNAL(settingsChanged()), confWidget, SLOT(configDone()));
    connect(configDialog, SIGNAL(settingsChanged()), this, SLOT(configDone()));
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
    KKeyDialog::configure(m_actions, this);
}

void KSCD::setDevicePaths()
{
    cddrive_is_ok = false;
    int ret = wm_cd_init(
#if defined(BUILD_CDDA)
        (Prefs::digitalPlayback())?WM_CDDA:WM_CDIN,
        QFile::encodeName(Prefs::cdDevice()),
        (Prefs::digitalPlayback())?Prefs::audioSystem().ascii():"",
        (Prefs::digitalPlayback())?Prefs::audioDevice().ascii():"",
        0);
    kdDebug(67000) << "Device changed to "
        << ((Prefs::digitalPlayback())?"WM_CDDA, ":"WM_CDIN, ")
        << Prefs::cdDevice() << ", "
        << ((Prefs::digitalPlayback())?Prefs::audioSystem():"") << ", "
        << ((Prefs::digitalPlayback())?Prefs::audioDevice():"") << ". returns status " << ret << "\n";
#else
        WM_CDIN, QFile::encodeName(Prefs::cdDevice()), 0, 0, 0);
    kdDebug(67000) << "Device changed to " << Prefs::cdDevice() << ". returns status  " << ret << "\n";
#endif

    setArtist("");
    setTitle(0);
    tracktitlelist.clear();
    extlist.clear();
    clearSongList();
    cddrive_is_ok = true;
    initCDROM();
} // setDevicePath()

void KSCD::setDocking(bool dock)
{
    Prefs::setDocking(dock);
    if (Prefs::docking())
    {
        if (!m_dockWidget)
        {
            m_dockWidget = new DockWidget(this, "dockw");
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
    if (!cddrive_is_ok)
        return;

    QString str;
    str = QString::fromUtf8(QCString().sprintf(i18n("Vol: %02d%%").utf8(), vol));
    volumelabel->setText(str);
    if(!wm_cd_volume(vol, WM_BALANCE_SYMMETRED))
        Prefs::setVolume(vol);
} // volChanged

void KSCD::make_random_list()
{
    /* koz: 15/01/00. I want a random list that does not repeat tracks. Ie, */
    /* a list is created in which each track is listed only once. The tracks */
    /* are picked off one by one until the end of the list */

    int selected = 0;
    bool rejected = false;

    //kdDebug(67000) << "Playlist has " << size << " entries\n" << endl;
    random_list.clear();
    for(int i = 0; i < wm_cd_getcountoftracks(); i++)
    {
        do {
            selected = 1 + (int) randSequence.getLong(wm_cd_getcountoftracks());
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
    else if(random_current == random_list.fromLast())
    {
        if(!Prefs::looping())
        {
            wm_cd_stop();
            return N_TRACK_UNKNOW;
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
        random_current = random_list.fromLast();
    }
    else if(random_current == random_list.begin())
    {
        if(!Prefs::looping())
        {
            wm_cd_stop();
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

/*
 * cdMode
 *
 * - Data discs not recognized as data discs.
 *
 */
void KSCD::cdMode()
{
    static int prev_cdmode = -1;
    static int prev_track = -1;
    int cdmode = wm_cd_status();
    int track = currentTrack();

    if(cdmode < 0) {
        if(cddrive_is_ok) {
            statuslabel->setText(i18n("Error"));
            cddrive_is_ok = false;
            QString errstring =
                i18n("CD-ROM read or access error (or no audio disc in drive).\n"\
                     "Please make sure you have access permissions to:\n%1")
                .arg(Prefs::cdDevice().isNull() ? wm_drive_device() : Prefs::cdDevice());
            KMessageBox::error(this, errstring, i18n("Error"));
        }
        return;
    } else if(WM_CDS_NO_DISC(cdmode)) {
        /* only one place, where we set it */
        have_new_cd = true;
        prev_track = -1;
    } else if(have_new_cd) {
        have_new_cd = false;
	get_cddb_info(false);

        if(Prefs::autoplay() && cdmode == WM_CDM_STOPPED)
            playClicked();
    }

    cddrive_is_ok = true; // cd drive ok

    if (cdmode == prev_cdmode)
    {
        if (cdmode == WM_CDM_PLAYING)
        {
            if ( updateTime )
            {
                playtime();
            }
            if( prev_track != track )
            {
                updateDisplayedTrack(track);
                prev_track = track;
            }
        }
    } else {
        kdDebug(67000) << "Mode changed from " << prev_cdmode << " to " << cdmode << endl;
        cdModeChanged(prev_cdmode, cdmode);
        prev_cdmode = cdmode;
    }
} /* cdMode */

void KSCD::setRandomLabel(int mode)
{
     if(mode == WM_CDM_PLAYING) {
            updatePlayPB(true);
            statuslabel->setText(Prefs::randomPlay()?i18n("Random"):i18n("Playing"));
     }
}

void KSCD::cdModeChanged(int previous, int cdmode)
{
    switch (cdmode)
    {
        case WM_CDM_DEVICECHANGED:
            updatePlayPB(false);
            break;

        case WM_CDM_TRACK_DONE: // == WM_CDM_BACK
            if (!nextClicked())
            {
                statuslabel->setText(i18n("Disc Finished"));
                wm_cd_stop();
            }
            break;

        case WM_CDM_PLAYING:
            setRandomLabel(cdmode);
            break;

        case WM_CDM_FORWARD:
            break;

        case WM_CDM_PAUSED:
            updatePlayPB(false);
            statuslabel->setText(i18n("Pause"));
            break;

        case WM_CDM_STOPPED:
            updatePlayPB(false);
            statuslabel->setText(i18n("Stopped"));

            if (Prefs::ejectOnFinish() && !stoppedByUser && WM_CDS_DISC_READY(previous))
            {
                ejectClicked();
                break;
            }

            /* reset to initial value, only stopclicked() sets this to true */
            stoppedByUser = false;

            updateDisplayedTrack(N_TRACK_UNKNOW);
            break;

        case WM_CDM_UNKNOWN:
        case WM_CDM_NO_DISC:
        case WM_CDM_EJECTED:
            updatePlayPB(false);
            if (cdmode == WM_CDM_NO_DISC)
            {
                statuslabel->setText(i18n("no disc"));
            }
            else
            {
                statuslabel->setText(i18n("Ejected"));
            }

            tracktitlelist.clear();
            extlist.clear();
            clearSongList();
            totaltimelabel->clear();
            totaltimelabel->lower();
            setArtist("");

            updateDisplayedTrack(N_TRACK_UNKNOW);
            break;
    }
} /* cdModeChanged */

void KSCD::setLEDs(const QString& symbols)
{
    if(symbols.length() < 5)
    {
        return;
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

    QColorGroup colgrp( led_color, background_color, led_color,led_color , led_color,
                        led_color, white );

    titlelabel ->setPalette( QPalette(colgrp,colgrp,colgrp) );
    artistlabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
    volumelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
    statuslabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
    tracklabel ->setPalette( QPalette(colgrp,colgrp,colgrp) );
    totaltimelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );

    queryled->setPalette( QPalette(colgrp,colgrp,colgrp) );
    loopled->setPalette( QPalette(colgrp,colgrp,colgrp) );

    for (int u = 0; u< 5;u++){
        trackTimeLED[u]->setLEDoffColor(background_color);
        trackTimeLED[u]->setLEDColor(led_color,background_color);
    }

    titlelabel ->setFont( smallfont );
    artistlabel->setFont( smallfont );
    volumelabel->setFont( smallfont );
    statuslabel->setFont( smallfont );
    tracklabel ->setFont( smallfont );
    totaltimelabel->setFont( smallfont );
}

void KSCD::readSettings()
{
/*
        time_display_mode = config->readNumEntry("TimeDisplay", TRACK_SEC);
*/

#ifndef DEFAULT_CD_DEVICE
#define DEFAULT_CD_DEVICE "/dev/cdrom"
        // sun ultrix etc have a canonical cd rom device specified in the
        // respective plat_xxx.c file. On those platforms you need nnot
        // specify the cd rom device and DEFAULT_CD_DEVICE is not defined
        // in config.h
#endif

    if (Prefs::cdDevice().isEmpty())
    {
        Prefs::setCdDevice(DEFAULT_CD_DEVICE);
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

        cddialog->setData(cd,tracktitlelist,extlist,xmcd_data,category, genre,
                        revision,year,playlist,pathlist);

        connect(cddialog,SIGNAL(cddbQuery(bool)),SLOT(get_cddb_info(bool)));
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

void KSCD::get_cddb_info(bool /*_updateDialog*/)
{
    kdDebug(67000) << "get_cddb_info() called" << endl;
    if (!cd ||
        !cddrive_is_ok ||
        wm_cd_status() < 1)
    {
        kdDebug(67000) << "CD pointer invalid cd " << cd << " cddrive_is_ok " << cddrive_is_ok << endl;
        cddb_no_info();
        return;
    }

    // Don't crash if no disc is in
    if( cd->length == 0 ) {
        kdDebug(67000) << "CD length seems to be zero" << endl;
        cddb_no_info();
        return;
    }

    // FIXME: what is the total time label setting stuff doing in CDDB info?!
    QTime dml;
    dml = dml.addSecs(cd->length);

    QString fmt;
    if(dml.hour() > 0)
        fmt.sprintf("%02d:%02d:%02d",dml.hour(),dml.minute(),dml.second());
    else
        fmt.sprintf("%02d:%02d",dml.minute(),dml.second());

    totaltimelabel->setText(fmt);

    KCDDB::TrackOffsetList querylist;
    tracktitlelist.clear();
    tracktitlelist.append(i18n("Start freedb lookup."));
    populateSongList();

    for(int i = 0 ; i < wm_cd_getcountoftracks(); i++)
    {
        querylist << cd->trk[i].start;
    }
    setShuffle(2);

    querylist << cd->trk[0].start << cd->trk[wm_cd_getcountoftracks()].start;
    led_on();

    delete cddb;
    cddb = new KCDDB::Client();
    cddb->setBlockingMode(false);
    connect(cddb, SIGNAL(finished(CDDB::Result)),
            this, SLOT(cddb_done(CDDB::Result)));

    cddb->lookup(querylist);
} // get_cddb_info

void KSCD::cddb_done(CDDB::Result result)
{
    // CDDBTODO: figure out why using CDDB::Success doesn't compile?!
    if ((result != 0 /*KCDDB::CDDB::Success*/) &&
        (result != KCDDB::CDDB::MultipleRecordFound))
    {
        cddb_failed();
        return;
    }
    KCDDB::CDInfo cddbInfo = cddb->bestLookupResponse();

    tracktitlelist.clear();
    extlist.clear();

    // CDDBTODO: we really should get the artist off the 'tracktitlelist'
    tracktitlelist << cddbInfo.artist + " / " + cddbInfo.title;

    revision = cddbInfo.revision;
    category = cddbInfo.category;
    genre = cddbInfo.genre;
    extlist << cddbInfo.extd;
    year = cddbInfo.year;

    KCDDB::TrackInfoList::ConstIterator it(cddbInfo.trackInfoList.begin());
    KCDDB::TrackInfoList::ConstIterator end(cddbInfo.trackInfoList.end());
    for (; it != end; ++it)
    {
        tracktitlelist << (*it).title;
        extlist << (*it).extt;
    }

    populateSongList();

    // In case the cddb dialog is open, update it
    if (cddialog)
      cddialog->setData(cd,tracktitlelist,extlist,xmcd_data,category, genre,
                        revision,year,playlist,pathlist);

    led_off();
} // cddb_done

void KSCD::get_cdtext_info(void)
{
    struct cdtext_info *p_cdtext;
    kdDebug(67000) << "get_cdtext_info() called" << endl;
    //setArtist("");
    //setTitle(0);
    //songListCB->clear();
    tracktitlelist.clear();
    extlist.clear();

    p_cdtext = wm_cd_get_cdtext();
    if(p_cdtext && p_cdtext->valid) {
        tracktitlelist.append(QString().sprintf("%s / %s", (const char*)(p_cdtext->blocks[0]->name[0]),
            (const char*)(p_cdtext->blocks[0]->performer[0])));
        //titlelabel->setText(QString((const char*)(p_cdtext->blocks[0]->name[1])));
        //artistlabel->setText(tracktitlelist.first());


        // if it's a sampler, we'll do artist/title
        bool isSampler = (qstricmp(reinterpret_cast<char*>(p_cdtext->blocks[0]->performer[0]), "various") == 0);

        int at = 1;
        for (; at < (p_cdtext->count_of_entries); ++at) {
            QString title;
            if (isSampler) {
                title.sprintf("%s / %s", p_cdtext->blocks[0]->performer[at], p_cdtext->blocks[0]->name[at]);
            } else {
                title = reinterpret_cast<char*>(p_cdtext->blocks[0]->name[at]);
            }
            tracktitlelist.append(title);
        }
    } else {
        kdDebug(67000) << "cdtext invalid" << endl;
    }
      
    populateSongList();
}

void KSCD::cddb_no_info()
{
    kdDebug(67000) << "cddb_no_info() called\n" << endl;

    tracktitlelist.clear();
    extlist.clear();
    tracktitlelist.append(i18n("No matching freedb entry found."));

    led_off();
    
    get_cdtext_info();
} // cddb_no_info

void KSCD::cddb_failed()
{
    // TODO differentiate between those casees where the communcition really
    // failed and those where we just couldn't find anything
    //        cddb_ready_bug = 0;
    kdDebug(67000) << "cddb_failed() called\n" << endl;

    tracktitlelist.clear();
    extlist.clear();
    tracktitlelist.append(i18n("Error getting freedb entry."));
    revision=year=0;
    category=genre=QString::null;

    led_off();
    
    get_cdtext_info();
} // cddb_failed

void KSCD::mycddb_inexact_read()
{
/*    if(cddb_inexact_sentinel == true)
        return;

    QString pick;

    cddb_inexact_sentinel = true;
    QStringList inexact_list;
    cddb->get_inexact_list(inexact_list);

    // Whatever happens, we better clear the list beforehand
    tracktitlelist.clear();
    extlist.clear();

    if( inexact_list.count() == 1)
    {
        pick = inexact_list.first();
        cddb->query_exact("200 " + pick);
        return;
    }

    InexactDialog *dialog;
    dialog = new InexactDialog(0,"inexactDialog",true);
    dialog->insertList(inexact_list);

    if(dialog->exec() != QDialog::Accepted)
    {
        cddb->close_connection();
        timer.start(1000);
        led_off();
        return;
    }

    pick = dialog->selection();
    delete dialog;

    if(pick.isEmpty())
    {
        timer.start(1000);
        led_off();
        return;
    }

    pick = "200 " + pick;
    cddb->query_exact(pick);*/
} // mycddb_inexact_read

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
    kapp->flushX();
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

void KSCD::setArtist(const QString& artist)
{
    if (!artist.isEmpty()) {
        artistlabel->setText(artist);
    } else {
        artistlabel->clear();
    }
}

void KSCD::setTitle(int track)
{
    QString tooltip = artistlabel->text();

    if(track < N_TRACK_FIRST)
        titlelabel->clear();
    else {
        QString title;
        if(track > (int)tracktitlelist.count())
            title = i18n("<Unknown>");
        else
            title = *tracktitlelist.at(track);

        titlelabel->setText(title);
	tooltip += " / ";
        tooltip += KStringHandler::rsqueeze(title, 30);
    }

    emit trackChanged(tooltip);
}

QString KSCD::calculateDisplayedTime()
{
    return calculateDisplayedTime(cur_pos_rel, cd->curtrack);
} //calculateDisplayedTime()

QString KSCD::calculateDisplayedTime(int sec)
{
    return calculateDisplayedTime(sec, cd->curtrack);
} //calculateDisplayedTime(int sec)

QString KSCD::calculateDisplayedTime(int sec, int track)
{
    // should check if tracknumber is valid.
    static int mymin;
    static int mysec;
    int tmp;

    int total_sec = 0; // Number of Seconds from the beginning of the CD.

    if (sec < 0)
    {
        sec = 0;
    }
    if (Prefs::timeDisplayMode() == TOTAL_SEC || Prefs::timeDisplayMode() == TOTAL_REM)
    {
        for (int i = 0; i < track - 1; i++)
        {
            total_sec += cd->trk[i].length;
        }
        total_sec += sec;
    }

    switch (Prefs::timeDisplayMode())
    {
        case TRACK_REM:
            tmp = cd->trk[track - 1].length - sec;
            mysec = tmp % 60;
            mymin = tmp / 60;
            break;

        case TOTAL_SEC:
            mysec = total_sec % 60;
            mymin = total_sec / 60;
            break;

        case TOTAL_REM:
            tmp = cd->length - total_sec;
            mysec = tmp % 60;
            mymin = tmp / 60;
            break;

        case TRACK_SEC:
        default:
            mysec = sec % 60;
            mymin = sec / 60;
            break;
    }

    QString tmptime;
    tmptime.sprintf("%02d:%02d", mymin, mysec);
    return tmptime;
} // calculateDisplayedTime(int sec, int track)

void KSCD::playtime()
{
    playtime(cur_pos_rel);
} // playtime()

void KSCD::playtime(int seconds)
{
    timeSlider->blockSignals( true );
    timeSlider->setValue(seconds);
    timeSlider->blockSignals( false );
    setLEDs(calculateDisplayedTime(seconds));
} // playtime(int seconds)

void KSCD::cycleplaytimemode()
{
    cycletimer.stop();

    if (Prefs::timeDisplayMode() > 2) {
        Prefs::setTimeDisplayMode(TRACK_SEC);
    } else {
        Prefs::setTimeDisplayMode(Prefs::timeDisplayMode() + 1);
    }
    playtime();

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
    str = QString::fromUtf8( QCString().sprintf(i18n("Vol: %02d%%").utf8(), Prefs::volume()) );
    volumelabel->setText(str);
} // cycletimeout


bool KSCD::getArtist(QString& artist)
{
    if((int)tracktitlelist.isEmpty()){
        return false;
    }

    artist = tracktitlelist.first();

    int pos;
    pos = artist.find('/', 0, true);
    if(pos != -1)
        artist.truncate(pos);

    artist = artist.stripWhiteSpace();
    return true;
} // getArtist

void KSCD::information(int i)
{
    //kdDebug(67000) << "Information " << i << "\n" << endl;

    QString artist;
    QString str;

    if(!getArtist(artist))
        return;

    artist = KURL::encode_string_no_slash(artist);

    switch(i)
    {
        case 0:
            str = QString("http://musicmoz.org/cgi-bin/ext.cgi?artist=%1")
                   .arg(artist);
            break;

         case 1:
            str = QString("http://ubl.artistdirect.com/cgi-bin/gx.cgi/AppLogic+Search?select=MusicArtist&searchstr=%1&searchtype=NormalSearch")
                .arg(artist);
            break;

        case 2:
            str = QString("http://www.cduniverse.com/cgi-bin/cdubin.exe/rlinka/ean=%1")
                  .arg(artist);

        case 3:
            str = QString("http://www.alltheweb.com/search?cat=web&q=%1")
                    .arg(artist);
            break;

        case 4:
            str = QString("http://altavista.com/web/results?q=%1&kgs=0&kls=1&avkw=xytx")
                  .arg(artist);
            break;

        case 5:
            str = QString("http://msxml.excite.com/_1_2UDOUB70SVHVHR__info.xcite/dog/results?otmpl=dog/webresults.htm&qkw=%1&qcat=web&qk=20&top=1&start=&ver=14060")
                  .arg(artist);
            break;

        case 6:
            str = QString("http://www.google.com/search?q=%1")
                  .arg(artist);
            break;

        case 7:
            str = QString("http://groups.google.com/groups?oi=djq&as_q=%1&num=20")
                  .arg(artist);
            break;

        case 8:
            str = QString("http://www.hotbot.com/default.asp?prov=Inktomi&query=%1&ps=&loc=searchbox&tab=web")
                  .arg(artist);
            break;

        case 9:
            str = QString("http://search.lycos.com/default.asp?lpv=1&loc=searchhp&tab=web&query=%1")
                  .arg(artist);
             break;

         case 10:
             str = QString("http://search.dmoz.org/cgi-bin/search?search=%1")
                   .arg(artist);
             break;

         case 11:
             str = QString("http://search.yahoo.com/bin/search?p=%1")
                   .arg(artist);
             break;

         default:
            return;
            break;
    } // switch()

    KRun::runURL(KURL( str ), "text/html");
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
        kapp->invokeHelp();
    }
    else if (isNum)
    {
        value = (jumpToTrack * 10) + value;

        if (value <= (int)tracktitlelist.count())
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
    if (jumpToTrack > 0 && jumpToTrack <= (int)tracktitlelist.count())
    {
        wm_cd_play(jumpToTrack, 0, jumpToTrack + 1);
    }

    jumpToTrack = 0;
} // jumpTracks

bool KSCD::playing()
{
    return ( wm_cd_status() == WM_CDM_PLAYING );
}

int KSCD::currentTrack()
{
    return wm_cd_getcurtrack();
}

QString KSCD::currentTrackTitle()
{
    int track = currentTrack();
    return (track > -1) ? tracktitlelist[track] : QString::null;
}

QString KSCD::currentAlbum()
{
    QString album = tracktitlelist[0];
    int slash = album.find('/');

    if (slash < 2)
    {
        return album;
    }


    return album.right(album.length() - slash - 2);
}

QString KSCD::currentArtist()
{
    QString artist = tracktitlelist[0];
    int slash = artist.find('/');

    if (slash < 1)
    {
        return artist;
    }

    return artist.left(slash - 1);
}

QStringList KSCD::trackList()
{
    return tracktitlelist;
}

void KSCD::clearSongList()
{
    songListCB->clear();
    QToolTip::remove(songListCB);
    QToolTip::add(songListCB, i18n("Track list"));
}

void KSCD::populateSongList()
{
    // need to start i at 0 for the case when tracktitlelist is empty
    int i = 0;
    clearSongList();
    QStringList::Iterator it = tracktitlelist.begin();
    if(it != tracktitlelist.end() && !(*it).isEmpty())
        setArtist((*it));
    else
        setArtist(i18n("<Unknown>"));

    for (++it; it != tracktitlelist.end(); ++it, ++i )
    {
        int mins = wm_cd_gettracklen(i + 1);
        QString time;
        if (mins > 0)
        {
            int secs = mins % 60;
            mins /= 60;
            time.sprintf(" (%02d:%02d)", mins, secs);
        }

        songListCB->insertItem(QString::fromLocal8Bit("%1: %2%3")
                               .arg(QString::number(i + 1).rightJustify(2, '0'), *it, time));
    }

    for(; i < wm_cd_getcountoftracks(); ++i)
    {
        int mins = wm_cd_gettracklen(i + 1);
        QString time;
        if (mins > 0)
        {
            int secs = mins % 60;
            mins /= 60;
            time.sprintf(" (%02d:%02d)", mins, secs);
        }
        songListCB->insertItem(i18n("%1: <Unknown>%3")
                               .arg(QString::number(i + 1).rightJustify(2, '0'), time));
    }

    updateDisplayedTrack(currentTrack());
}

void KSCD::setSongListTo(int cb_index)
{
    songListCB->setCurrentItem(cb_index);
    // drop the number.
    // for Mahlah, a picky though otherwise wonderful person - AJS
    QString justTheName = songListCB->currentText();
    justTheName = justTheName.right(justTheName.length() - 4);

    QToolTip::remove(songListCB);
    QToolTip::add(songListCB, i18n("Current track: %1").arg(justTheName));
}

/**
 * main()
 */
int main( int argc, char *argv[] )
{

    KAboutData aboutData( "kscd", I18N_NOOP("KsCD"),
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

    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start())
    {
        fprintf(stderr, "kscd is already running!\n");
        exit(0);
    }

    KUniqueApplication a;

    kapp->dcopClient()->setDefaultObject("CDPlayer");

    KSCD *k = new KSCD();

    a.setTopWidget( k );
    a.setMainWidget( k );

    k->setCaption(a.caption());

    if (kapp->isRestored())
    {
        KConfig* config = KApplication::kApplication()->sessionConfig();
        config->setGroup("General");
        if (config->readBoolEntry("Show"))
            k->show();
    }
    else
    {
        k->show();
    }

    return a.exec();
} // main()


#include "kscd.moc"
