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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


 */



#ifndef __KSCD__
#define __KSCD__

#include "bwlednum.h"

// CDDB support via libkcddb
#include <libkcddb/cddb.h>
#include <libkcddb/client.h>


#include <qpushbutton.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qtabdialog.h>
#include <qtooltip.h>
#include <qpopupmenu.h>
#include <qvaluelist.h>

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


#include "ledlamp.h"
#include "panel.h"
#include <kapplication.h>
#include <kprocess.h>
#include <krandomsequence.h>
#include <dcopobject.h>

class CDDBSetup;
class ConfigDlg;
class CDDialog;
class QGridLayout;
class KActionCollection;
class KVolumeControl;


using namespace KCDDB;

struct mgconfigstruct
{
  int width;
  int height;
  int brightness;
  int fadeMode;
  bool pointsAreDiamonds;
  double starSize;
};

typedef QValueList<int> RandomList;

class KSCD : public kscdPanelDlg, public KSessionManaged, virtual public DCOPObject {

    Q_OBJECT
    K_DCOP

    // time display modes
    enum time_display { TRACK_SEC = 0, TRACK_REM = 1, TOTAL_SEC = 2, TOTAL_REM = 3 };


k_dcop:
    void play() { playClicked(); }
    void stop() { stopClicked(); }
    void previous() { prevClicked(); }
    void next() { nextClicked(); }
    void jumpTo(int seconds) { jumpToTime(seconds); }
    void eject() { ejectClicked(); }
    void quit() { quitClicked(); }
    void toggleLoop() { loopClicked(); }
    void toggleShuffle() { randomSelected(); }
    void toggleTimeDisplay() { cycleplaytimemode(); }
    void cddbDialog() { CDDialogSelected(); }
    void optionDialog() { showConfig(); }
    void setTrack(int t) { trackSelected(t >= 1 ? t - 1 : 0); }
    void setVolume(int v);
    int  getVolume() { return volume; }
    int currentTrack();
    QString currentTrackTitle();
    QString currentAlbum();
    QString currentArtist();
    QStringList trackList();

public:
    KSCD( QWidget *parent = 0, const char *name = 0 );
    ~KSCD();
    void initialShow();
    virtual bool saveState(QSessionManager& sm);

    bool dock() { return docking; }
    void setDocking(bool dock);
    bool stopOnExit() { return stopexit; }
    void setStopOnExit(bool stop) { stopexit = stop; }
    bool autoPlay() { return autoplay; }
    void setAutoplay(bool play) { autoplay = play; }
    bool ejectOnFinish() { return ejectonfinish; }
    void setEjectOnFinish(bool eject) { ejectonfinish = eject; }
    bool digitalPlayback();
    unsigned int skipInterval() { return skipDelta; }
    void setSkipInterval(unsigned int skip) { skipDelta = skip; }
    QColor ledColor() { return led_color; }
    QColor bgColor() { return background_color; }
    void setColors(const QColor& LEDs, const QColor& bground);
    void setDevicePaths(QString cd_device, QString audio_system, QString audio_device);
    QString devicePath() { return cd_device_str; }
    QString audioSystem() { return audio_system_str; }
    QString audioDevice() { return audio_device_str; }
    QStringList audioSystems() { return audio_systems_list; }

signals:
    void trackChanged(const QString&);
    void newServerList(const QStringList&);

public slots:
    void setColors();
    void togglequeryled();
    void randomSelected();
    void setShuffle(bool shuffle);
    void writeSettings();
    void initCDROM();
    void playClicked();
    void nextClicked();
    void prevClicked();
    void stopClicked();
    void ejectClicked();
    void jumpToTime(int seconds);
    void quitClicked();
    void loopOn();
    void loopOff();
    void loopClicked();
    void cdMode();
    void trackSelected( int );
    void showConfig();
    void incVolume();
    void decVolume();
    void volChanged( int );
    void led_on();
    void led_off();
    void titlelabeltimeout();
    void cycleplaytimemode();
    void cycletimeout();

    void performances(int);
    void purchases(int);
    void information(int);
    void jumpTracks();

    void make_random_list(); /* koz: 15/01/00 */

protected slots:
    void configDone();
    void configureKeys();
    void setIcons();

protected:
    // mostly start up stuff
    void readSettings();
    void initFont();
    void initWorkMan();
    void drawPanel();
    void setupPopups();
    void setLEDs(const QString& symbols);

    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent* e );
    bool event( QEvent *e );
    //    void focusOutEvent(QFocusEvent *e);
    void playtime();
    bool getArtist(QString& artist);

    void clearSongList();
    void setSongListTo(int whichTrack);
    void populateSongList();

private:
    ConfigDlg       *configDialog;
    CDDialog        *cddialog;
    QPopupMenu      *mainPopup;
    QPopupMenu      *purchPopup;
    QPopupMenu      *infoPopup;

    // ML XXX
    QGridLayout		*outerLO;

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
    QTimer              timer;
    QTimer              titlelabeltimer;
    QTimer              queryledtimer;
    QTimer              cycletimer;
    QTimer              jumpTrackTimer;

    // random playlists
    KRandomSequence     randSequence;
    RandomList          random_list;
    RandomList::iterator random_current;


    int                 jumpToTrack;
    unsigned int        skipDelta;
    int                 volume;
    LedLamp             *queryled;
    LedLamp             *loopled;
    bool                randomplay;
    bool                randomplay_pending;
    void song_list_complete(void);
    bool                looping;
    bool                cddrive_is_ok;
    bool                have_new_cd;
    int                 time_display_mode;
    QString             cd_device_str;
    QString             audio_system_str; /* arts, alsa or sun. empty means no cdda */
    QString             audio_device_str; /* for alsa hw:0,0, for sun /dev/audio */
    QStringList         audio_systems_list;

    QPushButton         *makeButton( int, int, int, int, const QString& );



    void cdtext(struct cdtext_info* p_cdtext);

  /**
   * select a random track from the current disc.
   *
   */
    int                 next_randomtrack();
    int                 prev_randomtrack();
    int                 real_randomtrack();

  /**
   * set the artist and title labels as well as the dock tooltip.
   *
   */
    void setArtistAndTitle(const QString& artist, const QString& title);

    int             tmppos;
    int             save_track;

    QStringList     pathlist;
    QStringList     tracktitlelist;
    QStringList     playlist;
    int             playlistpointer;
    QStringList     extlist;
    QString         category;
    QString         genre;
    QFont           smallfont;
    QFont           verysmallfont;
    bool            docking;
    bool            autoplay;
    bool            stopexit;
    bool            ejectonfinish;
    bool            digitalplayback;
    bool            currentlyejected;

// cddb support
public slots:
    void cddb_done(CDDB::Result);
    void cddb_failed();
    void cddb_no_info();
    void mycddb_inexact_read();
    void CDDialogSelected();
    void CDDialogDone();
    void get_cddb_info(bool);

private:
    KCDDB::Client*  cddb;
    QString         xmcd_data;
    QStringList     cddbserverlist;
    QString         current_server;
    QStringList     cddbsubmitlist;
    bool            updateDialog;
    int             revision;
    KActionCollection* m_actions;
    KVolumeControl* m_volume;
};



#endif

