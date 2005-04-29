/*
   Kscd - A simple cd player for the KDE Project

   $Id$

   Copyright (c) 1997 Bernd Johannes Wuebben math.cornell.edu
   Copyright (c) 2002 Aaron J. Seigo <aseigo@kde.org>
   Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>

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

// CD support.
class KCompactDisc;

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

#include "ledlamp.h"
#include "panel.h"
#include "prefs.h"
#include "configWidget.h"
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kprocess.h>
#include <krandomsequence.h>
#include <dcopobject.h>
#include <kglobalaccel.h>

class CDDBDlg;
class DockWidget;
class QGridLayout;
class KActionCollection;
class KToggleAction;
class KVolumeControl;

using namespace KCDDB;

typedef QValueList<int> RandomList;

class KSCD : public kscdPanelDlg, public KSessionManaged, virtual public DCOPObject {

    Q_OBJECT
    K_DCOP

    // time display modes
    enum time_display { TRACK_SEC = 0, TRACK_REM = 1, TOTAL_SEC = 2, TOTAL_REM = 3 };


k_dcop:
    bool playing();
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
    void setTrack(int t) { trackSelected(t > 0 ? t - 1 : 0); }
    void volumeDown() { decVolume(); }
    void volumeUp() { incVolume(); }
    void setVolume(int v);
    void setDevice(const QString& dev);
    int  getVolume() { return Prefs::volume(); }
    int currentTrack();
    int currentTrackLength();
    int currentPosition();
    int getStatus();
    QString currentTrackTitle();
    QString currentAlbum();
    QString currentArtist();
    QStringList trackList();

public:
    KSCD( QWidget *parent = 0, const char *name = 0 );
    ~KSCD();
    virtual bool saveState(QSessionManager& sm);

    void setDocking(bool dock);
    bool digitalPlayback();
    void setDevicePaths();
    QStringList audioSystems() { return audio_systems_list; }

    KActionCollection* actionCollection() { return m_actions; }

signals:
    void trackChanged(const QString&);

public slots:
    void setColors();
    void togglequeryled();
    void randomSelected();
    void setShuffle(int shuffle); /* 0 -off, 1 - on, 2 - remake random list */
    void writeSettings();
    void playClicked();
    bool nextClicked();
    void prevClicked();
    void stopClicked();
    void ejectClicked();
    void jumpToTime(int seconds, bool forcePlay = false);
    void quitClicked();
    void loopOn();
    void loopOff();
    void loopClicked();
    void trackSelected(int);
    void showConfig();
    void incVolume();
    void decVolume();
    void volChanged(int);
    void led_on();
    void led_off();
    void titlelabeltimeout();
    void cycleplaytimemode();
    void cycletimeout();

    void information(int);
    void jumpTracks();

    void make_random_list(); /* koz: 15/01/00 */

protected:
    // mostly start up stuff
    void readSettings();
    void initFont();
    void drawPanel();
    void setupPopups();
    void setLEDs(int milliseconds);
    void resetTimeSlider(bool enabled);

    void dragTime(int sec);

    void closeEvent(QCloseEvent *e);
    void keyPressEvent( QKeyEvent* e );
    bool event( QEvent *e );
    //    void focusOutEvent(QFocusEvent *e);
    void playtime();
    void playtime(int seconds);
    void calculateDisplayedTime();
    void calculateDisplayedTime(int sec);
    void setSongListTo(int whichTrack);
    void populateSongList(QString infoStatus);
    void updatePlayPB(bool playing);

    void updateConfigDialog(configWidget* widget);

private:
    KConfigDialog   *configDialog;
    CDDBDlg         *cddialog;
    QPopupMenu      *mainPopup;
    QPopupMenu      *infoPopup;

    BW_LED_Number       *trackTimeLED[6];

    KCompactDisc *m_cd;
    QTimer              titlelabeltimer;
    QTimer              queryledtimer;
    QTimer              cycletimer;
    QTimer              jumpTrackTimer;

    // random playlists
    KRandomSequence     randSequence;
    RandomList          random_list;
    RandomList::iterator random_current;


    int                 jumpToTrack;
    LedLamp             *queryled;
    LedLamp             *loopled;
    bool                randomplay_pending;
    bool                updateTime;
    QStringList         audio_systems_list;

  /**
   * select a random track from the current disc.
   *
   */
    int                 next_randomtrack();
    int                 prev_randomtrack();
    int                 real_randomtrack();

    void setTitle(int track);

    /**
     * Info from CDDB, and exploded versions thereof.
     */
    KCDDB::CDInfo cddbInfo;
    QStringList     playlist;
    KCDDB::Client*  cddb;
    KActionCollection* m_actions;
    KGlobalAccel* m_globalAccel;
    KToggleAction* m_togglePopupsAction;
    KVolumeControl* m_volume;
    DockWidget* m_dockWidget;
    void lookupDevice();
    void initGlobalShortcuts();
public slots:
    void lookupCDDB();

private slots:
    void CDDialogSelected();
    void CDDialogDone();
    void lookupCDDBDone(CDDB::Result);
    void discStopped();
    void trackUpdate(unsigned track, unsigned trackPosition);
    void trackChanged(unsigned track, unsigned trackLength);
    void discChanged(unsigned discId);
//    void trayClosing();
    void trayOpening();
    void configDone();
    void configureKeys();
    void configureGlobalKeys();
    void setIcons();

    void timeSliderPressed();
    void timeSliderReleased();
    void timeSliderMoved(int milliseconds);
};



#endif

