#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <qkeycode.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qlistview.h>
#include <klineedit.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <stdio.h>
#include <math.h>

#include "version.h"
#include "kscd.h"
#include "cddbdlg.h"
#include "cddbdlgbase.h"

extern "C" {
#include "libwm/include/workman.h"
}

struct mytoc
{
  int	min;
  int	sec;
  int	frame;
  int     absframe;
};

QTime framestoTime(int frames);

CDDBDlg::CDDBDlg( QWidget* parent, const char* name )
    : KDialogBase( parent, name, false, i18n( "CD Database Editor" ),
      Ok|Cancel|User1|User2, Ok, true )
{
  m_dlgBase = new CDDBDlgBase( this, "m_dlgBase" );
  setMainWidget( m_dlgBase );

  setButtonText( User1, i18n( "Upload" ) );
  setButtonText( User2, i18n( "Fetch Info" ) );

  ntracks = 0;	/* Number of tracks on the disc */
  length  = 0;	/* Total running time in seconds */
  cddbtoc = 0;

  connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( upload() ) );
  connect( this, SIGNAL( user2Clicked() ), SIGNAL( cddbQuery() ) );
  connect( m_dlgBase, SIGNAL( play( int ) ), SIGNAL( play( int ) ) );
  connect( m_dlgBase, SIGNAL( discInfoClicked() ), SLOT( extIB() ) );
  connect( m_dlgBase, SIGNAL( trackInfoClicked( int ) ), SLOT( extITB( int ) ) );

  catlist << "rock"
          << "classical"
          << "jazz"
          << "soundtrack"
          << "newage"
          << "blues"
          << "folk"
          << "country"
          << "reggae"
          << "misc"
          << "data";

  cddbClient = new KCDDB::Client();
  cddbClient->setBlockingMode(false);
  connect (cddbClient, SIGNAL(finished(CDDB::Result)),
                       SLOT(submitFinished(CDDB::Result)));
}


CDDBDlg::~CDDBDlg()
{
  if(cddbtoc)
    delete [] cddbtoc;
  delete cddbClient;
}

void CDDBDlg::setData(
  struct wm_cdinfo *cd,
  const KCDDB::CDInfo &_cddbInfo,
  const QStringList &_playlist)
{
  // Let's make a deep copy of the cd struct info so that the data won't
  // change the cd changes while we are playing with the dialog.
  cddbInfo = _cddbInfo;
  playlist = _playlist;

  // put one of these into the destructor too..
  if(cddbtoc)
    delete [] cddbtoc;
  if(!cd)
    return;
  /*
   * Avoid people who need to edit titles of "no discs" to crash kscd.
   */
  if( cd->ntracks == 0 )
  {
    ntracks = 0;
    length = 0;
    m_dlgBase->le_title->setText("No Disc");
    m_dlgBase->lb_discId->clear();
    m_dlgBase->lv_trackList->clear();
    return;
  }

  cddbtoc =  new struct mytoc [cd->ntracks + 2];
  ntracks = cd->ntracks;	/* Number of tracks on the disc  */
  length  = cd->length;	/* Total running time in seconds */

  KCDDB::TrackOffsetList offsetList;

  for( int i = 0; i < cd->ntracks + 1; i++)
  {
    cddbtoc[i].min = cd->trk[i].start / 4500;
    cddbtoc[i].sec = (cd->trk[i].start % 4500) / 75;
    cddbtoc[i].frame = cd->trk[i].start - ((cd->trk[i].start / 75)*75);
    cddbtoc[i].absframe = cd->trk[i].start;
    if (i < cd->ntracks)
      offsetList.append(cd->trk[i].start);
  }
  offsetList.append(cd->trk[0].start);
  offsetList.append(cd->trk[cd->ntracks].start);

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

  m_dlgBase->le_artist->setText(cddbInfo.artist.stripWhiteSpace());
  m_dlgBase->le_title->setText(cddbInfo.title.stripWhiteSpace());
  m_dlgBase->le_genre->setText(cddbInfo.genre.stripWhiteSpace());
  m_dlgBase->le_category->setText(cddbInfo.category.stripWhiteSpace());
  m_dlgBase->le_year->setText(QString::number(cddbInfo.year));
  m_dlgBase->le_revision->setText(QString::number(cddbInfo.revision));
  m_dlgBase->lb_discId->setText(cddbInfo.id.stripWhiteSpace());

  QTime   dl;
  dl = dl.addSecs(length);

  QString temp2;
  if(dl.hour() > 0)
    temp2 = dl.toString("hh:mm:ss");
  else
    temp2 = dl.toString("mm:ss");
  m_dlgBase->lb_totalTime->setText(temp2);

  QString fmt;

  m_dlgBase->lv_trackList->clear();

  for(unsigned i = 1; i <= ntracks; i++)
  {
    dl = framestoTime(cddbtoc[i].absframe-cddbtoc[i-1].absframe);

    QListViewItem * item = new QListViewItem( m_dlgBase->lv_trackList, 0 );
    item->setText( 0, QString().sprintf("%02d",i) );
    if (dl.hour() > 0)
      item->setText( 1, dl.toString("hh:mm:ss"));
    else
      item->setText( 1, dl.toString("mm:ss"));

    item->setText( 2,  cddbInfo.trackInfoList[i-1].title);
  }

  m_dlgBase->le_playOrder->setText( playlist.join( "," ) );
} // setData

void CDDBDlg::extITB( int trackNumber )
{
  QString trackTitle = cddbInfo.trackInfoList[trackNumber-1].title;
  QString dialogTitle;
  bool ok;

  if (!trackTitle.isEmpty())
    dialogTitle = "<qt>" + i18n("Annotate track #%1: %2").arg(trackNumber)
                                                .arg(trackTitle) + "</qt>";
  else
    dialogTitle = i18n("Annotate track #%1").arg(trackNumber);

  QString s = KInputDialog::getMultiLineText( i18n( "Annotate Track" ),
      dialogTitle, cddbInfo.trackInfoList[trackNumber].extt, &ok, this );

  if ( ok )
    cddbInfo.trackInfoList[trackNumber].extt = s;
} // extIB

void CDDBDlg::extIB()
{
  bool ok;

  QString s = KInputDialog::getMultiLineText( i18n( "Annotate Album" ),
      i18n( "Enter annotation for this album:" ), cddbInfo.trackInfoList[0].extt,
      &ok, this );

  if ( ok )
    cddbInfo.trackInfoList[0].extt = s;
} // extITB

void CDDBDlg::submitFinished(KCDDB::CDDB::Result r)
{
  if (r == KCDDB::CDDB::Success)
  {
    KMessageBox::information(this, i18n("Record submitted successfully."),
         i18n("Record Submission"));
  }
  else
  {
    QString str = i18n("Error sending message via SMTP.\n\n%1")
      .arg(KCDDB::CDDB::resultToString(r));
    KMessageBox::error(this, str, i18n("Record Submission"));
  }
} // submitFinished()

QTime framestoTime(int _frames)
{
  QTime dml;
  double frames;
  double dsecs;
  int secs;
  double ip;

  frames = (double) _frames;
  dml.setHMS(0,0,0);

  dsecs = frames/ (75.0);

  if(modf(dsecs,&ip) >= 0.5)
  {
    ip = ip + 1.0;
  }
  secs = (int) ip;

  dml = dml.addSecs(secs);
  return dml;
} // framestotime

void CDDBDlg::upload()
{
  updateTrackList();

  if(!checkit())
    return;

  QString submitcat;

  if( cddbInfo.category.isEmpty() )
  {
    bool ok;

    submitcat = KInputDialog::getItem( i18n( "Select Album Category" ),
        i18n( "Select a category for this album:" ), catlist, 0,
        false, &ok, this );

    if ( !ok )
      return;
  }
  else
    submitcat = cddbInfo.category.copy();

  KCDDB::CDInfo info;
  setCdInfo(info, submitcat);

  info.revision++;

  KCDDB::TrackOffsetList offsetList;

  for( int i = 0; i < cd->ntracks + 1; i++)
  {
    if (i < cd->ntracks)
      offsetList.append(cddbtoc[i].absframe);
  }
  offsetList.append(cddbtoc[0].absframe);
  offsetList.append(cddbtoc[ntracks].absframe);

  cddbClient->submit(info, offsetList);
} // upload

void CDDBDlg::setCdInfo(KCDDB::CDInfo &info, const QString& category)
{
  info = cddbInfo;
  info.category = category;
  info.extd = cddbInfo.trackInfoList[0].extt;
} // setCdInfo

void CDDBDlg::save()
{
  updateTrackList();

  if(!checkit())
    return;

  QString savecat;

  if( cddbInfo.category.isEmpty() )
  {
    bool ok;

    savecat = KInputDialog::getItem( i18n( "Select Album Category" ),
        i18n( "Select a category for this album:" ), catlist, 0,
        false, &ok, this );

    if ( !ok )
      return;
  }
  else
    savecat = cddbInfo.category.copy();

  KCDDB::CDInfo info;
  setCdInfo(info, savecat);
  // Playorder...

  KCDDB::Cache::store(info);

  emit cddbQuery();
} // save

bool CDDBDlg::checkit()
{
  QString artist = m_dlgBase->le_artist->text().stripWhiteSpace();
  if(artist.isEmpty())
  {
    KMessageBox::sorry(this,
        i18n("The artist name of the disc has to be entered.\n"
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  QString title = m_dlgBase->le_title->text().stripWhiteSpace();
  if(title.isEmpty())
  {
    KMessageBox::sorry(this,
        i18n("The title of the disc has to be entered.\n"
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  if(cddbInfo.trackInfoList.count() < 1)
  {
    KMessageBox::sorry(this,
        i18n("At least one track title must be entered.\n"\
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  bool have_nonempty_title = false;
  KCDDB::TrackInfoList::ConstIterator it(cddbInfo.trackInfoList.begin());
  KCDDB::TrackInfoList::ConstIterator end(cddbInfo.trackInfoList.end());
  for (; it != end; ++it)
  {
    QString songTitle = (*it).title;
    title = songTitle.stripWhiteSpace();
    if(!songTitle.isEmpty())
    {
      have_nonempty_title = true;
      break;
    }
  }

  if(!have_nonempty_title)
  {
    KMessageBox::sorry(this,
        i18n("At least one track title must be entered.\n"\
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  if(ntracks != cddbInfo.trackInfoList.count() )
  {
    KMessageBox::error(this,
        i18n("ntracks != track_list->count() \n"
             "Please email the author."),
    i18n("Internal Error"));
    return false;
  }

  QStringList strlist = QStringList::split( ',', m_dlgBase->le_playOrder->text() );

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

  return true;
} // checkit

void CDDBDlg::updateTrackList()
{
  cddbInfo.title = m_dlgBase->le_title->text().stripWhiteSpace();
  cddbInfo.artist = m_dlgBase->le_artist->text().stripWhiteSpace();

  m_dlgBase->lv_trackList->setSorting(0, true);

  unsigned int i=0;
  for (QListViewItem* item = m_dlgBase->lv_trackList->firstChild(); item ; item=item->nextSibling())
  {
    if (cddbInfo.trackInfoList.count() <= i)
    {
      kdWarning() << "track_list.count <= " << i << endl;
      continue;
    }
    cddbInfo.trackInfoList[i].title = item->text(2);
    i++;
  }
}

#include "cddbdlg.moc"
