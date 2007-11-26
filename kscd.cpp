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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "docking.h"
#include "kscd.h"
#include "prefs.h"
#include "cddbdlg.h"
#include "configWidget.h"
#include "kcompactdisc.h"

#include <config-alsa.h>


#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QMenu>
#include <QtDBus>

#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kcmdlineargs.h>
#include <khelpmenu.h>
#include <kshortcutsdialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <kuniqueapplication.h>
#include <kcmoduleloader.h>
#include <ktoolinvocation.h>

using namespace KCDDB;

static const char description[] = I18N_NOOP("KDE CD player");

bool stoppedByUser = true;


/****************************************************************************
                  The GUI part
*****************************************************************************/

KSCD::KSCD( QWidget *parent )
  : QWidget( parent ),
    kscdPanelDlg( ),
    configDialog(0L),
    cddialog(0L),  //!!!!
    updateTime(true),
    m_dockWidget(0)
{
	QDBusConnection::sessionBus().registerObject("/CDPlayer", this, QDBusConnection::ExportScriptableSlots);
	setupUi(this);
	m_cd = new KCompactDisc();
	cddbInfo.clear(); // The first freedb revision is "0" //!!!!
	cddb = new KCDDB::Client();
	connect(cddb, SIGNAL(finished(KCDDB::Result)), this, SLOT(lookupCDDBDone(KCDDB::Result)));

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
	showVolumeInLabel();
	connect(volumeSlider, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));

	connect(m_cd, SIGNAL(discChanged(unsigned)), this, SLOT(discChanged(unsigned)));
	connect(m_cd, SIGNAL(discInformation(KCompactDisc::DiscInfo)), this, SLOT(discInformation(KCompactDisc::DiscInfo)));
	connect(m_cd, SIGNAL(discStatusChanged(KCompactDisc::DiscStatus)), this,
		SLOT(discStatusChanged(KCompactDisc::DiscStatus)));

	connect(m_cd, SIGNAL(playoutTrackChanged(unsigned)), this, SLOT(trackChanged(unsigned)));
	connect(m_cd, SIGNAL(playoutPositionChanged(unsigned)), this, SLOT(trackPosition(unsigned)));

	connect(&queryledtimer, SIGNAL(timeout()),  SLOT(togglequeryled()) );
	connect(&titlelabeltimer, SIGNAL(timeout()),  SLOT(titlelabeltimeout()) );

	connect(playPB, SIGNAL(clicked()), SLOT(playClicked()) );
	connect(nextPB, SIGNAL(clicked()), SLOT(nextClicked()) );
	connect(prevPB, SIGNAL(clicked()), SLOT(prevClicked()) );
	connect(stopPB, SIGNAL(clicked()), SLOT(stopClicked()) );
	connect(ejectPB, SIGNAL(clicked()), SLOT(ejectClicked()) );
	connect(songListCB, SIGNAL(activated(int)), SLOT(trackSelected(int)));

	connect(shufflePB, SIGNAL(clicked()), SLOT(randomClicked()));
	connect(repeatPB, SIGNAL(clicked()), SLOT(loopClicked()) );
	connect(m_cd, SIGNAL(randomPlaylistChanged(bool)), this, SLOT(randomChanged(bool)));
	connect(m_cd, SIGNAL(loopPlaylistChanged(bool)), this, SLOT(loopChanged(bool)));

	connect(cddbPB, SIGNAL(clicked()), SLOT(CDDialogSelected()));
	connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(setColors()));
	connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)), this, SLOT(setIcons()));
	songListCB->setToolTip(i18n("Track list"));


	// set up the actions and keyboard accels
	m_actions = new KActionCollection(this);
	m_actions->setConfigGroup("Shortcuts");

	QAction* action;
	action = m_actions->addAction(i18n("Play/Pause"), this, SLOT(playClicked()));
	action->setShortcut(Qt::Key_P);
	qobject_cast<KAction*>(action)->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_P));

	action = m_actions->addAction(i18n("Stop"), this, SLOT(stopClicked()));
	action->setShortcut(Qt::Key_S);
	qobject_cast<KAction*>(action)->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_S));

	action = m_actions->addAction(i18n("Previous"), this, SLOT(prevClicked()));
	action->setShortcut(Qt::Key_B);
	//NOTE: WIN+B collidates with amarok's default global shortcut.
	qobject_cast<KAction*>(action)->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_B));

	action = m_actions->addAction(i18n("Next"), this, SLOT(nextClicked()));
	action->setShortcut(Qt::Key_N);
	qobject_cast<KAction*>(action)->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_N));

	action = m_actions->addAction(KStandardAction::Quit, this, SLOT(quitClicked()));

	action = m_actions->addAction(KStandardAction::KeyBindings, this, SLOT(configureKeys()));

	action = m_actions->addAction(KStandardAction::Preferences, this, SLOT(showConfig()));

	action = m_actions->addAction(i18n("Loop"), this, SLOT(loopClicked()));
	action->setShortcut(Qt::Key_L);
	action = m_actions->addAction(i18n("Eject"), this, SLOT(ejectClicked()));
	action->setShortcut(Qt::CTRL + Qt::Key_E);
	action = m_actions->addAction(i18n("Increase Volume"), this, SLOT(incVolume()));
	action->setShortcuts(KShortcut(QKeySequence(Qt::Key_Plus), QKeySequence(Qt::Key_Equal)));
	qobject_cast<KAction*>(action)->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Plus));

	action = m_actions->addAction(i18n("Decrease Volume"), this, SLOT(decVolume()));
	action->setShortcut(Qt::Key_Minus);
	qobject_cast<KAction*>(action)->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Minus));

	action = m_actions->addAction(i18n("Options"), this, SLOT(showConfig()));
	action->setShortcut(Qt::CTRL + Qt::Key_T);
	action = m_actions->addAction(i18n("Shuffle"), this, SLOT(randomClicked()));
	action->setShortcut(Qt::Key_R);
	qobject_cast<KAction*>(action)->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_R));

	action = m_actions->addAction(i18n("CDDB"), this, SLOT(CDDialogSelected()));
	action->setShortcut(Qt::CTRL + Qt::Key_D);

	m_actions->readSettings();

	setupPopups();

	m_cd->setLoopPlaylist(Prefs::looping());
	m_cd->setRandomPlaylist(Prefs::randomPlay());

	setDocking(Prefs::docking());
	setFocusPolicy(Qt::NoFocus);

	songListCB->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	adjustSize();
	setFixedHeight(this->height());

	devices = new HWControler();

/* FIXME check for return value */
	setDevicePaths();
}

KSCD::~KSCD()
{
	delete cddb;
	delete m_cd;
}

void KSCD::setVolume(int v)
{
    volChanged(v);
    volumeSlider->setValue(v);
}

void KSCD::setDevice(const QString& dev)
{
    Prefs::setCdDevice(dev);
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
}

/**
 * drawPanel() constructs KSCD's little black LED area
 * all settings are made via panel.ui
 */
void KSCD::drawPanel()
{
	setIcons();
	adjustSize();

	connect(lcdNumber, SIGNAL(clicked()), this, SLOT(cycleplaytimemode()));

	setLEDs(-1);

	queryled->setVisible(false);
	totaltimelabel->hide();
}

void KSCD::setIcons()
{
    playPB->setIcon(KIcon(SmallIcon("media-playback-start")));
    stopPB->setIcon(KIcon(SmallIcon("media-playback-stop")));
    ejectPB->setIcon(KIcon(SmallIcon("media-eject")));
    prevPB->setIcon(KIcon(SmallIcon("media-skip-backward")));
    nextPB->setIcon(KIcon(SmallIcon("media-skip-forward")));
    cddbPB->setIcon(KIcon(SmallIcon("fileview-text")));
    infoPB->setIcon(KIcon(SmallIcon("system-run")));
}

void KSCD::setupPopups()
{
    QMenu *infoPopup, *mainPopup;

    mainPopup = new QMenu(this);
    infoPB->setMenu(mainPopup);
    infoPopup = mainPopup->addMenu(i18n("Artist Information"));

    connect(infoPopup, SIGNAL(triggered(QAction *)), SLOT(information(QAction *)));

    infoPopup->addAction("MusicMoz");
    infoPopup->addAction("Ultimate Bandlist");
    infoPopup->addAction("CD Universe");
    infoPopup->addSeparator();
    infoPopup->addAction("AlltheWeb");
    infoPopup->addAction("Altavista");
    infoPopup->addAction("Excite");
    infoPopup->addAction("Google");
    infoPopup->addAction("Google Groups");
    infoPopup->addAction("HotBot");
    infoPopup->addAction("Lycos");
    infoPopup->addAction("Open Directory");
    infoPopup->addAction("Yahoo!");

    mainPopup->addAction(m_actions->action(KStandardAction::name(KStandardAction::Preferences)));
    mainPopup->addAction(m_actions->action(KStandardAction::name(KStandardAction::KeyBindings)));
    mainPopup->addSeparator();

    KHelpMenu* helpMenu = new KHelpMenu(this, KGlobal::mainComponent().aboutData(), false);
    mainPopup->addMenu(helpMenu->menu());
    mainPopup->addSeparator();
    mainPopup->addAction(m_actions->action(KStandardAction::name(KStandardAction::Quit)));
}

void KSCD::playClicked()
{
    if (m_cd->isPlaying() || m_cd->isPaused())
        m_cd->pause();
    else
        m_cd->play();
}

void KSCD::stopClicked()
{
    stoppedByUser = true;

    m_cd->stop();
}

void KSCD::prevClicked()
{
    m_cd->prev();
}

void KSCD::nextClicked()
{
	m_cd->next();
}

void KSCD::jumpToTime(int seconds)
{
	m_cd->playPosition(seconds);
}

void KSCD::timeSliderPressed()
{
    updateTime = false;
}

void KSCD::timeSliderMoved(int seconds)
{
    setLEDs(seconds);
    jumpToTime(seconds);
}

void KSCD::timeSliderReleased()
{
    updateTime = true;
}

void KSCD::quitClicked()
{
    // ensure nothing else starts happening
    queryledtimer.stop();
    titlelabeltimer.stop();

    writeSettings();

    m_cd->stop();

    delete m_cd;

    kapp->quit();
}

bool KSCD::event( QEvent *e )
{
    return QWidget::event(e);
}

/**
 * Do everything needed if the user requested to eject the disc.
 *
 */
void KSCD::ejectClicked()
{
    m_cd->eject();
}

void KSCD::closeEvent(QCloseEvent *e)
{
    if (Prefs::docking() && !kapp->sessionSaving())
    {
        hide();
        e->ignore();
        return;
    }
    quitClicked() ;

//    e->accept();
}

void KSCD::randomClicked()
{
    Prefs::setRandomPlay(!Prefs::randomPlay());
    m_cd->setRandomPlaylist(Prefs::randomPlay());
}

void KSCD::randomChanged(bool on)
{
    randomled->setVisible(on);
    shufflePB->setChecked(on);
}

void KSCD::loopClicked()
{
    Prefs::setLooping(!Prefs::looping()) ;
    m_cd->setLoopPlaylist(Prefs::looping());
}

void KSCD::loopChanged(bool on)
{
    loopled->setVisible(on);
    repeatPB->setChecked(on);
}

/**
 * A Track was selected for playback from the drop down box.
 *
 */
void KSCD::trackSelected(int cb_index)
{
    m_cd->playTrack(cb_index + 1);
}

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

    configDialog->setHelp(QString());

    confWidget = new configWidget(this, 0);

    // kscd config page
    configDialog->addPage(confWidget, i18n("CD Player"), "kscd", i18n("Settings & Behavior"));

    // libkcddb page
    KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
    if (libkcddb && libkcddb->isValid())
    {
        KCModuleInfo info(libkcddb->entryPath());
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

    connect(configDialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(configDone()));
    configDialog -> show();
}

void KSCD::configDone()
{
    Prefs::self()->writeConfig();

    setColors();
    setDocking(Prefs::docking());

    setDevicePaths();
    // dialog deletes itself
    configDialog = 0L;
}

void KSCD::configureKeys()
{
    KShortcutsDialog::configure(m_actions, KShortcutsEditor::LetterShortcutsAllowed, this);
}

void KSCD::setDevicePaths()
{
	QString audioSystem;
	if(!Prefs::digitalPlayback()) {
		audioSystem = QString("cdin");
	} else {
		switch(Prefs::audioSystem())
		{
		case Prefs::EnumAudioSystem::phonon:
			audioSystem = QString("phonon");
			break;
		case Prefs::EnumAudioSystem::arts:
			audioSystem = QString("arts");
			break;
		case Prefs::EnumAudioSystem::alsa:
			audioSystem = QString("alsa");
			break;
		case Prefs::EnumAudioSystem::sun:
			audioSystem = QString("sun");
			break;
		default:
			return;
		}
	}

    if (!m_cd->setDevice(Prefs::cdDevice(), Prefs::volume(), Prefs::digitalPlayback(),
         audioSystem, Prefs::audioDevice()))
    {
        // This device did not seem usable.
        QString str = i18n("CD-ROM access error (or error in startup of audio system).\n"\
			"Please make sure you have access permissions to cdrom device:\n"\
			"device '%1'(%2), audio system '%3'",
			Prefs::cdDevice(), KCompactDisc::urlToDevice(Prefs::cdDevice()), audioSystem);
        KMessageBox::error(this, str, i18n("Error"));

    } else {
        kDebug(67000) << "Vendor: " << m_cd->deviceVendor();
        kDebug(67000) << "Model: " << m_cd->deviceModel();
        kDebug(67000) << "Revision: " << m_cd->deviceRevision();
    }

	m_cd->setRandomPlaylist(Prefs::randomPlay());
    m_cd->setLoopPlaylist(Prefs::looping());
}

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
        connect(this, SIGNAL(tooltipCurrentTrackChanged(const QString&)),
                m_dockWidget, SLOT(setToolTip(const QString&)));
        connect(this, SIGNAL(tooltipCurrentTrackChanged(const QString&)),
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
}

void KSCD::decVolume()
{
   int v = Prefs::volume() - 5;

   if (v < 0)
   {
       v = 0;
   }

   volChanged(v);
   volumeSlider->setValue(v);
}

void KSCD::volChanged(int vol)
{
    m_cd->setVolume(vol);
    Prefs::setVolume(vol);
    showVolumeInLabel();
}

void KSCD::setLEDs(int seconds)
{
    QString symbols;

    if (seconds < 0)
    {
        symbols = "--:--";
    }
    else
    {
        unsigned mymin;
        unsigned mysec;
        mymin = seconds / 60;
        mysec = (seconds % 60);
        symbols.sprintf("%02d:%02d", mymin, mysec);
    }

	lcdNumber->display(symbols);
}

void KSCD::resetTimeSlider(bool enabled)
{
    timeSlider->setEnabled(enabled);
    timeSlider->blockSignals(true);
    timeSlider->setValue(0);
    timeSlider->blockSignals(false);
}

void KSCD::setColors()
{
    QColor led_color = Prefs::ledColor();
    QColor background_color = Prefs::backColor();
    QPalette pal(led_color, background_color,
                 led_color, led_color,
                 led_color, led_color, Qt::white);

    backdrop->setPalette(pal);

    titlelabel->setFont(Prefs::ledFont());
    artistlabel->setFont(Prefs::ledFont());
    volumelabel->setFont(Prefs::ledFont());
    statuslabel->setFont(Prefs::ledFont());
    tracklabel->setFont(Prefs::ledFont());
    totaltimelabel->setFont(Prefs::ledFont());
}

void KSCD::readSettings()
{
    if (Prefs::cdDevice().isEmpty())
        Prefs::setCdDevice(KCompactDisc::defaultCdromDeviceName());
}

void KSCD::writeSettings()
{
    Prefs::self()->writeConfig();
}

void KSCD::CDDialogSelected()
{
    if (!cddialog)
    {
        cddialog = new CDDBDlg(this);

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
    if (m_cd->isNoDisc() || m_cd->discId() == 0)
        return;

    kDebug(67000) << "lookupCDDB() called";

    showArtistLabel(i18n("Start freedb lookup."));

    led_on();

    cddb->config().reparse();
    cddb->setBlockingMode(false);
    cddb->lookup(m_cd->discSignature());
}

void KSCD::lookupCDDBDone(Result result)
{
    led_off();
    if ((result != KCDDB::Success) &&
        (result != KCDDB::MultipleRecordFound))
    {
        showArtistLabel(result == NoRecordFound ? i18n("No matching freedb entry found.") :
            i18n("Error getting freedb entry."));
	QTimer::singleShot(3000, this, SLOT(restoreArtistLabel()));
        return;
    }

    // The intent of the original code here seems to have been to perform the
    // lookup, and then to convert all the string data within the CDDB response
    // using the use Prefs::selectedEncoding() and a QTextCodec. However, that
    // seems to be irrelevant these days.
    KCDDB::CDInfo info = cddb->lookupResponse().first();
    // TODO Why doesn't libcddb not return MultipleRecordFound?
    //if( result == KCDDB::MultipleRecordFound ) {
    if(cddb->lookupResponse().count() > 1) {
      CDInfoList cddb_info = cddb->lookupResponse();
      CDInfoList::iterator it;
      QStringList list;
      for ( it = cddb_info.begin(); it != cddb_info.end(); ++it  ) {
        list.append( QString("%1, %2, %3").arg((*it).get(Artist).toString())
            .arg((*it).get(Title).toString()).arg((*it).get(Genre).toString()));
      }

      bool ok(false);
      QString res = KInputDialog::getItem(
              i18n("Select CDDB Entry"),
              i18n("Select a CDDB entry:"), list, 0, false, &ok,
              this );
      if ( ok ) {
        // The user selected and item and pressed OK
        int c = 0;
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
}

void KSCD::setCDInfo(KCDDB::CDInfo info)
{
	// Some sanity provisions to ensure that the number of records matches what
	// the CD actually contains.

	//Q_ASSERT(info.numberOfTracks() == cddbInfo.numberOfTracks());

	cddbInfo = info;
	populateSongList();
	restoreArtistLabel();
}

void KSCD::led_off()
{
    queryledtimer.stop();
    queryled->setVisible(false);
    totaltimelabel->raise();
    totaltimelabel->show();
}

void KSCD::led_on()
{
    totaltimelabel->hide();
    totaltimelabel->lower();
    queryledtimer.start(800);
    queryled->setVisible(true);
}

void KSCD::togglequeryled()
{
    queryled->setVisible(!queryled->isVisible());
}

void KSCD::titlelabeltimeout()
{
    // clear the cddb error message on the title label.
    titlelabeltimer.stop();
    titlelabel->clear();
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
    else if (!m_cd->isNoDisc())
      return 5;
    else
      return 6;
}

bool KSCD::playing()
{
    return m_cd->isPlaying();
}

void KSCD::trackPosition(unsigned trackPosition)
{
    unsigned tmp;

    kDebug(67000) << "trackPosition(" << trackPosition << ")";
    switch (Prefs::timeDisplayMode())
    {
    case Prefs::EnumTimeDisplayMode::TRACK_REM:
        tmp = m_cd->trackLength() - trackPosition;
        break;

    case Prefs::EnumTimeDisplayMode::TOTAL_SEC:
        tmp = m_cd->discPosition();
        break;

    case Prefs::EnumTimeDisplayMode::TOTAL_REM:
        tmp = m_cd->discLength() - m_cd->discPosition();
        break;

    case Prefs::EnumTimeDisplayMode::TRACK_SEC:
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

void KSCD::discChanged(unsigned tracks)
{
	kDebug(67000) << "discChanged(" << tracks << ")";
	if (tracks > 0) {
		populateSongList();

		// Set the total time.
		QTime dml;
		dml = dml.addSecs(m_cd->discLength());

		QString fmt;
		if(dml.hour() > 0)
			fmt.sprintf("%02d:%02d:%02d", dml.hour(), dml.minute(), dml.second());
		else
			fmt.sprintf("%02d:%02d", dml.minute(), dml.second());
		totaltimelabel->setText(fmt);

		if ((Prefs::autoplay() || KCmdLineArgs::parsedArgs()->isSet("start"))
			&& !m_cd->isPlaying())
			playClicked();

		// We just populated the GUI with what we got from the CD. Now look for
		// more from the Internet...
		showArtistLabel(i18n("Start freedb lookup."));

		led_on();

		cddb->config().reparse();
		cddb->setBlockingMode(false);
		cddb->lookup(m_cd->discSignature());
	} else {
		trackChanged(0);
		populateSongList();
		restoreArtistLabel();
		totaltimelabel->hide();
	}
}

void KSCD::discInformation(KCompactDisc::DiscInfo info)
{
	populateSongList();
	restoreArtistLabel();
}

void KSCD::discStatusChanged(KCompactDisc::DiscStatus status)
{
	kDebug(67000) << "discStatusChanged(" << m_cd->discStatusString(status) << ")";
	statuslabel->setText(m_cd->discStatusString(status));

	switch(status)
	{
		case KCompactDisc::Stopped:
			trackChanged(0);
			trackPosition(-1);
			if (Prefs::ejectOnFinish() && !stoppedByUser)
				ejectClicked();

			playPB->setText(i18n("Play"));
			playPB->setIcon(KIcon(SmallIcon("media-playback-start")));

			/* reset to initial value, only stopclicked() sets this to true */
			stoppedByUser = false;
			break;

		case KCompactDisc::Playing:
			playPB->setIcon(KIcon(SmallIcon("media-playback-pause")));
			playPB->setText(i18n("Pause"));
			break;

		case KCompactDisc::Paused:
			playPB->setIcon(KIcon(SmallIcon("media-playback-start")));
			playPB->setText(i18n("Play"));
			break;

		default:
			break;
	}
}

void KSCD::trackChanged(unsigned track)
{
	kDebug(67000) << "trackChanged(" << track << ")";
	QString tooltip = artistlabel->text();

    if (track < 1)
    {
        setLEDs(-1);
        resetTimeSlider(true);
        tracklabel->setText("--/--");
        titlelabel->clear();
        if (songListCB->count())
        {
            songListCB->setCurrentIndex(0);
        }
    }
    else
    {
        if (songListCB->count())
        {
            songListCB->setCurrentIndex(track - 1);
            // drop the number.
            // for Mahlah, a picky though otherwise wonderful person - AJS
            QString justTheName = songListCB->currentText();
            justTheName = justTheName.right(justTheName.length() - 4);

            songListCB->setToolTip( i18n("Current track: %1", justTheName));
        }
        timeSlider->blockSignals(true);
        timeSlider->setRange(0, m_cd->trackLength(track));
        timeSlider->blockSignals(false);
        QString str;
        str.sprintf("%02d/%02d", track, m_cd->tracks());
        tracklabel->setText(str);

        QString title;
        if (m_cd->trackArtist() != m_cd->discArtist())
            title.append(m_cd->trackArtist()).append(" - ");
        title.append(m_cd->trackTitle());
        titlelabel->setText(title);
        tooltip += '/' + KStringHandler::rsqueeze(title, 30);
    }
    emit tooltipCurrentTrackChanged(tooltip);
}

void KSCD::cycleplaytimemode()
{
    /* switch to the next mode */
    switch (Prefs::timeDisplayMode())
    {
    case Prefs::EnumTimeDisplayMode::TRACK_SEC:
        Prefs::setTimeDisplayMode(Prefs::EnumTimeDisplayMode::TRACK_REM);
        break;

    case Prefs::EnumTimeDisplayMode::TRACK_REM:
        Prefs::setTimeDisplayMode(Prefs::EnumTimeDisplayMode::TOTAL_SEC);
        break;

    case Prefs::EnumTimeDisplayMode::TOTAL_SEC:
        Prefs::setTimeDisplayMode(Prefs::EnumTimeDisplayMode::TOTAL_REM);
        break;

    case Prefs::EnumTimeDisplayMode::TOTAL_REM:
    default:
        Prefs::setTimeDisplayMode(Prefs::EnumTimeDisplayMode::TRACK_SEC);
        break;
    }

    /* and display it */
    switch(Prefs::timeDisplayMode())
    {
    case Prefs::EnumTimeDisplayMode::TRACK_REM:
        volumelabel->setText(i18n("Tra Rem"));
        break;

    case Prefs::EnumTimeDisplayMode::TOTAL_SEC:
        volumelabel->setText(i18n("Tot Sec"));
        break;

    case Prefs::EnumTimeDisplayMode::TOTAL_REM:
        volumelabel->setText(i18n("Tot Rem"));
        break;

    case Prefs::EnumTimeDisplayMode::TRACK_SEC:
    default:
        volumelabel->setText(i18n("Tra Sec"));
        break;
    }

    QTimer::singleShot(3000, this, SLOT(showVolumeInLabel()));
}

void KSCD::showVolumeInLabel()
{
    QString str;
    str = ki18n("Vol: %1%").subs(Prefs::volume(), 2).toString();
    volumelabel->setText(str);
}

void KSCD::showArtistLabel(QString infoStatus)
{
    artistlabel->setText(infoStatus);
}

void KSCD::restoreArtistLabel()
{
    if(m_cd->tracks())
        showArtistLabel(QString("%1 - %2").arg(m_cd->discArtist(), m_cd->discTitle()));
    else
        showArtistLabel(i18n("NO DISC"));
}

void KSCD::information(QAction *action)
{
    const QString artist = m_cd->trackArtist();
    if(artist.isEmpty())
        return;

    //QString encodedArtist = KUrl::encode_string_no_slash(cddbInfo.get(Artist).toString());

    KUrl url;
    QString server = action->text();

    if(server == "MusicMoz") {
        url = KUrl("http://musicmoz.org/cgi-bin/ext.cgi");
        url.addQueryItem( "artist", artist );
    } else if (server == "Ultimate Bandlist") {
        url = KUrl("http://ubl.artistdirect.com/cgi-bin/gx.cgi/AppLogic+Search?select=MusicArtist&searchtype=NormalSearch");
        url.addQueryItem( "searchstr", artist );
    } else if (server == "CD Universe") {
        url = KUrl( QString( "http://www.cduniverse.com/cgi-bin/cdubin.exe/rlinka/ean=%1" ).arg( QString::fromLatin1(KUrl::toPercentEncoding(artist)) ) );
    } else if (server == "AlltheWeb") {
        url = KUrl("http://www.alltheweb.com/search?cat=web");
        url.addQueryItem( "q", artist );
    } else if (server == "Altavista") {
        url = KUrl("http://altavista.com/web/results?kgs=0&kls=1&avkw=xytx");
        url.addQueryItem( "q", artist );
    } else if (server == "Excite") {
        url = KUrl("http://msxml.excite.com/_1_2UDOUB70SVHVHR__info.xcite/dog/results?otmpl=dog/webresults.htm&qcat=web&qk=20&top=1&start=&ver=14060");
        url.addQueryItem( "qkw", artist );
    } else if (server == "Google") {
        url = KUrl("http://www.google.com/search");
        url.addQueryItem( "q", artist );
    } else if (server == "Google Groups") {
        url = KUrl("http://groups.google.com/groups?oi=djq&num=20");
        url.addQueryItem( "as_q", artist );
    } else if (server == "HotBot") {
        url = KUrl("http://www.hotbot.com/default.asp?prov=Inktomi&ps=&loc=searchbox&tab=web");
        url.addQueryItem( "query", artist );
    } else if (server == "Lycos") {
        url = KUrl("http://search.lycos.com/default.asp?lpv=1&loc=searchhp&tab=web");
        url.addQueryItem( "query", artist );
    } else if (server == "Open Directory") {
        url = KUrl("http://search.dmoz.org/cgi-bin/search");
        url.addQueryItem( "search", artist );
    } else if (server == "Yahoo!") {
        url = KUrl("http://search.yahoo.com/bin/search");
        url.addQueryItem( "p", artist );
    } else {
        return;
    }

    KRun::runUrl( url, "text/html", 0L);
}

/**
 * Save state on session termination
 */
bool KSCD::saveState(QSessionManager& /*sm*/)
{
  writeSettings();
  KConfigGroup config(KApplication::kApplication()->sessionConfig(), "General");
  config.writeEntry("Show", isVisible());
  return true;
}

/**
 * Allow the user to type in the number of the track
 */
void KSCD::keyPressEvent(QKeyEvent* e)
{
    bool isNum;
    uint value = e->text().toUInt(&isNum);

    if (e->key() == Qt::Key_F1)
    {
        KToolInvocation::invokeHelp();
    }
    else if (isNum)
    {
        if (0 < value && value <= m_cd->tracks())
            songListCB->setCurrentIndex(value - 1);
    }
    else
    {
      QWidget::keyPressEvent(e);
    }
}

QString KSCD::currentTrackTitle()
{
    return m_cd->trackTitle();
}

QString KSCD::currentAlbum()
{
    return m_cd->discTitle();
}

QString KSCD::currentArtist()
{
    return m_cd->trackArtist();
}

QStringList KSCD::trackList()
{
  QStringList tracks;
  for (uint i = 0; i < m_cd->tracks(); ++i)
    tracks << m_cd->trackTitle(i);

  return tracks;
}

void KSCD::populateSongList()
{
    songListCB->clear();
    for (uint i = 1; i <= m_cd->tracks(); ++i)
    {
        unsigned tmp = m_cd->trackLength(i);
        unsigned mymin;
        unsigned mysec;
        mymin = tmp / 60;
        mysec = (tmp % 60);
        QString str1;
        str1.sprintf("%02u: ", i);
        QString str2;
        str2.sprintf(" (%02u:%02u) ", mymin,  mysec);
        if (m_cd->discArtist() != m_cd->trackArtist(i))
            str1.append(m_cd->trackArtist(i)).append(" - ");
        str1.append(m_cd->trackTitle(i));
        str1.append(str2);
        songListCB->addItem(str1);
    }
}


/**
 * main()
 */
int main( int argc, char *argv[] )
{
    KAboutData aboutData("kscd", 0, ki18n("KsCD"),
                            "1.5", ki18n(description),
                            KAboutData::License_GPL,
                            ki18n("(c) 2001, Dirk Försterling\n(c) 2003, Aaron J. Seigo"));
    aboutData.addAuthor(ki18n("Aaron J. Seigo"), ki18n("Current maintainer"), "aseigo@kde.org");
    aboutData.addAuthor(ki18n("Alexander Kern"),ki18n("Workman library update, CDTEXT, CDDA"), "kernalex@kde.org");
    aboutData.addAuthor(ki18n("Bernd Johannes Wuebben"),KLocalizedString(), "wuebben@kde.org");
    aboutData.addAuthor(ki18n("Dirk Försterling"), ki18n("Workman library, previous maintainer"), "milliByte@gmx.net");
    aboutData.addCredit(ki18n("Wilfried Huss"), ki18n("Patches galore"));
    aboutData.addCredit(ki18n("Steven Grimm"), ki18n("Workman library"));
    aboutData.addCredit(ki18n("Sven Lueppken"), ki18n("UI Work"));
    aboutData.addCredit(ki18n("freedb.org"), ki18n("Special thanks to freedb.org for providing a free CDDB-like CD database"), 0, "http://freedb.org");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("s");
    options.add("start", ki18n("Start playing"));
    options.add("+[device]", ki18n("CD device, can be a path or a media:/ URL"));
    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication::addCmdLineOptions();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (!KUniqueApplication::start())
    {
        fprintf(stderr, "kscd is already running\n");
        if (args->count() > 0 || args->isSet("start"))
        {
            QDBusInterface kscd("org.kde.kscd", "/CDPlayer", "org.kde.kscd.CDPlayer");
            // Forward the command line args to the running instance.
            if (args->count() > 0)
            {
                kscd.call( "setDevice",  QString(args->arg(0)));
            }
            if (args->isSet("start"))
            {
                kscd.call( "play" );
            }
        }
        exit(0);
    }
    KUniqueApplication a;

    KSCD *k = new KSCD();

    a.setTopWidget( k );
    a.setMainWidget( k );

    k->setWindowTitle(KGlobal::caption());

    if (kapp->isSessionRestored())
    {
        KConfigGroup group(KApplication::kApplication()->sessionConfig(), "General");
        if (group.readEntry("Show", false))
            k->show();
    }
    else
    {
        k->show();
    }

    if (args->count() > 0)
		Prefs::self()->setCdDevice(args->arg(0));

    return a.exec();
}

#include "kscd.moc"
