/*
   Kscd - A simple cd player for the KDE Project

   $Id$

   Copyright (c) 1997 Bernd Johannes Wuebben math.cornell.edu

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


 */



#ifndef __KSCD__
#define __KSCD__

#include "bwlednum.h"


#include <qpushbutton.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qbitmap.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qtabdialog.h>
#include <qtooltip.h>
#include <qpopupmenu.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/* this is for glibc 2.x which the ust structure in ustat.h not stat.h */
#ifdef __GLIBC__
#include <sys/ustat.h>
#endif

#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

#ifdef __linux__
#include <mntent.h>
#define KSCDMAGIC 0
#endif

#include "cddb.h"
#include "smtp.h"
#include "ledlamp.h"

#include <kapp.h>
#include <kprocess.h>
#include <dcopobject.h>

class CDDBSetup;
class ConfigDlg;
class CDDialog;

class SMTPConfig;
struct SMTPConfigData;

struct mgconfigstruct
{
  int width;
  int height;
  int brightness;
  int fadeMode;
  bool pointsAreDiamonds;
  double starSize;
};

class KSCD : public QWidget, public KSessionManaged, virtual public DCOPObject {

    Q_OBJECT
    K_DCOP

    // time display modes
    enum time_display { TRACK_SEC = 0, TRACK_REM = 1, TOTAL_SEC = 2, TOTAL_REM = 3 };


k_dcop:
    void play() { playClicked(); }
    void stop() { stopClicked(); }
    void previous() { prevClicked(); }
    void next() { nextClicked(); }
    void forward() { fwdClicked(); }
    void backward() { bwdClicked(); }
    void eject() { ejectClicked(); }
    void quit() { quitClicked(); }
    void toggleLoop() { loopClicked(); }
    void toggleShuffle() { randomSelected(); }
    void toggleTimeDisplay() { cycleplaytimemode(); }
    void cddbDialog() { CDDialogSelected(); }
    void optionDialog() { aboutClicked(); }
    void setTrack(int t) { trackSelected(t >= 1 ? t - 1 : 0); }
    void setVolume(int v) { volChanged(v); volSB->setValue(v); }
    int currentTrack();
    QString currentTrackTitle();
    QStringList trackList();

public:
    KSCD( QWidget *parent = 0, const char *name = 0 );
    ~KSCD();
    void initialShow();
	virtual bool saveState(QSessionManager& sm);
    
    bool dock() { return docking; }
    void setDocking(bool dock);    
    bool randomOnce() { return randomonce; }
    void setRandomOnce(bool shuffle);
    bool stopOnExit() { return stopexit; }
    void setStopOnExit(bool stop) { stopexit = stop; }
    bool autoPlay() { return autoplay; }
    void setAutoplay(bool play) { autoplay = play; }
    bool ejectOnFinish() { return ejectonfinish; }
    void setEjectOnFinish(bool eject) { ejectonfinish = eject; }
    unsigned int skipInterval() { return skipDelta; }
    void setSkipInterval(unsigned int skip) { skipDelta = skip; }
    QColor ledColor() { return led_color; }
    QColor bgColor() { return background_color; }
    void setColors(const QColor& LEDs, const QColor& bground);
    void setToolTips(bool on);
    bool toolTips() { return tooltips; }
    void setDevicePath(QString path);
    QString devicePath() { return cd_device_str; }
    SMTPConfigData* smtpData() { return smtpConfigData; }

signals:
    void trackChanged(const QString&);
    void newServerList(const QStringList&);

public slots:
    void smtpMessageSent(void);
    void smtpError(int);
    void magicdone(KProcess*);
    void magicslot();
    void magicslot(int);
    void togglequeryled();
    void cddb_done();
    void cddb_timed_out();
    void cddb_ready();
    void edm_save_cddb_entry(QString& path);
    void cddb_failed();
    void randomSelected();
    void readSettings();
    void writeSettings();
    void setColors();
    void playClicked();
    void initCDROM();
    void stopClicked();
    void prevClicked();
    void nextClicked();
    void fwdClicked();
    void bwdClicked();
    void quitClicked();
    void loopOn();
    void loopOff();
    void loopClicked();
    void cdMode();
    void ejectClicked();
    void trackSelected( int );
    void aboutClicked();
    void incVolume();
    void decVolume();
    void volChanged( int );
    void mycddb_inexact_read();
    void cddb_no_info();
    void led_on();
    void led_off();
    void titlelabeltimeout();
    void CDDialogSelected();
    void cycleplaytimemode();
    void cycletimeout();
    void get_cddb_info(bool);
    void CDDialogDone();
    void getCDDBservers();
    void getCDDBserversDone();
    void getCDDBserversFailed();
    void updateCurrentCDDBServer(const QString&);
    void performances(int);
    void purchases(int);
    void information(int);
    void showPopup();
    void jumpTracks();
    
    /*
     * TODO
     * not the prettiest things in the world
     * but getting rid of them will require some more work
     * with the CDDB and magic config pages
     */
    void getCDDBOptions(CDDBSetup* config);
    void setCDDBOptions(CDDBSetup* config);
    void getMagicOptions(mgconfigstruct& config);
    void setMagicOptions(mgconfigstruct& config);

    void make_random_list(); /* koz: 15/01/00 */

protected slots:
    void configDone();

protected:
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent* e );
    bool event( QEvent *e );
    //    void focusOutEvent(QFocusEvent *e);
    void playtime();
    void setupPopups();
    void startBrowser(const QString& querystring);
    bool getArtist(QString& artist);
    void get_pathlist(QStringList& _patlist);

private:
    SMTPConfigData  *smtpConfigData;   
    ConfigDlg       *configDialog;
    CDDialog        *cddialog;
    QPushButton     *playPB;
    QPushButton     *stopPB;
    QPushButton     *prevPB;
    QPushButton     *nextPB;
    QPushButton     *fwdPB;
    QPushButton     *bwdPB;
    QPushButton     *dockPB;
    QPushButton     *magicPB;
    QPushButton     *replayPB;
    QPushButton     *ejectPB;
    QPushButton     *aboutPB;
    QPushButton     *infoPB;
    QPopupMenu      *mainPopup;
    QPopupMenu      *perfPopup;
    QPopupMenu      *purchPopup;
    QPopupMenu      *infoPopup;

    QColor              background_color;
    QColor              led_color;
    BW_LED_Number       *trackTimeLED[6];
    QLabel              *statuslabel;
    QLabel              *titlelabel;
    QLabel              *artistlabel;
    QLabel              *volumelabel;
    QLabel              *tracklabel;
    QLabel              *totaltimelabel;
    QLabel              *nLEDs;
    QPushButton         *optionsbutton;
    QPushButton         *shufflebutton;
    QPushButton         *cddbbutton;
    QPushButton         *volLA;
    QTimer              *timer;
    QTimer              *titlelabeltimer;
    QTimer              *queryledtimer;
    QTimer              *initimer;
    QTimer              *cycletimer;
    QTimer              *jumpTrackTimer;
    QComboBox           *songListCB;
    QSlider             *volSB;

    KProcess*           magicproc;
    int                 jumpToTrack;
    unsigned int        skipDelta;
    int                 volChnl;
    int                 mixerFd;
    int                 volume;
    int                 magic_width;
    int                 magic_height;
    int                 magic_brightness;
    bool                magic_pointsAreDiamonds;
    QFrame              *backdrop;
    LedLamp             *queryled;
    LedLamp             *loopled;
    KConfig             *config;
    bool                tooltips;
    bool                randomplay ;
    bool                looping;
    bool                cycle_flag;
    QString             cycle_str;
    bool                volstartup;
    bool                cddrive_is_ok;
    bool                have_new_cd;
    int                 time_display_mode;
    QString             cd_device_str;
    bool                hidden_controls;

    QPushButton         *makeButton( int, int, int, int, const QString& );


    void                initFont();
    void                initWorkMan();
    //  void            checkMount();

    void                drawPanel();
    void                cleanUp();
    void                loadBitmaps();
    void                setLEDs(const QString& symbols);
  /**
   * select a random track from the current disc.
   *
   */
    int                 randomtrack();

  /**
   * set the artist and title labels as well as the dock tooltip.
   *
   */
    void setArtistAndTitle(const QString& artist, const QString& title);

//TODO get rid of the mixe stuff

    void                initMixer( const char *mixer = "/dev/mixer" );

// These are the variables from workbone

        int             sss;
        int             sel_stat;
        int             dly;
        int             fastin;
        int             scmd;
        int             tmppos;
        int             save_track;
        struct timeval  mydelay;
        struct mntent   *mnt;
        FILE            *fp;

        CDDB            cddb;
        QStringList        querylist;
        QStringList        pathlist;
        QStringList        discidlist;
        QStringList     tracktitlelist;
        QStringList     playlist;
        int             playlistpointer;
        QStringList     extlist;
        int             revision;
        QString         category;
        QString         xmcd_data;
        QStringList     cddbserverlist;
        QString         cddbbasedir;
        QString         current_server;
        QString         mailcmd;
        QString         submitaddress;
        QStringList        cddbsubmitlist;
        QFont           smallfont;
        QFont           verysmallfont;
        bool            cddb_remote_enabled;
        /* edm new section */
        bool            cddb_auto_enabled;
        /* edm new section end */
        bool            docking;
        bool            autoplay;
        bool            stopexit;
        bool            ejectonfinish;
        bool            randomonce;
        bool            Fetch_remote_cddb;
        bool            cddb_inexact_sentinel;
        bool            updateDialog;
        bool            ejectedBefore;
        bool            currentlyejected;
};



#endif

