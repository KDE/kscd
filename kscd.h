/*
   Kscd - A simple cd player for the KDE Project

   $Id$

   Copyright (c) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __KSCD__
#define __KSCD__

#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QCloseEvent>

// CD support.
class KCompactDisc;

// CDDB support via libkcddb
#include <libkcddb/kcddb.h>
#include <libkcddb/client.h>


#include <QPushButton>
#include <qdialog.h>
#include <qapplication.h>
#include <QTimer>
#include <qscrollbar.h>
#include <qslider.h>
#include <q3tabdialog.h>

#include "ledlamp.h"
#include "ui_panel.h"
#include "prefs.h"
#include "configWidget.h"
#include <kapplication.h>
#include <kconfigdialog.h>
#include <k3process.h>
#include <krandomsequence.h>
#include <kglobalaccel.h>
#include <ksessionmanager.h>

class CDDBDlg;
class DockWidget;
class QGridLayout;
class KActionCollection;
class KToggleAction;

class KSCD : public QWidget, Ui::kscdPanelDlg, public KSessionManager {

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KSCD")

public /*Q_SLOTS*/slots:
    Q_SCRIPTABLE bool playing();
    Q_SCRIPTABLE void play() { playClicked(); }
    Q_SCRIPTABLE void stop() { stopClicked(); }
    Q_SCRIPTABLE void previous() { prevClicked(); }
    Q_SCRIPTABLE void next() { nextClicked(); }
    Q_SCRIPTABLE void jumpTo(int seconds) { jumpToTime(seconds); }
    Q_SCRIPTABLE void eject() { ejectClicked(); }
    Q_SCRIPTABLE void quit() { quitClicked(); }
    Q_SCRIPTABLE void toggleLoop() { loopClicked(); }
    Q_SCRIPTABLE void toggleShuffle() { randomClicked(); }
    Q_SCRIPTABLE void toggleTimeDisplay() { cycleplaytimemode(); }
    Q_SCRIPTABLE void cddbDialog() { CDDialogSelected(); }
    Q_SCRIPTABLE void optionDialog() { showConfig(); }
    Q_SCRIPTABLE void setTrack(int t) { trackSelected(t > 0 ? t - 1 : 0); }
    Q_SCRIPTABLE void volumeDown() { decVolume(); }
    Q_SCRIPTABLE void volumeUp() { incVolume(); }
    Q_SCRIPTABLE void setVolume(int v);
    Q_SCRIPTABLE void setDevice(const QString& dev);
    Q_SCRIPTABLE int getVolume() { return Prefs::volume(); }
    Q_SCRIPTABLE int currentTrack();
    Q_SCRIPTABLE int currentTrackLength();
    Q_SCRIPTABLE int currentPosition();
    Q_SCRIPTABLE int getStatus();
    Q_SCRIPTABLE QString currentTrackTitle();
    Q_SCRIPTABLE QString currentAlbum();
    Q_SCRIPTABLE  QString currentArtist();
    Q_SCRIPTABLE QStringList trackList();

public:
    explicit KSCD(QWidget *parent = 0);
    ~KSCD();
    virtual bool saveState(QSessionManager& sm);

    void setDocking(bool dock);
    void setDevicePaths();

signals:
    void tooltipCurrentTrackChanged(const QString &);

public slots:
    void setColors();
    void writeSettings();
    void playClicked();
    void nextClicked();
    void prevClicked();
    void stopClicked();
    void ejectClicked();
    void jumpToTime(int);
    void quitClicked();
    void trackSelected(int);
    void showConfig();
    void incVolume();
    void decVolume();
    void volChanged(int);
    void led_on();
    void led_off();
    void titlelabeltimeout();
    void togglequeryled();
    void cycleplaytimemode();
    void showVolumeInLabel();
    void showArtistLabel(QString);
    void restoreArtistLabel();

    void randomClicked();	
    void randomChanged(bool);
    void loopClicked();
    void loopChanged(bool);

    void information(QAction *action);

protected:
    // mostly start up stuff
    void readSettings();
    void drawPanel();
    void setupPopups();
    void setLEDs(int seconds);
    void resetTimeSlider(bool enabled);

    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool event(QEvent *e);

    void populateSongList();
    void updateConfigDialog(configWidget *widget);

private:
    KConfigDialog   *configDialog;
    CDDBDlg         *cddialog;
    KCompactDisc    *m_cd;
    QTimer           titlelabeltimer;
    QTimer           queryledtimer;
    bool             updateTime;

    /**
     * Info from CDDB, and exploded versions thereof.
     */
    KCDDB::CDInfo cddbInfo;
    KCDDB::Client *cddb;
    KActionCollection *m_actions;
    KToggleAction *m_togglePopupsAction;
    DockWidget *m_dockWidget;

public slots:
    void lookupCDDB();

private slots:
    void CDDialogSelected();
    void CDDialogDone();
    void setCDInfo(KCDDB::CDInfo);
    void lookupCDDBDone(KCDDB::Result);
    void trackChanged(unsigned);
    void trackPosition(unsigned);
    void discChanged(unsigned);
	void discInformation(KCompactDisc::DiscInfo);
    void discStatusChanged(KCompactDisc::DiscStatus);
    void configDone();
    void configureKeys();
    void setIcons();

    void timeSliderPressed();
    void timeSliderReleased();
    void timeSliderMoved(int seconds);
};

#endif
