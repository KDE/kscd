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
#include <qlineedit.h>

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

QTime framestoTime(int frames);

CDDBDlg::CDDBDlg( QWidget* parent, const char* name )
    : KDialogBase( parent, name, false, i18n( "CD Database Editor" ),
      Ok|Cancel|User1|User2, Ok, true )
{
  m_dlgBase = new CDDBDlgBase( this, "m_dlgBase" );
  setMainWidget( m_dlgBase );

  setButtonText( User1, i18n( "Upload" ) );
  setButtonText( User2, i18n( "Fetch Info" ) );

  cdinfo.magicID = QString::null;	/*cddb magic disk id BERND*/
  cdinfo.ntracks = 0;	/* Number of tracks on the disc */
  cdinfo.length  = 0;	/* Total running time in seconds */
  cdinfo.cddbtoc = 0L;

  connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( upload() ) );
  connect( this, SIGNAL( user2Clicked() ), SLOT( load_cddb() ) );
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
  if(cdinfo.cddbtoc)
    delete [] cdinfo.cddbtoc;
  delete cddbClient;
}

void CDDBDlg::setData(
        struct wm_cdinfo *cd,
        const QStringList& tracktitlelist,
        const QStringList& extlist,
        const QString& _xmcd_data,
        const QString& cat,
        const QString& _genre,
        int rev,
        int _year,
        const QStringList& _playlist,
        const QStringList& _pathlist
        )
{
  int ntr;

  ext_list = extlist;
  track_list = tracktitlelist;
  xmcd_data = _xmcd_data.copy();
  category = cat.copy();
  genre = _genre.copy();
  revision = rev;
  year = _year;
  playlist = _playlist;
  pathlist = _pathlist;

  ntr = track_list.count();

  // Let's make a deep copy of the cd struct info so that the data won't
  // change the cd changes while we are playing with the dialog.

  // put one of these into the destructor too..
  if(cdinfo.cddbtoc)
    delete [] cdinfo.cddbtoc;
  if(!cd)
    return;
  /*
   * Avoid people who need to edit titles of "no discs" to crash kscd.
   */
  if( cd->ntracks == 0 )
  {
    cdinfo.magicID = QString::null;
    cdinfo.ntracks = 0;
    cdinfo.length = 0;
    m_dlgBase->le_title->setText("No Disc");
    m_dlgBase->lb_discId->clear();
    m_dlgBase->lv_trackList->clear();
    return;
  }

  cdinfo.cddbtoc =  new struct mytoc [cd->ntracks + 2];
  cdinfo.ntracks = cd->ntracks;	/* Number of tracks on the disc  */
  cdinfo.length  = cd->length;	/* Total running time in seconds */

  KCDDB::TrackOffsetList offsetList;

  for( int i = 0; i < cd->ntracks + 1; i++)
  {
    cdinfo.cddbtoc[i].min = cd->trk[i].start / 4500;
    cdinfo.cddbtoc[i].sec = (cd->trk[i].start % 4500) / 75;
    cdinfo.cddbtoc[i].frame = cd->trk[i].start - ((cd->trk[i].start / 75)*75);
    cdinfo.cddbtoc[i].absframe = cd->trk[i].start;
    if (i < cd->ntracks)
      offsetList.append(cd->trk[i].start);
  }
  offsetList.append(cd->trk[0].start);
  offsetList.append(cd->trk[cd->ntracks].start);
  cdinfo.magicID = KCDDB::CDDB::trackOffsetListToId(offsetList);

  // some sanity provisions

  if ((int)track_list.count() < cdinfo.ntracks + 1)
  {
    int k = track_list.count();
    for (int i = 0 ; i < (int)( cdinfo.ntracks + 1 - k) ; i ++)
      track_list.append("");
  }

  if ((int)ext_list.count() < cdinfo.ntracks + 1)
  {
    int l = ext_list.count();
    for (int i = 0 ; i < (int) ( cdinfo.ntracks + 1 - l) ; i ++)
      ext_list.append("");
  }

  while((int)track_list.count() > cdinfo.ntracks + 1)
  {
    track_list.remove(track_list.at(track_list.count() - 1));
  }

  while((int)ext_list.count() > cdinfo.ntracks + 1)
  {
    ext_list.remove(ext_list.at(ext_list.count() - 1));
  }

  m_dlgBase->le_artist->setText((track_list.first().section('/',0,0)).stripWhiteSpace());
  m_dlgBase->le_title->setText((track_list.first().section('/',1,1)).stripWhiteSpace());

  QString idstr;
  idstr = category + "  " + cdinfo.magicID;

  m_dlgBase->lb_discId->setText(idstr.stripWhiteSpace());

  QTime   dl;
  dl = dl.addSecs(cdinfo.length);

  QString temp2;
  if(dl.hour() > 0)
    temp2 = dl.toString("hh:mm:ss:");
  else
    temp2 = dl.toString("mm:ss");
  m_dlgBase->lb_totalTime->setText(temp2);

  QString fmt;

  m_dlgBase->lv_trackList->clear();

  for(int i = 1; i <= cdinfo.ntracks; i++)
  {
    dl = framestoTime(cdinfo.cddbtoc[i].absframe-cdinfo.cddbtoc[i-1].absframe);

    QListViewItem * item = new QListViewItem( m_dlgBase->lv_trackList, 0 );
    item->setText( 0, QString().sprintf("%02d",i) );
    if (dl.hour() > 0)
      item->setText( 1, dl.toString("hh:mm:ss"));
    else
      item->setText( 1, dl.toString("mm:ss"));

    if((ntr >=  i) && (ntr > 0))
      item->setText( 2,  *(track_list.at(i)));
  }

  m_dlgBase->le_playOrder->setText( playlist.join( "," ) );
} // setData

void CDDBDlg::extITB( int trackNumber )
{
  QString trackTitle = track_list[ trackNumber ];
  QString dialogTitle;
  bool ok;

  if (!trackTitle.isEmpty())
    dialogTitle = "<qt>" + i18n("Annotate track #%1: %2").arg(trackNumber)
                                                .arg(trackTitle) + "</qt>";
  else
    dialogTitle = i18n("Annotate track #%1").arg(trackNumber);

  QString s = KInputDialog::getMultiLineText( i18n( "Annotate Track" ),
      dialogTitle, *ext_list.at( trackNumber ), &ok, this );

  if ( ok )
    *ext_list.at( trackNumber ) = s;
} // extIB

void CDDBDlg::extIB()
{
  bool ok;

  QString s = KInputDialog::getMultiLineText( i18n( "Annotate Album" ),
      i18n( "Enter annotation for this album:" ), ext_list.first(),
      &ok, this );

  if ( ok )
    *ext_list.at(0) = s;
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

  if( category.isEmpty() )
  {
    bool ok;

    submitcat = KInputDialog::getItem( i18n( "Select Album Category" ),
        i18n( "Select a category for this album:" ), catlist, 0,
        false, &ok, this );

    if ( !ok )
      return;
  }
  else
    submitcat = category.copy();

  KCDDB::CDInfo info;
  setCdInfo(info, submitcat);

  info.revision++;

  KCDDB::TrackOffsetList offsetList;

  for( int i = 0; i < cd->ntracks + 1; i++)
  {
    if (i < cd->ntracks)
      offsetList.append(cdinfo.cddbtoc[i].absframe);
  }
  offsetList.append(cdinfo.cddbtoc[0].absframe);
  offsetList.append(cdinfo.cddbtoc[cdinfo.ntracks].absframe);

  cddbClient->submit(info, offsetList);
} // upload

void CDDBDlg::setCdInfo(KCDDB::CDInfo &info, const QString& category)
{
  info.artist = track_list.first().section('/',0,0).stripWhiteSpace();
  info.title = track_list.first().section('/',1,1).stripWhiteSpace();
  info.category = category;
  info.genre = genre;
  info.id = cdinfo.magicID;
  info.extd = ext_list.first();
  info.year = year;
  info.length = cdinfo.length;
  info.revision = revision;

  info.trackInfoList.clear();
  QStringList::Iterator it = track_list.begin();
  ++it;
  unsigned i=0;
  for ( ; it != track_list.end(); ++it)
  {
    TrackInfo t;
    t.title = *it;
    t.extt = ext_list[i+1];

    info.trackInfoList.append(t);
    ++i;
  }
} // setCdInfo

void CDDBDlg::save()
{
  updateTrackList();

  if(!checkit())
    return;

  QString savecat;

  if( category.isEmpty() )
  {
    bool ok;

    savecat = KInputDialog::getItem( i18n( "Select Album Category" ),
        i18n( "Select a category for this album:" ), catlist, 0,
        false, &ok, this );

    if ( !ok )
      return;
  }
  else
    savecat = category.copy();

  KCDDB::CDInfo info;
  setCdInfo(info, savecat);
  // Playorder...

  KCDDB::Cache::store(info);

  load_cddb();
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

  if(track_list.count() < 2)
  {
    KMessageBox::sorry(this,
        i18n("At least one track title must be entered.\n"\
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  bool have_nonempty_title = false;
  for ( QStringList::Iterator it = track_list.at(1);
        it != track_list.end();
        ++it )
  {
    title = *it;
    title = title.stripWhiteSpace();
    if(!title.isEmpty())
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

  if(cdinfo.ntracks +1 != (int)track_list.count() )
  {
    KMessageBox::error(this,
        i18n("cdinfo.ntracks != title_list->count() + 1\n"
             "Please email the author."),
    i18n("Internal Error"));
    return false;
  }

  QStringList strlist = QStringList::split( ',', m_dlgBase->le_playOrder->text() );

  bool ret = true;

  QString teststr;
  bool ok;
  int num;

  for ( QStringList::Iterator it = strlist.begin();
        it != strlist.end();
        ++it )
  {
    teststr = *it;
    num = teststr.toInt(&ok);

    if( !ok || num > cdinfo.ntracks )
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

void CDDBDlg::load_cddb()
{
  emit cddbQuery(true);
} // load

void CDDBDlg::updateTrackList()
{
  QString title = m_dlgBase->le_title->text().stripWhiteSpace();
  QString artist = m_dlgBase->le_artist->text().stripWhiteSpace();

  title = QString("%1 / %2").arg(artist).arg(title);
  *track_list.begin() = title;

  m_dlgBase->lv_trackList->setSorting(0, true);

  unsigned int i=1;
  for (QListViewItem* item = m_dlgBase->lv_trackList->firstChild(); item ; item=item->nextSibling())
  {
    if (track_list.count() <= i)
    {
      kdWarning() << "track_list.count <= " << i << endl;
      continue;
    }
    track_list[i] = item->text(2);
    i++;
  }
}

#include "cddbdlg.moc"
