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

extern "C" {
#include "struct.h"
}

#include <qdir.h>
#include <qregexp.h>

#include <kcharsets.h>
#include <kconfig.h>
#include <klocale.h>
#include <krun.h>

#include "docking.h"
#include "kscd.h"
#include "configdlg.h"
#include "mgconfdlg.h"
#include "version.h"
#include "config.h"
#include "inexact.h"
#include "CDDialog.h"
#include "CDDBSetup.h"
#include "bitmaps/playpaus.xbm"
#include "bitmaps/stop.xbm"
#include "bitmaps/repeat.xbm"
#include "bitmaps/nexttrk.xbm"
#include "bitmaps/prevtrk.xbm"
#include "bitmaps/ff.xbm"
#include "bitmaps/rew.xbm"
#include "bitmaps/info.xbm"
#include "bitmaps/poweroff.xbm"
#include "bitmaps/eject.xbm"
#include "bitmaps/db.xbm"
#include "bitmaps/logo.xbm"
#include "bitmaps/shuffle.xbm"
#include "bitmaps/options.xbm"

#include <kwm.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kmessagebox.h>

DockWidget*     dock_widget;
SMTP                *smtpMailer;
bool dockinginprogress = 0;
bool quitPending = 0;
bool stoppedByUser = 1;

char 		tmptime[100];
char 		*tottime;
//static void 	playtime (void);
void 		kcderror(const QString& title, const QString& message);
void 		kcdworker(int );

//void 		parseargs(char* buf, char** args);
extern QTime framestoTime(int frames);
extern void cddb_decode(QString& str);
extern void cddb_encode(QString& str, QStrList &returnlist);
extern void cddb_playlist_encode(QStrList& list,QString& playstr);
extern bool cddb_playlist_decode(QStrList& list, QString& str);

/************************************************************************
 new for workman: (BERND)
 *****************************************************************************/
extern "C" {
  /*
   * These definitions will change heavily in the future.
   */
    int play_cd(int start,int pos,int end);
    int pause_cd();
    int cd_close();
    int stop_cd();
    int cd_status();
    int eject_cd();
    int cd_closetray();
    int cd_volume(int vol, int bal,int max);
}

extern struct play *playlist ;
extern struct cdinfo_wm thiscd, *cd ;
extern const char *cd_device;
extern int have_new_cd;

extern int	cur_track;	/* Current track number, starting at 1 */
extern int	cur_index;	/* Current index mark */
extern int	cur_lasttrack ;	/* Last track to play in current chunk */
extern int	cur_firsttrack;	/* First track of current chunk */
/*
 * have to remove this.
 */
int    random_current;  /* koz: Current track in random list */
int   *random_list;     /* koz: Used in Random - once through */

int    cur_pos_abs;	/* Current absolute position in seconds */
int    cur_frame;	/* Current frame number */
int    cur_pos_rel;	/* Current track-relative position in seconds */
int    cur_tracklen;	/* Length in seconds of current track */
int    cur_cdlen;	/* Length in seconds of entire CD */
int    cur_ntracks;	/* Number of tracks on CD (= tracks + sections) */
int    cur_nsections;	/* Number of sections currently defined */
extern enum cd_modes	cur_cdmode;
int    cur_listno;	/* Current index into the play list, if playing */
char  *cur_artist;	/* Name of current CD's artist */
char  *cur_cdname;	/* Album name */
char  *cur_trackname;	/* Take a guess */
char   cur_contd;	/* Continued flag */
char   cur_avoid;	/* Avoid flag */

int cur_balance;
int info_modified;
int cur_stopmode;
int cur_playnew;
int mark_a, mark_b;

int cddb_error = 0;

/****************************************************************************
				The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name ) :
    QWidget( parent, name )
{

    connect(kapp, SIGNAL (saveYourself() ), SLOT (doSM()));
    magicproc           = 0L;
    cd_device_str       = "";
    background_color 	= black;
    led_color 		= green;
    randomplay 		= false;
    looping 		= false;
    cddrive_is_ok 	= true;
    tooltips 		= true;
    magic_width         = 320;
    magic_height        = 200;
    magic_brightness    = 3;

    cycle_flag		= false;
    cddb_remote_enabled = false;
    setup 	       	= 0L;
    smtpconfig          = 0L;
    time_display_mode 	= TRACK_SEC;
    cddb_inexact_sentinel = false;
    revision		= 1;
    use_kfm 		= true;
    docking 		= true;
    autoplay            = false;
    stopexit            = true;
    ejectonfinish       = false;
    randomonce          = true;
    updateDialog        = false;
    ejectedBefore       = false;
    currentlyejected    = false;

    drawPanel();
    loadBitmaps();
    setColors();
    setToolTips();

    cddialog 	    = 0L;
    timer 	    = new QTimer( this );
    queryledtimer   = new QTimer( this );
    titlelabeltimer = new QTimer( this );
    initimer 	    = new QTimer( this );
    cycletimer      = new QTimer( this );

    connect( initimer, SIGNAL(timeout()),this,  SLOT(initCDROM()) );
    connect( queryledtimer, SIGNAL(timeout()),  SLOT(togglequeryled()) );
    connect( titlelabeltimer, SIGNAL(timeout()),  SLOT(titlelabeltimeout()) );
    connect( cycletimer, SIGNAL(timeout()),  SLOT(cycletimeout()) );
    connect( timer, SIGNAL(timeout()),  SLOT(cdMode()) );
    connect( playPB, SIGNAL(clicked()), SLOT(playClicked()) );
    connect( stopPB, SIGNAL(clicked()), SLOT(stopClicked()) );
    connect( prevPB, SIGNAL(clicked()), SLOT(prevClicked()) );
    connect( nextPB, SIGNAL(clicked()), SLOT(nextClicked()) );
    connect( fwdPB, SIGNAL(clicked()), SLOT(fwdClicked()) );
    connect( bwdPB, SIGNAL(clicked()), SLOT(bwdClicked()) );
    connect( dockPB, SIGNAL(clicked()), SLOT(quitClicked()) );
    connect( replayPB, SIGNAL(clicked()), SLOT(loopClicked()) );
    connect( ejectPB, SIGNAL(clicked()), SLOT(ejectClicked()) );
    connect( songListCB, SIGNAL(activated(int)), SLOT(trackSelected(int)));
    connect( volSB, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));
    connect( aboutPB, SIGNAL(clicked()), SLOT(cycleplaytimemode()));
    //connect( aboutPB, SIGNAL(clicked()), SLOT(aboutClicked()));
    connect( optionsbutton, SIGNAL(clicked()), SLOT(aboutClicked()));
    connect( shufflebutton, SIGNAL(clicked()), SLOT(randomSelected()));
    connect( cddbbutton, SIGNAL(clicked()), SLOT(CDDialogSelected()));
    connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(setColors()));
	
    readSettings();
    setColors();
    initWorkMan();

    setupPopups();
    volstartup = TRUE;
    volSB->setValue(volume);

    dock_widget = new DockWidget( this, "dockw");
    if(docking)
      {
        dock_widget->show();
      }

    smtpMailer = new SMTP;
    connect(smtpMailer, SIGNAL(messageSent()), this, SLOT(smtpMessageSent()));
    connect(smtpMailer, SIGNAL(error(int)), this, SLOT(smtpError(int)));

    setFocusPolicy ( QWidget::NoFocus );
    srandom(time(0L));
    initimer->start(500,TRUE);
} // KSCD

void 
KSCD::smtpMessageSent(void)
{
    KMessageBox::information(this, i18n("Record submitted successfully"),
                                   i18n("Record Submission"));
} // smtpMessageSent()

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
        lstr = i18n("Time out waiting for server interaction.");
        break;
    default:
        lstr = i18n("Server said:\n\"%1\"").arg(smtpMailer->getLastLine());
    }
    str = i18n("Error #%1 sending message via SMTP.\n\n%2")
      .arg(errornum).arg(lstr);
    KMessageBox::error(this, str, i18n("Record Submission"));
} // smptError()

	
// Initialize the variables only in WorkMan
void 
KSCD::initWorkMan() 
{

    mark_a	 = 0;
    mark_b 	 = 0;
    cur_balance  = 10;
    fastin 	 = FALSE;
    scmd 	 = 0;
    tmppos 	 = 0;
    save_track 	 = 1;
    thiscd.trk 	 = NULL;
    thiscd.lists = NULL;
    tottime 	 = tmptime;

} // initWorkMan()
	
void 
KSCD::initCDROM()
{
    initimer->stop();
    kapp->processEvents();
    kapp->flushX();

    cdMode();
    volstartup = FALSE;
    if(cddrive_is_ok)
        volChanged(volume);

    timer->start(1000);
    //  cdMode();
} // initCDROM


QPushButton *
KSCD::makeButton( int x, int y, int w, int h, const QString& n )
{
    QPushButton *pb = new QPushButton( n, this );
    pb->setGeometry( x, y, w, h );
    pb->setFocusPolicy ( QWidget::NoFocus );
    return pb;
} // makeButton
	
void 
KSCD::drawPanel()
{
    int ix = 0;
    int iy = 0;
    //	const int WIDTH = 100;
    const int WIDTH = 90;
    //	const int HEIGHT = 29;
    const int HEIGHT = 27;
    //	const int SBARWIDTH = 180; //140
    const int SBARWIDTH = 220; //140

    aboutPB = makeButton( ix, iy, WIDTH, 2 * HEIGHT, i18n("About") );

    ix = 0;
    iy += 2 * HEIGHT;

    infoPB = makeButton( ix, iy, WIDTH/2, HEIGHT, "" );
    ejectPB = makeButton( ix + WIDTH/2, iy, WIDTH/2, HEIGHT, "" );

    iy += HEIGHT;
    dockPB = makeButton( ix, iy, WIDTH, HEIGHT, i18n("Quit") );
	
    ix += WIDTH;
    iy = 0;

    backdrop = new QFrame(this);
    backdrop->setGeometry(ix,iy,SBARWIDTH -2, 2* HEIGHT + HEIGHT /2 -1);
    backdrop->setFocusPolicy ( QWidget::NoFocus );

    int D = 6;

    //ix += 2 * SBARWIDTH / 7;
    ix = WIDTH + 8;
    /*
      nLEDs = new QLabel(this);
      nLEDs->setGeometry(ix + 20,iy +5 + D, 100, 30);
      nLEDs->setFont( QFont( "Helvetica", 26, QFont::Bold ) );
    */

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
    artistlabel->setFont( QFont( "helvetica", 10, QFont::Bold) );
    artistlabel->setAlignment( AlignLeft );
    artistlabel->setText("");

    titlelabel = new QLabel(this);
    titlelabel->setGeometry(WIDTH + 5, iy + 50 , SBARWIDTH -15, 13);
    QFont ledfont( "Helvetica", 8, QFont::Bold );
    //    KGlobal::charsets()->setQFont(ledfont);
    titlelabel->setFont( ledfont );
    titlelabel->setAlignment( AlignLeft );
    titlelabel->setText("");

    statuslabel = new QLabel(this);
    statuslabel->setGeometry(WIDTH + 110, iy  +D, 50, 14);
    statuslabel->setFont( ledfont );
    statuslabel->setAlignment( AlignLeft );
    //	statuslabel->setText("Ready");

    queryled = new LedLamp(this);
    queryled->move(WIDTH + 200, iy  +D +1 );
    queryled->off();
    queryled->hide();

    loopled = new LedLamp(this, LedLamp::Loop);
    loopled->move(WIDTH + 200, iy  +D +18 );
    loopled->off();
    //    loopled->hide();

    volumelabel = new QLabel(this);
    volumelabel->setGeometry(WIDTH + 110, iy + 14 + D, 50, 14);
    volumelabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
    volumelabel->setAlignment( AlignLeft );
    volumelabel->setText(i18n("Vol: --"));

    tracklabel = new QLabel(this);
    tracklabel->setGeometry(WIDTH + 168, iy + 14 +D, 50, 14);
    tracklabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
    tracklabel->setAlignment( AlignLeft );
    tracklabel->setText("--/--");

    totaltimelabel = new QLabel(this);
    totaltimelabel->setGeometry(WIDTH + 168, iy  +D, 50, 14);
    totaltimelabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
    totaltimelabel->setAlignment( AlignLeft );
    totaltimelabel->hide();

    /*
      trackTimeLED = new QLEDNumber( this );
      trackTimeLED->setGeometry( ix -10, iy+5, 100    , 2 * HEIGHT );// petit
      trackTimeLED->display("");
      trackTimeLED->setFrameStyle( QFrame::NoFrame );
    */

    ix = WIDTH;
    iy = HEIGHT + HEIGHT + HEIGHT/2;
    // Volume control here

    volSB = new QSlider( 0, 100, 5,  50, QSlider::Horizontal, this, "Slider" );
    volSB->setGeometry( ix, iy, SBARWIDTH, HEIGHT/2 );
    volSB->setFocusPolicy ( QWidget::NoFocus );

    iy += HEIGHT/2  +1 ;
    cddbbutton = new QPushButton( this );
    cddbbutton->setGeometry( ix , iy, SBARWIDTH/10 *2 , HEIGHT );
    cddbbutton->setFont( QFont( "helvetica", 12 ) );
    cddbbutton->setFocusPolicy ( QWidget::NoFocus );

    ix += SBARWIDTH/10*2;
    shufflebutton = new QPushButton( this );
    shufflebutton->setGeometry( ix , iy, SBARWIDTH/10 *2  , HEIGHT );
    shufflebutton->setFont( QFont( "helvetica", 12 ) );
    shufflebutton->setFocusPolicy ( QWidget::NoFocus );

    ix += SBARWIDTH/10*2;

    optionsbutton = new QPushButton( this );
    optionsbutton->setGeometry( ix, iy, SBARWIDTH/10 *2  , HEIGHT );
    optionsbutton->setFont( QFont( "helvetica", 12 ) );
    optionsbutton->setFocusPolicy ( QWidget::NoFocus );

    ix += SBARWIDTH/10*2;
    songListCB = new QComboBox( this );
    songListCB->setGeometry( ix, iy, SBARWIDTH/10*4, HEIGHT );
    songListCB->setFont( QFont( "helvetica", 12 ) );
    songListCB->setFocusPolicy ( QWidget::NoFocus );

    debug("Width %d Height %d\n",WIDTH, HEIGHT);

    iy = 0;
    ix = WIDTH + SBARWIDTH;
    playPB = makeButton( ix, iy, WIDTH, HEIGHT, "Play/Pause" );

    iy += HEIGHT;
    stopPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Stop" );

    ix += WIDTH / 2;
    replayPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Replay" );

    ix = WIDTH + SBARWIDTH;
    iy += HEIGHT;
    bwdPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Bwd" );

    ix += WIDTH / 2;
    fwdPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Fwd" );

    ix = WIDTH + SBARWIDTH;
    iy += HEIGHT;
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

    // This is UGLY .... -- Bernd
    // dockPB->setFont(QFont("helvetica", 12, QFont::Bold));
    // dockPB->setText("DOCK");

    aboutPB->setPixmap( aboutBmp );
    shufflebutton->setPixmap( shuffleBmp );
    cddbbutton->setPixmap( databaseBmp );
    optionsbutton->setPixmap( optionsBmp );
} // loadBitmaps


void 
KSCD::setupPopups()
{
    mainPopup   = new QPopupMenu ();
    perfPopup   = new QPopupMenu ();
    purchPopup   = new QPopupMenu ();
    infoPopup   = new QPopupMenu ();

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

    mainPopup->insertItem (i18n("Performances"), perfPopup);
    connect( perfPopup, SIGNAL(activated(int)), SLOT(performances(int)) );

    mainPopup->insertItem (i18n("Purchases"), purchPopup);
    connect( purchPopup, SIGNAL(activated(int)), SLOT(purchases(int)) );

    mainPopup->insertItem (i18n("Information"), infoPopup);

#ifdef KSCDMAGIC
    mainPopup->insertSeparator(-1);
    mainPopup->insertItem (i18n("KSCD Magic"));
    connect( mainPopup, SIGNAL(activated(int)), SLOT(magicslot(int)) );
#endif

    connect( infoPopup, SIGNAL(activated(int)), SLOT(information(int)) );
    connect( infoPB, SIGNAL(clicked()), SLOT(showPopup()) );
} // setupPopups

void 
KSCD::showPopup()
{
    QPoint point = this->mapToGlobal (QPoint(0,0));

    if(mainPopup)
        mainPopup->popup(point);

} // showPopup

void 
KSCD::setToolTips()
{
    if(tooltips)
      {	
        QToolTip::add( playPB, 		i18n("Play/Pause") );
        QToolTip::add( stopPB, 		i18n("Stop") );
        QToolTip::add( replayPB, 	i18n("Loop") );
        QToolTip::add( songListCB, 	i18n("Track Selection") );
        QToolTip::add( fwdPB, 		i18n("30 Secs Forward") );
        QToolTip::add( bwdPB, 		i18n("30 Secs Backward") );
        QToolTip::add( nextPB, 		i18n("Next Track") );
        QToolTip::add( prevPB, 		i18n("Previous Track") );
        QToolTip::add( dockPB, 		i18n("Quit Kscd") );
        QToolTip::add( aboutPB, 	i18n("Cycle Time Display") );
        QToolTip::add( optionsbutton, 	i18n("Configure Kscd") );
        QToolTip::add( ejectPB, 	i18n("Eject CD") );
        QToolTip::add( infoPB, 	        i18n("The Artist on the Web") );
	if (!randomonce)
	  QToolTip::add( shufflebutton, 	i18n("Random Play") );
	else
	  QToolTip::add( shufflebutton, 	i18n("Shuffle Play") );

        QToolTip::add( cddbbutton, 	i18n("CDDB Dialog") );
        QToolTip::add( volSB, 		i18n("CD Volume Control") );
      } else {
        QToolTip::remove( playPB);
        QToolTip::remove( stopPB);
        QToolTip::remove( replayPB);
        QToolTip::remove( songListCB);
        QToolTip::remove( fwdPB );
        QToolTip::remove( bwdPB);
        QToolTip::remove( nextPB );
        QToolTip::remove( prevPB );
        QToolTip::remove( dockPB );
        QToolTip::remove( aboutPB );
        QToolTip::remove( optionsbutton );
        QToolTip::remove( ejectPB );
        QToolTip::remove( infoPB );
        QToolTip::remove( cddbbutton );
        QToolTip::remove( shufflebutton );
        QToolTip::remove( volSB );
    }
} // setToolTips

void 
KSCD::cleanUp()
{
    if (thiscd.trk != NULL)
        delete thiscd.trk;
    signal (SIGINT, SIG_DFL);
    if(magicproc)
      {
        delete magicproc;
        magicproc = 0L;
      }
} // cleanUp()

void 
KSCD::playClicked()
{
    qApp->processEvents();
    qApp->flushX();

    if(
#ifdef NEW_BSD_PLAYCLICKED
        cur_cdmode == STOPPED || cur_cdmode == UNKNOWN  || cur_cdmode == BACK
#else
        cur_cdmode == STOPPED || cur_cdmode == UNKNOWN
#endif
    ){

        statuslabel->setText( i18n("Playing") );
        songListCB->clear();
        setLEDs( "00:00" );
	
        int i = 0;
        songListCB->clear();
        char *ptr = tracktitlelist.first();
        for(ptr = tracktitlelist.next(); ptr != 0; ptr = tracktitlelist.next())
	  {
            i++;
            songListCB->insertItem( QString("").sprintf("%02d: %s", i, ptr));
	  }
        for(; i < cur_ntracks; i++)
	  {
            songListCB->insertItem( QString("").sprintf(i18n("%02d: <Unknown>").ascii(), i+1));
	  }


//        for (int i = 0; i < cur_ntracks; i++){
//            songListCB->insertItem( QString( 0 ).sprintf( i18n("Track %02d"), i + 1 ) );
//        }

        qApp->processEvents();
        qApp->flushX();

        if(playlist.count()>0)
	  {
            if(playlistpointer >=(int) playlist.count())
	      playlistpointer = 0;
            play_cd (atoi(playlist.at(playlistpointer)), 0,
                     atoi(playlist.at(playlistpointer)) + 1);
            save_track = cur_track = atoi(playlist.at(playlistpointer));
	  } else {
            play_cd (save_track, 0, cur_ntracks + 1);
	  }
    } else { // if (STOPPED||UNKNOWN)
      if (cur_cdmode == PLAYING || cur_cdmode == PAUSED) 
	{
	
            switch (cur_cdmode) 
	      {
	      case PLAYING:
                statuslabel->setText( i18n("Pause") );
                break;
	      case PAUSED:
                if(randomplay)
		  {
		    if (randomonce)
		      statuslabel->setText( i18n("Shuffle") );
		    else
		      statuslabel->setText( i18n("Random") );
		  } else {
                    statuslabel->setText( i18n("Playing") );
		  }
                break;
	
	      default:
                statuslabel->setText( i18n("Strange....") );
                break;
	      } // switch
            pause_cd ();
            qApp->processEvents();
            qApp->flushX();
        } // if (PLAYING||PAUSED)
    } // if (STOPPED||UNKNOWN) else
    cdMode();
} // playClicked()

void 
KSCD::stopClicked()
{

  //    looping = FALSE;
    randomplay = FALSE;
    stoppedByUser = TRUE;
    statuslabel->setText(i18n("Stopped"));
    setLEDs("--:--");
    qApp->processEvents();
    qApp->flushX();

    save_track = cur_track = 1;
    playlistpointer = 0;
    stop_cd ();

} // stopClicked()

void 
KSCD::prevClicked()
{
    setLEDs("00:00");
    qApp->processEvents();
    qApp->flushX();

    if(playlist.count()>0)
      {
        playlistpointer--;
        if(playlistpointer < 0 )
	  playlistpointer = playlist.count() -1;
        cur_track = atoi(playlist.at(playlistpointer));
      } else {
        cur_track--;
        if (cur_track < 1)
	  cur_track = cur_ntracks;
      }
    if(randomplay)
      {
        play_cd (cur_track, 0, cur_track + 1);
      } else {
        play_cd (cur_track, 0, cur_ntracks + 1);
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
        QString str;
        str.sprintf("%02d/%02d",j,cd->ntracks );
        tracklabel->setText(str);

        if(j < (int)tracktitlelist.count())
	  {
            titlelabel->setText(tracktitlelist.at(j));
            artistlabel->setText(tracktitlelist.at(0));
	  }
        qApp->processEvents();
        qApp->flushX();
	
        play_cd( j, 0, j + 1 );
	
      } else if(playlist.count() > 0)
	{
	  if(playlistpointer < (int)playlist.count() - 1)
            playlistpointer++;
	  else
            playlistpointer = 0;
	  
	  play_cd (atoi(playlist.at(playlistpointer)),
		   0, atoi(playlist.at(playlistpointer)) + 1);
	  cur_track = atoi(playlist.at(playlistpointer));	  
	} else {
	  if (cur_track == cur_ntracks)
            cur_track = 0;
	  play_cd (cur_track + 1, 0, cur_ntracks + 1);
	}
} // nextClicked()

void 
KSCD::fwdClicked()
{
    qApp->processEvents();
    qApp->flushX();

    if (cur_cdmode == PLAYING) 
      {
        tmppos = cur_pos_rel + 30;
        if (tmppos < thiscd.trk[cur_track - 1].length) 
	  {
            if(randomplay || playlist.count() > 0)
	      play_cd (cur_track, tmppos, cur_track + 1);		
            else
	      play_cd (cur_track, tmppos, cur_ntracks + 1);
	  }
      }
} // fwdClicked()

void 
KSCD::bwdClicked()
{
    qApp->processEvents();
    qApp->flushX();

    if (cur_cdmode == PLAYING) 
      {
        tmppos = cur_pos_rel - 30;
        if(randomplay || playlist.count() > 0)
	  play_cd (cur_track, tmppos > 0 ? tmppos : 0, cur_track + 1);
        else
	  play_cd (cur_track, tmppos > 0 ? tmppos : 0, cur_ntracks + 1);
      }
    cdMode();
} // bwdClicked()

void 
KSCD::quitClicked()
{
    quitPending = true;
    randomplay = FALSE;
    statuslabel->setText("");
    setLEDs( "--:--" );

    qApp->processEvents();
    qApp->flushX();

    if(stopexit)
        stop_cd ();

    cd_status();
    cd_status();

    cleanUp();
    writeSettings();
    qApp->quit();
    //e->accept();

} // quitClicked()


void 
KSCD::closeEvent( QCloseEvent *e )
{
    quitPending = true;
    randomplay = FALSE;

    statuslabel->setText("");

    setLEDs( "--:--" );

    qApp->processEvents();
    qApp->flushX();

    if(stopexit)
	stop_cd ();

    cd_status();
    cd_status();
    cleanUp();
    writeSettings();
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
  //    randomplay = FALSE;

    if(looping)
      {
	loopOff();
      } else {
	loopOn();
      }
} // loopClicked

void 
KSCD::ejectClicked()
{
  if(!cddrive_is_ok)
    return;
  if(!currentlyejected)
    {
      //      looping = FALSE;
      randomplay = FALSE;
      statuslabel->setText(i18n("Ejecting"));
      qApp->processEvents();
      qApp->flushX();
      artistlabel->setText("");
      titlelabel->setText("");
      tracktitlelist.clear();
      extlist.clear();
      
      stop_cd();
      //  timer->stop();
      /*
       * new checkmount goes here
       *
       */
      eject_cd();
    } else {
      statuslabel->setText(i18n("Closing"));
      cd_closetray();
      cd_status();
    }
} // ejectClicked

void 
KSCD::randomSelected()
{
    if(randomplay == TRUE)
      {
        randomplay = FALSE;
      } else {
	if( randomonce )
	  {
	    statuslabel->setText(i18n("Shuffle"));
	  } else {
	    statuslabel->setText(i18n("Random"));
	  }

        randomplay = TRUE;
	
	make_random_list(); /* koz: Build a unique, once, random list */
        int j = randomtrack();
        QString str;
        str.sprintf("%02d/%02d",j,cd->ntracks );
        tracklabel->setText(str);
        if(j < (int)tracktitlelist.count())
	  {
            titlelabel->setText(tracktitlelist.at(j));
            artistlabel->setText(tracktitlelist.at(0));
	  }
        qApp->processEvents();
        qApp->flushX();
	
        play_cd( j, 0, j + 1 );
        cur_track = j;
      }
} // randomSelected

void 
KSCD::trackSelected( int trk )
{
    randomplay = false;
    QString str;
    str.sprintf("%02d/%02d",trk + 1,cd->ntracks );
    tracklabel->setText(str);

    if(trk+1 < (int)tracktitlelist.count())
      {
        titlelabel->setText(tracktitlelist.at(trk+1));
        artistlabel->setText(tracktitlelist.at(0));
      }
    
    setLEDs("00:00");
    qApp->processEvents();
    qApp->flushX();
    
    cur_track = trk + 1;
    //  pause_cd();
    play_cd( cur_track, 0, cur_ntracks + 1 );
} // trackSelected

void 
KSCD::aboutClicked()
{
    QString server_copy;
    server_copy = current_server;

    QTabDialog * tabdialog;

    tabdialog = new QTabDialog(this,"tabdialog",TRUE);
    tabdialog->setCaption( i18n("kscd Configuraton") );
    tabdialog->resize(559, 512);
    tabdialog->setCancelButton( i18n("Cancel") );

    QWidget *about = new QWidget(tabdialog,"about");

    QGroupBox *box = new QGroupBox(about,"box");
    QLabel  *label = new QLabel(box,"label");

    box->setGeometry(10,10,520,420);

    label->setGeometry(160,20,340,390);
    label->setAlignment( AlignCenter);
    QString labelstring;
    labelstring = i18n("kscd %1\n").arg(KSCDVERSION);
    labelstring += i18n(
    "Copyright (c) 1997-1998 \nBernd Johannes Wuebben <wuebben@kde.org>\n"
    "Copyright (c) 1999-2000 \nDirk Försterling <milliByte@gmx.net>\n"
    "   (current Maintainer)\n\n"
    "Kscd is based on WorkMan,\n"
                                      "Copyright (c) 1991-1996 Steven Grimm\n"
                                      "Copyright (c) 1996-2000 Dirk Försterling <milliByte@gmx.net>\n\n"
                                      "Special thanks to Ti Kan and "
                                      "Steve Scherf, the inventors of "
                                      "the CDDB database concept. "
                                      "Visit http://www.cddb.com/ for "
                                      "more information on CDDB.\n\n"
                                     );

#ifdef KSCDMAGIC
     labelstring += i18n(
        "KSCD Magic based on Synaesthesia by "
        "Paul Harrison <pfh@yoyo.cc.monash.edu.au>\n\n");
#endif
     labelstring += i18n(
        "Thanks to Vadim Zaliva <lord@crocodile.org>\n"
        "for his work on the http proxy code.\n\n") ;


    label->setAlignment(AlignLeft|WordBreak|ExpandTabs);
    label->setText(labelstring);

    QPixmap pm = UserIcon("kscdlogo");
    QLabel *logo = new QLabel(box);
    logo->setPixmap(pm);
    logo->setGeometry(40, 50, pm.width(), pm.height());

    ConfigDlg* dlg;
    struct configstruct config;
    config.background_color = background_color;
    config.led_color = led_color;
    config.tooltips = tooltips;
    config.cd_device = cd_device;
    config.mailcmd = mailcmd;
    config.browsercmd = browsercmd;
    config.use_kfm    = use_kfm;
    config.docking    = docking;
    config.autoplay   = autoplay;
    config.stopexit = stopexit;
    config.ejectonfinish = ejectonfinish;
    config.randomonce = randomonce;

    dlg = new ConfigDlg(tabdialog,&config,"configdialg");

    setup = new CDDBSetup(tabdialog,"cddbsetupdialog");
    connect(setup,SIGNAL(updateCDDBServers()),this,SLOT(getCDDBservers()));
    connect(setup,SIGNAL(updateCurrentServer()),this,SLOT(updateCurrentCDDBServer()));
    setup->insertData(cddbserverlist,
                      cddbsubmitlist,
		      cddbbasedir,
                      submitaddress,
                      current_server,
                      cddb_remote_enabled,
		      cddb.getTimeout(),
                      cddb.useHTTPProxy(),
                      cddb.getHTTPProxyHost(),
                      cddb.getHTTPProxyPort()
    );

    MGConfigDlg* mgdlg;
    struct mgconfigstruct mgconfig;
    mgconfig.width = magic_width;
    mgconfig.height = magic_height;
    mgconfig.brightness = magic_brightness;
    mgdlg = new MGConfigDlg(tabdialog,&mgconfig,"mgconfigdialg");

    smtpconfig = new SMTPConfig(tabdialog, "smtpconfig", &smtpConfigData);

    tabdialog->addTab(setup,"CDDB");
    tabdialog->addTab(smtpconfig, i18n("SMTP Setup"));
    tabdialog->addTab(dlg,i18n("Kscd Options"));
    tabdialog->addTab(mgdlg,i18n("Kscd Magic"));
    tabdialog->addTab(about,i18n("About"));



    if(tabdialog->exec() == QDialog::Accepted)
      {
        smtpconfig->commitData();
        background_color = dlg->getData()->background_color;
        led_color = dlg->getData()->led_color;
        tooltips = dlg->getData()->tooltips;
        mailcmd = dlg->getData()->mailcmd;

        browsercmd = dlg->getData()->browsercmd;
        use_kfm = dlg->getData()->use_kfm;
        docking = dlg->getData()->docking;
        autoplay = dlg->getData()->autoplay;
        stopexit = dlg->getData()->stopexit;
        ejectonfinish = dlg->getData()->ejectonfinish;
	randomonce = dlg->getData()->randomonce;

        if( (QString)cd_device != dlg->getData()->cd_device)
	  {
            cd_device_str = dlg->getData()->cd_device;
            cd_close();
            cd_device = cd_device_str.ascii();

	  }
        cddrive_is_ok = true;

        magic_width = mgdlg->getData()->width;
        magic_height = mgdlg->getData()->height;
        magic_brightness = mgdlg->getData()->brightness;

        bool cddb_proxy_enabled;
        QString cddb_proxy_host;
        unsigned short int cddb_proxy_port;
	unsigned short int cddb_timeout;

        setup->getData(cddbserverlist,
                       cddbsubmitlist,
                       cddbbasedir,
                       submitaddress,
                       current_server,
                       cddb_remote_enabled,
		       cddb_timeout,
                       cddb_proxy_enabled,
                       cddb_proxy_host,
                       cddb_proxy_port
        );
	cddb.setTimeout(cddb_timeout);
        cddb.setHTTPProxy(cddb_proxy_host,cddb_proxy_port);
        cddb.useHTTPProxy(cddb_proxy_enabled);

        setColors();
        setToolTips();

        if(docking)
	    dock_widget->show();
	else
	    dock_widget->hide();
	
      } else {
        // reset the current server in case we played with it ...
	
        current_server = server_copy;
        debug("RESETTING SERVER TO: %s\n",current_server.ascii());
      }
    
    disconnect(setup,SIGNAL(updateCDDBServers()),this,SLOT(getCDDBservers()));
    disconnect(setup,SIGNAL(updateCurrentServer()),this,SLOT(updateCurrentCDDBServer()));

    delete dlg;
    dlg = 0L;
    delete setup;
    setup = 0L;
    delete smtpconfig;
    smtpconfig = 0L;
    delete about;
    delete tabdialog;

} // aboutClicked()

void 
KSCD::updateCurrentCDDBServer()
{
    if(setup)
      {
        setup->getCurrentServer(current_server);
        debug("SET SERVER TO: %s\n",current_server.ascii());
      }
} // updateCurrentCDDBServer

void 
KSCD::volChanged( int vol )
{
    if(volstartup)
        return;

    QString str;
    str.sprintf(i18n("Vol: %02d%%").ascii(),vol);
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
  if( randomonce )
    {
      if( playlist.count() > 0 ) 
	{
	  /* Check to see if we are at the end of the list */
	  if( (unsigned int)random_current >= playlist.count() ) 
	    {
	      if( !looping )
		{
		  stopClicked();
		  return -1;
		} else {
		  random_current=0;
		}
	    }
	  int j = random_list[random_current++];
	  playlistpointer = j;
	  return( atoi( playlist.at(j) ) );
	} else { // playlist.count > 0
	  if( random_current >= cur_ntracks ) 
	    {
	      if( !looping )
		{
		  stopClicked();
		  return -1;
		} else {
		  random_current = 0;
		}
	    }
	  return( random_list[random_current++] );
	} // playlist.count > 0
    } // randomonce
  
  if( playlist.count() > 0)
    {
      int j;
      j=(int) (((double) playlist.count()  ) * rand()/(RAND_MAX+1.0));
      playlistpointer = j;
      return atoi(playlist.at(j));
    } else {
      int j;
      j=1+(int)(((double)cur_ntracks) *rand()/(RAND_MAX+1.0));
      return j;
    }
} // randomtrack
  
/*
 * cdMode
 *
 * - 'No disc' handling is missing
 * - Data discs not recognized as data discs.
 * 
 */
void 
KSCD::cdMode()
{
    static char *p = new char[10];
    static bool damn = TRUE;
    QString str;

    sss = cd_status();

    if(sss < 0)
      {
        if(cddrive_is_ok)
	  {
            statuslabel->setText( i18n("Error") );
            cddrive_is_ok = false;
            QString errstring =
              i18n("CDROM read or access error (or no audio disc in drive).\n"\
                   "Please make sure you have access permissions to:\n%1")
              .arg(cd_device);
            KMessageBox::error(this, errstring, i18n("Error"));
	  }
        return;
      }
    cddrive_is_ok = true; // cd drive ok

    if(cur_cdmode == 5)
        currentlyejected = true;
    else
        currentlyejected = false;

    switch (cur_cdmode) {
    case -1:         /* UNKNOWN */
        cur_track = save_track = 1;
        statuslabel->setText( "" ); // TODO how should I properly handle this
        damn = TRUE;
        break;

    case 0:         /* TRACK DONE */
        if( randomplay )
	  {
            int j = randomtrack();
            play_cd( j, 0, j + 1 );
	    
	  }
        else if (playlist.count() > 0)
	  {
	    if(playlistpointer < (int)playlist.count() - 1)
	      playlistpointer++;
            else
	      playlistpointer = 0;
            play_cd(atoi(playlist.at(playlistpointer)),0,atoi(playlist.at(playlistpointer))+1);
	  }
        else if ( looping )
	  {
            if (cur_track == cur_ntracks)
	      {
                cur_track = 0;
                play_cd (1, 0, cur_ntracks + 1);
	      }

	  } else {
            cur_track = save_track = 1;
            statuslabel->setText( "" ); // TODO how should I properly handle this
            damn = TRUE;
	  }
        break;

    case 1:         /* PLAYING */
        playtime ();
        if(randomplay)
	  if(randomonce) 
	    {
	      statuslabel->setText( i18n("Shuffle") );
	    } else {
	      statuslabel->setText( i18n("Random") );
	    }
	//        else if(looping)
	//            statuslabel->setText( i18n("Loop") );
        else
            statuslabel->setText( i18n("Playing") );

        sprintf( p, "%02d  ", cur_track );
        if(songListCB->count() == 0)
	  {
            // we are in here when we start kscd and
            // the cdplayer is already playing.	
            int i = 0;
            songListCB->clear();
            char *ptr = tracktitlelist.first();
            for(ptr = tracktitlelist.next(); ptr != 0; ptr = tracktitlelist.next())
	      {
                i++;
                songListCB->insertItem( QString("").sprintf("%02d: %s", i, ptr));
	      }
            for(; i < cur_ntracks; i++)
	      {
                songListCB->insertItem( QString("").sprintf(i18n("%02d: <Unknown>").ascii(), i+1));
	      }

//            for (int i = 0; i < cur_ntracks; i++){
//                songListCB->insertItem( QString( 0 ).sprintf(i18n("Track %02d").ascii(),i + 1 ) );
//            }
            songListCB->setCurrentItem( cur_track - 1 );
            have_new_cd = false;
            get_cddb_info(false); // false == do not update dialog if open
        } else {
	  songListCB->setCurrentItem( cur_track - 1 );
        }
        str.sprintf("%02d/%02d",cur_track,cd->ntracks );
        tracklabel->setText(str);
	
        if((cur_track < (int)tracktitlelist.count()) && (cur_track >= 0))
	  {
            titlelabel->setText(tracktitlelist.at(cur_track));
            artistlabel->setText(tracktitlelist.at(0));
	  }

        setLEDs( tmptime );
        damn = TRUE;
        stoppedByUser = false;
        break;

    case 2: /*FORWARD*/
        break;

    case 3:         /* PAUSED */
        statuslabel->setText( i18n("Pause") );
        damn = TRUE;
        break;

    case 4:         /* STOPPED */
        if (damn) {
            if(ejectonfinish && !stoppedByUser){
                stoppedByUser = true;
                ejectClicked();
                break;
            }
            statuslabel->setText( i18n("Ready") );
            setLEDs( "--:--" );
            songListCB->clear();

            int i = 0;
            songListCB->clear();
            char *ptr = tracktitlelist.first();
            for(ptr = tracktitlelist.next(); ptr != 0; ptr = tracktitlelist.next()){
                i++;
                songListCB->insertItem( QString("").sprintf("%02d: %s", i, ptr));
            }
            for(; i < cur_ntracks; i++){
                songListCB->insertItem( QString("").sprintf(i18n("%02d: <Unknown>").ascii(), i+1));
            }

//            for (i = 0; i < cur_ntracks; i++)
//                songListCB->insertItem( QString( 0 ).sprintf( i18n("Track %02d"), i + 1 ) );

            int w = ((cur_track >= 0) ? cur_track : 1);

            str.sprintf("%02d/%02d",cur_track >=0? cur_track :1 ,cd->ntracks );
            tracklabel->setText(str);

            if( w < (int)tracktitlelist.count()){
                titlelabel->setText(tracktitlelist.at( w ));
                artistlabel->setText(tracktitlelist.at(0));
            }
        }
        damn = FALSE;
        if(have_new_cd){

            //      timer->stop();
            have_new_cd = false;
            // timer must be restarted when we are doen
            // with getting the cddb info
            get_cddb_info(false); // false == do not update dialog if open
            if(autoplay && ejectedBefore)
                playClicked();
        }

        break;

    case 5:         /* CDEJECT */
        statuslabel->setText( i18n("Ejected") );
        songListCB->clear();
        setLEDs( "--:--" );	
        tracklabel->setText( "--/--" );
        titlelabel->setText("");
        artistlabel->setText("");
        totaltimelabel->setText("");
        totaltimelabel->lower();
        damn = TRUE;
        ejectedBefore = TRUE;

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

    titlelabel ->setFont( QFont( "Helvetica", 10, QFont::Bold) );
    artistlabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
    volumelabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
    statuslabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
    tracklabel ->setFont( QFont( "Helvetica", 10, QFont::Bold) );
    totaltimelabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
    artistlabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );

}


void 
KSCD::readSettings()
{
    config = kapp->config();

    config->setGroup("GENERAL");
    volume     = config->readNumEntry("Volume",40);
    tooltips   = config->readBoolEntry("ToolTips", true);
    randomplay = config->readBoolEntry("RandomPlay", false);
    use_kfm    = config->readBoolEntry("USEKFM", true);
    docking    = config->readBoolEntry("DOCKING", true);
    autoplay = config->readBoolEntry("AUTOPLAY", false);
    stopexit = config->readBoolEntry("STOPEXIT", true);
    ejectonfinish = config->readBoolEntry("EJECTONFINISH", false);
    mailcmd    =        config->readEntry("UnixMailCommand","/bin/mail -s \"%s\"");
    randomonce = (bool)config->readBoolEntry("RANDOMONCE",true);


#ifdef DEFAULT_CD_DEVICE

    // sun ultrix etc have a canonical cd rom device specified in the
    // respective plat_xxx.c file. On those platforms you need nnot
    // specify the cd rom device and DEFAULT_CD_DEVICE is not defined
    // in config.h

    cd_device_str = config->readEntry("CDDevice",DEFAULT_CD_DEVICE);
    cd_device = cd_device_str.ascii();

#endif


    QColor defaultback = black;
    QColor defaultled = QColor(226,224,255);
    background_color = config->readColorEntry("BackColor",&defaultback);	
    led_color = config->readColorEntry("LEDColor",&defaultled);

    config->setGroup("MAGIC");
    magic_width      = config->readNumEntry("magicwidth",320);
    magic_height     = config->readNumEntry("magicheight",200);
    magic_brightness = config->readNumEntry("magicbrightness", 3);

    config->setGroup("SMTP");
    smtpConfigData.enabled = config->readBoolEntry("enabled", true);
    smtpConfigData.serverHost = config->readEntry("serverHost", "localhost");
    smtpConfigData.serverPort = config->readEntry("serverPort", "25");
    smtpConfigData.senderAddress = config->readEntry("senderAddress", "someone@somewhere.org");

    config->setGroup("CDDB");

    cddb.setTimeout(config->readNumEntry("CDDBTimeout",60));

    cddbbasedir = config->readEntry("LocalBaseDir");
    if (!cddbbasedir.isNull())
	KGlobal::dirs()->addResourceDir("cddb", cddbbasedir);

// Set this to false by default. Look at the settings dialog source code
// for the reason. - Juraj.
    cddb_remote_enabled = config->readBoolEntry("CDDBRemoteEnabled",
						false);
    cddb.useHTTPProxy(config->readBoolEntry("CDDBHTTPProxyEnabled",
					    false));
    cddb.setHTTPProxy(config->readEntry("HTTPProxyHost",""),
                      config->readNumEntry("HTTPProxyPort",0));

    current_server = config->readEntry("CurrentServer",DEFAULT_CDDB_SERVER);
    //Let's check if it is in old format and if so, convert it to new one:
    if(CDDB::normalize_server_list_entry(current_server))
    {
        debug("Default CDDB server entry converted to new format and saved.\n");
        config->writeEntry("CurrentServer",current_server);
        config->sync();
    }
    submitaddress = config->readEntry("CDDBSubmitAddress","xmcd-cddb@amb.org");
    int num;
    num = config->readListEntry("SeverList",cddbserverlist,',');
    if (num == 0)
        cddbserverlist.append(DEFAULT_CDDB_SERVER);
    else
    {
        //Let's check if it is in old format and if so, convert it to new one:
        bool needtosave=false;
        QStrList nlist;
        for(QString entry=cddbserverlist.first();
            entry != 0;
            entry=cddbserverlist.next())
        {
            needtosave|=CDDB::normalize_server_list_entry(entry);
            nlist.append(entry.ascii());
        }
        if(needtosave)
        {
            // I have to recreate list because of sytange behaviour of sync()
            // function in configuration - it does not notice if QStrList
            // String contents is changed.
            cddbserverlist=nlist;
            debug("CDDB server list converted to new format and saved.\n");
            config->writeEntry("SeverList",cddbserverlist,',',true);
            config->sync();
        }
    }
    num = config->readListEntry("SubmitList", cddbsubmitlist, ',');
    if(!num){
        cddbsubmitlist.append(DEFAULT_SUBMIT_EMAIL);
        cddbsubmitlist.append(DEFAULT_TEST_EMAIL);
    }
}

void 
KSCD::writeSettings()
{		
    config = kapp->config();

    config->setGroup("GENERAL");	
    config->writeEntry("ToolTips", tooltips);
    config->writeEntry("RandomPlay", randomplay);
    config->writeEntry("USEKFM", use_kfm);
    config->writeEntry("DOCKING", docking);
    config->writeEntry("AUTOPLAY", autoplay);
    config->writeEntry("STOPEXIT", stopexit);
    config->writeEntry("EJECTONFINISH", ejectonfinish);
    config->writeEntry("RANDOMONCE", randomonce);
    config->writeEntry("CDDevice",cd_device);
    config->writeEntry("CustomBroserCmd",browsercmd);
    config->writeEntry("Volume", volume);
    config->writeEntry("BackColor",background_color);
    config->writeEntry("LEDColor",led_color);
    config->writeEntry("UnixMailCommand",mailcmd);

    config->setGroup("SMTP");
    config->writeEntry("enabled", smtpConfigData.enabled);
    config->writeEntry("serverHost", smtpConfigData.serverHost);
    config->writeEntry("serverPort", smtpConfigData.serverPort);
    config->writeEntry("senderAddress", smtpConfigData.senderAddress);

    config->setGroup("CDDB");
    config->writeEntry("CDDBRemoteEnabled",cddb_remote_enabled);
    config->writeEntry("CDDBTimeout",cddb.getTimeout());
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

    config->sync();
}

void 
KSCD::CDDialogSelected()
{
    if(cddialog)
        return;

    cddialog = new CDDialog();

    cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
                      revision,playlist,pathlist,mailcmd,submitaddress, &smtpConfigData);

    connect(cddialog,SIGNAL(cddb_query_signal(bool)),this,SLOT(get_cddb_info(bool)));
    connect(cddialog,SIGNAL(dialog_done()),this,SLOT(CDDialogDone()));
    connect(cddialog,SIGNAL(play_signal(int)),this,SLOT(trackSelected(int)));
    cddialog->show();
}

void 
KSCD::CDDialogDone()
{
    disconnect(cddialog,SIGNAL(play_signal(int)),this,SLOT(trackSelected(int)));
    disconnect(cddialog,SIGNAL(dialog_done()),this,SLOT(CDDialogDone()));
    disconnect(cddialog,SIGNAL(cddb_query_signal(bool)),this,SLOT(get_cddb_info(bool)));
    delete cddialog;
    cddialog = 0L;

}


void 
KSCD::getCDDBservers()
{
    led_on();

    // Get current server and proxy settings from CDDB setup dialog befoe proceed
    bool    cddb_proxy_enabled;
    QString cddb_proxy_host;
    unsigned short int cddb_proxy_port;
    unsigned short int cddb_timeout;

    setup->getData(cddbserverlist,
                   cddbsubmitlist,
                   cddbbasedir,
                   submitaddress,
                   current_server,
                   cddb_remote_enabled,
		   cddb_timeout,
                   cddb_proxy_enabled,
                   cddb_proxy_host,
                   cddb_proxy_port
    );
    cddb.setTimeout(cddb_timeout);
    cddb.setHTTPProxy(cddb_proxy_host,cddb_proxy_port);
    cddb.useHTTPProxy(cddb_proxy_enabled);

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
    titlelabel->setText(i18n("Unable to get CDDB server list."));
    artistlabel->setText("");
    titlelabeltimer->start(10000,TRUE); // 10 secs
}

void 
KSCD::getCDDBserversDone()
{

    led_off();
    disconnect(&cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
    disconnect(&cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));
    cddb.serverList(cddbserverlist);
    if(setup)
        setup->insertServerList(cddbserverlist);
}

void 
KSCD::get_cddb_info(bool _updateDialog)
{
    static int connected = 0;

    updateDialog = _updateDialog;

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

    if(!connected){
        connect(&cddb,SIGNAL(cddb_ready()),this,SLOT(cddb_ready()));
        connect(&cddb,SIGNAL(cddb_failed()),this,SLOT(cddb_failed()));
        connect(&cddb,SIGNAL(cddb_done()),this,SLOT(cddb_done()));
        connect(&cddb,SIGNAL(cddb_timed_out()),this,SLOT(cddb_timed_out()));
        connect(&cddb,SIGNAL(cddb_inexact_read()),this,SLOT(mycddb_inexact_read()));
        connect(&cddb,SIGNAL(cddb_no_info()),this,SLOT(cddb_no_info()));
        connected = 1;
    }
    led_on();

    bool res = cddb.local_query(
        cd->magicID,
        xmcd_data,
        tracktitlelist,
        extlist,
        category,
        discidlist,
        revision,
        playlist
    );

    if(!res && !cddb_remote_enabled){
        //    have_new_cd = false;
        cddb_no_info();
        return;
    }

    if(!res){

        debug("STARTING REMOTE QUERY\n");
        cddb.cddb_connect(current_server);
    }
    else{
        debug("FOUND RECORD LOCALLY\n");
        if((int)tracktitlelist.count() != (cd->ntracks + 1)){
                debug("WARNING LOCAL QUERY tracktitleslist.count = %d != cd->ntracks +1 = %d\n",
                tracktitlelist.count(),cd->ntracks + 1
            );
        }

        if((int)extlist.count() != (cd->ntracks + 1)){
            debug("WARNING LOCAL QUERYextlist.count = %d != cd->ntracks +1 = %d\n",
                                  extlist.count(),cd->ntracks + 1);
        }

        if(tracktitlelist.count() > 1){
            titlelabel->setText(tracktitlelist.at(1));
            artistlabel->setText(tracktitlelist.at(0));
        }

        int i = 0;
        songListCB->clear();
        char *ptr = tracktitlelist.first();
        for(ptr = tracktitlelist.next(); ptr != 0; ptr = tracktitlelist.next()){
            i++;
            songListCB->insertItem( QString("").sprintf("%02d: %s", i, ptr));
        }
        for(; i < cur_ntracks; i++){
            songListCB->insertItem( QString("").sprintf(i18n("%02d: <Unknown>").ascii(), i+1).ascii());
        }


        led_off();
        timer->start(1000);

        if(cddialog && updateDialog)
            cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
                              revision,playlist,pathlist,mailcmd,submitaddress, &smtpConfigData);
        playlistpointer = 0;
    }

    //  have_new_cd = false;

} // get_cddb_info

int cddb_ready_bug = 0;
void 
KSCD::cddb_ready()
{
    debug("cddb_ready() called\n");

    if(!cd)
        return;
/*    if(cddb_ready_bug)
        return;
    cddb_ready_bug = 1;
*/
    querylist.clear();
    tracktitlelist.clear();
    extlist.clear();
    discidlist.clear();

    QString num;

    for(int i = 0 ; i < cd->ntracks; i++){
        querylist.append(num.setNum(cd->cddbtoc[i].absframe).ascii());
    }

    querylist.append(num.setNum(cd->cddbtoc[cd->ntracks].absframe/75).ascii());
    cddb_inexact_sentinel =false;
    cddb.queryCD(cd->magicID,querylist);
} // cddb_ready

void 
KSCD::cddb_no_info()
{
    //        cddb_ready_bug = 0;
    debug("cddb_no_info() called\n");

    titlelabel->setText(i18n("No matching CDDB entry found."));
//    artistlabel->setText("");
//    titlelabeltimer->start(10000,TRUE); // 10 secs

    tracktitlelist.clear();
    tracktitlelist.append(i18n("No matching CDDB entry found.").ascii());
    for(int i = 0 ; i < cd->ntracks; i++)
        tracktitlelist.append("");

    extlist.clear();
    for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

    discidlist.clear();

    timer->start(1000);
    led_off();
    cddb_inexact_sentinel =false;
//    cddb_error = 1;
} // cddb_no_info

void 
KSCD::cddb_failed()
{
    // TODO differentiate between those casees where the communcition really
    // failed and those where we just couldn't find anything
    //        cddb_ready_bug = 0;
    debug("cddb_failed() called\n");
    tracktitlelist.clear();
    tracktitlelist.append(i18n("Error getting CDDB entry.").ascii());
    for(int i = 0 ; i < cd->ntracks; i++)
        tracktitlelist.append("");

    extlist.clear();
    for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

    discidlist.clear();

    titlelabel->setText(i18n("Error getting CDDB entry."));
    artistlabel->setText("");
//    titlelabeltimer->start(10000,TRUE); // 10 secs

    timer->start(1000);
    led_off();
    cddb_inexact_sentinel =false;
//    cddb_error = 1;
} // cddb_failed

void 
KSCD::cddb_timed_out()
{
    //    cddb_ready_bug = 0;
    debug("cddb_timed_out() called\n");
    tracktitlelist.clear();
    tracktitlelist.append(i18n("CDDB query timed out.").ascii());
    for(int i = 0 ; i <= cd->ntracks; i++)
        tracktitlelist.append("");

    extlist.clear();
    for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

    discidlist.clear();

    titlelabel->setText(i18n("CDDB query timed out."));
    artistlabel->setText("");
//    titlelabeltimer->start(10000,TRUE); // 10 secs

    timer->start(1000);
    led_off();
    cddb_inexact_sentinel =false;
//    cddb_error = 1;
} // cddb_timed_out()

void 
KSCD::mycddb_inexact_read()
{
    if(cddb_inexact_sentinel == true)
        return;

    cddb_inexact_sentinel = true;
    QStrList inexact_list;
    cddb.get_inexact_list(inexact_list);

    InexactDialog *dialog;
    dialog = new InexactDialog(0,"inexactDialog",true);
    dialog->insertList(inexact_list);

    if(dialog->exec() != QDialog::Accepted){
        cddb.close_connection();
        timer->start(1000);
        led_off();
        return;
    }

    QString pick;
    dialog->getSelection(pick);
    delete dialog;


    if(pick.isEmpty()){
        timer->start(1000);
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

    debug("cddb_done() called\n");
//    cddb_ready_bug = 0;
    cddb.getData(xmcd_data,tracktitlelist,extlist,category,discidlist,revision,playlist);
    playlistpointer = 0;

    if((int)tracktitlelist.count() != (cd->ntracks + 1)){
        debug("WARNING tracktitleslist.count = %d != cd->ntracks +1 = %d\n",
                             tracktitlelist.count(),cd->ntracks + 1);
    }

    if((int)extlist.count() != (cd->ntracks + 1)){
        debug("WARNING extlist.count = %d != cd->ntracks +1 = %d\n",
                              extlist.count(),cd->ntracks + 1);
    }

    if(tracktitlelist.count() > 1){
        titlelabel->setText(tracktitlelist.at(1));
        artistlabel->setText(tracktitlelist.at(0));
    }

    if(cddialog && updateDialog)
        cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
                          revision,playlist,pathlist,mailcmd,submitaddress, &smtpConfigData);

    int i = 0;
    songListCB->clear();
    char *ptr = tracktitlelist.first();
    for(ptr = tracktitlelist.next(); ptr != 0; ptr = tracktitlelist.next()){
        i++;
        songListCB->insertItem( QString("").sprintf("%02d: %s", i, ptr));
    }
    for(; i < cur_ntracks; i++){
        songListCB->insertItem( QString("").sprintf(i18n("%02d: <Unknown>").ascii(), i+1));
    }

//    for (int i = 0; i < cur_ntracks; i++){
//        songListCB->insertItem( QString( 0 ).sprintf( i18n("Track %02d"), i + 1 ) );
    //    }

    led_off();
    timer->start(1000);
} // cddb_done

void 
KSCD::led_off()
{
    queryledtimer->stop();
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
    queryledtimer->start(800);
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
    titlelabeltimer->stop();
    titlelabel->setText("");

} // titlelabeltimeout

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
    cycletimer->stop();

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

    cycletimer->start(3000,TRUE);
} // cycleplaymode

void 
KSCD::cycletimeout()
{
    cycletimer->stop();
    QString str;
    str.sprintf(i18n("Vol: %02d%%").ascii(),volume);
    volumelabel->setText(str);

} // cycletimeout


bool  
KSCD::getArtist(QString& artist)
{
    if((int)tracktitlelist.count()== 0){
        return false;
    }

    artist = tracktitlelist.at(0);

    int pos;
    pos = artist.find("/",0,true);
    if(pos != -1)
        artist = artist.left(pos);

    artist = artist.stripWhiteSpace();
    return true;

} // getArtist

void  
KSCD::performances(int i)
{
    debug("preformances %d\n",i);

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
    debug("purchases %d\n",i);

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
KSCD::magicslot( int )
{
    if(magicproc && magicproc->isRunning())
      {
        return;
      }

    magicproc = 0L;
    magicproc = new KProcess;

    QString b;
    b.setNum(magic_brightness);
    QString w;
    w.setNum(magic_width);
    QString h;
    h.setNum(magic_height);

    *magicproc << "kscdmagic" << "-b" << b.data() << "-w"<< w.data() << "-h" << h.data();

    connect(magicproc,
            SIGNAL(processExited(KProcess *)),this, SLOT(magicdone(KProcess*)));


    bool result = magicproc->start(KProcess::NotifyOnExit , KProcess::NoCommunication);

    if(!result)
        KMessageBox::error(this, i18n("Cannot start kscdmagic."));
    return;
} // magicslot

void 
KSCD::magicdone(KProcess* proc)
{
    if(proc->normalExit())
      {
        //    fprintf(stderr,"kscdmagic exit status %d\n",proc->exitStatus());
        if(proc->exitStatus()!=0)
	  KMessageBox::error(this, i18n("KSCD Magic exited abnormally.\n"
					"Are you sure kscdmagic is installed?"));
      }
    //  printf("KSCD Magic Process Exited\n");

    if(proc)
        delete proc;
    magicproc = 0L;
} // magicdone

void 
KSCD::information(int i)
{
    debug("Information %d\n",i);

    QString artist;
    QString str;

    if(!getArtist(artist))
        return;

    // primitive incomplete http encoding TODO fix!
    artist = artist.replace( QRegExp(" "), "+" );

    switch(i){
        /*  case 0:
            str = str.sprintf(
            "http://musiccentral.msn.com/MCGen/ReEntry?theurl=http%%3A//musiccentral.msn.com/Search/DoFullSearch%%3FArtists%3Don%%26Albums%3D%%26Songs%%3D%%26ArticleTitles%3D%%26FullText%3D%%26strInput%%3D%s",
            artist.data());
            startBrowser(str.data());
            printf("%s\n",str.data());

            break;*/
    case 0:
        str =
            QString("http://www.ubl.com/find/form?SEARCH=%1")
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
    }

} // information

void 
KSCD::startBrowser(const QString &querystring)
{
    if(use_kfm){
        (void) new KRun ( querystring ); // replacement for KFM::openURL (David)
    }
    else{
        if (browsercmd.isEmpty())
          browsercmd="netscape";
        KProcess proc;
        proc << browsercmd << querystring;
        proc.start(KProcess::DontCare);
    }
} //startBrowser


void 
KSCD::get_pathlist(QStrList& _pathlist)
{
    QDir d;
    QStrList list;
    InexactDialog *dialog;

    d.setFilter( QDir::Dirs);
    d.setSorting( QDir::Size);
    d.setPath(cddbbasedir.data());
    if(!d.exists()){

        dialog = new InexactDialog(0, "dialog", false);
        dialog->insertText(cddbbasedir.data());
        dialog->setTitle(i18n("Enter the local CDDB base Directory"));

        if(dialog->exec() != QDialog::Accepted){
            delete dialog;
            return;
        }
        
        dialog->getSelection(cddbbasedir);
        d.setPath(cddbbasedir.data());
        delete dialog;
    }

    if(!d.exists()) // Bogus directory, don't try to read it
      return;

    _pathlist.clear();
    // FIXME: QStringList
    list = d.encodedEntryList();

    for(uint i = 0; i < list.count(); i++){
        if(QString(list.at(i)) != QString (".") && QString(list.at(i)) != QString (".."))
            _pathlist.append( QString (cddbbasedir + "/" +  list.at(i)));
    }
} // get_pathlist


void KSCD::doSM()
{
//WABA: Session management has changed
#if 1
#warning Session management is broken
#else
    if (isVisible())
	kapp->setWmCommand(QString(kapp->argv()[0])+" -caption \""+kapp->caption()+"\"");
    else
	kapp->setWmCommand(QString(kapp->argv()[0])+" -caption \""+kapp->caption()+"\" -hide ");
#endif
} // doSM


int 
main( int argc, char *argv[] )
{

    KApplication a( argc, argv,"kscd" );
    KGlobal::dirs()->addResourceType("cddb", KStandardDirs::kde_default("data") + "kscd/cddb/");

    KSCD* k = new KSCD();

    cur_track = 1;

    bool hide = FALSE;

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i],"-hide") == 0){
	    hide = TRUE;
	}
	
        if(strcmp(argv[i],"-h") == 0){
            printf("KSCD "KSCDVERSION
                   " Copyright 1997-98 Bernd Johannes Wuebben wuebben@kde.org\n");
            printf(i18n("-h: display commandline options\n").ascii());
            printf(i18n("-d: enable debugging output.\n").ascii());
            exit(0);
        }
    }

//WABA: Session management has changed
#if 1
#warning Session management is broken
#else
    a.enableSessionManagement(true);
#endif
    a.setTopWidget(k);
    a.setMainWidget( k );
    k->setCaption(a.caption());
    if (!hide)
	k->show();

    return a.exec();
} // main()

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

#ifdef linux

// check if drive is mounted (from Mark Buckaway's cdplayer code)
void 
KSCD::checkMount()
{
  	if ((fp = setmntent (MOUNTED, "r")) == NULL) {
		fprintf (stderr, i18n("Couldn't open %s: %s\n"),
			 MOUNTED, strerror (errno));
		exit (1);
	}
	while ((mnt = getmntent (fp)) != NULL) {
		if (strcmp (mnt->mnt_type, "iso9660") == 0) {
			fputs (i18n("CDROM already mounted. Operation aborted.\n"),
			       stderr);
			endmntent (fp);
			exit (1);
		}
	}
	endmntent (fp);
	
}

#elif defined (__FreeBSD__)
void 
KSCD::checkMount()
{
      struct statfs *mnt;
      int i, n;

      n = getmntinfo(&mnt, MNT_WAIT);
      for (i=0; i<n; i++) {
              if (mnt[i].f_type == MOUNT_CD9660) {
                      fputs(i18n("CDROM already mounted. Operation aborted.\n"),
                            stderr);
                      exit(1);
              }
      }
}

#else

// TODO Can I do this for other platforms?
void 
KSCD::checkMount(){
}

#endif
*/

void 
KSCD::make_random_list()
{
  /* koz: 15/01/00. I want a random list that does not repeat tracks. Ie, */
  /* a list is created in which each track is listed only once. The tracks */
  /* are picked off one by one until the end of the list */
  
  int selected,size,i,j;
  bool rejected;
  
  if( (size = playlist.count()) <= 0 )
    size = cur_ntracks;

  debug ( "Playlist has %d entries\n", size );
  random_list = (int *)malloc((size_t)size*sizeof(int));
  for( i=0; i < size; i++ ) 
    {
      do {
	rejected = false;
	if( playlist.count() <= 0 )
	  selected = 1+ (int)((double)size*rand()/(RAND_MAX+1.0));
	else
	  selected = (int)((double)size*rand()/(RAND_MAX+1.0));

	for(j=0;j<i;j++) 
	  {
	    if(random_list[j] == selected) 
	      {
		rejected = true;
		break;
	      }
	  }
      } while(rejected == true);
      random_list[i] = selected;
    }
  random_current = 0; /* Index of array we are on */
  return;
}

#include "kscd.moc"



