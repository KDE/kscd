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

#include <kglobal.h>
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

struct mytoc
{
  unsigned absframe;
};

CDDBDlg::CDDBDlg( QWidget* parent, const char* name )
    : KDialogBase( parent, name, false, i18n( "CD Editor" ),
      Ok|Cancel|User1|User2, Ok, true )
{
  KGlobal::locale()->insertCatalogue("libkcddb");

  m_dlgBase = new CDInfoDialogBase( this, "m_dlgBase" );

  setMainWidget( m_dlgBase );

  setButtonText( User1, i18n( "Upload" ) );
  setButtonText( User2, i18n( "Fetch Info" ) );

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
  const KCDDB::CDInfo &_cddbInfo,
  const KCDDB::TrackOffsetList &_trackStartFrames,
  const QStringList &_playlist)
{
    // Let's make a deep copy of the cd struct info so that the data won't
    // change the cd changes while we are playing with the dialog.
    cddbInfo = _cddbInfo;
    trackStartFrames = _trackStartFrames;
    playlist = _playlist;

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
    if (!validInfo())
        return;

    updateFromDialog();

    // Create a copy with a bumped revision number.
    KCDDB::CDInfo copyInfo = cddbInfo;
    copyInfo.revision++;
    cddbClient->submit(copyInfo, trackStartFrames);
} // upload

void CDDBDlg::save()
{
    updateFromDialog();

    KCDDB::Cache::store(cddbInfo);

    emit newCDInfoStored(cddbInfo);
} // save

bool CDDBDlg::validInfo()
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

  return true;
}

void CDDBDlg::updateFromDialog()
{
  KCDDB::CDInfo copy = m_dlgBase->info();

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

    if( !ok || num > cddbInfo.trackInfoList.count() )
      ret = false;
  }

  if(!ret)
  {
    KMessageBox::sorry(this,
        i18n("Invalid Playlist\nPlease use valid track numbers, "
             "separated by commas."));
  }

  cddbInfo = copy;
} // updateFromDialog

#include "cddbdlg.moc"
