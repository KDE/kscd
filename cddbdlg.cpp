#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <qkeycode.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <klistview.h>
#include <klineedit.h>
#include <knuminput.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kmessagebox.h>

#include <stdio.h>
#include <math.h>

#include "version.h"
#include "kscd.h"
#include "cddbdlg.h"
#include "libkcddb/cdinfodialogbase.h"

extern "C" {
#include "libwm/include/workman.h"
}

struct mytoc
{
  unsigned absframe;
};

CDDBDlg::CDDBDlg( QWidget* parent, const char* name )
    : KDialogBase( parent, name, false, i18n( "CD Database Editor" ),
      Ok|Cancel|User1|User2, Ok, true )
{
  m_dlgBase = new CDInfoDialogBase( this, "m_dlgBase" );

  setMainWidget( m_dlgBase );

  setButtonText( User1, i18n( "Upload" ) );
  setButtonText( User2, i18n( "Fetch Info" ) );

  ntracks = 0;
  trackStartFrames.clear();

  connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( upload() ) );
  connect( this, SIGNAL( user2Clicked() ), SIGNAL( cddbQuery() ) );
  connect( m_dlgBase, SIGNAL( play( int ) ), SIGNAL( play( int ) ) );

  cddbClient = new KCDDB::Client();
  cddbClient->setBlockingMode(false);
  connect (cddbClient, SIGNAL(finished(CDDB::Result)),
                       SLOT(submitFinished(CDDB::Result)));
}


CDDBDlg::~CDDBDlg()
{
  delete cddbClient;
}

void CDDBDlg::setData(
  struct wm_cdinfo *cd,
  const KCDDB::CDInfo &_cddbInfo,
  const QStringList &_playlist)
{
    /*
    * Avoid people who need to edit titles of "no discs" to crash kscd.
    */
    if (!cd || cd->ntracks == 0)
    {
        ntracks = 0;
        KCDDB::CDInfo tmp;
        tmp.title = "No Disc";
        m_dlgBase->setInfo(tmp, trackStartFrames);
        return;
    }

    // Let's make a deep copy of the cd struct info so that the data won't
    // change the cd changes while we are playing with the dialog.
    cddbInfo = _cddbInfo;
    playlist = _playlist;
    ntracks = cd->ntracks;

    trackStartFrames.clear();
    for (int i = 0; i < cd->ntracks + 1; i++)
    {
        trackStartFrames.append(cd->trk[i].start);
    }

    // Some sanity provisions to ensure that the number of records matches what
    // the CD actually contains.
    while (cddbInfo.trackInfoList.count() < ntracks)
    {
        cddbInfo.trackInfoList.append(KCDDB::TrackInfo());
    }
    while (cddbInfo.trackInfoList.count() > ntracks)
    {
        cddbInfo.trackInfoList.remove(cddbInfo.trackInfoList.end());
    }

    // Write the complete record to the dialog.
    m_dlgBase->setInfo(cddbInfo, trackStartFrames);
    // FIXME: KDE4, move this logic into m_dlgBase->setInfo() once KCDDB:CDInfo is updated.
    m_dlgBase->m_playOrder->setText( playlist.join( "," ) );
} // setData

void CDDBDlg::submitFinished(KCDDB::CDDB::Result r)
{
  if (r == KCDDB::CDDB::Success)
  {
    KMessageBox::information(this, i18n("Record submitted successfully."),
         i18n("Record Submission"));
  }
  else
  {
    QString str = i18n("Error sending record.\n\n%1")
      .arg(KCDDB::CDDB::resultToString(r));
    KMessageBox::error(this, str, i18n("Record Submission"));
  }
} // submitFinished()

void CDDBDlg::upload()
{
    if (!updateFromDialog())
        return;

    // Create a copy with a bumped revision number.
    KCDDB::CDInfo copyInfo = cddbInfo;
    copyInfo.revision++;

    KCDDB::TrackOffsetList offsetList;

    for (int i = 0; i < cd->ntracks; i++)
    {
        offsetList.append(trackStartFrames[i]);
    }
    offsetList.append(trackStartFrames[0]);
    offsetList.append(trackStartFrames[ntracks]);

    cddbClient->submit(copyInfo, offsetList);
} // upload

void CDDBDlg::save()
{
    if (!updateFromDialog())
        return;
    KCDDB::Cache::store(cddbInfo);

    emit cddbQuery();
} // save

bool CDDBDlg::updateFromDialog()
{
  KCDDB::CDInfo copy = m_dlgBase->info();
  if (copy.artist.isEmpty())
  {
    KMessageBox::sorry(this,
        i18n("The artist name of the disc has to be entered.\n"
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  if (copy.title.isEmpty())
  {
    KMessageBox::sorry(this,
        i18n("The title of the disc has to be entered.\n"
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  bool have_nonempty_title = false;
  for (unsigned i = 0; i < copy.trackInfoList.count(); i++)
  {
      if (!copy.trackInfoList[i].title.isEmpty())
      {
          have_nonempty_title = true;
          break;
      }
  }

  if (!have_nonempty_title)
  {
    KMessageBox::sorry(this,
        i18n("At least one track title must be entered.\n"\
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  if (ntracks != copy.trackInfoList.count() )
  {
    KMessageBox::error(this,
        i18n("ntracks != track_list->count() \n"
             "Please email the author."),
    i18n("Internal Error"));
    return false;
  }

  // Playorder...
  QStringList strlist = QStringList::split( ',', m_dlgBase->m_playOrder->text() );

  bool ret = true;

  QString teststr;
  bool ok;
  unsigned num;

  for ( QStringList::Iterator it = strlist.begin();
        it != strlist.end();
        ++it )
  {
    teststr = *it;
    num = teststr.toInt(&ok);

    if( !ok || num > ntracks )
      ret = false;
  }

  if(!ret)
  {
    KMessageBox::sorry(this,
        i18n("Invalid Playlist\nPlease use valid track numbers, "
             "separated by commas."));
    return false;
  }

  // The information all passed our checks. Update the stored values from the dialog.
  cddbInfo = copy;
  return true;
} // updateFromDialog

#include "cddbdlg.moc"
