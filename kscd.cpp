/*
 * Kscd - A simple cd player for the KDE Project
 *
 * $Id$
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
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
#include "CDDBSetup.h"
#include "configdlg.h"
#include "configWidget.h"
#include "smtpconfig.h"

#include "bitmaps/playpaus.xbm"
#include "bitmaps/stop.xbm"
#include "bitmaps/repeat.xbm"
#include "bitmaps/nexttrk.xbm"
#include "bitmaps/prevtrk.xbm"
#include "bitmaps/ff.xbm"
#include "bitmaps/rew.xbm"
#include "bitmaps/info.xbm"
#include "bitmaps/poweroff.xbm"
#if KSCDMAGIC
#include "bitmaps/magic.xbm"
#endif
#include "bitmaps/eject.xbm"
#include "bitmaps/db.xbm"
#include "bitmaps/logo.xbm"
#include "bitmaps/shuffle.xbm"
#include "bitmaps/options.xbm"


static const char description[] = I18N_NOOP("KDE CD player");

DockWidget*     dock_widget;
SMTP                *smtpMailer;
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

class KSCDSlider : public QSlider
{
    public:
        KSCDSlider(QWidget* parent = 0, const char* name = 0)
            : QSlider(0, 100, 5,  50, QSlider::Horizontal, parent, name)
        {}
        ~KSCDSlider() {}

    protected:
        void wheelEvent(QWheelEvent *e)
        {
            bool up = e->delta() > 0;

            if (e->state() & ControlButton)
            {
                if (up)
                {
                    if (value() < (maxValue() / 2))
                    {
                        setValue(maxValue() / 2);
                    }
                    else
                    {
                        setValue(maxValue());
                    }
                }
                else if (value() > (maxValue() / 2))
                {
                    setValue(maxValue() / 2);
                }
                else
                {
                    setValue(minValue());
                }

                return;
            }
            else if (up)
            {
                addStep();
            }
            else
            {
                subtractStep();
            }
        }
};

/****************************************************************************
                  The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name )
  : DCOPObject("CDPlayer"),
    QWidget( parent, name, Qt::WDestructiveClose ),
    smtpConfigData(new SMTPConfigData),  //!!!!
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
    currentlyejected(false),
    updateDialog(false), //!!!!
    revision(0) // The first freedb revision is "0" //!!!!
#if KSCDMAGIC
    ,
    magicproc(0L),
    magic_width(330),
    magic_height(135),
    magic_brightness(3),
    magic_pointsAreDiamonds(false)
#endif
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
  loadBitmaps();
  setColors();
  initWorkMan();
  setupPopups();

  setToolTips();

  /* FIXME check for return value */
  setDevicePaths(cd_device_str, audio_system_str, audio_device_str);

  // set the volume BEFORE setting up the signals
  volSB->setValue(volume);

  connect( &queryledtimer, SIGNAL(timeout()),  SLOT(togglequeryled()) );
  connect( &titlelabeltimer, SIGNAL(timeout()),  SLOT(titlelabeltimeout()) );
  connect( &cycletimer, SIGNAL(timeout()),  SLOT(cycletimeout()) );
  connect( &timer, SIGNAL(timeout()),  SLOT(cdMode()) );
  connect( &jumpTrackTimer, SIGNAL(timeout()),  SLOT(jumpTracks()) );
  connect( playPB, SIGNAL(clicked()), SLOT(playClicked()) );
  connect( stopPB, SIGNAL(clicked()), SLOT(stopClicked()) );
  connect( prevPB, SIGNAL(clicked()), SLOT(prevClicked()) );
  connect( nextPB, SIGNAL(clicked()), SLOT(nextClicked()) );
  connect( fwdPB, SIGNAL(clicked()), SLOT(fwdClicked()) );
  connect( bwdPB, SIGNAL(clicked()), SLOT(bwdClicked()) );
  connect( dockPB, SIGNAL(clicked()), SLOT(quitClicked()) );
#if KSCDMAGIC
  connect( magicPB, SIGNAL(clicked()), SLOT(magicslot()) );
#endif
  connect( replayPB, SIGNAL(clicked()), SLOT(loopClicked()) );
  connect( ejectPB, SIGNAL(clicked()), SLOT(ejectClicked()) );
  connect( songListCB, SIGNAL(activated(int)), SLOT(trackSelected(int)));
  connect( volSB, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));
  connect( aboutPB, SIGNAL(clicked()), SLOT(cycleplaytimemode()));
  connect( optionsbutton, SIGNAL(clicked()), SLOT(showConfig()));
  connect( shufflebutton, SIGNAL(clicked()), SLOT(randomSelected()));
  connect( cddbbutton, SIGNAL(clicked()), SLOT(CDDialogSelected()));
  connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(setColors()));

  // set up the actions and keyboard accels
  KActionCollection* actions = new KActionCollection(this);

  KAction* action;
  action = new KAction(i18n("Play/Pause"), Key_P, this, SLOT(playClicked()), actions, "Play/Pause");
  action = new KAction(i18n("Stop"), Key_S, this, SLOT(stopClicked()), actions, "Stop");
  action = new KAction(i18n("Previous"), Key_B, this, SLOT(prevClicked()), actions, "Previous");
  action = new KAction(i18n("Next"), Key_N, this, SLOT(nextClicked()), actions, "Next");
  action = new KAction(i18n("Forward"), Key_Right, this, SLOT(fwdClicked()), actions, "Forward");
  action = new KAction(i18n("Backward"), Key_Left, this, SLOT(bwdClicked()), actions, "Backward");
  action = KStdAction::action(KStdAction::Quit, this, SLOT(quitClicked()), actions);
  action = new KAction(i18n("Loop"), Key_L, this, SLOT(loopClicked()), actions, "Loop");
  action = new KAction(i18n("Eject"), CTRL + Key_E, this, SLOT(ejectClicked()), actions, "Eject");
  action = new KAction(i18n("Increase Volume"), Key_Plus, this, SLOT(incVolume()), actions, "IncVolume");
  action = new KAction(i18n("Increase Volume"), Key_Equal, this, SLOT(incVolume()), actions, "IncVolume Alt");
  action = new KAction(i18n("Decrease Volume"), Key_Minus, this, SLOT(decVolume()), actions, "DecVolume");
  action = new KAction(i18n("Options"), CTRL + Key_T, this, SLOT(showConfig()), actions, "Options");
  action = new KAction(i18n("Shuffle"), Key_R, this, SLOT(randomSelected()), actions, "Shuffle");
  action = new KAction(i18n("CDDB"), CTRL + Key_D, this, SLOT(CDDialogSelected()), actions, "CDDB");


  if(looping)
  {
    loopled->on();
  }

  smtpMailer = new SMTP;
  connect(smtpMailer, SIGNAL(messageSent()), this, SLOT(smtpMessageSent()));
  connect(smtpMailer, SIGNAL(error(int)), this, SLOT(smtpError(int)));

  dock_widget = new DockWidget( this, "dockw");
  setDocking(docking);

  connectDCOPSignal(0, 0, "KDE_emailSettingsChanged()", "emailSettingsChanged()", false);

  setFocusPolicy ( QWidget::NoFocus );

  this->adjustSize();
  this->setFixedSize(this->width(), this->height());

  QTimer::singleShot(500, this, SLOT(initCDROM()));
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
#if KSCDMAGIC
    delete magicproc;
    magicproc = 0L;
#endif
    delete smtpConfigData;
    delete smtpMailer;
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

void
KSCD::smtpMessageSent(void)
{
  KMessageBox::information(this, i18n("Record submitted successfully"),
         i18n("Record Submission"));
} // smtpMessageSent()

/**
 * Interpret error codes from the SMTP mailer.
 */
void
KSCD::smtpError(int errornum)
{
  QString str, lstr;

  switch(errornum){
  case 10:
    lstr = i18n("Error connecting to server.");
    break;
  case 11:
    lstr = i18n("Not connected.");
    break;
  case 15:
    lstr = i18n("Connection timed out.");
    break;
  case 16:
    lstr = i18n("Timeout waiting for server interaction.");
    break;
  default:
    lstr = i18n("Server said:\n\"%1\"").arg(smtpMailer->getLastLine());
  }
  str = i18n("Error #%1 sending message via SMTP.\n\n%2")
    .arg(errornum).arg(lstr);
  KMessageBox::error(this, str, i18n("Record Submission"));
} // smptError()


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
  kapp->processEvents();
  kapp->flushX();

  cdMode();
  if(cddrive_is_ok)
    volChanged(volume);

  int cur_cdmode = wm_cd_status();
  if (autoplay && cur_cdmode != WM_CDM_PLAYING)
  {
    playClicked();
  }
  else
  {
    timer.start(1000);
  }
} // initCDROM

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
 * drawPanel() constructs KSCD's main window.
 * This is oldstyle and needs to be redesigned.
 * Redone using QGridLayout -- but the inner part still uses fixed layout.
 *
 * @author Bernd Wübben
 * @author Dirk Försterling
 * @author Luciano Montanaro
 */
void
KSCD::drawPanel()
{
  const int SBARWIDTH = 240;
  const int HEIGHT = 27;
  const int WIDTH = 92;

  outerLO = new QGridLayout(this, 5, 9);

  aboutPB = makeButton(0, 0, 2, 2, i18n("About"));
  aboutPB->setMinimumWidth(WIDTH);
  aboutPB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  infoPB = makeButton(0, 2, 1, 1, "");
  infoPB->setMinimumHeight(HEIGHT);

  optionsbutton = makeButton(1, 2, 1, 1, "");

#if KSCDMAGIC
  dockPB = makeButton(0, 3, 1, 1, i18n("Quit"));

  magicPB = makeButton(1, 3, 1, 1, "");
#else
  dockPB = makeButton(0, 3, 2, 1, i18n("Quit"));
#endif
  dockPB->setMinimumHeight(HEIGHT);

  QVBox* innerVB = new QVBox(this);
  outerLO->addMultiCellWidget(innerVB, 0, 2, 2, 6);

  backdrop = new QFrame(innerVB);
  backdrop->setFixedSize(SBARWIDTH - 2, 2 * HEIGHT + HEIGHT / 2 - 1);
  backdrop->setFocusPolicy(QWidget::NoFocus);
  backdrop->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

  const int D = 6;

  for (int u = 0; u < 5; u++) {
     trackTimeLED[u] = new BW_LED_Number(backdrop);
     trackTimeLED[u]->setLEDoffColor(background_color);
     trackTimeLED[u]->setLEDColor(led_color, background_color);
     trackTimeLED[u]->setGeometry(u * 18, D, 23,  30);
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

  volSB = new KSCDSlider(innerVB, "Slider");
  volSB->setFocusPolicy(QWidget::NoFocus);

  cddbbutton = makeButton(2, 3, 1, 1, "");

  shufflebutton = makeButton(3, 3, 1, 1, "");

  replayPB = makeButton(4, 3, 1, 1, "");

  songListCB = new QComboBox(this);
  songListCB->setFocusPolicy (QWidget::NoFocus);
  outerLO->addMultiCellWidget(songListCB, 4 , 4, 0, 8);

  playPB = makeButton(7, 0, 2, 2, "Play/Pause");
  playPB->setMinimumWidth(WIDTH);
  playPB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  stopPB = makeButton(7, 2, 1, 1, "Stop");

  ejectPB = makeButton(8, 2, 1, 1, "Replay");

  bwdPB = makeButton(5, 3, 1, 1, "Bwd");

  fwdPB = makeButton(6, 3, 1, 1, "Fwd");

  prevPB = makeButton(7, 3, 1, 1, "Prev");

  nextPB = makeButton(8, 3, 1, 1, "Next");

} // drawPanel

void
KSCD::loadBitmaps()
{
    QBitmap playBmp( playpause_width, playpause_height, playpause_bits,TRUE );
    QBitmap stopBmp( stop_width, stop_height, stop_bits, TRUE );
    QBitmap prevBmp( prevtrk_width, prevtrk_height, prevtrk_bits, TRUE );
    QBitmap nextBmp( nexttrk_width, nexttrk_height, nexttrk_bits, TRUE );
    QBitmap replayBmp( repeat_width, repeat_height, repeat_bits, TRUE );
    QBitmap fwdBmp( ff_width, ff_height, ff_bits, TRUE );
    QBitmap bwdBmp( rew_width, rew_height, rew_bits, TRUE );
    QBitmap ejectBmp( eject_width, eject_height, eject_bits, TRUE );
    QBitmap infoBmp( info_width, info_height,info_bits, TRUE );
    QBitmap dockBmp( poweroff_width, poweroff_height, poweroff_bits, TRUE );
    QBitmap shuffleBmp( shuffle_width, shuffle_height, shuffle_bits, TRUE );
    QBitmap databaseBmp( db_width, db_height, db_bits, TRUE );
    QBitmap aboutBmp( logo_width, logo_height, logo_bits, TRUE );
    QBitmap optionsBmp( options_width, options_height, options_bits, TRUE );

    playPB->setPixmap( playBmp );
    stopPB->setPixmap( stopBmp );
    prevPB->setPixmap( prevBmp );
    nextPB->setPixmap( nextBmp );
    replayPB->setPixmap( replayBmp );
    fwdPB->setPixmap( fwdBmp );
    bwdPB->setPixmap( bwdBmp );
    ejectPB->setPixmap( ejectBmp );
    infoPB->setPixmap( infoBmp );
    dockPB->setPixmap( dockBmp );

#if KSCDMAGIC
    QBitmap magicBmp( magicxbm_width, magicxbm_height, magicxbm_bits, TRUE );
    magicPB->setPixmap( magicBmp );
#endif

    aboutPB->setPixmap( aboutBmp );
    shufflebutton->setPixmap( shuffleBmp );
    cddbbutton->setPixmap( databaseBmp );
    optionsbutton->setPixmap( optionsBmp );

} // loadBitmaps


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

    mainPopup->insertItem (i18n("Purchases"), purchPopup);
    connect( purchPopup, SIGNAL(activated(int)), SLOT(purchases(int)) );

    mainPopup->insertItem (i18n("Information"), infoPopup);

#if KSCDMAGIC
    mainPopup->insertSeparator(-1);
    mainPopup->insertItem (i18n("KSCD Magic"));
    connect( mainPopup, SIGNAL(activated(int)), SLOT(magicslot(int)) );
#endif

    connect( infoPopup, SIGNAL(activated(int)), SLOT(information(int)) );

    KHelpMenu* helpMenu = new KHelpMenu(this, KGlobal::instance()->aboutData(), false);
    mainPopup->insertItem(i18n("Help"), helpMenu->menu());
} // setupPopups


void
KSCD::setToolTips()
{
    QToolTip::add(playPB,          i18n("Play/Pause"));
    QToolTip::add(stopPB,          i18n("Stop"));
    QToolTip::add(replayPB,        i18n("Loop"));
    QToolTip::add(songListCB,      i18n("Track selection"));

    // if you change these, change them in Config Done as well!
    QToolTip::add(fwdPB,           i18n("%1 secs forward").arg(skipDelta));
    QToolTip::add(bwdPB,           i18n("%1 secs backward").arg(skipDelta));
    QToolTip::add(nextPB,          i18n("Next track"));
    QToolTip::add(prevPB,          i18n("Previous track"));
    QToolTip::add(dockPB,          i18n("Quit CD player"));
#if KSCDMAGIC
    QToolTip::add(magicPB,         i18n("Run Kscd Magic"));
#endif
    QToolTip::add(aboutPB,         i18n("Cycle time display"));
    QToolTip::add(optionsbutton,   i18n("Configure CD player"));
    QToolTip::add(ejectPB,         i18n("Eject CD"));
    QToolTip::add(infoPB,          i18n("The artist on the Web"));
    QToolTip::add(cddbbutton,      i18n("freedb dialog"));
    QToolTip::add(volSB,           i18n("CD volume control"));
    QToolTip::add(shufflebutton,   i18n("Shuffle play"));
} // setToolTips

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

    qApp->processEvents();
    qApp->flushX();



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
        setLEDs( "00:00" );
        populateSongList();

        qApp->processEvents();
        qApp->flushX();

        if(!playlist.isEmpty())
        {
            if(playlistpointer >=(int) playlist.count())
                playlistpointer = 0;

            if (playlist.at(playlistpointer) != playlist.end())
            {
                save_track = track = atoi((*playlist.at(playlistpointer)).ascii());
                wm_cd_play (track, 0, track + 1);
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
    } else { // if (WM_CDM_STOPPED||UNKNOWN)
        if (cur_cdmode == WM_CDM_PLAYING || cur_cdmode == WM_CDM_PAUSED)
        {

            switch (cur_cdmode)
            {
                case WM_CDM_PLAYING:
                    statuslabel->setText( i18n("Pause") );
                    wm_cd_pause();
                    break;
                case WM_CDM_PAUSED:
                    if(randomplay)
                    {
                        statuslabel->setText( i18n("Shuffle") );
                    } else {
                        statuslabel->setText( i18n("Playing") );
                    }
                    wm_cd_pause();
                    break;

                default:
                    // next release: force "stop".
                    statuslabel->setText( i18n("Strange...") );
                    break;
            } // switch
            qApp->processEvents();
            qApp->flushX();
        } // if (PLAYING||PAUSED)
    } // if (WM_CDM_STOPPED||UNKNOWN) else
    cdMode();
} // playClicked()

void
KSCD::stopClicked()
{
    //    looping = FALSE;
    // TODO: figure out what it will take to not reset randomplay!
    randomplay = FALSE;
    stoppedByUser = TRUE;
    statuslabel->setText(i18n("Stopped"));
    setLEDs("--:--");
    qApp->processEvents();
    qApp->flushX();

    save_track = 1;
    playlistpointer = 0;
    wm_cd_stop();
} // stopClicked()

void
KSCD::prevClicked()
{
    int track;
    setLEDs("00:00");
    qApp->processEvents();
    qApp->flushX();

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
    qApp->processEvents();
    qApp->flushX();

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
KSCD::fwdClicked()
{
    qApp->processEvents();
    qApp->flushX();

    int cur_cdmode = wm_cd_status();
    int track = wm_cd_getcurtrack();

    if (cur_cdmode == WM_CDM_PLAYING)
    {
        tmppos = cur_pos_rel + skipDelta;
        if (tmppos < cd->trk[track - 1].length)
        {
            if(randomplay || !playlist.isEmpty())
                wm_cd_play (track, tmppos, track + 1);
            else
                wm_cd_play (track, tmppos, WM_ENDTRACK);
        }
    }
} // fwdClicked()

void
KSCD::bwdClicked()
{
    qApp->processEvents();
    qApp->flushX();

    int cur_cdmode = wm_cd_status();
    int track = wm_cd_getcurtrack();

    if (cur_cdmode == WM_CDM_PLAYING)
    {
        tmppos = cur_pos_rel - skipDelta;
        if(randomplay || !playlist.isEmpty())
            wm_cd_play (track, tmppos > 0 ? tmppos : 0, track + 1);
        else
            wm_cd_play (track, tmppos > 0 ? tmppos : 0, WM_ENDTRACK);
    }
    cdMode();
} // bwdClicked()

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
    randomplay = FALSE;
    statuslabel->clear();
    setLEDs( "--:--" );

    // Good GOD this is evil
    qApp->processEvents();
    qApp->flushX();

    if(stopexit)
        wm_cd_stop();

    wm_cd_destroy();

    qApp->quit();
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
    randomplay = FALSE;

    statuslabel->clear();

    setLEDs( "--:--" );

    qApp->processEvents();
    qApp->flushX();

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
    qApp->processEvents();
    qApp->flushX();
} // loopOn;

void
KSCD::loopOff()
{
    looping = false;
    loopled->off();
    loopled->show();
    qApp->processEvents();
    qApp->flushX();
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
    if(!currentlyejected)
    {
      randomplay = FALSE;
      statuslabel->setText(i18n("Ejecting"));
      qApp->processEvents();
      qApp->flushX();
      setArtistAndTitle("", "");
      tracktitlelist.clear();
      extlist.clear();
      category = "";

      wm_cd_stop();
      //  timer.stop();
      /*
       * new checkmount goes here
       *
       */
      wm_cd_eject();
    } else {
      statuslabel->setText(i18n("Closing"));
      qApp->processEvents();
      qApp->flushX();
      have_new_cd = true;
      wm_cd_closetray();
    }
} // ejectClicked

void
KSCD::randomSelected()
{
    randomplay = !randomplay;

    if (randomplay)
    {
        statuslabel->setText(i18n("Shuffle"));

        if(songListCB->count()==0)
            return;
        make_random_list(); /* koz: Build a unique, once, random list */
        nextClicked();
    }
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

    randomplay = false;
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
    qApp->processEvents();
    qApp->flushX();

    //  pause_cd();
    wm_cd_play( trk + 1, 0, WM_ENDTRACK );
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

    // update the tooltips
    QToolTip::remove(fwdPB);
    QToolTip::remove(bwdPB);
    QToolTip::add(fwdPB, i18n("%1 secs forward").arg(skipDelta));
    QToolTip::add(bwdPB, i18n("%1 secs backward").arg(skipDelta));
}

void
KSCD::getCDDBOptions(CDDBSetup* /*config*/)  //!!!!
{
/*    if (!config)
    {
        return;
    }

    config->insertData(cddbserverlist,
                       cddbsubmitlist,
                       cddbbasedir,
                       submitaddress,
                       current_server,
                       cddb_auto_enabled,
                       cddb_remote_enabled,
                       cddb->getTimeout(),
                       cddb->useHTTPProxy(),
                       cddb->getHTTPProxyHost(),
                       cddb->getHTTPProxyPort());*/
} // getCDDBOptions(CDDBSetup* config)


void
KSCD::setCDDBOptions(CDDBSetup* config)  //!!!!
{
    if (!config)
    {
        return;
    }

/*    bool cddb_proxy_enabled;
    QString cddb_proxy_host;
    unsigned short int cddb_proxy_port;
    unsigned short int cddb_timeout;

    config->getData(cddbserverlist,
                   cddbsubmitlist,
                   cddbbasedir,
                   submitaddress,
                   current_server,
                   cddb_auto_enabled,
                   cddb_remote_enabled,
                   cddb_timeout,
                   cddb_proxy_enabled,
                   cddb_proxy_host,
                   cddb_proxy_port);
    cddb->setTimeout(cddb_timeout);
    cddb->setHTTPProxy(cddb_proxy_host,cddb_proxy_port);
    cddb->useHTTPProxy(cddb_proxy_enabled);*/
} // setCDDBOptions

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
    printf("setDevicePatches return %i\n", ret);

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
KSCD::updateCurrentCDDBServer(const QString& newCurrentServer)
{
    current_server = newCurrentServer;
} // updateCurrentCDDBServer

void
KSCD::incVolume()
{
    volSB->addStep();
} // incVolume

void
KSCD::decVolume()
{
    volSB->subtractStep();
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

/* Alex Kern:
   let me explain, i try to avoid problem, that first track in random
   sequence will bee never played */
#define INVALID_RANDOM_ITERATOR random_list.end()
#define NEXT_VALID_ITERATOR(x) ((x)++)
#define PREV_VALID_ITERATOR(x) ((x)--)
#define FIRST_VALID_ITERATOR random_list.begin()
#define LAST_VALID_ITERATOR  random_list.fromLast()
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
#if 0
    for(random_current = random_list.begin();
        random_current != random_list.end();
        random_current++)
        fprintf(stderr, "random_list %i\n", *random_current);
#endif
    random_current = INVALID_RANDOM_ITERATOR;
} // make_random_list()

int
KSCD::next_randomtrack()
{
    /* Check to see if we are at invalid state */
    if(random_current == INVALID_RANDOM_ITERATOR) {
        random_current = FIRST_VALID_ITERATOR;
    } else if(random_current == LAST_VALID_ITERATOR) {
        if(!looping) {
                stopClicked();
                return -1;
        } else {
                // playing the same random list isn't very random, is it?
                make_random_list();
                return next_randomtrack();
        }
    } else {
        NEXT_VALID_ITERATOR(random_current);
    }

    return *random_current;
} // next_randomtrack

int
KSCD::prev_randomtrack()
{
    /* Check to see if we are at invalid state */
    if(random_current == INVALID_RANDOM_ITERATOR) {
        random_current = LAST_VALID_ITERATOR;
    } else if(random_current == FIRST_VALID_ITERATOR) {
        if(!looping) {
            stopClicked();
            return -1;
        } else {
            // playing the same random list isn't very random, is it?
            make_random_list();
            return prev_randomtrack();
        }
    } else {
        PREV_VALID_ITERATOR(random_current);
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
    static bool damn = TRUE;
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

    // FIXME: does this need to be set EVERY time? ok, so it's only one bool
    currentlyejected = (cur_cdmode == WM_CDM_EJECTED);

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
            damn = TRUE;
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
                damn = TRUE;
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

            damn = TRUE;
            stoppedByUser = false;
            break;

        case WM_CDM_FORWARD:
            break;

        case WM_CDM_PAUSED:
            statuslabel->setText( i18n("Pause") );
            damn = TRUE;
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
            damn = FALSE;
            if(have_new_cd){

                //      timer.stop();
                save_track = 1;
                have_new_cd = false;
                // timer must be restarted when we are doen
                // with getting the cddb info
                get_cddb_info(false); // false == do not update dialog if open
                if(autoplay && currentlyejected)
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
            damn = TRUE;
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
            damn = TRUE;
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
        // TODO: this breaks if randomplay comes up true to begin with!
//	randomplay 	= config->readBoolEntry("RandomPlay", false);
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

#if KSCDMAGIC
	config->setGroup("MAGIC");
	magic_width      = config->readNumEntry("magicwidth",320);
	magic_height     = config->readNumEntry("magicheight",200);
	magic_brightness = config->readNumEntry("magicbrightness", 3);
	magic_pointsAreDiamonds = config->readBoolEntry("magicPointsAreDiamonds", false);
#endif

	config->setGroup("SMTP");
	smtpConfigData->enabled = config->readBoolEntry("enabled", true);
	smtpConfigData->useGlobalSettings = config->readBoolEntry("useGlobalSettings", true);
	smtpConfigData->serverHost = config->readEntry("serverHost");
	smtpConfigData->serverPort = config->readEntry("serverPort", "25");
	smtpConfigData->senderAddress = config->readEntry("senderAddress");
	smtpConfigData->senderReplyTo = config->readEntry("senderReplyTo");

	// serverHost used to be stored via KEMailSettings, so we attempt to read the
   // value via KEMailSettings to preserve the user's settings when upgrading.
   if( !config->readEntry("mailProfile").isNull() )
	{
		KEMailSettings kes;
		kes.setProfile( i18n("Default") );
		smtpConfigData->serverHost = kes.getSetting( KEMailSettings::OutServer );
	}

	if(smtpConfigData->useGlobalSettings)
		smtpConfigData->loadGlobalSettings();

	// Don't accept obviously bogus settings.
	if(!smtpConfigData->isValid())
		smtpConfigData->enabled = false;

/*	config->setGroup("CDDB");

	cddb->setTimeout(config->readNumEntry("CDDBTimeout",60));
	cddb_auto_enabled = config->readBoolEntry("CDDBLocalAutoSaveEnabled",true);
	cddbbasedir = config->readPathEntry("LocalBaseDir");

	// Changed global KDE apps dir by local KDE apps dir
	if (cddbbasedir.isEmpty())
		cddbbasedir = KGlobal::dirs()->resourceDirs("cddb").first();
	KGlobal::dirs()->addResourceDir("cddb", cddbbasedir);

	// Set this to false by default. Look at the settings dialog source code
	// for the reason. - Juraj.
	cddb_remote_enabled = config->readBoolEntry( "CDDBRemoteEnabled", false );
#if QT_VERSION == 0x030102 // ### REMOVEME
    cddb_remote_enabled = false;
#endif
	cddb->useHTTPProxy( config->readBoolEntry("CDDBHTTPProxyEnabled", KProtocolManager::useProxy()) );
	KURL proxyURL;
	QString proxyHost;
	int proxyPort;
	QString proxy = KProtocolManager::proxyFor("http");
	if( !proxy.isEmpty() )
	{
		proxyURL = proxy;
		proxyHost = proxyURL.host();
		proxyPort = proxyURL.port();
	}
	else
	{
		proxyHost = "";
		proxyPort = 0;
		cddb->useHTTPProxy(false);
	}
	cddb->setHTTPProxy(config->readEntry("HTTPProxyHost",proxyHost),
			config->readNumEntry("HTTPProxyPort",proxyPort));

	current_server = config->readEntry("CurrentServer",DEFAULT_CDDB_SERVER);
	//Let's check if it is in old format and if so, convert it to new one:
	if(CDDB::normalize_server_list_entry(current_server))
	{
		kdDebug() << "Default freedb server entry converted to new format and saved.\n" << endl;
		config->writeEntry("CurrentServer",current_server);
		config->sync();
	}
	submitaddress = config->readEntry("CDDBSubmitAddress","freedb-submit@freedb.org");
	cddbserverlist = config->readListEntry("SeverList", ',');
	int num = cddbserverlist.count();
	if (num == 0)
		cddbserverlist.append(DEFAULT_CDDB_SERVER);
	else
	{
		//Let's check if it is in old format and if so, convert it to new one:
      bool needtosave=false;
		QStringList nlist;

		for ( QStringList::Iterator it = cddbserverlist.begin(); it != cddbserverlist.end(); ++it )
		{
			needtosave|=CDDB::normalize_server_list_entry(*it);
			nlist.append(*it);
		}
		if(needtosave)
		{
			// I have to recreate list because of sytange behaviour of sync()
			// function in configuration - it does not notice if QStringList
         // String contents is changed.
         cddbserverlist=nlist;
			kdDebug() << "freedb server list converted to new format and saved.\n" << endl;
			config->writeEntry("SeverList",cddbserverlist,',',true);
			config->sync();
		}
	}
	cddbsubmitlist = config->readListEntry("SubmitList", ',');
	num = cddbsubmitlist.count();
	if(!num){
		cddbsubmitlist.append(DEFAULT_SUBMIT_EMAIL);
		cddbsubmitlist.append(DEFAULT_TEST_EMAIL);
	}*/
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

    config->setGroup("SMTP");
    config->writeEntry("enabled", smtpConfigData->enabled);
    config->writeEntry("useGlobalSettings", smtpConfigData->useGlobalSettings);
    config->writeEntry("serverHost", smtpConfigData->serverHost);
    config->writeEntry("serverPort", smtpConfigData->serverPort);
    if(smtpConfigData->useGlobalSettings)
    {
        config->writeEntry("senderAddress", "");
        config->writeEntry("senderReplyTo", "");
    }
    else
    {
        config->writeEntry("senderAddress", smtpConfigData->senderAddress);
        config->writeEntry("senderReplyTo", smtpConfigData->senderReplyTo);
    }
    // delete legacy mailProfile option
    config->deleteEntry("mailProfile");

/*    config->setGroup("CDDB");
    config->writeEntry("CDDBRemoteEnabled",cddb_remote_enabled);
    config->writeEntry("CDDBTimeout", static_cast<int>(cddb->getTimeout()));
    config->writeEntry("CDDBLocalAutoSaveEnabled",cddb_auto_enabled);

    config->writePathEntry("LocalBaseDir",cddbbasedir);
    config->writeEntry("SeverList",cddbserverlist);
    config->writeEntry("SubmitList", cddbsubmitlist);
    config->writeEntry("CDDBSubmitAddress",submitaddress);
    config->writeEntry("CurrentServer",current_server);
    config->writeEntry("CDDBHTTPProxyEnabled",cddb->useHTTPProxy());
    config->writeEntry("HTTPProxyHost",cddb->getHTTPProxyHost());
    config->writeEntry("HTTPProxyPort",(int)cddb->getHTTPProxyPort());*/

#if KSCDMAGIC
    config->setGroup("MAGIC");
    config->writeEntry("magicwidth",magic_width);
    config->writeEntry("magicheight",magic_height);
    config->writeEntry("magicbrightness",magic_brightness);
    config->writeEntry("magicPointsAreDiamonds",magic_pointsAreDiamonds);
#endif

    config->sync();
} // writeSettings()

void
KSCD::CDDialogSelected()
{
    if(cddialog)
        return;

    cddialog = new CDDialog();

    cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
                      revision,playlist,pathlist,cddbbasedir,submitaddress, smtpConfigData);

    connect(cddialog,SIGNAL(cddb_query_signal(bool)),this,SLOT(get_cddb_info(bool)));
    connect(cddialog,SIGNAL(dialog_done()),this,SLOT(CDDialogDone()));
    connect(cddialog,SIGNAL(play_signal(int)),this,SLOT(trackSelected(int)));

    cddialog->show();
}

void
KSCD::CDDialogDone()
{
  delete cddialog;
  cddialog = 0L;
}


void
KSCD::getCDDBservers()
{

    led_on();
/*
     *
     * minorly fugly hack, but i couldn't think of a nice clean way to
     * remove this last bit of stupidity. i got rid of the rest of the
     * rediculousness, though =)  - aseigo
     *

    if (configDialog)
    {
        // Get current server and proxy settings from CDDB setup dialog befoe proceed
        bool    cddb_proxy_enabled;
        QString cddb_proxy_host;
        unsigned short int cddb_proxy_port;
        unsigned short int cddb_timeout;


        configDialog->cddb()->getData(cddbserverlist,
                                      cddbsubmitlist,
                                      cddbbasedir,
                                      submitaddress,
                                      current_server,
                                      cddb_auto_enabled,
                                      cddb_remote_enabled,
                                      cddb_timeout,
                                      cddb_proxy_enabled,
                                      cddb_proxy_host,
                                      cddb_proxy_port);

        cddb->setTimeout(cddb_timeout);
        cddb->setHTTPProxy(cddb_proxy_host,cddb_proxy_port);
        cddb->useHTTPProxy(cddb_proxy_enabled);
    }

    connect(&cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
    connect(&cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));

    // For now, just don't update if there's no current server.
    if(!current_server.isEmpty())
        cddb->cddbgetServerList(current_server);*/
} // getCDDBservers()

void
KSCD::getCDDBserversFailed()
{
    led_off();
/*    disconnect(&cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
    disconnect(&cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));
    setArtistAndTitle(i18n("Unable to get freedb server list."), "");
    titlelabeltimer.start(10000,TRUE); // 10 secs*/
}

void
KSCD::getCDDBserversDone()
{
    led_off();
    /*disconnect(&cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
    disconnect(&cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));
    cddb->serverList(cddbserverlist);
    emit newServerList(cddbserverlist);*/
}

#ifdef NEEDOWNCDDBDISCID
int
cddb_sum(int n)
{
    char    buf[12];
    char    *p;
    int     ret = 0;
    long unsigned int ntemp = 0;
    ntemp = (long unsigned int) n;

    /* For backward compatibility this algorithm must not change */
    sprintf(buf, "%lu", ntemp);
    for (p = buf; *p != '\0'; p++)
        ret += (*p - '0');

    return (ret);
}

unsigned long
cddb_discid()
{
    int     i;
    int     t = 0;
    int     n = 0;
    int     min, sec;


    /* For backward compatibility this algorithm must not change */
    for (i = 0; i < cd->ntracks; i++)
    {
        n += cddb_sum(cd->trk[i].start / 75);
    }

    t = ((cd->trk[cd->ntracks].start / 75) -
         (cd->trk[0].start / 75));

    return ((n % 0xff) << 24 | t << 8 | cd->ntracks);
}

#endif

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
    // CDDBTODO: handle multiple records returned
    KCDDB::CDInfoList cddbInfoList = cddb->lookupResponse();

    // shouldn't need to clear it, but i fear the maintainability monster
    // so let's just clear it to be safe, m'kay
    tracktitlelist.clear();

    if (!cddbInfoList.isEmpty())
    {
        KCDDB::CDInfo cddbInfo(cddbInfoList.first());
        setArtistAndTitle(cddbInfo.artist, cddbInfo.title);

        // CDDBTODO: we really should get the artist off the 'tracktitlelist'
        tracktitlelist << cddbInfo.artist;

        KCDDB::TrackInfoList::ConstIterator it(cddbInfo.trackInfoList.begin());
        KCDDB::TrackInfoList::ConstIterator end(cddbInfo.trackInfoList.end());
        for (; it != end; ++it)
        {
            tracktitlelist << (*it).title;
        }
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
       discidlist.clear();
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

      discidlist.clear();
      populateSongList();

      setArtistAndTitle(i18n("Error getting freedb entry."), "");
    }
    timer.start(1000);
    led_off();
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

    emit trackChanged(tooltip);
}

void
KSCD::playtime()
{
    static int mymin;
    static int mysec;
    int tmp = 0;

    switch(time_display_mode){

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

            if (cur_pos_rel > 0 && (tmp = cur_pos_rel % 60) == mysec)
                return;
            mysec = tmp;
            mymin = cur_pos_rel / 60;

            break;
    }


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

    cycletimer.start(3000,TRUE);
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
KSCD::get_pathlist(QStringList& _pathlist)
{
    QDir d;
    QStringList list;

    d.setFilter( QDir::Dirs);
    d.setSorting( QDir::Size);
    d.setPath( cddbbasedir );
    if( !d.exists() )
    {
		 if ( ! KStandardDirs::makeDir( cddbbasedir ) )
		 {
			 QString msg = i18n("Unable to create directory %1"
                                "\nCheck permissions!" ).arg(cddbbasedir);
			 KMessageBox::error( this, msg );
			 return;
		 }

	 }

	 _pathlist.clear();
	 list = d.entryList();

	 for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
	 {
		 if( *it != QString::fromLocal8Bit(".") && *it != QString::fromLocal8Bit("..") )
		 {
			 _pathlist.append( cddbbasedir + '/' +  *it);
		 }
	 }
} // get_pathlist

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
    int track = wm_cd_getcurtrack();
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

void KSCD::emailSettingsChanged()
{
    if(smtpConfigData->useGlobalSettings)
        smtpConfigData->loadGlobalSettings();

    if(configDialog)
        configDialog->updateGlobalSettings();
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

    KAboutData aboutData( "kscd", I18N_NOOP("kscd"),
                          KSCDVERSION, description,
                          KAboutData::License_GPL,
                          "(c) 2001, Dirk FÃ¶rsterling");
    aboutData.addAuthor("Aaron J. Seigo", I18N_NOOP("Current maintainer"), "aseigo@olympusproject.org");
    aboutData.addAuthor("Bernd Johannes Wuebben",0, "wuebben@kde.org");
    aboutData.addAuthor("Dirk FÃ¶rsterling", I18N_NOOP("Workman library, previous maintainer"), "milliByte@gmx.net");
    aboutData.addCredit("Steven Grimm", I18N_NOOP("Workman library"));
    aboutData.addCredit("Vadim Zaliva", I18N_NOOP("HTTP proxy code"));
    aboutData.addCredit("Paul Harrison", I18N_NOOP("KSCD Magic based on Synaesthesia"), "pfh@yoyo.cc.monash.edu.au");
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

    KGlobal::dirs()->addResourceType("cddb",
                                     KStandardDirs::kde_default("data") +
                                     "kscd/cddb/");

    KSCD *k = new KSCD();

    a.setTopWidget( k );
    a.setMainWidget( k );

    k->setCaption(a.caption());
    k->initialShow();

    return a.exec();
} // main()


#if KSCDMAGIC
void
KSCD::getMagicOptions(mgconfigstruct& config)
{
    config.width = magic_width;
    config.height = magic_height;
    config.brightness = magic_brightness;
    config.pointsAreDiamonds = magic_pointsAreDiamonds;
} // getMagicOptions(mgconfigstruct& config)


void
KSCD::setMagicOptions(const mgconfigstruct& config)
{
    magic_width = config.width;
    magic_height = config.height;
    magic_brightness  =config.brightness;
    magic_pointsAreDiamonds = config.pointsAreDiamonds;
} // setMagicOptions(mgconfigstruct& config)

void
KSCD::magicslot()
{
    magicslot(0);
}

void
KSCD::magicslot( int )
{
    if(magicproc && magicproc->isRunning())
    {
        return;
    }

    magicproc = new KProcess;

    QString b;
    b.setNum(magic_brightness);
    QString w;
    w.setNum(magic_width);
    QString h;
    h.setNum(magic_height);

    *magicproc << "kscdmagic" << " -b" << b << " -w"<< w << " -h" << h;

    connect(magicproc,
            SIGNAL(processExited(KProcess *)),this, SLOT(magicdone(KProcess*)));


    bool result = magicproc->start(KProcess::NotifyOnExit , KProcess::Stdin);

    if(!result)
        KMessageBox::error(this, i18n("Cannot start kscdmagic."));
    return;
} // magicslot

void
KSCD::magicdone(KProcess* proc)
{
    if(proc->normalExit())
    {
        if(proc->exitStatus()!=0)
            KMessageBox::error(this, i18n("KSCD Magic exited abnormally.\n"
                                          "Are you sure kscdmagic is installed?"));
    }

    delete proc;
    magicproc = 0L;
} // magicdone
#else
void KSCD::getMagicOptions(mgconfigstruct&) {}
void KSCD::setMagicOptions(const mgconfigstruct&) {}
void KSCD::magicdone(KProcess*) {}
void KSCD::magicslot() {}
void KSCD::magicslot(int) {}
#endif

#include "kscd.moc"
