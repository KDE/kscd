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

#include "docking.h"
#include "kscd.h"
#include "mgconfdlg.h"
#include "version.h"

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

#include "inexact.h"
#include "CDDialog.h"
#include "configdlg.h"
#include "configWidget.h"
#include "kvolumecontrol.h"

static const char description[] = I18N_NOOP("KDE CD player");

DockWidget*     dock_widget;
bool stoppedByUser = true;
bool device_change = true;

void            kcderror(const QString& title, const QString& message);
void            kcdworker(int );

//void          parseargs(char* buf, char** args);
extern QTime framestoTime(int frames);

static QString formatTrack(int d1, int d2)
{
  QString str = QString::fromLocal8Bit("%1/%2")
    .arg( QString::number(d1).rightJustify(2, '0') )
    .arg( QString::number(d2).rightJustify(2, '0') );
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
    background_color(black),
    led_color(green),
    jumpToTrack(0L),
    skipDelta(30),
    volume(40),
    randomplay(false),
    looping(false),
    cddrive_is_ok(true),
    have_new_cd(true),
    time_display_mode(TRACK_SEC),
    docking(true),
    autoplay(false),
    stopexit(true),
    ejectonfinish(false),
    updateDialog(false), //!!!!
    revision(0) // The first freedb revision is "0" //!!!!
{
  random_current      = random_list.begin();

  cddb = new KCDDB::Client();
  cddb->setBlockingMode(false);
  connect(cddb, SIGNAL(finished(CDDB::Result)),
          this, SLOT(cddb_done(CDDB::Result)));

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
  initWorkMan();

  /* debug
    wm_cd_set_verbosity(255);
   */

  // the volume slider
  m_volume = new KVolumeControl(volumePB, this);
  m_volume->setValue(volume);
  connect(m_volume, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));

  connect(timeSlider, SIGNAL(valueChanged(int)), SLOT(jumpToTime(int)));

  /* FIXME check for return value */
  setDevicePaths(cd_device_str, audio_system_str, audio_device_str);

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

  if (looping)
  {
    loopled->on();
    loopled->show();
    repeatPB->setOn(true);
  }

  dock_widget = new DockWidget( this, "dockw");
  setDocking(docking);

  connectDCOPSignal(0, 0, "KDE_emailSettingsChanged()", "emailSettingsChanged()", false);

  setFocusPolicy(QWidget::NoFocus);

  songListCB->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  adjustSize();
  setFixedHeight(this->height());
  QTimer::singleShot(0, this, SLOT(initCDROM()));
} // KSCD


KSCD::~KSCD()
{
    if (cd->trk)
    {
        free(cd->trk);
        cd->trk = 0L;
        cd->ntracks = 0;
    }

    signal (SIGINT, SIG_DFL);
    delete cddb;
} // ~KSCD

bool
KSCD::digitalPlayback() {
#if defined(BUILD_CDDA)
        return !(audio_system_str.isEmpty());
#else
        return false;
#endif
}

void
KSCD::initialShow()
{
    KConfig* config = kapp->config();

    config->setGroup("GENERAL");
    if (!config->readBoolEntry("HiddenControls", !docking))
    {
        show();
    }
}

/**
 * Initialize the variables only in WorkMan
 * FIXME: What is needed exactly?
 */
void
KSCD::initWorkMan()
{
  tmppos       = 0;
  save_track   = 1;
} // initWorkMan()


/**
 * Init CD-ROM and display
 */
void
KSCD::initCDROM()
{
  cdMode();
  populateSongList();

  if(cddrive_is_ok)
    volChanged(volume);

  int cur_cdmode = wm_cd_status();
  if (autoplay && cur_cdmode != WM_CDM_PLAYING)
  {
    playClicked();
  }

  timer.start(1000);
} // initCDROM

void
KSCD::setVolume(int v)
{
    volChanged(v);
    m_volume->setValue(v);
}

/**
 * Initialize smallfont which fits into the 13 and 14 pixel widgets
 * and verysmallfont which is slightly smaller.
 */
void
KSCD::initFont()
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

QPushButton *
KSCD::makeButton( int x, int y, int w, int h, const QString& n )
{
  // ML XXX
  QPushButton *pb = new QPushButton(n, this);
  pb->setFocusPolicy(QWidget::NoFocus);
  if (w <= 1 && h <= 1) outerLO->addWidget(pb, y, x);
  else outerLO->addMultiCellWidget(pb, y, h + y - 1 , x, w + x - 1);
  pb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  return pb;
} // makeButton

/**
 * drawPanel() constructs KSCD's little black LED area
 */
void
KSCD::drawPanel()
{
  const int SBARWIDTH = 240;
  const int SBARHEIGHT = 27;
  const int BACKDROPHEIGHT = (2 * SBARHEIGHT) + (SBARHEIGHT / 2) - 1;

  setIcons();

  backdrop->setFixedSize(SBARWIDTH, BACKDROPHEIGHT);
  backdrop->setFocusPolicy(QWidget::NoFocus);

  // mildly gross.. but...
  // take the height of the buttons add the three pixels of space in between.
  // then subtract that from the height of the LED and the
  // button height we're after and voila, peace on earth....
  // well, would you believe a decent looking layout?
  adjustSize();
  int buttonHeight = shufflePB->height();
  int buttonStackHeight = playPB->height() + stopPB->height() + prevPB->height() + buttonHeight + 3;
  int ledSpacing = buttonStackHeight - (BACKDROPHEIGHT + buttonHeight) - 1;
  infoPB->setFixedHeight(buttonHeight);
  cddbPB->setFixedHeight(buttonHeight);
  ledLayout->setRowSpacing(1, ledSpacing);

  const int D = 6;

  for (int u = 0; u < 5; u++) {
     trackTimeLED[u] = new BW_LED_Number(backdrop);
     trackTimeLED[u]->setLEDoffColor(background_color);
     trackTimeLED[u]->setLEDColor(led_color, background_color);
     trackTimeLED[u]->setGeometry(1 + u * 18, D, 23,  30);
     connect(trackTimeLED[u], SIGNAL(clicked()), this, SLOT(cycleplaytimemode()));
  }

  QString zeros("--:--");
  setLEDs(zeros);
  artistlabel = new QLabel(backdrop);
  artistlabel->setFont(smallfont);
  artistlabel->setAlignment(AlignLeft);
  artistlabel->setGeometry(5, 38, SBARWIDTH -15, 13);
  artistlabel->clear();

  titlelabel = new QLabel(backdrop);
  titlelabel->setFont(verysmallfont);
  titlelabel->setAlignment(AlignLeft);
  titlelabel->setGeometry(5, 50, SBARWIDTH -15, 13);
  titlelabel->clear();

  statuslabel = new QLabel(backdrop);
  statuslabel->setFont(verysmallfont);
  statuslabel->setAlignment(AlignLeft);
  statuslabel->setGeometry(110, D, 60, 14);

  queryled = new LedLamp(backdrop);
  queryled->move(220, D + 1);
  queryled->off();
  queryled->hide();

  loopled = new LedLamp(backdrop, LedLamp::Loop);
  loopled->move(220, D + 18);
  loopled->off();

  volumelabel = new QLabel(backdrop);
  volumelabel->setFont(smallfont);
  volumelabel->setAlignment(AlignLeft);
  volumelabel->setGeometry(110, 14 + D, 60, 14);
  volumelabel->setText(i18n("Vol: --"));

  tracklabel = new QLabel(backdrop);
  tracklabel->setFont(smallfont);
  tracklabel->setAlignment(AlignLeft);
  tracklabel->setGeometry(178, 14 + D, 40, 14);
  tracklabel->setText("--/--");

  totaltimelabel = new QLabel(backdrop);
  totaltimelabel->setFont( smallfont );
  totaltimelabel->setAlignment( AlignLeft );
  totaltimelabel->setGeometry(178, D, 60, 14);
  totaltimelabel->hide();
} // drawPanel

void
KSCD::setIcons()
{
  playPB->setIconSet(SmallIconSet("player_play"));
  stopPB->setIconSet(SmallIconSet("player_stop"));
  ejectPB->setIconSet(SmallIconSet("player_eject"));
  prevPB->setIconSet(SmallIconSet(QApplication::reverseLayout() ? "player_fwd" : "player_rew"));
  nextPB->setIconSet(SmallIconSet(QApplication::reverseLayout() ? "player_rew" : "player_fwd"));
  cddbPB->setIconSet(SmallIconSet("view_text"));
  infoPB->setIconSet(SmallIconSet("run"));
}

void
KSCD::setupPopups()
{
    QPopupMenu* mainPopup   = new QPopupMenu(this);
    infoPB->setPopup(mainPopup);
    QPopupMenu* perfPopup = new QPopupMenu (this);
    purchPopup   = new QPopupMenu (this);
    infoPopup   = new QPopupMenu (this);

    purchPopup->insertItem("CD Now", 0);
    purchPopup->insertItem("CD Universe", 1);

    perfPopup->insertItem("Tourdates.com", 0);

    infoPopup->insertItem("MusicMoz", 0);
    infoPopup->insertItem("Ultimate Bandlist", 1);
    infoPopup->insertSeparator();
    infoPopup->insertItem("AlltheWeb", 2);
    infoPopup->insertItem("Altavista", 3);
    infoPopup->insertItem("Excite", 4);
    infoPopup->insertItem("Google", 5);
    infoPopup->insertItem("Google Groups", 6);
    infoPopup->insertItem("HotBot", 7);
    infoPopup->insertItem("Lycos", 8);
    infoPopup->insertItem("Open Directory", 9);
    infoPopup->insertItem("Yahoo!", 10);

    m_actions->action(KStdAction::name(KStdAction::Preferences))->plug(mainPopup);
    m_actions->action(KStdAction::name(KStdAction::KeyBindings))->plug(mainPopup);
    mainPopup->insertSeparator();
    mainPopup->insertItem(i18n("Purchases"), purchPopup);
    connect( purchPopup, SIGNAL(activated(int)), SLOT(purchases(int)) );

    mainPopup->insertItem (i18n("Information"), infoPopup);

    connect( infoPopup, SIGNAL(activated(int)), SLOT(information(int)) );

    KHelpMenu* helpMenu = new KHelpMenu(this, KGlobal::instance()->aboutData(), false);
    mainPopup->insertItem(i18n("Help"), helpMenu->menu());
    mainPopup->insertSeparator();
    m_actions->action(KStdAction::name(KStdAction::Quit))->plug(mainPopup);
} // setupPopups


void
KSCD::playClicked()
{
    int cur_cdmode;
    int track;

    cur_cdmode = wm_cd_status();
    if (!cddrive_is_ok || cur_cdmode < 1)
    {
        return;
    }

    kapp->processEvents();
    kapp->flushX();
    bool playing = false;


#ifdef NEW_BSD_PLAYCLICKED
    if(cur_cdmode == WM_CDM_STOPPED ||
       cur_cdmode == WM_CDM_UNKNOWN ||
       cur_cdmode == WM_CDM_BACK)
#else
    if(cur_cdmode == WM_CDM_STOPPED ||
       cur_cdmode == WM_CDM_UNKNOWN)
#endif
    {
        statuslabel->setText( i18n("Playing") );
        playing = true;
        setLEDs( "00:00" );
        populateSongList();

        kapp->processEvents();
        kapp->flushX();

        if(randomplay)
        {
            nextClicked();
        }
        else if(!playlist.isEmpty())
        {
            if(playlistpointer >=(int) playlist.count())
                playlistpointer = 0;

            if (playlist.at(playlistpointer) != playlist.end())
            {
                save_track = track = atoi((*playlist.at(playlistpointer)).ascii());
                wm_cd_play (track, 0, track + 1);
                playing = true;
            }
            else
            {
                wm_cd_play (save_track, 0, WM_ENDTRACK);
            }
        }
        else
        {
            wm_cd_play (save_track, 0, WM_ENDTRACK);
        }
    }
    else if (cur_cdmode == WM_CDM_PLAYING || cur_cdmode == WM_CDM_PAUSED)
    {
        switch (cur_cdmode)
        {
            case WM_CDM_PLAYING:
                statuslabel->setText( i18n("Pause") );
                playing = false;
                wm_cd_pause();
                break;
            case WM_CDM_PAUSED:
                if(randomplay)
                {
                    statuslabel->setText( i18n("Shuffle") );
                }
                else
                {
                    statuslabel->setText( i18n("Playing") );
                }
                wm_cd_pause();
                playing = true;
                break;

            default:
                // TODO: next release: force "stop".
                statuslabel->setText( i18n("Strange...") );
                break;
        } // switch

        kapp->processEvents();
        kapp->flushX();
    } // if (WM_CDM_STOPPED||UNKNOWN) else

    updatePlayPB(playing);
    cdMode();
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

void KSCD::song_list_complete(void)
{
    if (randomplay) {
        setShuffle(false);
        setShuffle(true);
    }
}

void KSCD::setShuffle(bool shuffle)
{
    if (randomplay == shuffle)
    {
        return;
    }

    randomplay = shuffle;
    shufflePB->blockSignals(true);
    shufflePB->setOn(shuffle);
    shufflePB->blockSignals(false);

    if (randomplay) {
        statuslabel->setText(i18n("Shuffle"));
        if(cd && cd->ntracks > 0) {
            make_random_list(); /* koz: Build a unique, once, random list */
            if(WM_CDM_PLAYING == wm_cd_status())
                nextClicked();
        }
    }
}

void
KSCD::stopClicked()
{
    //    looping = false;
    stoppedByUser = true;
    statuslabel->setText(i18n("Stopped"));
    updatePlayPB(false);
    setLEDs("--:--");
    kapp->processEvents();
    kapp->flushX();

    save_track = 1;
    playlistpointer = 0;
    wm_cd_stop();
} // stopClicked()

void
KSCD::prevClicked()
{
    int track;
    setLEDs("00:00");
    kapp->processEvents();
    kapp->flushX();

    if(randomplay) {
        if((track = prev_randomtrack()) < 0)
            return;
    } else if(!playlist.isEmpty()) {
        playlistpointer--;
        if(playlistpointer < 0 )
        {
            playlistpointer = playlist.count() -1;
        }
        track = atoi((*playlist.at(playlistpointer)).ascii());
    } else {
        // djoham@netscape.net suggested the real-world cd-player behaviour
        // of only jumping to the beginning of the current track if playing
        // advanced more than 2 seconds. I think that's good, but maybe I'll
        // make this configurable.
        track = wm_cd_getcurtrack();
        if(!(cur_pos_rel > 2))
            track--;
    }

    if(randomplay || !playlist.isEmpty())
        wm_cd_play (track, 0, track + 1);
    else
        wm_cd_play (track, 0, WM_ENDTRACK);
} // prevClicked()

void
KSCD::nextClicked()
{
    int track;
    setLEDs("00:00");
    kapp->processEvents();
    kapp->flushX();

    if(randomplay) {
        if((track = next_randomtrack()) < 0)
            return;
    } else if(!playlist.isEmpty()) {
        if(playlistpointer < (int)playlist.count() - 1)
            playlistpointer++;
        else
            playlistpointer = 0;

         track = atoi((*playlist.at(playlistpointer)).ascii() );
    } else {
        // TODO: determine if this should indeed be cur_track + 2?
        track = wm_cd_getcurtrack() + 1;
    }

    if(randomplay || !playlist.isEmpty())
         wm_cd_play (track, 0, track + 1);
    else
         wm_cd_play (track, 0, WM_ENDTRACK);
} // nextClicked()

void
KSCD::jumpToTime(int seconds)
{
    kapp->processEvents();
    kapp->flushX();

    int track = wm_cd_getcurtrack();

    if (wm_cd_status() == WM_CDM_PLAYING && seconds < cd->trk[track - 1].length)
    {
        if(randomplay || !playlist.isEmpty())
            wm_cd_play (track, seconds, track + 1);
        else
            wm_cd_play (track, seconds, WM_ENDTRACK);
    }
} // jumpToTime(int seconds)

void
KSCD::quitClicked()
{
    // ensure nothing else starts happening
    queryledtimer.stop();
    titlelabeltimer.stop();
    cycletimer.stop();
    timer.stop();
    jumpTrackTimer.stop();

    writeSettings();
    //setShuffle(false);
    statuslabel->clear();
    setLEDs( "--:--" );

    // Good GOD this is evil
    kapp->processEvents();
    kapp->flushX();

    if(stopexit)
        wm_cd_stop();

    wm_cd_destroy();

    kapp->quit();
} // quitClicked()


void
KSCD::closeEvent( QCloseEvent *e )
{
    // we need to figure out if we were called by the system tray
    // to decide whether or not to actually quit or not =/
    // this behaviour of ksystemtray is, IMHO, very silly
    const QObject* caller = sender();
    while (caller)
    {
        if (caller == dock_widget)
        {
            break;
        }
        caller = caller->parent();
    }

    if (docking && !caller && !kapp->sessionSaving())
    {
        hide();
        e->ignore();
        return;
    }

    /* Stop playing the CD */
    int cur_cdmode = wm_cd_status();
    if ( stopexit && cur_cdmode == WM_CDM_PLAYING )
        stopClicked();

    writeSettings();
    //setShuffle(false);

    statuslabel->clear();

    setLEDs( "--:--" );

    kapp->processEvents();
    kapp->flushX();

    // TODO: is this really necessary given the stopClicked() above?
    if (stopexit)
        wm_cd_stop();

     e->accept();
} // closeEvent

bool
KSCD::event( QEvent *e )
{
    return QWidget::event(e);
} // event


void
KSCD::loopOn()
{
    looping = true;
    loopled->on();
    loopled->show();
    kapp->processEvents();
    kapp->flushX();
} // loopOn;

void
KSCD::loopOff()
{
    looping = false;
    loopled->off();
    loopled->show();
    kapp->processEvents();
    kapp->flushX();
} // loopOff;

void
KSCD::loopClicked()
{
    if(looping)
    {
        loopOff();
    } else {
        loopOn();
    }
} // loopClicked

/**
 * Do everything needed if the user requested to eject the disc.
 *
 */
void
KSCD::ejectClicked()
{
    if(!cddrive_is_ok)
        return;

    // no matter what happens after this,
    // we aren't going to be playing the CD anymore
    // in theory, we should only need this when !curentlyejected
    // but better safe than sorry (or until i get the time to
    // do more testing ;) AJS
    updatePlayPB(false);

    if (wm_cd_status() == WM_CDM_EJECTED)
    {
      statuslabel->setText(i18n("Closing"));
      kapp->processEvents();
      kapp->flushX();
      have_new_cd = true;
      wm_cd_closetray();
    }
    else
    {
      statuslabel->setText(i18n("Ejecting"));
      kapp->processEvents();
      kapp->flushX();
      setArtistAndTitle("", "");
      tracktitlelist.clear();
      extlist.clear();
      category = "";
      genre = "";

      wm_cd_stop();
      wm_cd_eject();
    }

    // if you can read this, you're too close
    cdMode();
} // ejectClicked

void
KSCD::randomSelected()
{
    setShuffle(!randomplay);
} // randomSelected

/**
 * A Track was selected for playback from the drop down box.
 *
 */
void
KSCD::trackSelected( int trk )
{
    if (trk < 0)
    {
        return;
    }

    setShuffle(false);
    tracklabel->setText( formatTrack( trk + 1, cd->ntracks ) );

    if(trk+1 < (int)tracktitlelist.count())
    {
        setArtistAndTitle(tracktitlelist.first(),
                          *tracktitlelist.at(trk+1));
    }
    else
    {
        setArtistAndTitle(tracktitlelist.first(),
                          i18n("<Unknown>"));
    }

    setLEDs("00:00");
    kapp->processEvents();
    kapp->flushX();

    wm_cd_play(trk + 1, 0, trk + 2);
    updatePlayPB(true);
} // trackSelected

void
KSCD::showConfig()
{
    if (!configDialog)
    {
        configDialog = new ConfigDlg(this);
        connect(configDialog, SIGNAL(finished()), this, SLOT(configDone()));
    }

    configDialog->show();
    configDialog->setActiveWindow();
    configDialog->raise();
} // aboutClicked()

void
KSCD::configDone()
{
    // dialog deletes itself
    configDialog = 0L;
}

void
KSCD::configureKeys()
{
    KKeyDialog::configure(m_actions, this);
}

void
KSCD::setDevicePaths(QString cd_device, QString audio_system, QString audio_device)
{
    static bool first_init = true;
    if(first_init) {
        first_init = false;
    } else if (cd_device_str == cd_device &&
        audio_system_str == audio_system &&
        audio_device_str == audio_device)
    {
        return;
    }

    cddrive_is_ok = false;
    cd_device_str = cd_device;
    audio_system_str = audio_system;
    audio_device_str = audio_device;

    int ret = wm_cd_init(
#if defined(BUILD_CDDA)
        audio_system_str.isEmpty()?WM_CDIN:WM_CDDA,
        QFile::encodeName(cd_device_str), audio_system_str.ascii(), audio_device_str.ascii(), 0);
    kdDebug() << "Device changed to " << cd_device_str << ", " << audio_system_str
        << ", " << audio_device_str << ". return " << ret << "\n";
#else
        WM_CDIN, QFile::encodeName(cd_device_str), 0, 0, 0);
    kdDebug() << "Device changed to " << cd_device_str << ". return " << ret << "\n";
#endif

    device_change = true;
    setArtistAndTitle("", "");
    tracktitlelist.clear();
    extlist.clear();
    clearSongList();
    initWorkMan();
    initCDROM();
    cddrive_is_ok = true;
} // setDevicePath(QString)

void
KSCD::setDocking(bool dock)
{
    docking = dock;
    if(docking)
    {
        dock_widget->show();
        connect(this, SIGNAL(trackChanged(const QString&)),
                dock_widget, SLOT(setToolTip(const QString&)));
    }
    else
    {
        dock_widget->hide();
        disconnect(this, SIGNAL(trackChanged(const QString&)),
                   dock_widget, SLOT(setToolTip(const QString&)));
    }
}

void
KSCD::incVolume()
{
    // FIXME volSB->addStep();
} // incVolume

void
KSCD::decVolume()
{
    // FIXME volSB->subtractStep();
} // decVolume

void
KSCD::volChanged( int vol )
{
    if (!cddrive_is_ok)
        return;

    QString str;
    str = QString::fromUtf8( QCString().sprintf( i18n("Vol: %02d%%").utf8(),vol) );
    volumelabel->setText(str);
    if(!wm_cd_volume(vol, WM_BALANCE_SYMMETRED))
        volume = vol;
} // volChanged

void
KSCD::make_random_list()
{
    /* koz: 15/01/00. I want a random list that does not repeat tracks. Ie, */
    /* a list is created in which each track is listed only once. The tracks */
    /* are picked off one by one until the end of the list */

    int selected = 0;
    bool rejected = false;

    //kdDebug() << "Playlist has " << size << " entries\n" << endl;
    random_list.clear();
    for(int i = 0; i < cd->ntracks; i++)
    {
        do {
            selected = 1 + (int) randSequence.getLong(cd->ntracks);
            rejected = (random_list.find(selected) != random_list.end());
        } while(rejected == true);
        random_list.append(selected);
    }

/*  debug
    RandomList::iterator it;
    for(it = random_list.begin(); it != random_list.end(); it++) {
        printf("%i ", *it);
    }
    printf("\n");
*/
    random_current = random_list.end();
} // make_random_list()

int
KSCD::next_randomtrack()
{
    /* Check to see if we are at invalid state */
    if(random_current == random_list.end()) {
        random_current = random_list.begin();
    } else if(random_current == random_list.fromLast()) {
        if(!looping) {
                stopClicked();
                return -1;
        } else {
                // playing the same random list isn't very random, is it?
                make_random_list();
                return next_randomtrack();
        }
    } else {
        ++random_current;
    }

    return *random_current;
} // next_randomtrack

int
KSCD::prev_randomtrack()
{
    /* Check to see if we are at invalid state */
    if(random_current == random_list.end()) {
        random_current = random_list.fromLast();
    } else if(random_current == random_list.begin()) {
        if(!looping) {
            stopClicked();
            return -1;
        } else {
            // playing the same random list isn't very random, is it?
            make_random_list();
            return prev_randomtrack();
        }
    } else {
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
void
KSCD::cdMode()
{
    static bool damn = true;
    int cur_cdmode = wm_cd_status();
    int track = wm_cd_getcurtrack();

    if(WM_CDS_NO_DISC(cur_cdmode))
    {
        have_new_cd = true;
    }

    if(cur_cdmode < 0)
    {
        if(cddrive_is_ok)
        {
            statuslabel->setText( i18n("Error") );
            cddrive_is_ok = false;
            QString errstring =
                i18n("CD-ROM read or access error (or no audio disc in drive).\n"\
                     "Please make sure you have access permissions to:\n%1")
                .arg(cd_device_str.isNull() ? wm_drive_device() : cd_device_str);
            KMessageBox::error(this, errstring, i18n("Error"));
        }
        return;
    }

    cddrive_is_ok = true; // cd drive ok

    if (device_change == true)
    {
        device_change = false;
        wm_cd_stop();
        damn = false;
    }

    switch (cur_cdmode) {
        case WM_CDM_DEVICECHANGED:
            break;
        case WM_CDM_UNKNOWN:
            save_track = 1;
            statuslabel->setText( "" ); // TODO how should I properly handle this
            damn = true;
            break;

        case WM_CDM_TRACK_DONE: // == WM_CDM_BACK
            if( randomplay ) /*FIXME:  propably nex_clicked ?? */
            {
                if((track = next_randomtrack()) < 0)
                    return;
                wm_cd_play( track, 0, track + 1 );
            }
            else if (playlist.count() > 0)
            {
                if(playlistpointer < (int)playlist.count() - 1)
                    playlistpointer++;
                else
                    playlistpointer = 0;

                track = atoi( (*playlist.at(playlistpointer)).ascii() );
                wm_cd_play(track, 0, track + 1);
            }
            else if ( looping )
            {
                if (track == cd->ntracks)
                {
                    wm_cd_play (1, 0, WM_ENDTRACK);
                }
            }
            else
            {
                save_track = 1;
                statuslabel->clear(); // TODO how should I properly handle this
                damn = true;
            }
            break;

        case WM_CDM_PLAYING:
            playtime ();
            if(randomplay)
                statuslabel->setText( i18n("Shuffle") );
            else
                statuslabel->setText( i18n("Playing") );

            //sprintf( p, "%02d  ", track );
            if (songListCB->count() == 0)
            {
                // we are in here when we start kscd and
                // the cdplayer is already playing.
                populateSongList();
                setSongListTo( track - 1 );

                have_new_cd = false;
                get_cddb_info(false); // false == do not update dialog if open
            } else {
                setSongListTo( track - 1 );
            }
            tracklabel->setText( formatTrack(track, cd->ntracks) );

            if((track < (int)tracktitlelist.count()) && (track >= 0))
            {
                setArtistAndTitle(tracktitlelist.first(),
                                  *tracktitlelist.at(track));
            }

            damn = true;
            stoppedByUser = false;
            break;

        case WM_CDM_FORWARD:
            break;

        case WM_CDM_PAUSED:
            statuslabel->setText( i18n("Pause") );
            damn = true;
            break;

        case WM_CDM_STOPPED:
            if (damn) {
                if(ejectonfinish && !stoppedByUser){
                    stoppedByUser = true;
                    ejectClicked();
                    break;
                }
                statuslabel->setText( i18n("Ready") );
                setLEDs( "--:--" );
                populateSongList();

                int w = ((track >= 0) ? track : 1);

                tracklabel->setText( formatTrack( track >= 0 ? track : 1, cd->ntracks) );

                if( w < (int)tracktitlelist.count()){
                    setArtistAndTitle(tracktitlelist.first(),
                                      *tracktitlelist.at( w ));
                }
            }
            damn = false;
            if (have_new_cd)
            {
                save_track = 1;
                have_new_cd = false;
                get_cddb_info(false); // false == do not update dialog if open
                if(autoplay && cur_cdmode == WM_CDM_EJECTED)
                    playClicked();
            }

            break;

        case WM_CDM_EJECTED:
            statuslabel->setText( i18n("Ejected") );
            setArtistAndTitle("", "");
            tracktitlelist.clear();
            extlist.clear();
            clearSongList();
            setLEDs( "--:--" );
            tracklabel->setText( "--/--" );
            setArtistAndTitle("", "");
            totaltimelabel->clear();
            totaltimelabel->lower();
            damn = true;
            break;

        case WM_CDM_NO_DISC:
            statuslabel->setText( i18n("no disc") );
            setArtistAndTitle("", "");
            tracktitlelist.clear();
            extlist.clear();
            clearSongList();
            setLEDs( "--:--" );
            tracklabel->setText( "--/--" );
            setArtistAndTitle("", "");
            totaltimelabel->clear();
            totaltimelabel->lower();
            damn = true;
      break;
    }
} /* cdMode */

void
KSCD::setLEDs(const QString& symbols)
{

    // nLEDs->setText(symbols);

    if(symbols.length() < 5){
        return;
    }

    for(int i=0;i<5;i++){
        trackTimeLED[i]->display((char)symbols.local8Bit().at(i));
    }
}


void
KSCD::setColors()
{
    backdrop->setBackgroundColor(background_color);

    QColorGroup colgrp( led_color, background_color, led_color,led_color , led_color,
                        led_color, white );

    titlelabel ->setPalette( QPalette(colgrp,colgrp,colgrp) );
    artistlabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
    volumelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
    statuslabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
    tracklabel ->setPalette( QPalette(colgrp,colgrp,colgrp) );
    totaltimelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
    // nLEDs->setPalette( QPalette(colgrp,colgrp,colgrp) );


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
    artistlabel->setFont( smallfont );

}

void
KSCD::setColors(const QColor& LEDs, const QColor& bground)
{
    background_color = bground;
    led_color = LEDs;
    setColors();
}

void
KSCD::readSettings()
{
	KConfig* config = kapp->config();

	config->setGroup("GENERAL");
	volume     	= config->readNumEntry("Volume", volume);
	randomplay = config->readBoolEntry("RandomPlay", false);
	docking = config->readBoolEntry("DOCKING", docking);
	autoplay		= config->readBoolEntry("AUTOPLAY", autoplay);
	stopexit 	= config->readBoolEntry("STOPEXIT", stopexit);
	ejectonfinish = config->readBoolEntry("EJECTONFINISH", ejectonfinish);
	looping    	= config->readBoolEntry("Looping", looping);
	skipDelta = config->readNumEntry("SkipDelta", skipDelta);
	time_display_mode = config->readNumEntry("TimeDisplay", TRACK_SEC);

#ifndef DEFAULT_CD_DEVICE
#define DEFAULT_CD_DEVICE "/dev/cdrom"
	// sun ultrix etc have a canonical cd rom device specified in the
	// respective plat_xxx.c file. On those platforms you need nnot
	// specify the cd rom device and DEFAULT_CD_DEVICE is not defined
	// in config.h
#endif

        cd_device_str = config->readEntry("CDDevice", DEFAULT_CD_DEVICE);
        audio_system_str = config->readEntry("AudioSystem", "");
        audio_device_str = config->readEntry("AudioDevice", "");

	QColor defaultback = black;
	QColor defaultled = QColor(226,224,255);
	background_color = config->readColorEntry("BackColor",&defaultback);
	led_color = config->readColorEntry("LEDColor",&defaultled);
}

/**
 * Write KSCD's Configuration into the kderc file.
 *
 */
void
KSCD::writeSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("GENERAL");
    config->writeEntry("RandomPlay", randomplay);
    config->writeEntry("DOCKING", docking);
    config->writeEntry("AUTOPLAY", autoplay);
    config->writeEntry("STOPEXIT", stopexit);
    config->writeEntry("EJECTONFINISH", ejectonfinish);
    config->writeEntry("CDDevice", cd_device_str);
    config->writeEntry("AudioSystem", audio_system_str);
    config->writeEntry("AudioDevice", audio_device_str);
    config->writeEntry("Volume", volume);
    config->writeEntry("BackColor",background_color);
    config->writeEntry("LEDColor",led_color);
    config->writeEntry("Looping", looping);
    config->writeEntry("HiddenControls", isHidden());
    config->writeEntry("SkipDelta", skipDelta);
    config->writeEntry("TimeDisplay", time_display_mode);

    config->sync();
} // writeSettings()

void
KSCD::CDDialogSelected()
{
    if (!cddialog)
    {
        cddialog = new CDDialog();

        cddialog->setData(cd,tracktitlelist,extlist,xmcd_data,category, genre,
                        revision,playlist,pathlist);

        connect(cddialog,SIGNAL(cddb_query_signal(bool)),this,SLOT(get_cddb_info(bool)));
        connect(cddialog,SIGNAL(dialog_done()),this,SLOT(CDDialogDone()));
        connect(cddialog,SIGNAL(play_signal(int)),this,SLOT(trackSelected(int)));
    }

    cddialog->show();
    cddialog->raise();
}

void
KSCD::CDDialogDone()
{
  delete cddialog;
  cddialog = 0L;
}

void
KSCD::get_cddb_info(bool /*_updateDialog*/)
{
    if (!cd ||
        !cddrive_is_ok ||
        wm_cd_status() < 1)
    {
         return;
    }

    // Don't crash if no disc is in
    if( cd->length == 0 ) {
      kdDebug() << "CD length seems to be zero" << endl;
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
    setArtistAndTitle("", "");

    for(int i = 0 ; i < cd->ntracks; i++)
    {
        querylist << cd->trk[i].start;
    }
    song_list_complete();

    querylist << cd->trk[0].start << cd->trk[cd->ntracks].start;
    led_on();
    cddb->lookup(querylist);
} // get_cddb_info

void
KSCD::cddb_done(CDDB::Result result)
{
    // CDDBTODO: figure out why using CDDB::Success doesn't compile?!
    if ((result != 0 /*KCDDB::CDDB::Success*/) &&
        (result != KCDDB::CDDB::MultipleRecordFound))
    {
        cddb_failed();
        return;
    }
    KCDDB::CDInfo cddbInfo = cddb->bestLookupResponse();

    // shouldn't need to clear it, but i fear the maintainability monster
    // so let's just clear it to be safe, m'kay
    tracktitlelist.clear();

    setArtistAndTitle(cddbInfo.artist, cddbInfo.title);

    // CDDBTODO: we really should get the artist off the 'tracktitlelist'
    tracktitlelist << cddbInfo.artist + " / " + cddbInfo.title;

    revision = cddbInfo.revision;
    category = cddbInfo.category;
    genre = cddbInfo.genre;

    KCDDB::TrackInfoList::ConstIterator it(cddbInfo.trackInfoList.begin());
    KCDDB::TrackInfoList::ConstIterator end(cddbInfo.trackInfoList.end());
    for (; it != end; ++it)
    {
        tracktitlelist << (*it).title;
    }

    playlistpointer = 0;
    populateSongList();

    led_off();
    timer.start(1000);
} // cddb_done

void KSCD::cdtext(struct cdtext_info* p_cdtext)
{
    kdDebug() << "cdtext() called" << endl;
    setArtistAndTitle("", "");
    tracktitlelist.clear();
    extlist.clear();
    tracktitlelist.append(QString().sprintf("%s / %s", (const char*)(p_cdtext->blocks[0]->name[0]),
        (const char*)(p_cdtext->blocks[0]->performer[0])));
    titlelabel->setText(QString((const char*)(p_cdtext->blocks[0]->name[1])));
    artistlabel->setText(tracktitlelist.first());
    songListCB->clear();

    // if it's a sampler, we'll do artist/title
    bool isSampler = (qstricmp(reinterpret_cast<char*>(p_cdtext->blocks[0]->performer[0]), "various") == 0);

    int at = 1;
    for (; at < (p_cdtext->count_of_entries); ++at)
    {
        QString title;
        if (isSampler)
        {
            title.sprintf("%s / %s", p_cdtext->blocks[0]->performer[at], p_cdtext->blocks[0]->name[at]);
        }
        else
        {
            title = reinterpret_cast<char*>(p_cdtext->blocks[0]->name[at]);
        }

        songListCB->insertItem(QString().sprintf("%02d: ", at) +  title);
        tracktitlelist.append(title);
    }

    for(; at < cd->ntracks; ++at)
    {
        songListCB->insertItem(QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), at)));
    }
}

void
KSCD::cddb_no_info()
{
    struct cdtext_info* cdtext_i;
    setArtistAndTitle(i18n("No matching freedb entry found."), "");
    tracktitlelist.clear();
    extlist.clear();
    //    tracktitlelist.append(i18n("No matching freedb entry found."));

    cdtext_i = wm_cd_get_cdtext();
    if(cdtext_i && cdtext_i->valid)
    {
        cdtext(cdtext_i);
    }
    else
    {
       populateSongList();
    }

    led_off();
    timer.start(1000);
} // cddb_no_info

void
KSCD::cddb_failed()
{
    struct cdtext_info* cdtext_i;
    // TODO differentiate between those casees where the communcition really
    // failed and those where we just couldn't find anything
    //        cddb_ready_bug = 0;
    kdDebug() << "cddb_failed() called\n" << endl;
    setArtistAndTitle("", "");
    tracktitlelist.clear();
    extlist.clear();
    tracktitlelist.append(i18n("Error getting freedb entry."));

    cdtext_i = wm_cd_get_cdtext();
    if(cdtext_i && cdtext_i->valid)
    {
        cdtext(cdtext_i);
    }
    else
    {
      for(int i = 0 ; i < cd->ntracks; i++)
        tracktitlelist.append("");

      extlist.clear();
      for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

      populateSongList();

      setArtistAndTitle(i18n("Error getting freedb entry."), "");
    }

    led_off();
    timer.start(1000);
} // cddb_failed

void
KSCD::mycddb_inexact_read()
{
/*    if(cddb_inexact_sentinel == true)
        return;

    QString pick;

    cddb_inexact_sentinel = true;
    QStringList inexact_list;
    cddb->get_inexact_list(inexact_list);

    // Whatever happens, we better clear the list beforehand
    setArtistAndTitle("", "");
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

void
KSCD::led_off()
{
    queryledtimer.stop();
    queryled->off();
    queryled->hide();
    totaltimelabel->raise();
    totaltimelabel->show();

} // led_off

void
KSCD::led_on()
{
    totaltimelabel->hide();
    totaltimelabel->lower();
    queryledtimer.start(800);
    queryled->off();
    queryled->show();
    kapp->processEvents();
    kapp->flushX();
} // led_on

void
KSCD::togglequeryled()
{
    queryled->show();
    queryled->toggle();
} // togglequeryled

void
KSCD::titlelabeltimeout()
{
    // clear the cddb error message on the title label.
    titlelabeltimer.stop();
    titlelabel->clear();

} // titlelabeltimeout

void
KSCD::setArtistAndTitle(const QString& artist, const QString& title)
{
    QString tooltip = "";
    if (!artist.isEmpty()) {
        artistlabel->setText(artist);
        tooltip = KStringHandler::rsqueeze(artist, 30) + "\n";
    }
    else {
        artistlabel->clear();
    }

    if (!title.isEmpty()) {
        titlelabel->setText(title);
        tooltip += KStringHandler::rsqueeze(title, 30);
    }
    else {
        titlelabel->clear();
    }

    if (currentTrack() < 0)
    {
        timeSlider->setRange(0, 0);
    }
    else
    {
        timeSlider->setRange(0, wm_cd_getcurtracklen());
    }

    emit trackChanged(tooltip);
}

void
KSCD::playtime()
{
    static int mymin;
    static int mysec;
    int tmp;

    switch (time_display_mode)
    {
        case TRACK_REM:
            tmp = wm_cd_getcurtracklen() - cur_pos_rel;
            mysec = tmp % 60;
            mymin = tmp / 60;
            break;

        case TOTAL_SEC:
            mysec = cur_pos_abs % 60;
            mymin = cur_pos_abs / 60;
            break;

        case TOTAL_REM:
            tmp = cd->length - cur_pos_abs;
            mysec = tmp % 60;
            mymin = tmp / 60;
            break;

        case TRACK_SEC:
        default:
            tmp = cur_pos_rel % 60;
            if (cur_pos_rel > 0 && tmp == mysec)
                return;
            mysec = tmp;
            mymin = cur_pos_rel / 60;
            break;
    }

    timeSlider->blockSignals( true );
    timeSlider->setValue(cur_pos_rel);
    timeSlider->blockSignals( false );
    QString tmptime;
    tmptime.sprintf("%02d:%02d", mymin, mysec);
    setLEDs(tmptime);
} // playtime

void
KSCD::cycleplaytimemode()
{
    cycletimer.stop();

    if (++time_display_mode > 3)
        time_display_mode = 0;
    playtime();

    switch(time_display_mode){

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

void
KSCD::cycletimeout()
{
    cycletimer.stop();
    QString str;
    str = QString::fromUtf8( QCString().sprintf(i18n("Vol: %02d%%").utf8(),volume) );
    volumelabel->setText(str);
} // cycletimeout


bool
KSCD::getArtist(QString& artist)
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

void
KSCD::performances(int i)
{
    //kdDebug() << "performances " << i << "\n" << endl;
    QString artist;
    QString str;

    if(!getArtist(artist))
        return;

    artist = KURL::encode_string_no_slash(artist);

    switch(i){
        case 0:
            str = QString("http://www.tourdates.com/cgi-bin/search.cgi?type=Artist&search=%1")
                  .arg(artist);
            break;

        default:
            return;
            break;
    }

    KRun::runURL(str, "text/html");
} // performances

void
KSCD::purchases(int i)
{
    //kdDebug() << "purchases " << i << "\n" << endl;

    QString artist;
    QString str;

    if(!getArtist(artist))
        return;

    artist = KURL::encode_string_no_slash(artist);

    switch(i){
        case 0:
            str = QString("http://cdnow.com/switch/from=sr-288025/target=buyweb_products/artfs=%1")
                  .arg(artist);
            break;
        case 1:
            str = QString("http://www.cduniverse.com/cgi-bin/cdubin.exe/rlinka/ean=%1")
                  .arg(artist);
            break;

        default:
            return;
            break;
    }

    KRun::runURL(str, "text/html");
} // purchases

void
KSCD::information(int i)
{
    //kdDebug() << "Information " << i << "\n" << endl;

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
            str = QString("http://www.alltheweb.com/search?cat=web&q=%1")
                    .arg(artist);
            break;

        case 3:
            str = QString("http://altavista.com/web/results?q=%1&kgs=0&kls=1&avkw=xytx")
                  .arg(artist);
            break;

        case 4:
            str = QString("http://msxml.excite.com/_1_2UDOUB70SVHVHR__info.xcite/dog/results?otmpl=dog/webresults.htm&qkw=%1&qcat=web&qk=20&top=1&start=&ver=14060")
                  .arg(artist);
            break;

        case 5:
            str = QString("http://www.google.com/search?q=%1")
                  .arg(artist);
            break;

        case 6:
            str = QString("http://groups.google.com/groups?oi=djq&as_q=%1&num=20")
                  .arg(artist);
            break;

        case 7:
            str = QString("http://www.hotbot.com/default.asp?prov=Inktomi&query=%1&ps=&loc=searchbox&tab=web")
                  .arg(artist);
            break;

        case 8:
            str = QString("http://search.lycos.com/default.asp?lpv=1&loc=searchhp&tab=web&query=%1")
                  .arg(artist);
             break;

         case 9:
             str = QString("http://search.dmoz.org/cgi-bin/search?search=%1")
                   .arg(artist);
             break;

         case 10:
             str = QString("http://search.yahoo.com/bin/search?p=%1")
                   .arg(artist);
             break;

         default:
            return;
            break;
    } // switch()

    KRun::runURL(str, "text/html");
} // information

void
kcderror(const QString& title, const QString& message)
{
 KMessageBox::information(0L, message, title);
}

/**
 * Save state on session termination
 */
bool
KSCD::saveState(QSessionManager& /*sm*/)
{
  writeSettings();
  return true;
} // saveState


/**
 * Allow the user to type in the number of the track
 */
void
KSCD::keyPressEvent(QKeyEvent* e)
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

void
KSCD::jumpTracks()
{
    if (jumpToTrack > 0 && jumpToTrack <= (int)tracktitlelist.count())
    {
        wm_cd_play(jumpToTrack, 0, jumpToTrack + 1);
    }

    jumpToTrack = 0;
} // jumpTracks

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

    if (slash < 1)
    {
        return album;
    }

    return album.left(slash - 1);
}

QString KSCD::currentArtist()
{
    QString artist = tracktitlelist[0];
    int slash = artist.find('/');

    if (slash < 2)
    {
        return artist;
    }

    return artist.right(slash - 2);
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
    for (++it; it != tracktitlelist.end(); ++it, ++i )
    {
        songListCB->insertItem(QString::fromLocal8Bit("%1: %2")
                               .arg(QString::number(i + 1).rightJustify(2, '0'))
                               .arg(*it));
    }

    for(; i < cd->ntracks; ++i)
    {
        songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), i+1)) );
    }
}

void KSCD::setSongListTo(int whichTrack)
{
    songListCB->setCurrentItem(whichTrack);
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
int
main( int argc, char *argv[] )
{

    KAboutData aboutData( "kscd", I18N_NOOP("KsCD"),
                          KSCDVERSION, description,
                          KAboutData::License_GPL,
                          "(c) 2001, Dirk Försterling\n(c) 2003, Aaron J. Seigo");
    aboutData.addAuthor("Aaron J. Seigo", I18N_NOOP("Current maintainer"), "aseigo@kde.org");
    aboutData.addAuthor("Alexander Kern",I18N_NOOP("Workman library update, CDTEXT, CDDA"), "kernalex@kde.org");
    aboutData.addAuthor("Bernd Johannes Wuebben",0, "wuebben@kde.org");
    aboutData.addAuthor("Dirk Försterling", I18N_NOOP("Workman library, previous maintainer"), "milliByte@gmx.net");
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

/*    KGlobal::dirs()->addResourceType("cddb",
                                     KStandardDirs::kde_default("data") +
                                     "kscd/cddb/");*/

    KSCD *k = new KSCD();

    a.setTopWidget( k );
    a.setMainWidget( k );

    k->setCaption(a.caption());
    k->initialShow();

    return a.exec();
} // main()


#include "kscd.moc"
