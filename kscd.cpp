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
#include <kcharsets.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprotocolmanager.h>
#include <krun.h>
#include <krandomsequence.h>
#include <kstddirs.h>
#include <kstringhandler.h>
#include <kurl.h>

#include "docking.h"
#include "kscd.h"
#include "configdlg.h"
#include "mgconfdlg.h"
#include "version.h"
extern "C" {
    // We don't have libWorkMan installed already, so get everything
    // from within our own directory
#include "libwm/include/workman.h"
}
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
#include "bitmaps/magic.xbm"
#include "bitmaps/eject.xbm"
#include "bitmaps/db.xbm"
#include "bitmaps/logo.xbm"
#include "bitmaps/shuffle.xbm"
#include "bitmaps/options.xbm"

static const char *description = I18N_NOOP("KDE CD player");

DockWidget*     dock_widget;
SMTP                *smtpMailer;
bool dockinginprogress = 0;
bool quitPending = 0;
bool stoppedByUser = 1;

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

int    random_current;  /* koz: Current track in random list */
int   *random_list;     /* koz: Used in Random - once through */

KRandomSequence randSequence;

static QString formatTrack(int d1, int d2)
{
    QString str = QString::fromLatin1("%1/%2")
                  .arg( QString::number(d1).rightJustify(2, '0') )
                  .arg( QString::number(d2).rightJustify(2, '0') );
    return str;
}

int cddb_error = 0;

/****************************************************************************
                                The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name )
    :   QWidget( parent, name ), DCOPObject("CDPlayer")
{

    //    connect(kapp, SIGNAL (saveYourself() ), SLOT (doSM()));
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
    setup               = 0L;
    smtpconfig          = 0L;
    time_display_mode   = TRACK_SEC;
    cddb_inexact_sentinel = false;
    revision            = 0; // The first freedb revision is "0"
    use_kfm             = true;
    docking             = true;
    autoplay            = false;
    stopexit            = true;
    ejectonfinish       = false;
    randomonce          = true;
    updateDialog        = false;
    ejectedBefore       = false;
    currentlyejected    = false;
    cddialog        = 0L;

    have_new_cd = true;

    drawPanel();
    loadBitmaps();
    setColors();
    setToolTips();

    timer           = new QTimer( this );
    queryledtimer   = new QTimer( this );
    titlelabeltimer = new QTimer( this );
    initimer        = new QTimer( this );
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
#if KSCDMAGIC
    connect( magicPB, SIGNAL(clicked()), SLOT(magicslot()) );
#endif
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

    if(looping)
    {
    	loopled->on();
    }

    dock_widget = new DockWidget( this, "dockw");
    if(docking)
    {
        dock_widget->show();
        connect(this, SIGNAL(trackChanged(const QString&)), dock_widget, SLOT(setToolTip(const QString&)));
    }

    smtpMailer = new SMTP;
    connect(smtpMailer, SIGNAL(messageSent()), this, SLOT(smtpMessageSent()));
    connect(smtpMailer, SIGNAL(error(int)), this, SLOT(smtpError(int)));

    setFocusPolicy ( QWidget::NoFocus );

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
    fastin       = FALSE;
    scmd         = 0;
    tmppos       = 0;
    save_track   = 1;
    thiscd.trk   = NULL;
    thiscd.lists = NULL;
    tottime      = tmptime;
} // initWorkMan()

void
KSCD::initCDROM()
{
    printf("initCDROM\n");
    initimer->stop();
    kapp->processEvents();
    kapp->flushX();

    printf("~initCDROM1\n");
    cdMode();
    volstartup = FALSE;
    printf("~initCDROM2\n");
    if(cddrive_is_ok)
        volChanged(volume);

    printf("~initCDROM4\n");
    timer->start(1000);
    //  cdMode();
    printf("~initCDROM\n");
} // initCDROM


/**
 * Return fitting Helvetica font size for the 13 and 14 pixel widgets.
 * @return a reasonably small font size
 */
int
KSCD::smallPtSize()
{
    static int theSmallPtSize = 0;

    if ( theSmallPtSize != 0 )
        return theSmallPtSize;

    // Find a font that fits the 13 and 14 pixel widgets
    theSmallPtSize = 10;
    QFont fn( "Helvetica", theSmallPtSize, QFont::Bold );
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
    return theSmallPtSize;
} // smallPtSize()

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
    //  const int WIDTH = 100;
    const int WIDTH = 90;
    //  const int HEIGHT = 29;
    const int HEIGHT = 27;
    //  const int SBARWIDTH = 180; //140
    const int SBARWIDTH = 220; //140

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
    artistlabel->setFont( QFont( "helvetica", smallPtSize(), QFont::Bold) );
    artistlabel->setAlignment( AlignLeft );
    artistlabel->clear();

    titlelabel = new QLabel(this);
    titlelabel->setGeometry(WIDTH + 5, iy + 50 , SBARWIDTH -15, 13);
    QFont ledfont( "Helvetica", smallPtSize()-2, QFont::Bold );
    //    KGlobal::charsets()->setQFont(ledfont);
    titlelabel->setFont( ledfont );
    titlelabel->setAlignment( AlignLeft );
    titlelabel->clear();

    statuslabel = new QLabel(this);
    statuslabel->setGeometry(WIDTH + 110, iy  +D, 50, 14);
    statuslabel->setFont( ledfont );
    statuslabel->setAlignment( AlignLeft );
    //  statuslabel->setText("Ready");

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
    volumelabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold ) );
    volumelabel->setAlignment( AlignLeft );
    volumelabel->setText(i18n("Vol: --"));

    tracklabel = new QLabel(this);
    tracklabel->setGeometry(WIDTH + 168, iy + 14 +D, 30, 14);
    tracklabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold ) );
    tracklabel->setAlignment( AlignLeft );
    tracklabel->setText("--/--");

    totaltimelabel = new QLabel(this);
    totaltimelabel->setGeometry(WIDTH + 168, iy  +D, 50, 14);
    totaltimelabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold ) );
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
    //    cddbbutton->setFont( QFont( "helvetica", 12 ) );
    cddbbutton->setFocusPolicy ( QWidget::NoFocus );

    ix += SBARWIDTH/10*2;
    shufflebutton = new QPushButton( this );
    shufflebutton->setGeometry( ix , iy, SBARWIDTH/10 *2  , HEIGHT );
    //    shufflebutton->setFont( QFont( "helvetica", 12 ) );
    shufflebutton->setFocusPolicy ( QWidget::NoFocus );

    ix += SBARWIDTH/10*2;

    optionsbutton = new QPushButton( this );
    optionsbutton->setGeometry( ix, iy, SBARWIDTH/10 *2  , HEIGHT );
    //    optionsbutton->setFont( QFont( "helvetica", 12 ) );
    optionsbutton->setFocusPolicy ( QWidget::NoFocus );

    ix += SBARWIDTH/10*2;
    songListCB = new QComboBox( this );
    songListCB->setGeometry( ix, iy, SBARWIDTH/10*4, HEIGHT );
    songListCB->setFont( QFont( "helvetica", smallPtSize() ) );
    songListCB->setFocusPolicy ( QWidget::NoFocus );

    kdDebug() << "Width " << WIDTH << " Height " << HEIGHT << "\n" << endl;

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

#if KSCDMAGIC
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
        QToolTip::add( playPB,          i18n("Play/Pause") );
        QToolTip::add( stopPB,          i18n("Stop") );
        QToolTip::add( replayPB,        i18n("Loop") );
        QToolTip::add( songListCB,      i18n("Track Selection") );
        QToolTip::add( fwdPB,           i18n("30 Secs Forward") );
        QToolTip::add( bwdPB,           i18n("30 Secs Backward") );
        QToolTip::add( nextPB,          i18n("Next Track") );
        QToolTip::add( prevPB,          i18n("Previous Track") );
        QToolTip::add( dockPB,          i18n("Quit Kscd") );
#if KSCDMAGIC
        QToolTip::add( magicPB,         i18n("Run Kscd Magic") );
#endif
        QToolTip::add( aboutPB,         i18n("Cycle Time Display") );
        QToolTip::add( optionsbutton,   i18n("Configure Kscd") );
        QToolTip::add( ejectPB,         i18n("Eject CD") );
        QToolTip::add( infoPB,          i18n("The Artist on the Web") );
        if (!randomonce)
            QToolTip::add( shufflebutton,         i18n("Random Play") );
        else
            QToolTip::add( shufflebutton,         i18n("Shuffle Play") );

        QToolTip::add( cddbbutton,      i18n("CDDB Dialog") );
        QToolTip::add( volSB,           i18n("CD Volume Control") );
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
#if KSCDMAGIC
        QToolTip::remove( magicPB );
#endif
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
    {
        delete thiscd.trk;
        thiscd.trk = 0L;
    }
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
        cur_cdmode == WM_CDM_STOPPED || cur_cdmode == WM_CDM_UNKNOWN  || cur_cdmode == WM_CDM_BACK
#else
        cur_cdmode == WM_CDM_STOPPED || cur_cdmode == WM_CDM_UNKNOWN
#endif
        ){

        statuslabel->setText( i18n("Playing") );
        songListCB->clear();
        setLEDs( "00:00" );

        songListCB->clear();
        QStringList::Iterator it = tracktitlelist.begin();
        ++it;
        int i = 0;
        for ( ; it != tracktitlelist.end(); ++it )
        {
            i++;
            QString str = QString::fromLatin1("%1: %2")
                          .arg(QString::number(i).rightJustify(2, '0'))
                          .arg(*it);
            songListCB->insertItem( str );
        }

        // We don't know the rest, but we should still have entries
        for( ; i < cur_ntracks; i++)
        {
            songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), i+1) ) );
        }

        qApp->processEvents();
        qApp->flushX();

        if(!playlist.isEmpty())
        {
            if(playlistpointer >=(int) playlist.count())
                playlistpointer = 0;
            wm_cd_play (atoi((*playlist.at(playlistpointer)).ascii()), 0,
                        atoi((*playlist.at(playlistpointer)).ascii()) + 1);
            save_track = cur_track = atoi((*playlist.at(playlistpointer)).ascii());
        } else {
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
                    statuslabel->setText( i18n("Strange....") );
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

    } else if(!playlist.isEmpty()) {
        if(playlistpointer < (int)playlist.count() - 1)
            playlistpointer++;
        else
            playlistpointer = 0;

        wm_cd_play (atoi((*playlist.at(playlistpointer)).ascii()),
                    0, atoi((*playlist.at(playlistpointer)).ascii()) + 1);
        cur_track = atoi( (*playlist.at(playlistpointer)).ascii() );
    } else {
        if (cur_track == cur_ntracks)
            cur_track = 0;
        wm_cd_play (cur_track + 1, 0, cur_ntracks + 1);
    }
} // nextClicked()

void
KSCD::fwdClicked()
{
    qApp->processEvents();
    qApp->flushX();

    if (cur_cdmode == WM_CDM_PLAYING)
    {
        tmppos = cur_pos_rel + 30;
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
        tmppos = cur_pos_rel - 30;
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
    quitPending = true;
    randomplay = FALSE;
    statuslabel->clear();
    setLEDs( "--:--" );

    qApp->processEvents();
    qApp->flushX();

    if(stopexit)
        wm_cd_stop ();

    wm_cd_status();
    wm_cd_status();

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

    statuslabel->clear();

    setLEDs( "--:--" );

    qApp->processEvents();
    qApp->flushX();

    if(stopexit)
        wm_cd_stop ();

    wm_cd_status();
    wm_cd_status();
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
        setArtistAndTitle("", "");
        tracktitlelist.clear();
        extlist.clear();

        wm_cd_stop();
        //  timer->stop();
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
        tracklabel->setText(formatTrack(j, cd->ntracks));
        if(j < (int)tracktitlelist.count())
        {
            setArtistAndTitle(tracktitlelist.first(),
                              *tracktitlelist.at(j));
        }
        qApp->processEvents();
        qApp->flushX();

        wm_cd_play( j, 0, j + 1 );
        cur_track = j;
    }
} // randomSelected

void
KSCD::trackSelected( int trk )
{
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
    QBoxLayout * lay1 = new QVBoxLayout ( about, 10 );
    QGroupBox *box = new QGroupBox(about,"box");
    lay1->addWidget ( box );
    QBoxLayout * lay2 = new QHBoxLayout ( box, 15 );

    QPixmap pm = UserIcon("kscdlogo");
    QLabel *logo = new QLabel(box);
    logo->setPixmap(pm);
    logo->setFixedSize(pm.width(), pm.height());
    lay2->addWidget ( logo );

    QString labelstring;
    labelstring = i18n("kscd %1\n").arg(KSCDVERSION);
    labelstring += i18n(
        "Copyright (c) 1997-2001 \nBernd Johannes Wuebben <wuebben@kde.org>\n"
        "Copyright (c) 1999-2001 \nDirk FÃ¶rsterling <milliByte@gmx.net>\n"
        "   (current Maintainer)\n\n"
        "Kscd is based in part on WorkMan,\n"
        "Copyright (c) 1991-1996 Steven Grimm\n"
        "Copyright (c) 1996-2001 Dirk FÃ¶rsterling <milliByte@gmx.net>\n\n"
        "Special thanks to Ti Kan and "
        "Steve Scherf, the inventors of "
        "the CDDB database concept. "
        "Visit http://www.cddb.com/ for "
        "more information on CDDB.\n\n"
        );

#if KSCDMAGIC
    labelstring += i18n(
        "KSCD Magic based on Synaesthesia by "
        "Paul Harrison <pfh@yoyo.cc.monash.edu.au>\n\n");
#endif
    labelstring += i18n(
        "Thanks to Vadim Zaliva <lord@crocodile.org>\n"
        "for his work on the http proxy code.\n\n") ;


    QLabel  *label = new QLabel(box,"label");
    label->setAlignment(AlignLeft|WordBreak|ExpandTabs);
    label->setText(labelstring);
    lay2->addWidget ( label );

    ConfigDlg* dlg;
    struct configstruct config;
    config.background_color = background_color;
    config.led_color = led_color;
    config.tooltips = tooltips;
    config.cd_device = QFile::decodeName(cd_device);
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
                      cddb_auto_enabled,
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
    mgconfig.pointsAreDiamonds = magic_pointsAreDiamonds;
    mgdlg = new MGConfigDlg(tabdialog,&mgconfig,"mgconfigdialg");

    smtpconfig = new SMTPConfig(tabdialog, "smtpconfig", &smtpConfigData);

    tabdialog->addTab(setup,"CDDB");
    tabdialog->addTab(smtpconfig, i18n("SMTP Setup"));
    tabdialog->addTab(dlg,i18n("Kscd Options"));
#if KSCDMAGIC
    tabdialog->addTab(mgdlg,i18n("Kscd Magic"));
#endif
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
            /*
            wmcd_close();
*/
            // FIXME
            cd_device = (char *)qstrdup(QFile::encodeName(cd_device_str));

        }
        cddrive_is_ok = true;

        magic_width = mgdlg->getData()->width;
        magic_height = mgdlg->getData()->height;
        magic_brightness = mgdlg->getData()->brightness;
        magic_pointsAreDiamonds = mgdlg->getData()->pointsAreDiamonds;

        bool cddb_proxy_enabled;
        QString cddb_proxy_host;
        unsigned short int cddb_proxy_port;
        unsigned short int cddb_timeout;

        setup->getData(cddbserverlist,
                       cddbsubmitlist,
                       cddbbasedir,
                       submitaddress,
                       current_server,
                       cddb_auto_enabled,
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
        kdDebug() << "RESETTING SERVER TO: " << current_server << "\n" << endl;
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
        kdDebug() << "SET SERVER TO: " << current_server << "\n" << endl;
    }
} // updateCurrentCDDBServer

void
KSCD::volChanged( int vol )
{
    if(volstartup)
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
    if( randomonce )
    {
        if( !playlist.isEmpty() )
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
            return atoi( (*playlist.at(j)).ascii() );
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

    sss = wm_cd_status();
    if( sss == 2 )
        have_new_cd = true;

    if(sss < 0)
    {
        if(cddrive_is_ok && (sss != WM_ERR_SCSI_INQUIRY_FAILED))
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
                wm_cd_play(atoi( (*playlist.at(playlistpointer)).ascii() ),0,atoi((*playlist.at(playlistpointer)).ascii())+1);
            }
            else if ( looping )
            {
                if (cur_track == cur_ntracks)
                {
                    cur_track = 0;
                    wm_cd_play (1, 0, cur_ntracks + 1);
                }

            } else {
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
            //        else if(looping)
            //            statuslabel->setText( i18n("Loop") );
            else
                statuslabel->setText( i18n("Playing") );

            sprintf( p, "%02d  ", cur_track );
            if (songListCB->count() == 0)
            {
                // we are in here when we start kscd and
                // the cdplayer is already playing.
                int i = 0;
                songListCB->clear();
                QStringList::Iterator it = tracktitlelist.begin();
                ++it;
                for ( ; it != tracktitlelist.end(); ++it )
                {
                    i++;
                    songListCB->insertItem( QString("").sprintf("%02d: %s", i, (*it).utf8().data()));
                }
                for(; i < cur_ntracks; i++)
                {
                    songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), i+1)) );
                }

//            for (int i = 0; i < cur_ntracks; i++){
//                songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("Track %02d").utf8(),i + 1 ) ) );
//            }
                songListCB->setCurrentItem( cur_track - 1 );
                have_new_cd = false;
                get_cddb_info(false); // false == do not update dialog if open
            } else {
                songListCB->setCurrentItem( cur_track - 1 );
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
                songListCB->clear();

                int i = 0;
                songListCB->clear();
                QStringList::Iterator it = tracktitlelist.begin();
                ++it;
                for ( ; it != tracktitlelist.end(); ++it )
                {
                    i++;
                    songListCB->insertItem( QString().sprintf("%02d: %s", i, (*it).utf8().data()));
                }
                for(; i < cur_ntracks; i++){
                    songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), i+1)) );
                }

//            for (i = 0; i < cur_ntracks; i++)
//                songListCB->insertItem( QString( 0 ).sprintf( i18n("Track %02d"), i + 1 ) );

                int w = ((cur_track >= 0) ? cur_track : 1);

                tracklabel->setText( formatTrack( cur_track >= 0 ? cur_track : 1, cd->ntracks) );

                if( w < (int)tracktitlelist.count()){
                    setArtistAndTitle(tracktitlelist.first(),
                                      *tracktitlelist.at( w ));
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

        case WM_CDM_EJECTED:
            statuslabel->setText( i18n("Ejected") );
            songListCB->clear();
            setLEDs( "--:--" );
            tracklabel->setText( "--/--" );
            setArtistAndTitle("", "");
            totaltimelabel->clear();
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

    titlelabel ->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold) );
    artistlabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold) );
    volumelabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold) );
    statuslabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold) );
    tracklabel ->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold) );
    totaltimelabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold) );
    artistlabel->setFont( QFont( "Helvetica", smallPtSize(), QFont::Bold) );

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
    looping    = config->readBoolEntry("Looping",false);
    

#ifdef DEFAULT_CD_DEVICE

    // sun ultrix etc have a canonical cd rom device specified in the
    // respective plat_xxx.c file. On those platforms you need nnot
    // specify the cd rom device and DEFAULT_CD_DEVICE is not defined
    // in config.h

    cd_device_str = config->readEntry("CDDevice",DEFAULT_CD_DEVICE);
    // FIXME
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
    smtpConfigData.enabled = config->readBoolEntry("enabled", true);
    smtpConfigData.mailProfile = config->readEntry("mailProfile", i18n("Default"));

    // Same as follows happens in smtpconfig.cpp. Try to remove one.
    KEMailSettings *kes = new KEMailSettings();
    kes->setProfile( smtpConfigData.mailProfile );
    smtpConfigData.serverHost = kes->getSetting( KEMailSettings::OutServer );
    smtpConfigData.serverPort = "25";
    smtpConfigData.senderAddress = kes->getSetting( KEMailSettings::EmailAddress );
    smtpConfigData.senderReplyTo = kes->getSetting( KEMailSettings::ReplyToAddress );
    // Don't accept obviously bogus settings.
    if( (smtpConfigData.serverHost == "") || (!smtpConfigData.senderAddress.contains("@")))
    {
        smtpConfigData.enabled = false;
    }

    config->setGroup("CDDB");

    cddb.setTimeout(config->readNumEntry("CDDBTimeout",60));
    cddb_auto_enabled = config->readBoolEntry("CDDBLocalAutoSaveEnabled",false);
    cddbbasedir = config->readEntry("LocalBaseDir");
    if (cddbbasedir.isEmpty())
        cddbbasedir = KGlobal::dirs()->resourceDirs("cddb").last();
    KGlobal::dirs()->addResourceDir("cddb", cddbbasedir);

// Set this to false by default. Look at the settings dialog source code
// for the reason. - Juraj.
    cddb_remote_enabled = config->readBoolEntry("CDDBRemoteEnabled",
                                                false);
    cddb.useHTTPProxy(config->readBoolEntry("CDDBHTTPProxyEnabled",
                                            KProtocolManager::useProxy()));
    KURL proxyURL;
    QString proxyHost;
    int proxyPort;
    QString proxy = KProtocolManager::httpProxy();
    if( !proxy.isEmpty() )
      {
	proxyURL = proxy;
	proxyHost = proxyURL.host();
	proxyPort = proxyURL.port();
      } else {
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
        kdDebug() << "Default CDDB server entry converted to new format and saved.\n" << endl;
        config->writeEntry("CurrentServer",current_server);
        config->sync();
    }
    submitaddress = config->readEntry("CDDBSubmitAddress","xmcd-cddb@amb.org");
    cddbserverlist = config->readListEntry("SeverList", ',');
    int num = cddbserverlist.count();
    if (num == 0)
        cddbserverlist.append(DEFAULT_CDDB_SERVER);
    else
    {
        //Let's check if it is in old format and if so, convert it to new one:
        bool needtosave=false;
        QStringList nlist;

        for ( QStringList::Iterator it = cddbserverlist.begin();
              it != cddbserverlist.end();
              ++it )
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
            kdDebug() << "CDDB server list converted to new format and saved.\n" << endl;
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
    config->writeEntry("CDDevice", QFile::decodeName(cd_device));
    config->writeEntry("CustomBroserCmd",browsercmd);
    config->writeEntry("Volume", volume);
    config->writeEntry("BackColor",background_color);
    config->writeEntry("LEDColor",led_color);
    config->writeEntry("UnixMailCommand",mailcmd);
    config->writeEntry("Looping", looping);

    config->setGroup("SMTP");
    config->writeEntry("enabled", smtpConfigData.enabled);
    config->writeEntry("mailProfile", smtpConfigData.mailProfile);

    config->setGroup("CDDB");
    config->writeEntry("CDDBRemoteEnabled",cddb_remote_enabled);
    config->writeEntry("CDDBTimeout",cddb.getTimeout());
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
                   cddb_auto_enabled,
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
    setArtistAndTitle(i18n("Unable to get CDDB server list."), "");
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
        /*
        min = thiscd.trk[i].start / (4500); // 60 * 75
            sec = (thiscd.trk[i].start % (4500)) / 75;
            n += cddb_sum((min * 60) + sec);
*/
        n += cddb_sum(thiscd.trk[i].start / 75);
    }

    /*
    t = ((thiscd.cddbtoc[thiscd.ntracks].min * 60)
             + thiscd.cddbtoc[thiscd.ntracks].sec) -
             ((thiscd.cddbtoc[0].min * 60) + thiscd.cddbtoc[0].sec);
*/
    t = ((thiscd.trk[thiscd.ntracks].start / 75) -
         (thiscd.trk[0].start / 75));

    return ((n % 0xff) << 24 | t << 8 | thiscd.ntracks);
}

#endif

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

        int i = 0;
        songListCB->clear();
        QStringList::Iterator it = tracktitlelist.begin();
        ++it;
        for ( ; it != tracktitlelist.end(); ++it )
        {
            i++;
            songListCB->insertItem( QString("").sprintf("%02d: %s", i, (*it).utf8().data()));
        }
        for(; i < cur_ntracks; i++){
            songListCB->insertItem( QString::fromUtf8(QCString("").sprintf(i18n("%02d: <Unknown>").utf8(), i+1)) );
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
    kdDebug() << "cddb_ready() called\n" << endl;

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

    QCString num;

    for(int i = 0 ; i < cd->ntracks; i++)
    {
        querylist.append(num.setNum(cd->trk[i].start));
    }

    querylist.append(num.setNum(cd->trk[cd->ntracks].start/75));
    cddb_inexact_sentinel =false;
    cddb.queryCD(cddb_discid(),querylist);
} // cddb_ready

void
KSCD::cddb_no_info()
{
    //        cddb_ready_bug = 0;
    kdDebug() << "cddb_no_info() called\n" << endl;

    setArtistAndTitle(i18n("No matching CDDB entry found."), "");
//    artistlabel->clear();
//    titlelabeltimer->start(10000,TRUE); // 10 secs

    tracktitlelist.clear();
    tracktitlelist.append(i18n("No matching CDDB entry found."));
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
    kdDebug() << "cddb_failed() called\n" << endl;
    tracktitlelist.clear();
    tracktitlelist.append(i18n("Error getting CDDB entry."));
    for(int i = 0 ; i < cd->ntracks; i++)
        tracktitlelist.append("");

    extlist.clear();
    for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

    discidlist.clear();

    setArtistAndTitle(i18n("Error getting CDDB entry."), "");
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
    kdDebug() << "cddb_timed_out() called\n" << endl;
    tracktitlelist.clear();
    tracktitlelist.append(i18n("CDDB query timed out."));
    for(int i = 0 ; i <= cd->ntracks; i++)
        tracktitlelist.append("");

    extlist.clear();
    for(int i = 0 ; i <= cd->ntracks; i++)
        extlist.append("");

    discidlist.clear();

    setArtistAndTitle(i18n("CDDB query timed out."),"");
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

    QString pick;

    cddb_inexact_sentinel = true;
    QStringList inexact_list;
    cddb.get_inexact_list(inexact_list);


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
        timer->start(1000);
        led_off();
        return;
    }

    dialog->getSelection(pick);
    delete dialog;


    if(pick.isEmpty())
    {
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

    kdDebug() << "cddb_done() called\n" << endl;
//    cddb_ready_bug = 0;
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
                          revision,playlist,pathlist,mailcmd,submitaddress, &smtpConfigData);

    int i = 0;
    songListCB->clear();
    QStringList::Iterator it = tracktitlelist.begin();
    ++it;
    for ( ; it != tracktitlelist.end(); ++it )
    {
        i++;
        songListCB->insertItem( QString("").sprintf("%02d: %s", i, (*it).utf8().data()));
    }
    for(; i < cur_ntracks; i++){
        songListCB->insertItem( QString::fromUtf8( QCString().sprintf(i18n("%02d: <Unknown>").utf8(), i+1)) );
    }

//    for (int i = 0; i < cur_ntracks; i++){
//        songListCB->insertItem( QString( 0 ).sprintf( i18n("Track %02d"), i + 1 ) );
    //    }

    led_off();
    if(Fetch_remote_cddb)
    {
        if(cddb_auto_enabled)
        {
            QString path,tmp;
            tmp.sprintf("/%08lx",cddb_discid());
            path = cddbbasedir;
            path += "/";
            path += category;
            path += tmp;
            //      kdDebug() << path << endl << cddbbasedir << category << cddb_discid() << endl;
            path.replace(QRegExp("//"),"/");
            edm_save_cddb_entry(path);
        }
    }
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

    magicproc = 0L;
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
    } // switch()
} // information

/**
 * Open an URL with the user's favourite browser.
 *
 */
void
KSCD::startBrowser(const QString &querystring)
{
    if(use_kfm || browsercmd.isEmpty()) // default browser is KDE browser, no?
    {
        (void) new KRun ( querystring ); // replacement for KFM::openURL (David)
    } else {
        KProcess proc;
        proc << browsercmd << querystring;
        proc.start(KProcess::DontCare);
    }
} //startBrowser


void
KSCD::get_pathlist(QStringList& _pathlist)
{
    QDir d;
    QStringList list;
    InexactDialog *dialog;

    d.setFilter( QDir::Dirs);
    d.setSorting( QDir::Size);
    d.setPath(cddbbasedir);
    if(!d.exists())
    {
        dialog = new InexactDialog(0, "dialog", false);
        dialog->insertText(cddbbasedir);
        dialog->setTitle(i18n("Enter the local CDDB base Directory"));

        if(dialog->exec() != QDialog::Accepted)
        {
            delete dialog;
            return;
        }

        dialog->getSelection(cddbbasedir);
        d.setPath(cddbbasedir);
        delete dialog;
    }

    if(!d.exists()) // Bogus directory, don't try to read it
        return;

    _pathlist.clear();
    list = d.entryList();

    for ( QStringList::ConstIterator it = list.begin();
          it != list.end();
          ++it )
    {
        if( *it != QString::fromLatin1(".") &&
            *it != QString::fromLatin1("..") )
        {
            _pathlist.append( cddbbasedir + '/' +  *it);
        }
    }
} // get_pathlist


/*
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
 */

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
        fputs (i18n("CDROM already mounted. Operation aborted.\n"),
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
        fputs(i18n("CDROM already mounted. Operation aborted.\n"),
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

    int selected,size,i,j;
    bool rejected;

    if ( playlist.isEmpty() )
        size = cur_ntracks;
    else
        size = playlist.count();

    kdDebug() << "Playlist has " << size << " entries\n" << endl;
    random_list = (int *)malloc((size_t)size*sizeof(int));
    for( i=0; i < size; i++ )
    {
        do {
            rejected = false;
            if( playlist.isEmpty() )
                selected = 1 + (int) randSequence.getLong(size);
            else
                selected = (int) randSequence.getLong(size);

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
} // make_random_list()


void
KSCD::edm_save_cddb_entry(QString& path)
{

    kdDebug() << "::save_cddb_entry(): path: " << path << " edm" << "\n" << endl;

    QFile file(path); //open the file


    if( !file.open( IO_WriteOnly  ))
    {
        QString str = i18n("Unable to write to file:\n%1\nPlease check "
                           "your permissions and make your category directories exist.")
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

    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication a;

    kapp->dcopClient()->attach();
    kapp->dcopClient()->registerAs("kscd");
    kapp->dcopClient()->setDefaultObject("CDPlayer");

    KGlobal::dirs()->addResourceType("cddb",
                                     KStandardDirs::kde_default("data") +
                                     "kscd/cddb/");

    //  if (a.isRestored())
    //    {
    //        RESTORE(KSCD);
    //    } else {

    KSCD *k = new KSCD();
    cur_track = 1;

    bool hide = FALSE;

    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i],"-hide") == 0)
        {
            hide = TRUE;
        }

        if(strcmp(argv[i],"-h") == 0)
        {
            printf("KSCD "KSCDVERSION
                   "\n Copyright 1997-99 Bernd Johannes Wuebben wuebben@kde.org\n"
                   " Copyright 1999-2001 Dirk Foersterling milliByte@gmx.de\n");
            printf(i18n("-h: display commandline options\n").local8Bit());
            printf(i18n("-d: enable debugging output.\n").local8Bit());
            exit(0);
        }
    }

    a.setTopWidget(k);
    a.setMainWidget( k );
    k->setCaption(a.caption());
    if(!hide)
        k->show();
    //    }
    return a.exec();
} // main()

#include "kscd.moc"
