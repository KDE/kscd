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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qlayout.h>

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
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
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

extern "C" {
    // We don't have libWorkMan installed already, so get everything
    // from within our own directory
#include "libwm/include/workman.h"
#include "libwm/include/wm_config.h"
#include "libwm/include/wm_cdinfo.h"
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
#include "bitmaps/magic.xbm"
#include "bitmaps/eject.xbm"
#include "bitmaps/db.xbm"
#include "bitmaps/logo.xbm"
#include "bitmaps/shuffle.xbm"
#include "bitmaps/options.xbm"

static const char *description = I18N_NOOP("KDE CD player");

DockWidget*     dock_widget;
SMTP                *smtpMailer;
bool stoppedByUser = true;
bool device_change = true;

char            tmptime[100];
char            *tottime;
//static void   playtime (void);
void            kcderror(const QString& title, const QString& message);
void            kcdworker(int );

//void          parseargs(char* buf, char** args);
extern QTime framestoTime(int frames);
extern void cddb_decode(QString& str);
extern void cddb_encode(QString& str, QStringList &returnlist);
extern void cddb_playlist_encode(QStringList& list,QString& playstr);
extern bool cddb_playlist_decode(QStringList& list, QString& str);

static QString formatTrack(int d1, int d2)
{
  QString str = QString::fromLatin1("%1/%2")
    .arg( QString::number(d1).rightJustify(2, '0') )
    .arg( QString::number(d2).rightJustify(2, '0') );
  return str;
}

int cddb_error = 0;

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
  :   QWidget( parent, name, Qt::WDestructiveClose ), DCOPObject("CDPlayer")
{
  magicproc           = 0L;
  cd_device_str       = "";
  background_color    = black;
  led_color           = green;
  randomplay          = false;
  looping             = false;
  cddrive_is_ok       = true;
  tooltips            = true;
  magic_width         = 330;
  magic_height        = 135;
  magic_brightness    = 3;
  magic_pointsAreDiamonds = false;

  cycle_flag          = false;
  cddb_remote_enabled = false;
  cddb_auto_enabled   = false;
  time_display_mode   = TRACK_SEC;
  cddb_inexact_sentinel = false;
  revision            = 0; // The first freedb revision is "0"
  docking             = true;
  autoplay            = false;
  stopexit            = true;
  ejectonfinish       = false;
  randomonce          = true;
  updateDialog        = false;
  ejectedBefore       = false;
  currentlyejected    = false;
  cddialog            = 0L;
  configDialog        = 0L;
  jumpToTrack         = 0L;

  smtpConfigData = new SMTPConfigData;

  have_new_cd = true;

  readSettings();
  initFont();
  drawPanel();
  loadBitmaps();
  setColors();
  tooltips = !tooltips;
  setToolTips(!tooltips);
  initWorkMan();
  setupPopups();

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

  // connect up the cddb stuff
  connect(&cddb,SIGNAL(cddb_ready()),this,SLOT(cddb_ready()));
  connect(&cddb,SIGNAL(cddb_failed()),this,SLOT(cddb_failed()));
  connect(&cddb,SIGNAL(cddb_done()),this,SLOT(cddb_done()));
  connect(&cddb,SIGNAL(cddb_timed_out()),this,SLOT(cddb_timed_out()));
  connect(&cddb,SIGNAL(cddb_inexact_read()),this,SLOT(mycddb_inexact_read()));
  connect(&cddb,SIGNAL(cddb_no_info()),this,SLOT(cddb_no_info()));
  
  // set up the actions and keyboard accels
  KAccel* accels = new KAccel(this);
  KActionCollection* actions = new KActionCollection(this);

  KAction* action;
  action = new KAction(i18n("Play/Pause"), Key_P, this, SLOT(playClicked()), this, "Play/Pause");
  action->plugAccel(accels);
  action = new KAction(i18n("Stop"), Key_S, this, SLOT(stopClicked()), this, "Stop");
  action->plugAccel(accels);
  action = new KAction(i18n("Previous"), Key_B, this, SLOT(prevClicked()), this, "Previous");
  action->plugAccel(accels);
  action = new KAction(i18n("Next"), Key_N, this, SLOT(nextClicked()), this, "Next");
  action->plugAccel(accels);
  action = new KAction(i18n("Forward"), Key_Right, this, SLOT(fwdClicked()), this, "Forward");
  action->plugAccel(accels);
  action = new KAction(i18n("Backward"), Key_Left, this, SLOT(bwdClicked()), this, "Backward");
  action->plugAccel(accels);
  action = KStdAction::action(KStdAction::Quit, this, SLOT(quitClicked()), actions);
  action->plugAccel(accels);
  action = new KAction(i18n("Loop"), Key_L, this, SLOT(loopClicked()), this, "Loop");
  action->plugAccel(accels);
  action = new KAction(i18n("Eject"), CTRL + Key_E, this, SLOT(ejectClicked()), this, "Eject");
  action->plugAccel(accels);
  action = new KAction(i18n("Increase Volume"), Key_Plus, this, SLOT(incVolume()), this, "IncVolume");
  action->plugAccel(accels);
  action = new KAction(i18n("Increase Volume"), Key_Equal, this, SLOT(incVolume()), this, "IncVolume Alt");
  action->plugAccel(accels);
  action = new KAction(i18n("Decrease Volume"), Key_Minus, this, SLOT(decVolume()), this, "DecVolume");
  action->plugAccel(accels);
  action = new KAction(i18n("Options"), CTRL + Key_T, this, SLOT(showConfig), this);
  action->plugAccel(accels);
  action = new KAction(i18n("Shuffle"), Key_R, this, SLOT(randomSelected()), this, "Shuffle");
  action->plugAccel(accels);
  action = new KAction(i18n("CDDB"), CTRL + Key_D, this, SLOT(CDDialogSelected()), this, "CDDB");
  action->plugAccel(accels);

  volstartup = TRUE;
  volSB->setValue(volume);

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

  QTimer::singleShot(500, this, SLOT(initCDROM()));
} // KSCD


KSCD::~KSCD()
{
    if (thiscd.trk)
    {
        free(thiscd.trk);
        thiscd.trk = 0L;
        thiscd.ntracks = 0;
    }

    signal (SIGINT, SIG_DFL);
    delete magicproc;
    magicproc = 0L;

    delete smtpConfigData;
    delete smtpMailer;
} // ~KSCD


void
KSCD::initialShow()
{
  if(!(docking && hidden_controls))
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
  fastin       = FALSE;
  scmd         = 0;
  tmppos       = 0;
  save_track   = 1;
  thiscd.trk   = NULL;
  thiscd.lists = NULL;
  thiscd.ntracks = 0;
  tottime      = tmptime;
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
  volstartup = FALSE;
  if(cddrive_is_ok)
    volChanged(volume);
  
  if (autoplay)
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
  QPushButton *pb = new QPushButton( n, this );
  pb->setGeometry( x, y, w, h );
  pb->setFocusPolicy ( QWidget::NoFocus );
  return pb;
} // makeButton

/**
 * drawPanel() constructs KSCD's main window.
 * This is oldstyle and needs to be redesigned.
 *
 * @author Bernd Wübben
 * @author Dirk Försterling
 */
void
KSCD::drawPanel()
{
  int ix = 0;
  int iy = 0;
  const int WIDTH = 90;
  const int HEIGHT = 27;
  const int SBARWIDTH = 220;

  aboutPB = makeButton( ix, iy, WIDTH, 2 * HEIGHT, i18n("About") );

  ix = 0;
  iy += 2 * HEIGHT;

  infoPB = makeButton( ix, iy, WIDTH/2, HEIGHT, "" );
  ejectPB = makeButton( ix + WIDTH/2, iy, WIDTH/2, HEIGHT, "" );

  iy += HEIGHT;
#if KSCDMAGIC
  dockPB = makeButton( ix, iy, WIDTH/2, HEIGHT, i18n("Quit") );
  magicPB = makeButton(ix+WIDTH/2, iy, WIDTH/2, HEIGHT, "" );
#else
  dockPB = makeButton( ix, iy, WIDTH, HEIGHT, i18n("Quit") );
#endif
  ix += WIDTH;
  iy = 0;

  backdrop = new QFrame(this);
  backdrop->setGeometry(ix,iy,SBARWIDTH -2, 2* HEIGHT + HEIGHT /2 -1);
  backdrop->setFocusPolicy ( QWidget::NoFocus );

  int D = 6;

  ix = WIDTH + 8;
  for (int u = 0; u<5;u++)
    {
      trackTimeLED[u] = new BW_LED_Number(this );
      trackTimeLED[u]->setGeometry( ix  + u*18, iy + D, 23 ,  30 );
      trackTimeLED[u]->setLEDoffColor(background_color);
      trackTimeLED[u]->setLEDColor(led_color,background_color);
    }

  QString zeros("--:--");
  setLEDs(zeros);

  artistlabel = new QLabel(this);
  artistlabel->setGeometry(WIDTH + 5, iy + 38 , SBARWIDTH -15, 13);
  artistlabel->setFont( smallfont );
  artistlabel->setAlignment( AlignLeft );
  artistlabel->clear();

  titlelabel = new QLabel(this);
  titlelabel->setGeometry(WIDTH + 5, iy + 50 , SBARWIDTH -15, 13);
  titlelabel->setFont( verysmallfont );
  titlelabel->setAlignment( AlignLeft );
  titlelabel->clear();

  statuslabel = new QLabel(this);
  statuslabel->setGeometry(WIDTH + 110, iy  +D, 50, 14);
  statuslabel->setFont( verysmallfont );
  statuslabel->setAlignment( AlignLeft );

  queryled = new LedLamp(this);
  queryled->move(WIDTH + 200, iy  +D +1 );
  queryled->off();
  queryled->hide();

  loopled = new LedLamp(this, LedLamp::Loop);
  loopled->move(WIDTH + 200, iy  +D +18 );
  loopled->off();

  volumelabel = new QLabel(this);
  volumelabel->setGeometry(WIDTH + 110, iy + 14 + D, 50, 14);
  volumelabel->setFont( smallfont );
  volumelabel->setAlignment( AlignLeft );
  volumelabel->setText(i18n("Vol: --"));

  tracklabel = new QLabel(this);
  tracklabel->setGeometry(WIDTH + 168, iy + 14 +D, 30, 14);
  tracklabel->setFont( smallfont );
  tracklabel->setAlignment( AlignLeft );
  tracklabel->setText("--/--");

  totaltimelabel = new QLabel(this);
  totaltimelabel->setGeometry(WIDTH + 168, iy  +D, 50, 14);
  totaltimelabel->setFont( smallfont );
  totaltimelabel->setAlignment( AlignLeft );
  totaltimelabel->hide();

  ix = WIDTH;
  iy = HEIGHT + HEIGHT + HEIGHT/2;

  volSB = new KSCDSlider( this, "Slider" );
  volSB->setGeometry( ix, iy, SBARWIDTH, HEIGHT/2 );
  volSB->setFocusPolicy ( QWidget::NoFocus );

  iy += HEIGHT/2  +1 ;
  cddbbutton = new QPushButton( this );
  cddbbutton->setGeometry( ix , iy, SBARWIDTH/10 *2 , HEIGHT );
  cddbbutton->setFocusPolicy ( QWidget::NoFocus );

  ix += SBARWIDTH/10*2;
  shufflebutton = new QPushButton( this );
  shufflebutton->setGeometry( ix , iy, SBARWIDTH/10 *2  , HEIGHT );
  shufflebutton->setFocusPolicy ( QWidget::NoFocus );

  ix += SBARWIDTH/10*2;

  optionsbutton = new QPushButton( this );
  optionsbutton->setGeometry( ix, iy, SBARWIDTH/10 *2  , HEIGHT );
  optionsbutton->setFocusPolicy ( QWidget::NoFocus );

  ix = 0;
  iy += HEIGHT;
  songListCB = new QComboBox( this );
  songListCB->setGeometry( ix, iy, SBARWIDTH/10*18+6, HEIGHT );
  songListCB->setFocusPolicy ( QWidget::NoFocus );

  iy = 0;
  ix = WIDTH + SBARWIDTH + 2;
  playPB = makeButton( ix, iy, WIDTH, HEIGHT*2, "Play/Pause" );

  iy += HEIGHT + HEIGHT;
  stopPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Stop" );

  ix += WIDTH / 2;
  replayPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Replay" );

  ix = WIDTH + SBARWIDTH/10*6;
  iy += HEIGHT;
  bwdPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Bwd" );

  ix += WIDTH / 2;
  fwdPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Fwd" );

  ix = WIDTH + SBARWIDTH + 2;
  prevPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Prev" );

  ix += WIDTH / 2;
  nextPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Next" );

  this->adjustSize();
  this->setFixedSize(this->width(),this->height());

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
#if KSCDMAGIC
    QBitmap magicBmp( magicxbm_width, magicxbm_height, magicxbm_bits, TRUE );
#endif
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

    infoPopup->insertItem("Ultimate Bandlist", 0);
    infoPopup->insertSeparator();
    infoPopup->insertItem("Deja News", 1);
    infoPopup->insertItem("Excite", 2);
    infoPopup->insertItem("HotBot", 3);
    infoPopup->insertItem("Info Seek", 4);
    infoPopup->insertItem("Lycos", 5);
    infoPopup->insertItem("Magellan", 6);
    infoPopup->insertItem("Yahoo!", 7);

    mainPopup->insertItem (i18n("Purchases"), purchPopup);
    connect( purchPopup, SIGNAL(activated(int)), SLOT(purchases(int)) );

    mainPopup->insertItem (i18n("Information"), infoPopup);

#if KSCDMAGIC
    mainPopup->insertSeparator(-1);
    mainPopup->insertItem (i18n("KSCD Magic"));
    connect( mainPopup, SIGNAL(activated(int)), SLOT(magicslot(int)) );
#endif

    connect( infoPopup, SIGNAL(activated(int)), SLOT(information(int)) );
} // setupPopups

void
KSCD::setRandomOnce(bool shuffle)
{
    randomonce = shuffle;
    QToolTip::remove(shufflebutton);
    
    if (tooltips)
    {
        if (!randomonce)
        {
            QToolTip::add(shufflebutton, i18n("Random Play"));
        }
        else
        {
            QToolTip::add(shufflebutton, i18n("Shuffle Play"));
        }
    }
} // setRandomOnce()

void
KSCD::setToolTips(bool on)
{
    if (tooltips == on)
    {
        return;
    }

    tooltips = on;
    if(tooltips)
    {
        QToolTip::add(playPB,          i18n("Play/Pause"));
        QToolTip::add(stopPB,          i18n("Stop"));
        QToolTip::add(replayPB,        i18n("Loop"));
        QToolTip::add(songListCB,      i18n("Track Selection"));
        
        // if you change these, change them in Config Done as well!
        QToolTip::add(fwdPB,           i18n("%1 Secs Forward").arg(skipDelta));
        QToolTip::add(bwdPB,           i18n("%1 Secs Backward").arg(skipDelta));
        QToolTip::add(nextPB,          i18n("Next Track"));
        QToolTip::add(prevPB,          i18n("Previous Track"));
        QToolTip::add(dockPB,          i18n("Quit CD Player"));
#if KSCDMAGIC
        QToolTip::add(magicPB,         i18n("Run Kscd Magic"));
#endif
        QToolTip::add(aboutPB,         i18n("Cycle Time Display"));
        QToolTip::add(optionsbutton,   i18n("Configure CD Player"));
        QToolTip::add(ejectPB,         i18n("Eject CD"));
        QToolTip::add(infoPB,          i18n("The Artist on the Web"));
        QToolTip::add(cddbbutton,      i18n("freedb Dialog"));
        QToolTip::add(volSB,           i18n("CD Volume Control"));

        if (!randomonce)
        {
            QToolTip::add(shufflebutton,         i18n("Random Play"));
        }
        else
        {
            QToolTip::add(shufflebutton,         i18n("Shuffle Play"));
        }
    }
    else
    {
        QToolTip::remove(playPB);
        QToolTip::remove(stopPB);
        QToolTip::remove(replayPB);
        QToolTip::remove(songListCB);
        QToolTip::remove(fwdPB);
        QToolTip::remove(bwdPB);
        QToolTip::remove(nextPB);
        QToolTip::remove(prevPB);
        QToolTip::remove(dockPB);
#if KSCDMAGIC
        QToolTip::remove(magicPB);
#endif
        QToolTip::remove(aboutPB);
        QToolTip::remove(optionsbutton);
        QToolTip::remove(ejectPB);
        QToolTip::remove(infoPB);
        QToolTip::remove(cddbbutton);
        QToolTip::remove(volSB);
        QToolTip::remove(shufflebutton);
    }
} // setToolTips

void
KSCD::playClicked()
{
    if (!cddrive_is_ok || 
        !wm_cd_status())
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
                wm_cd_play (atoi((*playlist.at(playlistpointer)).ascii()), 0,
                            atoi((*playlist.at(playlistpointer)).ascii()) + 1);
                save_track = cur_track = atoi((*playlist.at(playlistpointer)).ascii());
            }
            else
            {
                wm_cd_play (save_track, 0, cur_ntracks + 1);
            }
        } 
        else 
        {
            wm_cd_play (save_track, 0, cur_ntracks + 1);
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
                        if (randomonce)
                            statuslabel->setText( i18n("Shuffle") );
                        else
                            statuslabel->setText( i18n("Random") );
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

    save_track = cur_track = 1;
    playlistpointer = 0;
    wm_cd_stop ();
} // stopClicked()

void
KSCD::prevClicked()
{
    setLEDs("00:00");
    qApp->processEvents();
    qApp->flushX();

    if(!playlist.isEmpty())
    {
        playlistpointer--;
        if(playlistpointer < 0 )
        {
            playlistpointer = playlist.count() -1;
        }
        cur_track = atoi((*playlist.at(playlistpointer)).ascii());
    } else {
        // djoham@netscape.net suggested the real-world cd-player behaviour
        // of only jumping to the beginning of the current track if playing
        // advanced more than 2 seconds. I think that's good, but maybe I'll
        // make this configurable.
        if(!(cur_pos_rel > 2))
            cur_track--;
        if (cur_track < 1)
            cur_track = cur_ntracks;
    }
    if(randomplay)
    {
        wm_cd_play (cur_track, 0, cur_track + 1);
    } else {
        wm_cd_play (cur_track, 0, cur_ntracks + 1);
    }
} // prevClicked()

void
KSCD::nextClicked()
{
    setLEDs("00:00");
    qApp->processEvents();
    qApp->flushX();

    if(randomplay)
    {
        int j = randomtrack();
        if ( j < 0 )
            return;

        tracklabel->setText(formatTrack(j, cd->ntracks));
        if(j < (int)tracktitlelist.count())
        {
            setArtistAndTitle(tracktitlelist.first(),
                              *tracktitlelist.at(j));
        }
        qApp->processEvents();
        qApp->flushX();

        wm_cd_play( j, 0, j + 1 );
    }
    else if(!playlist.isEmpty())
    {
        if(playlistpointer < (int)playlist.count() - 1)
            playlistpointer++;
        else
            playlistpointer = 0;

        cur_track = atoi( (*playlist.at(playlistpointer)).ascii() );
        wm_cd_play(cur_track, 0, cur_track + 1);
    }
    else
    {
        if (cur_track == cur_ntracks)
            cur_track = 0;

        // TODO: determine if this should indeed be cur_track + 2?
        wm_cd_play (cur_track + 1, 0, cur_track + 2);
    }
} // nextClicked()

void
KSCD::fwdClicked()
{
    qApp->processEvents();
    qApp->flushX();

    if (cur_cdmode == WM_CDM_PLAYING)
    {
        tmppos = cur_pos_rel + skipDelta;
        if (tmppos < thiscd.trk[cur_track - 1].length)
        {
            if(randomplay || !playlist.isEmpty())
                wm_cd_play (cur_track, tmppos, cur_track + 1);
            else
                wm_cd_play (cur_track, tmppos, cur_ntracks + 1);
        }
    }
} // fwdClicked()

void
KSCD::bwdClicked()
{
    qApp->processEvents();
    qApp->flushX();

    if (cur_cdmode == WM_CDM_PLAYING)
    {
        tmppos = cur_pos_rel - skipDelta;
        if(randomplay || !playlist.isEmpty())
            wm_cd_play (cur_track, tmppos > 0 ? tmppos : 0, cur_track + 1);
        else
            wm_cd_play (cur_track, tmppos > 0 ? tmppos : 0, cur_ntracks + 1);
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

    wm_cd_status();
    // TODO: figure out why there is TWO of them?
    wm_cd_status();

    wm_free_cdtext();

    qApp->quit();
} // quitClicked()


void
KSCD::closeEvent( QCloseEvent *e )
{
    // Stop playing the CD
    if ( cur_cdmode == WM_CDM_PLAYING )
        stopClicked();

    writeSettings();
    randomplay = FALSE;

    statuslabel->clear();

    setLEDs( "--:--" );

    qApp->processEvents();
    qApp->flushX();

    if(stopexit)
        wm_cd_stop ();

    wm_cd_status();
    wm_cd_status();
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
        if( randomonce )
            statuslabel->setText(i18n("Shuffle"));
        else
            statuslabel->setText(i18n("Random"));

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

    setLEDs("00:00");
    qApp->processEvents();
    qApp->flushX();

    cur_track = trk + 1;
    //  pause_cd();
    wm_cd_play( cur_track, 0, cur_ntracks + 1 );
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
    if (tooltips)
    {
        QToolTip::remove(fwdPB);
        QToolTip::remove(bwdPB);
        QToolTip::add(fwdPB, i18n("%1 Secs Forward").arg(skipDelta));
        QToolTip::add(bwdPB, i18n("%1 Secs Backward").arg(skipDelta));
    }
}

void
KSCD::getCDDBOptions(CDDBSetup* config)
{
    if (config)
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
                       cddb.getTimeout(),
                       cddb.useHTTPProxy(),
                       cddb.getHTTPProxyHost(),
                       cddb.getHTTPProxyPort());
} // getCDDBOptions(CDDBSetup* config)


void
KSCD::setCDDBOptions(CDDBSetup* config)
{
    if (!config)
    {
        return;
    }

    bool cddb_proxy_enabled;
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
    cddb.setTimeout(cddb_timeout);
    cddb.setHTTPProxy(cddb_proxy_host,cddb_proxy_port);
    cddb.useHTTPProxy(cddb_proxy_enabled);
} // setCDDBOptions


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
KSCD::setDevicePath(QString path)
{
    if (cd_device_str == path)
    {
        return;
    }

    cddrive_is_ok = false;
    cd_device_str = path;
    cd_device = (char *)qstrdup(QFile::encodeName(cd_device_str));
    kdDebug() << "Device changed to " << cd_device << "\n";
    cur_cdmode = WM_CDM_DEVICECHANGED;
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
        //TODO: make it skip the taskbar when minimized something like:
        //KWin::setState(winId(), KWin::info(winId()).state | NET::SkipTaskbar);
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
    if(volstartup || !cddrive_is_ok)
        return;

    QString str;
    str = QString::fromUtf8( QCString().sprintf( i18n("Vol: %02d%%").utf8(),vol) );
    volumelabel->setText(str);
    cd_volume(vol, 10, 100); // 10 -> right == left balance
    volume = vol;
} // volChanged


int
KSCD::randomtrack()
{
    /* koz: 15/01/00. Check to see if we want to do a randomonce. If so */
    /* we execute the first set of statements. Else we execute the second */
    /* set, the original code.  */
    if( randomonce  )
    {
        if ( random_current == random_list.end() )
        {
            // playing the same random list isn't very random, is it?
            make_random_list();
            if( !looping )
            {
                stopClicked();
                return -1;
            } 
            else 
            {
                random_current = random_list.begin();
            }
        }

        int track = *random_current + 1;
        ++random_current;
        return track;
    } // randomonce

    if( !playlist.isEmpty() )
    {
        int j;
        j = (int) randSequence.getLong(playlist.count());
        playlistpointer = j;
        return atoi( (*playlist.at(j)).ascii() );
    } else {
        int j;
        j = (cur_ntracks == 0) ? 0 : (1 + (int) randSequence.getLong(cur_ntracks));
        return j;
    }
} // randomtrack

/*
 * cdMode
 *
 * - Data discs not recognized as data discs.
 *
 */
void
KSCD::cdMode()
{
//  static char *p = new char[10];
  static bool damn = TRUE;
  QString str;

  sss = wm_cd_status();

  if( sss == WM_CDS_JUST_INSERTED  || sss == WM_CDS_NO_DISC)
  {
      have_new_cd = true;
  }

  if(sss < 0)
  {
        if(cddrive_is_ok && (sss != WM_ERR_SCSI_INQUIRY_FAILED))
        {
            statuslabel->setText( i18n("Error") );
            cddrive_is_ok = false;
            QString errstring =
                i18n("CD-ROM read or access error (or no audio disc in drive).\n"\
                     "Please make sure you have access permissions to:\n%1")
                .arg(cd_device);
            KMessageBox::error(this, errstring, i18n("Error"));
        }
        return;
  }
  
  cddrive_is_ok = true; // cd drive ok

  if(cur_cdmode == WM_CDM_EJECTED)
    currentlyejected = true;
  else
    currentlyejected = false;


  if( device_change == true )
  {
    device_change = false;
    cur_cdmode = WM_CDM_STOPPED;
    damn = false;
  }

    switch (cur_cdmode) {
        case WM_CDM_DEVICECHANGED:
            break;
        case WM_CDM_UNKNOWN:
            cur_track = save_track = 1;
            statuslabel->setText( "" ); // TODO how should I properly handle this
            damn = TRUE;
            break;

        case WM_CDM_TRACK_DONE: // == WM_CDM_BACK
            if( randomplay )
            {
                int j = randomtrack();
                wm_cd_play( j, 0, j + 1 );

            }
            else if (playlist.count() > 0)
            {
                if(playlistpointer < (int)playlist.count() - 1)
                    playlistpointer++;
                else
                    playlistpointer = 0;
                int track = atoi( (*playlist.at(playlistpointer)).ascii() );
                wm_cd_play(track, 0, track + 1);
            }
            else if ( looping )
            {
                if (cur_track == cur_ntracks)
                {
                    cur_track = 0;
                    wm_cd_play (1, 0, cur_ntracks + 1);
                }

            }
            else
            {
                cur_track = save_track = 1;
                statuslabel->clear(); // TODO how should I properly handle this
                damn = TRUE;
            }
            break;

        case WM_CDM_PLAYING:
            playtime ();
            if(randomplay)
                if(randomonce)
                {
                    statuslabel->setText( i18n("Shuffle") );
                } else {
                    statuslabel->setText( i18n("Random") );
                }
            else
                statuslabel->setText( i18n("Playing") );

            //sprintf( p, "%02d  ", cur_track );
            if (songListCB->count() == 0)
            {
                // we are in here when we start kscd and
                // the cdplayer is already playing.
                populateSongList();
                setSongListTo( cur_track - 1 );

                have_new_cd = false;
                get_cddb_info(false); // false == do not update dialog if open
            } else {
                setSongListTo( cur_track - 1 );
            }
            tracklabel->setText( formatTrack(cur_track, cd->ntracks) );

            if((cur_track < (int)tracktitlelist.count()) && (cur_track >= 0))
            {
                setArtistAndTitle(tracktitlelist.first(),
                                  *tracktitlelist.at(cur_track));
            }

            setLEDs( tmptime );
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

                int w = ((cur_track >= 0) ? cur_track : 1);

                tracklabel->setText( formatTrack( cur_track >= 0 ? cur_track : 1, cd->ntracks) );

                if( w < (int)tracktitlelist.count()){
                    setArtistAndTitle(tracktitlelist.first(),
                                      *tracktitlelist.at( w ));
                }
            }
            damn = FALSE;
            if(have_new_cd){

                //      timer.stop();
                cur_track = save_track = 1;
                have_new_cd = false;
                // timer must be restarted when we are doen
                // with getting the cddb info
                get_cddb_info(false); // false == do not update dialog if open
                if(autoplay && ejectedBefore)
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
            ejectedBefore = TRUE;
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
        trackTimeLED[i]->display(symbols[i].latin1());
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
	volume     	= config->readNumEntry("Volume",40);
	tooltips   	= config->readBoolEntry("ToolTips", true);
        // TODO: this breaks if randomplay comes up true to begin with!
//	randomplay 	= config->readBoolEntry("RandomPlay", false);
    docking = config->readBoolEntry("DOCKING", true);
	autoplay		= config->readBoolEntry("AUTOPLAY", false);
	stopexit 	= config->readBoolEntry("STOPEXIT", true);
	ejectonfinish = config->readBoolEntry("EJECTONFINISH", false);
	randomonce 	= (bool)config->readBoolEntry("RANDOMONCE",true);
	looping    	= config->readBoolEntry("Looping",false);
	hidden_controls = config->readBoolEntry("HiddenControls",false);
	skipDelta = config->readNumEntry("SkipDelta", 30);
	time_display_mode = config->readNumEntry("TimeDisplay", TRACK_SEC);

#ifdef DEFAULT_CD_DEVICE

	// sun ultrix etc have a canonical cd rom device specified in the
	// respective plat_xxx.c file. On those platforms you need nnot
	// specify the cd rom device and DEFAULT_CD_DEVICE is not defined
	// in config.h

	cd_device_str = config->readEntry("CDDevice",DEFAULT_CD_DEVICE);
   cd_device = (char *)qstrdup(QFile::encodeName(cd_device_str));

#endif


	QColor defaultback = black;
	QColor defaultled = QColor(226,224,255);
	background_color = config->readColorEntry("BackColor",&defaultback);
	led_color = config->readColorEntry("LEDColor",&defaultled);

	config->setGroup("MAGIC");
	magic_width      = config->readNumEntry("magicwidth",320);
	magic_height     = config->readNumEntry("magicheight",200);
	magic_brightness = config->readNumEntry("magicbrightness", 3);
	magic_pointsAreDiamonds = config->readBoolEntry("magicPointsAreDiamonds", false);

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

	config->setGroup("CDDB");

	cddb.setTimeout(config->readNumEntry("CDDBTimeout",60));
	cddb_auto_enabled = config->readBoolEntry("CDDBLocalAutoSaveEnabled",true);
	cddbbasedir = config->readEntry("LocalBaseDir");

	// Changed global KDE apps dir by local KDE apps dir
	if (cddbbasedir.isEmpty())
		cddbbasedir = KGlobal::dirs()->resourceDirs("cddb").first();
	KGlobal::dirs()->addResourceDir("cddb", cddbbasedir);

	// Set this to false by default. Look at the settings dialog source code
	// for the reason. - Juraj.
	cddb_remote_enabled = config->readBoolEntry( "CDDBRemoteEnabled", false );
	cddb.useHTTPProxy( config->readBoolEntry("CDDBHTTPProxyEnabled", KProtocolManager::useProxy()) );
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
		cddb.useHTTPProxy(false);
	}
	cddb.setHTTPProxy(config->readEntry("HTTPProxyHost",proxyHost),
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
	}
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
    config->writeEntry("ToolTips", tooltips);
    config->writeEntry("RandomPlay", randomplay);
    config->writeEntry("DOCKING", docking);
    config->writeEntry("AUTOPLAY", autoplay);
    config->writeEntry("STOPEXIT", stopexit);
    config->writeEntry("EJECTONFINISH", ejectonfinish);
    config->writeEntry("RANDOMONCE", randomonce);
    config->writeEntry("CDDevice", QFile::decodeName(cd_device));
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

    config->setGroup("CDDB");
    config->writeEntry("CDDBRemoteEnabled",cddb_remote_enabled);
    config->writeEntry("CDDBTimeout", cddb.getTimeout());
    config->writeEntry("CDDBLocalAutoSaveEnabled",cddb_auto_enabled);

    config->writeEntry("LocalBaseDir",cddbbasedir);
    config->writeEntry("SeverList",cddbserverlist);
    config->writeEntry("SubmitList", cddbsubmitlist);
    config->writeEntry("CDDBSubmitAddress",submitaddress);
    config->writeEntry("CurrentServer",current_server);
    config->writeEntry("CDDBHTTPProxyEnabled",cddb.useHTTPProxy());
    config->writeEntry("HTTPProxyHost",cddb.getHTTPProxyHost());
    config->writeEntry("HTTPProxyPort",(int)cddb.getHTTPProxyPort());

    config->setGroup("MAGIC");
    config->writeEntry("magicwidth",magic_width);
    config->writeEntry("magicheight",magic_height);
    config->writeEntry("magicbrightness",magic_brightness);
    config->writeEntry("magicPointsAreDiamonds",magic_pointsAreDiamonds);

    config->sync();
} // writeSettings()

void
KSCD::CDDialogSelected()
{
    kdDebug() << "CDDialogSelected" << endl;
    if(cddialog)
        return;

    kdDebug() << "dialog was empty" << endl;

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
  //kdDebug() << "cddialog -> done" << endl;
  delete cddialog;
  cddialog = 0L;
}


void
KSCD::getCDDBservers()
{

    led_on();

    /*
     * minorly fugly hack, but i couldn't think of a nice clean way to
     * remove this last bit of stupidity. i got rid of the rest of the
     * rediculousness, though =)  - aseigo
     */

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

        cddb.setTimeout(cddb_timeout);
        cddb.setHTTPProxy(cddb_proxy_host,cddb_proxy_port);
        cddb.useHTTPProxy(cddb_proxy_enabled);
    }

    connect(&cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
    connect(&cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));

    // For now, just don't update if there's no current server.
    if(!current_server.isEmpty())
        cddb.cddbgetServerList(current_server);
} // getCDDBservers()

void
KSCD::getCDDBserversFailed()
{
    led_off();
    disconnect(&cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
    disconnect(&cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));
    setArtistAndTitle(i18n("Unable to get freedb server list."), "");
    titlelabeltimer.start(10000,TRUE); // 10 secs
}

void
KSCD::getCDDBserversDone()
{
    led_off();
    disconnect(&cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
    disconnect(&cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));
    cddb.serverList(cddbserverlist);
    emit newServerList(cddbserverlist);
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
    for (i = 0; i < thicd.ntracks; i++)
    {
        n += cddb_sum(thiscd.trk[i].start / 75);
    }

    t = ((thiscd.trk[thiscd.ntracks].start / 75) -
         (thiscd.trk[0].start / 75));

    return ((n % 0xff) << 24 | t << 8 | thiscd.ntracks);
}

#endif

void
KSCD::get_cddb_info(bool _updateDialog)
{
    updateDialog = _updateDialog;

    // Don't crash if no disc is in
    if( cd->length == 0 ) {
      kdDebug() << "CD length seems to be zero" << endl;
      cddb_no_info();
      return;
    }

    QTime dml;
    dml = dml.addSecs(cd->length);

    QString fmt;
    if(dml.hour() > 0)
        fmt.sprintf("%02d:%02d:%02d",dml.hour(),dml.minute(),dml.second());
    else
        fmt.sprintf("%02d:%02d",dml.minute(),dml.second());

    totaltimelabel->setText(fmt);
    get_pathlist(pathlist);
    cddb.setPathList(pathlist);
    led_on();
    bool res = cddb.local_query(
        cddb_discid(),
        xmcd_data,
        tracktitlelist,
        extlist,
        category,
        discidlist,
        revision,
        playlist
        );
    Fetch_remote_cddb = false;
    if(!res && !cddb_remote_enabled){
        //    have_new_cd = false;
        cddb_no_info();
        return;
    }

    if(!res){

        kdDebug() << "STARTING REMOTE QUERY\n" << endl;
        cddb.cddb_connect(current_server);
        Fetch_remote_cddb = true;

    }
    else{
        kdDebug() << "FOUND RECORD LOCALLY\n" << endl;
        if((int)tracktitlelist.count() != (cd->ntracks + 1)){
            kdDebug() << "WARNING LOCAL QUERY tracktitleslist.count = " << tracktitlelist.count() << " != cd->ntracks +1 = " << cd->ntracks + 1 << "\n" << endl;
        }

        if((int)extlist.count() != (cd->ntracks + 1)){
            kdDebug() << "WARNING LOCAL QUERYextlist.count = " << extlist.count() << " != cd->ntracks +1 = " << cd->ntracks + 1 << "\n" << endl;
        }

        if(tracktitlelist.count() > 1){
            setArtistAndTitle(tracktitlelist.first(),
                              *tracktitlelist.at(1));
        }

        populateSongList();

        led_off();
        timer.start(1000);

        if(cddialog && updateDialog)
            cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
                              revision,playlist,pathlist,cddbbasedir,submitaddress, smtpConfigData);
        playlistpointer = 0;
    }

    //  have_new_cd = false;

} // get_cddb_info

int cddb_ready_bug = 0;
void
KSCD::cddb_ready()
{
    kdDebug() << "cddb_ready() called\n" << endl;

    if(!cd)
        return;

    querylist.clear();
    tracktitlelist.clear();
  setArtistAndTitle("", "");
  extlist.clear();
    discidlist.clear();

    QCString num;

    for(int i = 0 ; i < cd->ntracks; i++)
    {
        querylist.append(num.setNum(cd->trk[i].start));
    }

    querylist.append(num.setNum(cd->trk[cd->ntracks].start/75));
    cddb_inexact_sentinel =false;
    cddb.queryCD(cddb_discid(),querylist);
} // cddb_ready

#define DEFINE_CDTEXT
#define CDTEXT_MACRO \
    if(wm_cdtext_info.valid){\
  kdDebug() << "CDTEXT_MACRO called" << endl;\
  int at;\
    setArtistAndTitle("", ""); \
    tracktitlelist.clear(); \
    extlist.clear(); \
  tracktitlelist.append(QString().sprintf("%s / %s", (const char*)(wm_cdtext_info.blocks[0]->name[0]),(const char*)(wm_cdtext_info.blocks[0]->performer[0])));\
  titlelabel->setText(QString((const char*)(wm_cdtext_info.blocks[0]->name[1])));\
  artistlabel->setText(tracktitlelist.first());\
  songListCB->clear();\
  for (at = 1 ; at < (wm_cdtext_info.count_of_entries); ++at ) {\
      songListCB->insertItem( QString().sprintf("%02d: %s", at, wm_cdtext_info.blocks[0]->name[at]));\
      tracktitlelist.append((const char*)(wm_cdtext_info.blocks[0]->name[at]));\
  }\
  for(; at < cur_ntracks; ++at){\
      songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), at)));\
  }\
}

void
KSCD::cddb_no_info()
{
    setArtistAndTitle(i18n("No matching freedb entry found."), "");
    tracktitlelist.clear();
    extlist.clear();
    //    tracktitlelist.append(i18n("No matching freedb entry found."));
#ifdef DEFINE_CDTEXT
    wm_cd_get_cdtext();
    CDTEXT_MACRO
    else
#endif /* DEFINE_CDTEXT */
    {
       discidlist.clear();
    }
    timer.start(1000);
    led_off();
    cddb_inexact_sentinel =false;

    populateSongList();
} // cddb_no_info

void
KSCD::cddb_failed()
{
    // TODO differentiate between those casees where the communcition really
    // failed and those where we just couldn't find anything
    //        cddb_ready_bug = 0;
    kdDebug() << "cddb_failed() called\n" << endl;
    setArtistAndTitle("", "");
    tracktitlelist.clear();
    extlist.clear();
    tracktitlelist.append(i18n("Error getting freedb entry."));
#ifdef DEFINE_CDTEXT
    CDTEXT_MACRO
    else
#endif /* DEFINE_CDTEXT */
    {
      for(int i = 0 ; i < cd->ntracks; i++)
        tracktitlelist.append("");

      extlist.clear();
      for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

      discidlist.clear();

      setArtistAndTitle(i18n("Error getting freedb entry."), "");
    }
    timer.start(1000);
    led_off();
    cddb_inexact_sentinel =false;
} // cddb_failed

void
KSCD::cddb_timed_out()
{
  kdDebug() << "cddb_timed_out() called\n" << endl;
  tracktitlelist.clear();
  setArtistAndTitle("", "");
  extlist.clear();
  tracktitlelist.append(i18n("freedb query timed out."));
#ifdef DEFINE_CDTEXT
    CDTEXT_MACRO
    else
#endif /* DEFINE_CDTEXT */
    {
      for(int i = 0 ; i <= cd->ntracks; i++)
        tracktitlelist.append("");

      extlist.clear();
      for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

      discidlist.clear();

      setArtistAndTitle(i18n("freedb query timed out."),"");
    }
    timer.start(1000);
    led_off();
    cddb_inexact_sentinel =false;
} // cddb_timed_out()

void
KSCD::mycddb_inexact_read()
{
    if(cddb_inexact_sentinel == true)
        return;

    QString pick;

    cddb_inexact_sentinel = true;
    QStringList inexact_list;
    cddb.get_inexact_list(inexact_list);

    // Whatever happens, we better clear the list beforehand
    setArtistAndTitle("", "");
    tracktitlelist.clear();
    extlist.clear();

    if( inexact_list.count() == 1)
    {
        pick = inexact_list.first();
        cddb.query_exact("200 " + pick);
        return;
    }

    InexactDialog *dialog;
    dialog = new InexactDialog(0,"inexactDialog",true);
    dialog->insertList(inexact_list);

    if(dialog->exec() != QDialog::Accepted)
    {
        cddb.close_connection();
        timer.start(1000);
        led_off();
        return;
    }

    dialog->getSelection(pick);
    delete dialog;


    if(pick.isEmpty())
    {
        timer.start(1000);
        led_off();
        return;
    }

    pick = "200 " + pick;
    cddb.query_exact(pick);
} // mycddb_inexact_read

void
KSCD::cddb_done()
{
    cddb_inexact_sentinel =false;

    kdDebug() << "cddb_done() called\n" << endl;
    cddb.getData(xmcd_data,tracktitlelist,extlist,category,discidlist,revision,playlist);
    playlistpointer = 0;

    if((int)tracktitlelist.count() != (cd->ntracks + 1)){
        kdDebug() << "WARNING tracktitleslist.count = " << tracktitlelist.count() << " != cd->ntracks +1 = " << cd->ntracks + 1 << "\n" << endl;
    }

    if((int)extlist.count() != (cd->ntracks + 1)){
        kdDebug() << "WARNING extlist.count = " << extlist.count() << " != cd->ntracks +1 = " << cd->ntracks + 1 << "\n" << endl;
    }

    if(tracktitlelist.count() > 1){
        setArtistAndTitle(tracktitlelist.first(),
                          *tracktitlelist.at(1));
    }

    if(cddialog && updateDialog)
        cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
                          revision,playlist,pathlist,cddbbasedir,submitaddress, smtpConfigData);

    populateSongList();
    led_off();
    if(Fetch_remote_cddb)
    {
        if(cddb_auto_enabled)
        {
            QString path,tmp;
            tmp.sprintf("/%08lx",cddb_discid());
            path = cddbbasedir;

            // check to see if the category dir is there yet and
            // create it (with the right permissions) if it's not
            QDir checkCat;
            checkCat.setPath(path);
            if ( !checkCat.exists(category) )
            {
                checkCat.mkdir(category);
                QString fulldir(QDir::cleanDirPath(checkCat.path()));
                fulldir += '/';
                fulldir += category;
                struct stat tmpBuf;
                stat (fulldir.local8Bit().data(), &tmpBuf);
                chmod(fulldir.local8Bit().data(), tmpBuf.st_mode | S_IWGRP);
            }
            path += "/";
            path += category;
            path += tmp;
            path.replace(QRegExp("//"),"/");
            edm_save_cddb_entry(path);
        }
    }
    timer.start(1000);
} // cddb_done

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
        tooltip += KStringHandler::rsqueeze(title,30);
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

            tmp = cur_tracklen - cur_pos_rel;
            mysec = tmp % 60;
            mymin = tmp / 60;
            break;

        case TOTAL_SEC:

            mysec = cur_pos_abs % 60;
            mymin = cur_pos_abs / 60;
            break;

        case TOTAL_REM:

            tmp = cur_cdlen - cur_pos_abs;
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


    sprintf( tmptime, "%02d:%02d", mymin, mysec );
    return;

} // playtime

void
KSCD::cycleplaytimemode()
{
    cycletimer.stop();

    if (++time_display_mode > 3)
        time_display_mode = 0;
    playtime();
    setLEDs( tmptime );


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
    kdDebug() << "preformances " << i << "\n" << endl;

    QString artist;
    QString str;

    if(!getArtist(artist))
        return;

    // primitive incomplete http encoding TODO fix!
    artist = artist.replace( QRegExp(" "), "+" );

    switch(i){
        case 0:
            str =
                QString("http://www.tourdates.com/cgi-bin/search.cgi?type=Artist&search=%1")
                .arg(artist);
            startBrowser(str);

            break;

        default:
            break;
    }

} // performances

void
KSCD::purchases(int i)
{
    kdDebug() << "purchases " << i << "\n" << endl;

    QString artist;
    QString str;

    if(!getArtist(artist))
        return;

    // primitive incomplete http encoding TODO fix!
    artist = artist.replace( QRegExp(" "), "+" );

    switch(i){
        case 0:
            str =
                QString("http://cdnow.com/switch/from=sr-288025/target=buyweb_products/artfs=%1")
                .arg(artist);
            startBrowser(str);

            break;
        case 1:
            str =
                QString("http://www.cduniverse.com/cgi-bin/cdubin.exe/rlinka/ean=%1")
                .arg(artist);
            startBrowser(str);

            break;

        default:
            break;
    }

} // purchases

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

void
KSCD::information(int i)
{
    kdDebug() << "Information " << i << "\n" << endl;

    QString artist;
    QString str;

    if(!getArtist(artist))
        return;

    // primitive incomplete http encoding TODO fix!
    artist = artist.replace( QRegExp(" "), "+" );

    switch(i)
    {
        case 0:
            str =
                QString("http://ubl.artistdirect.com/cgi-bin/gx.cgi/AppLogic+Search?select=MusicArtist&searchstr=%1&searchtype=NormalSearch")
                .arg(artist);
            startBrowser(str);
            break;

        case 2:
            str =
                QString("http://x8.dejanews.com/dnquery.xp?QRY=%1&defaultOp=AND&svcclass=dncurrent&maxhits=20&ST=QS&format=terse&DBS=2")
                .arg(artist);
            startBrowser(str);
            break;

        case 3:
            str =
                QString("http://www.excite.com/search.gw?c=web&search=%1&trace=a")
                .arg(artist);
            startBrowser(str);
            break;

        case 4:
            str =
                QString("http://www.search.hotbot.com/hResult.html?SW=web&SM=MC&MT=%1&DC=10&DE=2&RG=NA&_v=2")
                .arg(artist);
            startBrowser(str);
            break;

        case 5:
            str =
                QString("http://www.infoseek.com/Titles?qt=%1&col=WW&sv=IS&lk=ip-noframes&nh=10")
                .arg(artist);
            startBrowser(str);
            break;

        case 6:
            str =
                QString("http://www.lycos.com/cgi-bin/pursuit?cat=lycos&query=%1")
                .arg(artist);
            startBrowser(str);
            break;

        case 7:
            str =
                QString("http://www.mckinley.com/search.gw?search=%1&c=web&look=magellan")
                .arg(artist);
            startBrowser(str);
            break;

        case 8:
            str =
                QString("http://search.yahoo.com/bin/search?p=%1")
                .arg(artist);
            startBrowser(str);
            break;

        default:
            break;
    } // switch()
} // information

/**
 * Open an URL with the user's favourite browser.
 *
 */
void
KSCD::startBrowser(const QString &querystring)
{
    (void) new KRun (querystring);
} //startBrowser


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
		 if( *it != QString::fromLatin1(".") && *it != QString::fromLatin1("..") )
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

/* I am dropping this code for now. People seem to be having nothing
   but trouble with this code and it was of dubious value anyways....

   dfoerste: NOTES

1) Mark Buckaway checked only if there was an iso9660 filesystem mounted,
   not if our device is mounted.
2) Mount check only needs to be done before ejecting the disc.

#ifdef __linux__

// check if drive is mounted (from Mark Buckaway's cdplayer code)


void
KSCD::checkMount()
{
  if ((fp = setmntent (MOUNTED, "r")) == NULL)
    {
    fprintf (stderr, i18n("Couldn't open %s: %s\n"),
               MOUNTED, strerror (errno));
      exit (1);
    }

    while ((mnt = getmntent (fp)) != NULL)
    {
    if (strcmp (mnt->mnt_type, "iso9660") == 0)
        {
        fputs (i18n("CD-ROM already mounted. Operation aborted.\n"),
                 stderr);
          endmntent (fp);
          exit (1);
        }
        }
        endmntent (fp);
} // checkMount()

#elif defined (__FreeBSD__)
void
KSCD::checkMount()
{
  struct statfs *mnt;
  int i, n;

  n = getmntinfo(&mnt, MNT_WAIT);
  for (i=0; i<n; i++)
    {
    if (mnt[i].f_type == MOUNT_CD9660)
        {
        fputs(i18n("CD-ROM already mounted. Operation aborted.\n"),
                stderr);
          exit(1);
        }
        }
        }

        #else

// TODO Can I do this for other platforms?
   void
   KSCD::checkMount()
{
}

#endif
 */

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
    for(int i = 0; i < cur_ntracks; i++)
    {
        do {
            selected = (int) randSequence.getLong(cur_ntracks);
            rejected = (random_list.find(selected) != random_list.end());
        } while(rejected == true);
        random_list.append(selected);
    }
    random_current = random_list.begin(); /* Index of array we are on */
} // make_random_list()


void
KSCD::edm_save_cddb_entry(QString& path)
{

    kdDebug() << "::save_cddb_entry(): path: " << path << " edm" << "\n" << endl;

    QFile file( path ); //open the file
	 QFileInfo fileinfo( file );
	 QDir dir( fileinfo.dirPath() ); // Directory manipulation

	 if( ! dir.exists() )
	 {

		 if( ! dir.mkdir( fileinfo.dirPath() ) )
		 {
			 kdDebug() << "Output directory: " << fileinfo.dirPath() << endl;
			 QString str = i18n("Unable to write to file:\n%1\nPlease check "
					 "your permissions and ensure your category directories exist.")
				 .arg(path);
			 KMessageBox::error(this, str);
			 return;
		 }
	 }

    if( !file.open( IO_WriteOnly  ))
    {
        QString str = i18n("Unable to write to file:\n%1\nPlease check "
				  "your permissions and ensure your category directories exist.")
			  .arg(path);
		  KMessageBox::error(this, str);
        return;
    }

    QString tmp;
    QTextStream t(&file);

    t << "# xmcd CD database file\n";

    QString datestr;
    datestr = QDateTime::currentDateTime().toString();
    tmp = QString("# Generated: %1 by KSCD\n").arg(datestr);
    t << tmp;

    // Waste some disk space
    t << "# Copyright (C) 1997-1999 Bernd Johannes Wuebben.\n";
    t << "# Copyright (C) 1999-2001 Dirk Foersterling.\n";



    t << "# \n";
    t << "# Track frame offsets:\n";

    for(int i = 0 ; i < cd->ntracks+1 ;i ++)
    {
        tmp = QString("#       %1\n").arg(cd->trk[i].start);
        t << tmp;
    }

    t << "#\n";
    tmp = QString("# Disc length: %1 seconds\n").arg(cd->length);
    t << tmp;
    t << "#\n";
    tmp = QString("# Revision: %1\n").arg("8"); //if no revision put 8
    t << tmp;
    t << "# Submitted via: Kscd "KSCDVERSION"\n";
    t << "#\n";


    tmp = "DISCID=";
    int counter = 0;

    int num = 0;
    for ( QStringList::Iterator it = discidlist.begin();
          it != discidlist.end();
          ++it, ++num )
    {

        tmp += *it;

        if( num < (int) discidlist.count() - 1)
        {
            if( counter++ == 3 )
            {
                tmp += "\nDISCID=";
                counter = 0;
            } else {
                tmp += ",";
            }
        }
    }

    tmp += "\n";
    t << tmp;

    QStringList returnlist;
    QString tmp2;

    tmp2 = *tracktitlelist.at(0);
    cddb_encode(tmp2,returnlist);

    if(returnlist.count() == 0)
    {
        // sanity provision
        tmp = QString("DTITLE=%1\n").arg("");
        t << tmp;
    } else {
        for ( QStringList::Iterator it = returnlist.begin();
              it != returnlist.end();
              ++it )
        {
            tmp = QString("DTITLE=%1\n").arg(*it);
            t << tmp;
        }
    }

    num = 1;
    for ( QStringList::Iterator it = tracktitlelist.begin();
          it != tracktitlelist.end();
          ++it)
    {
        tmp2 = *it;
        cddb_encode(tmp2,returnlist);

        // no perfect solution, but it's working so far.
        if( it != tracktitlelist.begin() ) {
            if(returnlist.isEmpty())
            {
                // sanity provision
                tmp = QString("TTITLE%1=%2\n").arg(num-1).arg("");
                t << tmp;
            } else {
                tmp = QString("TTITLE%1=%2\n").arg(num-1).arg(*it);
                t << tmp;
            }
            num++;
        }
    }

    tmp2 = extlist.first();
    cddb_encode(tmp2,returnlist);

    if(returnlist.isEmpty())
    {
        // sanity provision
        tmp = tmp.sprintf("EXTD=%s\n","");
        t << tmp;
    } else {
        for ( QStringList::Iterator it = returnlist.begin();
              it != returnlist.end();
              ++it )
        {
            tmp = QString("EXTD=%1\n").arg(*it);
            t << tmp;
        }
    }

    int i = 1;
    for ( QStringList::Iterator it = extlist.at(1);
          it != extlist.end();
          ++it, i++ )
    {
        tmp2 = *it;
        cddb_encode(tmp2,returnlist);

        if(returnlist.count() == 0)
        {
            // sanity provision
            tmp = tmp.sprintf("EXTT%d=%s\n",i-1,"");
            t << tmp;
        } else {
            for(int j = 0; j < (int) returnlist.count();j++)
            {
                tmp = tmp.sprintf("EXTT%d=%s\n",i-1,(*returnlist.at(j)).utf8().data());
                t << tmp;
            }
        }
    }
    QString     playorder;
    cddb_encode(playorder,returnlist);

    for(int i = 0; i < (int) returnlist.count();i++)
    {
        tmp = tmp.sprintf("PLAYORDER=%s\n", (*returnlist.at(i)).utf8().data());
        t << tmp;
    }
    t << "\n";

    file.close();
    chmod(QFile::encodeName(file.name()), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
    return;
} // save_cddb_entry


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
    return cur_track;
}

QString KSCD::currentTrackTitle()
{
    return (cur_track > -1) ? tracktitlelist[cur_track] : QString::null;
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
    if (tooltips)
    {
        QToolTip::add(songListCB, i18n("Track list"));
    }
}

void KSCD::populateSongList()
{
    int i = 1;
    clearSongList();
    QStringList::Iterator it = tracktitlelist.begin();
    for (++it; it != tracktitlelist.end(); ++it, ++i )
    {
        songListCB->insertItem(QString::fromLatin1("%1: %2")
                               .arg(QString::number(i).rightJustify(2, '0'))
                               .arg(*it));
    }
    for(; i < cur_ntracks; i++)
    {
        songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), i+1)) );
    }
}

void KSCD::setSongListTo(int whichTrack)
{
    songListCB->setCurrentItem(whichTrack);
    if (tooltips)
    {
        // drop the number. 
        // for Mahlah, a picky though otherwise wonderful person - AJS
        QString justTheName = songListCB->currentText();
        justTheName = justTheName.right(justTheName.length() - 4);

        QToolTip::add(songListCB, i18n("Current Track: %1").arg(justTheName));
    }
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
                          "(c) 2001, Dirk Försterling");
    aboutData.addAuthor("Bernd Johannes Wuebben",0, "wuebben@kde.org");
    aboutData.addAuthor("Dirk Försterling",0, "milliByte@gmx.net");
    aboutData.addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");

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
    cur_track = 1;

    a.setTopWidget( k );
    a.setMainWidget( k );

    k->setCaption(a.caption());
    k->initialShow();

    return a.exec();
} // main()

#include "kscd.moc"
