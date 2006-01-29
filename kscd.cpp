/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
 * Copyright (c) 2003-2006 Richard Lärkäng <nouseforaname@home.se>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qapplication.h>
#include <qgroupbox.h>
#include <qsqlpropertymap.h>

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kaccel.h>
#include <kaction.h>
#include <dcopref.h>
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
#include <kinputdialog.h>
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
#include "version.h"
#include "prefs.h"

#include <kwin.h>
#include <netwm.h>
#include <stdlib.h>

#include <config.h>

#include "cddbdlg.h"
#include "configWidget.h"
#include <qtextcodec.h>
#include <kcompactdisc.h>
#include <fixx11h.h>

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = false;


/****************************************************************************
                  The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name )
  : DCOPObject("CDPlayer"),
    kscdPanelDlg( parent, name, Qt::WDestructiveClose ),
    configDialog(0L),
    cddialog(0L),  //!!!!
    jumpToTrack(0L),
    updateTime(true),
    m_dockWidget(0)
{
  m_cd = new KCompactDisc();
  cddbInfo.clear(); // The first freedb revision is "0" //!!!!
  random_current      = random_list.begin();

  cddb = new KCDDB::Client();
  connect(cddb, SIGNAL(finished(CDDB::Result)), this, SLOT(lookupCDDBDone(CDDB::Result)));

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

  // the time slider
  timeIcon->setPixmap(SmallIcon("player_time"));
  connect(timeSlider, SIGNAL(sliderPressed()), SLOT(timeSliderPressed()));
  connect(timeSlider, SIGNAL(sliderReleased()), SLOT(timeSliderReleased()));
  connect(timeSlider, SIGNAL(sliderMoved(int)), SLOT(timeSliderMoved(int)));
  connect(timeSlider, SIGNAL(valueChanged(int)), SLOT(jumpToTime(int)));

  // the volume slider
  volumeIcon->setPixmap(SmallIcon("player_volume"));
  volumeSlider->setValue(Prefs::volume());
  QString str;
  str = QString::fromUtf8( QCString().sprintf(i18n("Vol: %02d%%").utf8(), Prefs::volume()) );
  volumelabel->setText(str);
  connect(volumeSlider, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));

  /* FIXME check for return value */
  setDevicePaths(/*Prefs::cdDevice(), Prefs::audioSystem(), Prefs::audioDevice()*/);
  connect(m_cd, SIGNAL(trackPlaying(unsigned, unsigned)), this, SLOT(trackUpdate(unsigned, unsigned)));
  connect(m_cd, SIGNAL(trackPaused(unsigned, unsigned)), this, SLOT(trackUpdate(unsigned, unsigned)));
  connect(m_cd, SIGNAL(trackChanged(unsigned, unsigned)), this, SLOT(trackChanged(unsigned, unsigned)));
  connect(m_cd, SIGNAL(discStopped()), this, SLOT(discStopped()));
  connect(m_cd, SIGNAL(discChanged(unsigned)), this, SLOT(discChanged(unsigned)));
  connect( &queryledtimer, SIGNAL(timeout()),  SLOT(togglequeryled()) );
  connect( &titlelabeltimer, SIGNAL(timeout()),  SLOT(titlelabeltimeout()) );
  connect( &cycletimer, SIGNAL(timeout()),  SLOT(cycletimeout()) );
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
  QToolTip::remove(songListCB);
  QToolTip::add(songListCB, i18n("Track list"));


  // set up the actions and keyboard accels
  m_actions = new KActionCollection(this);

  KAction* action;
  action = new KAction(i18n("Play/Pause"), Key_P, this, SLOT(playClicked()), m_actions, "Play/Pause");
  action = new KAction(i18n("Stop"), Key_S, this, SLOT(stopClicked()), m_actions, "Stop");
  action = new KAction(i18n("Previous"), Key_B, this, SLOT(prevClicked()), m_actions, "Previous");
  action = new KAction(i18n("Next"), Key_N, this, SLOT(nextClicked()), m_actions, "Next");
  action = KStdAction::quit(this, SLOT(quitClicked()), m_actions);
  action = KStdAction::keyBindings(this, SLOT(configureKeys()), m_actions, "options_configure_shortcuts");
  action = KStdAction::keyBindings(this, SLOT(configureGlobalKeys()), m_actions, "options_configure_globals");
  action = KStdAction::preferences(this, SLOT(showConfig()), m_actions);
  action = new KAction(i18n("Loop"), Key_L, this, SLOT(loopClicked()), m_actions, "Loop");
  action = new KAction(i18n("Eject"), CTRL + Key_E, this, SLOT(ejectClicked()), m_actions, "Eject");
  action = new KAction(i18n("Increase Volume"), Key_Plus, this, SLOT(incVolume()), m_actions, "IncVolume");
  KShortcut increaseVolume = action->shortcut();
  increaseVolume.append( KKey( Key_Equal ) );
  action->setShortcut( increaseVolume );
  action = new KAction(i18n("Decrease Volume"), Key_Minus, this, SLOT(decVolume()), m_actions, "DecVolume");
  action = new KAction(i18n("Options"), CTRL + Key_T, this, SLOT(showConfig()), m_actions, "Options");
  action = new KAction(i18n("Shuffle"), Key_R, this, SLOT(randomSelected()), m_actions, "Shuffle");
  action = new KAction(i18n("CDDB"), CTRL + Key_D, this, SLOT(CDDialogSelected()), m_actions, "CDDB");
  
  m_actions->readShortcutSettings("Shortcuts");
  
  m_actions->action( "options_configure_globals" )->setText( i18n( "Configure &Global Shortcuts..." ) );

  kapp->installKDEPropertyMap();
  QSqlPropertyMap *map = QSqlPropertyMap::defaultMap();
  map->insert("KComboBox", "currentText");
  
  initGlobalShortcuts();
  
  setupPopups();

  if (Prefs::looping())
  {
    loopled->on();
    loopled->show();
    repeatPB->setOn(true);
  }

  setDocking(Prefs::docking());

  setFocusPolicy(QWidget::NoFocus);

  songListCB->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  adjustSize();
  setFixedHeight(this->height());
} // KSCD


KSCD::~KSCD()
{
    delete cddb;
    delete m_cd;
} // ~KSCD


void KSCD::initGlobalShortcuts() {

  m_globalAccel = new KGlobalAccel( this );
  
  //Definition of global shortcuts is based on 'local' shortcuts which follow
  //the WIN key.
  m_globalAccel->insert("Next", i18n("Next"), 0, KKey("WIN+N"), KKey("WIN+Right"),
                        this, SLOT(nextClicked()));
  //NOTE: WIN+B collidates with amarok's default global shortcut.
  m_globalAccel->insert("Previous", i18n("Previous"), 0, KKey("WIN+B"), KKey("WIN+Left"),
                        this, SLOT(prevClicked()));
  m_globalAccel->insert("Play/Pause", i18n("Play/Pause"), 0, KKey("WIN+P"), 0,
                        this, SLOT(playClicked()));
  m_globalAccel->insert("Stop", i18n("Stop"), 0, KKey("WIN+S"), 0,
                        this, SLOT(stopClicked()));
  m_globalAccel->insert("IncVolume", i18n("Increase Volume"), 0, KKey("WIN+Plus"), KKey("WIN+Up"),
                        this, SLOT(incVolume()));
  m_globalAccel->insert("DecVolume", i18n("Decrease Volume"), 0, KKey("WIN+Minus"), KKey("WIN+Down"),
                        this, SLOT(decVolume()));
  m_globalAccel->insert("Shuffle", i18n("Shuffle"), 0, KKey("WIN+R"), 0,
                        this, SLOT(incVolume()));
  
  m_globalAccel->setConfigGroup( "GlobalShortcuts" );
  m_globalAccel->readSettings( kapp->config() );
  m_globalAccel->updateConnections();
}

bool KSCD::digitalPlayback() {
#if defined(BUILD_CDDA)
        return !(Prefs::audioSystem().isEmpty());
#else
        return false;
#endif
}

void KSCD::setVolume(int v)
{
    volChanged(v);
    volumeSlider->setValue(v);
}

void KSCD::setDevice(const QString& dev)
{
    Prefs::self()->setCdDevice(dev);
    setDevicePaths();
}

/**
 * Initialize smallfont which fits into the 13 and 14 pixel widgets.
 */
void KSCD::initFont()
{
/*  int theSmallPtSize = 10;

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
*/
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

  setLEDs(-1);

  queryled = new LedLamp(symbols);
  queryled->move(+10, D + 1);
  queryled->off();
  queryled->hide();

  loopled = new LedLamp(symbols, LedLamp::Loop);
  loopled->move(+10, D + 18);
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

void KSCD::setupPopups()
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
    //NEW add the shortcut dialogs
    m_actions->action("options_configure_globals")->plug(mainPopup);
    m_actions->action("options_configure_shortcuts")->plug(mainPopup);
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
    if (m_cd->discId() == KCompactDisc::missingDisc)
        return;

    kapp->processEvents();
    kapp->flushX();

    if (!m_cd->isPlaying())
    {
        kapp->processEvents();
        kapp->flushX();

        if (m_cd->isPaused())
        {
            // Unpause (!!).
            m_cd->pause();
        }
        else
        {
            setLEDs(0);
            resetTimeSlider(true);

            if(Prefs::randomPlay())
            {
                make_random_list();
                // next clicked handles updating the play button, etc.
                nextClicked();
            }
            else
            {
                m_cd->play(0, 0, playlist.isEmpty() ? 0 : 1);
            }
        }

        // Update UI to allow a subsequent pause.
        statuslabel->setText(i18n("Play"));
        playPB->setIconSet(SmallIconSet("player_pause"));
        playPB->setText(i18n("Pause"));
    }
    else
    {
        m_cd->pause();

        // Update UI to allow a subsequent play.
        statuslabel->setText(i18n("Pause"));
        playPB->setIconSet(SmallIconSet("player_play"));
        playPB->setText(i18n("Play"));
    }

    kapp->processEvents();
    kapp->flushX();
} // playClicked()

void KSCD::setShuffle(int shuffle)
{
    if (shuffle == 2) {
        if(Prefs::randomPlay() && m_cd->tracks() > 0) {
            shufflePB->blockSignals(true);
            shufflePB->setOn(true);
            shufflePB->blockSignals(false);
            make_random_list(); /* koz: Build a unique, once, random list */
            if(m_cd->isPlaying())
                nextClicked();
        }

        return;
    }

    Prefs::setRandomPlay(shuffle);
    shufflePB->blockSignals(true);
    shufflePB->setOn(shuffle);
    shufflePB->blockSignals(false);

    if (Prefs::randomPlay() && m_cd->tracks() > 0) {
        make_random_list(); /* koz: Build a unique, once, random list */
        if(m_cd->isPlaying())
            nextClicked();
    }
}

void KSCD::stopClicked()
{
    stoppedByUser = true;

    kapp->processEvents();
    kapp->flushX();
    m_cd->stop();
} // stopClicked()

void KSCD::prevClicked()
{
    int track = m_cd->track();

    if (Prefs::randomPlay()) {
        track = prev_randomtrack();
        if (track == -1) {
            return;
        }
    } else {
        if (track <= 1) {
            if (Prefs::looping()) {
                track = m_cd->tracks();
            } else {
                return;
            }
        } else {
            track--;
        }
    }

    kapp->processEvents();
    kapp->flushX();
    m_cd->play(track, 0, playlist.isEmpty() ? 0 : track);
} // prevClicked()

bool KSCD::nextClicked()
{
    unsigned track = m_cd->track();

    if (Prefs::randomPlay()) {
        track = next_randomtrack();
        if(track == 0) {
            return false;
        }
    } else {
        if(track < 1) {
            track = 1;
        } else if (track >= m_cd->tracks()) {
            if (Prefs::looping()) {
                track = 1;
            } else {
                return true;
            }
        } else {
            track++;
        }
    }

    kapp->processEvents();
    kapp->flushX();
    m_cd->play(track, 0, Prefs::randomPlay() || !playlist.isEmpty() ? track + 1 : 0);
    return true;
} // nextClicked()

void KSCD::trackChanged(unsigned track, unsigned trackLength)
{
    QString tooltip = artistlabel->text();
    if (track < 1)
    {
        setLEDs(-1);
        resetTimeSlider(true);
        tracklabel->setText("--/--");
        titlelabel->clear();
    }
    else
    {
//             if (!nextClicked())
//             {
//                 statuslabel->setText(i18n("Disc Finished"));
//                 m_cd->stop();
//             }
//            break;

        if (songListCB->count())
        {
            songListCB->setCurrentItem(track - 1);
            // drop the number.
            // for Mahlah, a picky though otherwise wonderful person - AJS
            QString justTheName = songListCB->currentText();
            justTheName = justTheName.right(justTheName.length() - 4);

            QToolTip::remove(songListCB);
            QToolTip::add(songListCB, i18n("Current track: %1").arg(justTheName));
        }
        timeSlider->blockSignals(true);
        timeSlider->setRange(0, trackLength ? trackLength - 1 : 0);
        timeSlider->blockSignals(false);
        QString str;
        str.sprintf("%02d/%02d", track, m_cd->tracks());
        tracklabel->setText(str);

        QString title = cddbInfo.trackInfoList[track-1].title;
        titlelabel->setText(title);
        tooltip += "/";
        tooltip += KStringHandler::rsqueeze(title, 30);
    }
    emit trackChanged(tooltip);
} //trackChanged(int track)


void KSCD::jumpToTime(int ms, bool forcePlay)
{
    kapp->processEvents();
    kapp->flushX();

    int track = m_cd->track();
    if ((m_cd->isPlaying() || forcePlay) &&
        ms < (int)m_cd->trackLength())
    {
        if(Prefs::randomPlay() || !playlist.isEmpty())
        {
            m_cd->play(track, ms, track + 1);
        }
        else
        {
            m_cd->play(track, ms);
        }
    }
} // jumpToTime(int ms)

void KSCD::timeSliderPressed()
{
    updateTime = false;
} // timeSliderPressed()

void KSCD::timeSliderMoved(int milliseconds)
{
    setLEDs(milliseconds);
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
    jumpTrackTimer.stop();

    writeSettings();
    //setShuffle(0);
    statuslabel->clear();
    setLEDs(-1);

    // Good GOD this is evil
    kapp->processEvents();
    kapp->flushX();

    if(Prefs::stopExit())
        m_cd->stop();

    delete m_cd;

    kapp->quit();
} // quitClicked()

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
    m_cd->eject();
} // ejectClicked

void KSCD::closeEvent(QCloseEvent *e)
{
    if (Prefs::docking() && !kapp->sessionSaving())
    {
        hide();
        e->ignore();
        return;
    }
    e->accept();
}

void KSCD::randomSelected()
{
    setShuffle(Prefs::randomPlay()?0:1);

    /* FIXME this helps us to display "Random" in Status line
       should it maybe to be replaced with symbol "RAND" or something others */
    statuslabel->setText(Prefs::randomPlay()?i18n("Random"):i18n("Play"));
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

    m_cd->play(track, 0);
} // trackSelected

void KSCD::updateConfigDialog(configWidget* widget)
{
    if(!widget)
        return;

    static QString originalTitleOfGroupBox = widget->groupBox3->title();
    if(m_cd->isPlaying()) {
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
    configDialog->addPage(confWidget, i18n("CD Player"), "kscd", i18n("Settings & Behavior"));

    // libkcddb page
    KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
    if (libkcddb && libkcddb->isValid())
    {
        KCModuleInfo info(libkcddb->desktopEntryPath());
        if (info.service()->isValid())
        {
            KCModule *m = KCModuleLoader::loadModule(info, KCModuleLoader::Inline);
            if (m)
            {
                m->load();
                KCDDB::Config* cfg = new KCDDB::Config();
                cfg->readConfig();
                configDialog -> addPage(m, cfg, QString("CDDB"), "cdtrack", i18n("Configure Fetching Items"));

                connect(configDialog, SIGNAL(okClicked()), m, SLOT(save()));
                connect(configDialog, SIGNAL(applyClicked()), m, SLOT(save()));
                connect(configDialog, SIGNAL(defaultClicked()), m, SLOT(defaults()));
            }
        }
    }

    updateConfigDialog(confWidget);

    connect(configDialog, SIGNAL(settingsChanged()), this, SLOT(configDone()));
    configDialog -> show();
} // showConfig()

void KSCD::configDone()
{
    setColors();
    setDocking(Prefs::docking());

    setDevicePaths();

    volumeIcon->setEnabled(!Prefs::digitalPlayback());
    volumeSlider->setEnabled(!Prefs::digitalPlayback());

    // dialog deletes itself
    configDialog = 0L;
}

void KSCD::configureKeys()
{
    KKeyDialog::configure(m_actions, this);
}

void KSCD::configureGlobalKeys()
{
  KKeyDialog::configure(m_globalAccel, true, this, true);
}

void KSCD::setDevicePaths()
{
    if (!m_cd->setDevice(Prefs::cdDevice(), Prefs::volume(), Prefs::digitalPlayback(),
                         Prefs::audioSystem(), Prefs::audioDevice()))
    {
        // This device did not seem usable.
        QString str = i18n("CD-ROM read or access error (or no audio disc in drive).\n"\
                            "Please make sure you have access permissions to:\n%1").arg(
                             KCompactDisc::urlToDevice(Prefs::cdDevice()));
        KMessageBox::error(this, str, i18n("Error"));
    }
} // setDevicePath()

void KSCD::setDocking(bool dock)
{
    Prefs::setDocking(dock);
    if (Prefs::docking())
    {
        if (!m_dockWidget)
        {
            m_dockWidget = new DockWidget(this, "dockw");
            connect(m_dockWidget, SIGNAL(quitSelected()), this, SLOT(quitClicked()));
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
    QString str;
    str = QString::fromUtf8( QCString().sprintf(i18n("Vol: %02d%%").utf8(), vol) );
    volumelabel->setText(str);
    m_cd->setVolume(vol);
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
    for(unsigned i = 0; i < m_cd->tracks(); i++)
    {
        do {
            selected = 1 + (int) randSequence.getLong(m_cd->tracks());
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
            m_cd->stop();
            return 0;
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

void KSCD::discChanged(unsigned discId)
{
    cddbInfo.clear();
    if (discId == KCompactDisc::missingDisc)
    {
        statuslabel->setText(i18n("No disc"));
    }
    else
    {
        cddbInfo.id = QString::number(discId, 16).rightJustify(8,'0');
        cddbInfo.length = m_cd->discLength() / 1000;
        cddbInfo.artist = m_cd->discArtist();
        cddbInfo.title = m_cd->discTitle();

        // If it's a sampler, we'll do artist/title.
        bool isSampler = (cddbInfo.title.compare("Various") == 0);
        KCDDB::TrackInfo track;
        for (unsigned i = 1; i <= m_cd->tracks(); i++)
        {
            if (isSampler)
            {
                track.title = m_cd->trackArtist(i);
                track.title.append("/");
                track.title.append(m_cd->trackTitle(i));
            }
            else
            {
                track.title = m_cd->trackTitle(i);
            }

            // FIXME: KDE4
            // track.length = cd->trk[i - 1].length;
            cddbInfo.trackInfoList.append(track);
        }
    }

    // Set the total time.
    QTime dml;
    dml = dml.addSecs(m_cd->discLength() / 1000);

    QString fmt;
    if(dml.hour() > 0)
        fmt.sprintf("%02d:%02d:%02d",dml.hour(),dml.minute(),dml.second());
    else
        fmt.sprintf("%02d:%02d",dml.minute(),dml.second());
    totaltimelabel->setText(fmt);

    trackChanged(0, 0);
    populateSongList("");
    //totaltimelabel->clear();
    totaltimelabel->lower();

    if ((Prefs::autoplay() || KCmdLineArgs::parsedArgs()->isSet("start"))
        && !m_cd->isPlaying())
    {
        playClicked();
    }

    // We just populated the GUI with what we got from the CD. Now look for
    // more from the Internet...
    lookupCDDB();
}

void KSCD::discStopped()
{
    if (Prefs::ejectOnFinish() && !stoppedByUser && (m_cd->discId() != KCompactDisc::missingDisc))
    {
        ejectClicked();
    }

    if (!stoppedByUser)
    {
      if (Prefs::randomPlay())
      {
        // If nextClicked returns false, it was the last
        // random track, and the player should be stopped
        if (nextClicked())
          return;
      }
      else if (Prefs::looping())
      {
        playClicked();
        return;
      }
    }
    
    statuslabel->setText(i18n("Stopped"));
    playPB->setText(i18n("Play"));
    playPB->setIconSet(SmallIconSet("player_play"));

    /* reset to initial value, only stopclicked() sets this to true */
    stoppedByUser = false;

    trackChanged(0, 0);
    populateSongList("");
    totaltimelabel->clear();
    totaltimelabel->lower();
}

void KSCD::setLEDs(int milliseconds)
{
    QString symbols;

    if (milliseconds < 0)
    {
        symbols = "--:--";
    }
    else
    {
        unsigned mymin;
        unsigned mysec;
        mymin = milliseconds / 60000;
        mysec = (milliseconds % 60000) / 1000;
        symbols.sprintf("%02d:%02d", mymin, mysec);
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

    titlelabel ->setFont( Prefs::ledFont() );
    artistlabel->setFont( Prefs::ledFont() );
    volumelabel->setFont( Prefs::ledFont() );
    statuslabel->setFont( Prefs::ledFont() );
    tracklabel ->setFont( Prefs::ledFont() );
    totaltimelabel->setFont( Prefs::ledFont() );
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

    volumeIcon->setEnabled(!Prefs::digitalPlayback());
    volumeSlider->setEnabled(!Prefs::digitalPlayback());
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

        cddialog->setData(cddbInfo, m_cd->discSignature(), playlist);
        connect(cddialog,SIGNAL(cddbQuery()),SLOT(lookupCDDB()));
        connect(cddialog,SIGNAL(newCDInfoStored(KCDDB::CDInfo)),
            SLOT(setCDInfo(KCDDB::CDInfo)));
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

void KSCD::lookupCDDB()
{
    if (m_cd->discId() == KCompactDisc::missingDisc)
        return;
    kdDebug(67000) << "lookupCDDB() called" << endl;

    populateSongList(i18n("Start freedb lookup."));

    setShuffle(2);

    led_on();

    cddb->config().reparse();
    cddb->setBlockingMode(false);
    cddb->lookup(m_cd->discSignature());
} // lookupCDDB

void KSCD::lookupCDDBDone(CDDB::Result result)
{
    led_off();
    if ((result != KCDDB::CDDB::Success) &&
        (result != KCDDB::CDDB::MultipleRecordFound))
    {
        populateSongList(result == CDDB::NoRecordFound ? i18n("No matching freedb entry found.") : i18n("Error getting freedb entry."));
        return;
    }

    // The intent of the original code here seems to have been to perform the
    // lookup, and then to convert all the string data within the CDDB response
    // using the use Prefs::selectedEncoding() and a QTextCodec. However, that
    // seems to be irrelevant these days.
    KCDDB::CDInfo info = cddb->bestLookupResponse();
    // TODO Why doesn't libcddb not return MultipleRecordFound?
    //if( result == KCDDB::CDDB::MultipleRecordFound ) {
    if( cddb->lookupResponse().count() > 1 ) {
      CDInfoList cddb_info = cddb->lookupResponse();
      CDInfoList::iterator it;
      QStringList list;
      for ( it = cddb_info.begin(); it != cddb_info.end(); ++it  ) {
        list.append( QString("%1, %2, %3").arg((*it).artist).arg((*it).title)
            .arg((*it).genre));
      }

      bool ok(false);
      QString res = KInputDialog::getItem(
              i18n("Select CDDB Entry"),
              i18n("Select a CDDB entry:"), list, 0, false, &ok,
              this );
      if ( ok ) {
        // The user selected and item and pressed OK
        uint c = 0;
        for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
          if( *it == res)  break;
          c++;
        }
        if( c < cddb_info.size() )
          info = cddb_info[c];
      } else {
        return;
        // user pressed Cancel
      }
    }

    setCDInfo(info);

    // In case the cddb dialog is open, update it
    if (cddialog)
      cddialog->setData(cddbInfo, m_cd->discSignature(), playlist);
} // lookupCDDBDone

void KSCD::setCDInfo(KCDDB::CDInfo info)
{
    // Some sanity provisions to ensure that the number of records matches what
    // the CD actually contains.
    while (info.trackInfoList.count() < cddbInfo.trackInfoList.count())
    {
      info.trackInfoList.append(KCDDB::TrackInfo());
    }
    while (info.trackInfoList.count() > cddbInfo.trackInfoList.count())
    {
      info.trackInfoList.pop_back();
    }
    cddbInfo = info;
    populateSongList("");
}

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

void KSCD::trayOpening()
{
    statuslabel->setText(i18n("Ejected"));
    trackChanged(0, 0);
}

int KSCD::currentTrack()
{
    return m_cd->track();
}

int KSCD::currentTrackLength()
{
    return m_cd->trackLength();
}

int KSCD::currentPosition()
{
    return m_cd->trackPosition();
}

int KSCD::getStatus()
{
    if (m_cd->isPlaying())
      return 2;
    else if (m_cd->isPaused())
      return 4;
    else if (m_cd->discId() != KCompactDisc::missingDisc)
      return 5;
    else
      return 6;
}

bool KSCD::playing()
{
    return m_cd->isPlaying();
}

void KSCD::trackUpdate(unsigned /*track*/, unsigned trackPosition)
{
    unsigned tmp;

    switch (Prefs::timeDisplayMode())
    {
    case TRACK_REM:
        tmp = m_cd->trackLength() - trackPosition;
        break;

    case TOTAL_SEC:
        tmp = m_cd->discPosition();
        break;

    case TOTAL_REM:
        tmp = m_cd->discLength() - m_cd->discPosition();
        break;

    case TRACK_SEC:
    default:
        tmp = trackPosition;
        break;
    }
    if (updateTime)
    {
      setLEDs(tmp);
      timeSlider->blockSignals(true);
      timeSlider->setValue(trackPosition);
      timeSlider->blockSignals(false);
    }
}

void KSCD::cycleplaytimemode()
{
    cycletimer.stop();

    if (Prefs::timeDisplayMode() > 2) {
        Prefs::setTimeDisplayMode(TRACK_SEC);
    } else {
        Prefs::setTimeDisplayMode(Prefs::timeDisplayMode() + 1);
    }

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


void KSCD::information(int i)
{
    //kdDebug(67000) << "Information " << i << "\n" << endl;

    if(cddbInfo.artist.isEmpty())
        return;

    QString encodedArtist = KURL::encode_string_no_slash(cddbInfo.artist);

    QString str;

    switch(i)
    {
        case 0:
            str = QString("http://musicmoz.org/cgi-bin/ext.cgi?artist=%1")
                   .arg(encodedArtist);
            break;

         case 1:
            str = QString("http://ubl.artistdirect.com/cgi-bin/gx.cgi/AppLogic+Search?select=MusicArtist&searchstr=%1&searchtype=NormalSearch")
                .arg(encodedArtist);
            break;

        case 2:
            str = QString("http://www.cduniverse.com/cgi-bin/cdubin.exe/rlinka/ean=%1")
                  .arg(encodedArtist);
            break;

        case 3:
            str = QString("http://www.alltheweb.com/search?cat=web&q=%1")
                    .arg(encodedArtist);
            break;

        case 4:
            str = QString("http://altavista.com/web/results?q=%1&kgs=0&kls=1&avkw=xytx")
                  .arg(encodedArtist);
            break;

        case 5:
            str = QString("http://msxml.excite.com/_1_2UDOUB70SVHVHR__info.xcite/dog/results?otmpl=dog/webresults.htm&qkw=%1&qcat=web&qk=20&top=1&start=&ver=14060")
                  .arg(encodedArtist);
            break;

        case 6:
            str = QString("http://www.google.com/search?q=%1")
                  .arg(encodedArtist);
            break;

        case 7:
            str = QString("http://groups.google.com/groups?oi=djq&as_q=%1&num=20")
                  .arg(encodedArtist);
            break;

        case 8:
            str = QString("http://www.hotbot.com/default.asp?prov=Inktomi&query=%1&ps=&loc=searchbox&tab=web")
                  .arg(encodedArtist);
            break;

        case 9:
            str = QString("http://search.lycos.com/default.asp?lpv=1&loc=searchhp&tab=web&query=%1")
                  .arg(encodedArtist);
             break;

         case 10:
             str = QString("http://search.dmoz.org/cgi-bin/search?search=%1")
                   .arg(encodedArtist);
             break;

         case 11:
             str = QString("http://search.yahoo.com/bin/search?p=%1")
                   .arg(encodedArtist);
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

        if (value <= (int)cddbInfo.trackInfoList.count())
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
    if (jumpToTrack > 0 && jumpToTrack <= (int)cddbInfo.trackInfoList.count())
    {
        m_cd->play(jumpToTrack, 0, jumpToTrack + 1);
    }

    jumpToTrack = 0;
} // jumpTracks

QString KSCD::currentTrackTitle()
{
    int track = m_cd->track();
    return (track > -1) ? cddbInfo.trackInfoList[track-1].title : QString::null;
}

QString KSCD::currentAlbum()
{
  return cddbInfo.title;
}

QString KSCD::currentArtist()
{
        return cddbInfo.artist;
}

QStringList KSCD::trackList()
{
  QStringList tracks;
  for (TrackInfoList::const_iterator it = cddbInfo.trackInfoList.begin();
      it != cddbInfo.trackInfoList.end(); ++it)
    tracks << (*it).title;

  return tracks;
}

void KSCD::populateSongList(QString infoStatus)
{
    // set the artist and title labels as well as the dock tooltip.
    if (!infoStatus.isEmpty())
        artistlabel->setText(infoStatus);
    else
        artistlabel->setText(QString("%1 - %2").arg(cddbInfo.artist, cddbInfo.title));

    songListCB->clear();
    for (unsigned i = 0; i < cddbInfo.trackInfoList.count(); i++)
    {
        unsigned tmp = m_cd->trackLength(i + 1);
        unsigned mymin;
        unsigned mysec;
        mymin = tmp / 60000;
        mysec = (tmp % 60000) / 1000;
        QString str1;
        str1.sprintf("%02d: ", i + 1);
        QString str2;
        str2.sprintf(" (%02d:%02d) ", mymin,  mysec);
        str1.append(cddbInfo.trackInfoList[i].title);
        str1.append(str2);
        songListCB->insertItem(str1);
    }

    emit trackChanged(m_cd->track(), m_cd->trackLength());
}

static const KCmdLineOptions options[] =
{
    {"s",0,0},
    {"start",I18N_NOOP("Start playing"),0},
    {"+[device]",I18N_NOOP("CD device, can be a path or a media:/ URL"),0},
    KCmdLineLastOption
};


/**
 * main()
 */
int main( int argc, char *argv[] )
{
    KAboutData aboutData("kscd", I18N_NOOP("KsCD"),
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
    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication::addCmdLineOptions();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (!KUniqueApplication::start())
    {
        fprintf(stderr, "kscd is already running\n");
        if (args->count()>0 || args->isSet("start"))
        {
            DCOPClient client;
            if (client.attach())
            {
                // Forward the command line args to the running instance.
                DCOPRef ref("kscd", "CDPlayer");
                if (args->count() > 0)
                {
                    ref.send("setDevice(QString)", QString(args->arg(0)));
                }
                if (args->isSet("start"))
                {
                    ref.send("play()");
                }
            }
        }
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

    if (args->count()>0) Prefs::self()->setCdDevice(args->arg(0));

    return a.exec();
}

#include "kscd.moc"
