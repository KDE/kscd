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

  // These are only 11 Category values defined by CDDB. See
  //
  // http://www.freedb.org/modules.php?name=Sections&sop=viewarticle&artid=26
  //
  // DON'T i18n them!
  m_dlgBase->m_category->insertItem("blues");
  m_dlgBase->m_category->insertItem("classical");
  m_dlgBase->m_category->insertItem("country");
  m_dlgBase->m_category->insertItem("data");
  m_dlgBase->m_category->insertItem("folk");
  m_dlgBase->m_category->insertItem("jazz");
  m_dlgBase->m_category->insertItem("newage");
  m_dlgBase->m_category->insertItem("reggae");
  m_dlgBase->m_category->insertItem("rock");
  m_dlgBase->m_category->insertItem("soundtrack");
  m_dlgBase->m_category->insertItem("misc");

  // On the other hand, the Genre is completely arbitrary. But we follow
  // kaudiocreator's cue and make life easy for people.
  //
  // In addition, DON'T i18n this first one! It is to catch stupid people
  // who don't bother setting the genre and encoders that barf on empty
  // strings for the genre (e.g. lame).
  m_dlgBase->m_genre->insertItem("Unknown");
  m_dlgBase->m_genre->insertItem(i18n("A Cappella"));
  m_dlgBase->m_genre->insertItem(i18n("Acid Jazz"));
  m_dlgBase->m_genre->insertItem(i18n("Acid Punk"));
  m_dlgBase->m_genre->insertItem(i18n("Acid"));
  m_dlgBase->m_genre->insertItem(i18n("Acoustic"));
  m_dlgBase->m_genre->insertItem(i18n("Alternative"));
  m_dlgBase->m_genre->insertItem(i18n("Alt. Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Ambient"));
  m_dlgBase->m_genre->insertItem(i18n("Anime"));
  m_dlgBase->m_genre->insertItem(i18n("Avantgarde"));
  m_dlgBase->m_genre->insertItem(i18n("Ballad"));
  m_dlgBase->m_genre->insertItem(i18n("Bass"));
  m_dlgBase->m_genre->insertItem(i18n("Beat"));
  m_dlgBase->m_genre->insertItem(i18n("Bebop"));
  m_dlgBase->m_genre->insertItem(i18n("Big Band"));
  m_dlgBase->m_genre->insertItem(i18n("Black Metal"));
  m_dlgBase->m_genre->insertItem(i18n("Bluegrass"));
  m_dlgBase->m_genre->insertItem(i18n("Blues"));
  m_dlgBase->m_genre->insertItem(i18n("Booty Bass"));
  m_dlgBase->m_genre->insertItem(i18n("BritPop"));
  m_dlgBase->m_genre->insertItem(i18n("Cabaret"));
  m_dlgBase->m_genre->insertItem(i18n("Celtic"));
  m_dlgBase->m_genre->insertItem(i18n("Chamber Music"));
  m_dlgBase->m_genre->insertItem(i18n("Chanson"));
  m_dlgBase->m_genre->insertItem(i18n("Chorus"));
  m_dlgBase->m_genre->insertItem(i18n("Christian Gangsta Rap"));
  m_dlgBase->m_genre->insertItem(i18n("Christian Rap"));
  m_dlgBase->m_genre->insertItem(i18n("Christian Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Classical"));
  m_dlgBase->m_genre->insertItem(i18n("Classic Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Club-house"));
  m_dlgBase->m_genre->insertItem(i18n("Club"));
  m_dlgBase->m_genre->insertItem(i18n("Comedy"));
  m_dlgBase->m_genre->insertItem(i18n("Contemporary Christian"));
  m_dlgBase->m_genre->insertItem(i18n("Country"));
  m_dlgBase->m_genre->insertItem(i18n("Crossover"));
  m_dlgBase->m_genre->insertItem(i18n("Cult"));
  m_dlgBase->m_genre->insertItem(i18n("Dance Hall"));
  m_dlgBase->m_genre->insertItem(i18n("Dance"));
  m_dlgBase->m_genre->insertItem(i18n("Darkwave"));
  m_dlgBase->m_genre->insertItem(i18n("Death Metal"));
  m_dlgBase->m_genre->insertItem(i18n("Disco"));
  m_dlgBase->m_genre->insertItem(i18n("Dream"));
  m_dlgBase->m_genre->insertItem(i18n("Drum & Bass"));
  m_dlgBase->m_genre->insertItem(i18n("Drum Solo"));
  m_dlgBase->m_genre->insertItem(i18n("Duet"));
  m_dlgBase->m_genre->insertItem(i18n("Easy Listening"));
  m_dlgBase->m_genre->insertItem(i18n("Electronic"));
  m_dlgBase->m_genre->insertItem(i18n("Ethnic"));
  m_dlgBase->m_genre->insertItem(i18n("Eurodance"));
  m_dlgBase->m_genre->insertItem(i18n("Euro-House"));
  m_dlgBase->m_genre->insertItem(i18n("Euro-Techno"));
  m_dlgBase->m_genre->insertItem(i18n("Fast-Fusion"));
  m_dlgBase->m_genre->insertItem(i18n("Folklore"));
  m_dlgBase->m_genre->insertItem(i18n("Folk/Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Folk"));
  m_dlgBase->m_genre->insertItem(i18n("Freestyle"));
  m_dlgBase->m_genre->insertItem(i18n("Funk"));
  m_dlgBase->m_genre->insertItem(i18n("Fusion"));
  m_dlgBase->m_genre->insertItem(i18n("Game"));
  m_dlgBase->m_genre->insertItem(i18n("Gangsta Rap"));
  m_dlgBase->m_genre->insertItem(i18n("Goa"));
  m_dlgBase->m_genre->insertItem(i18n("Gospel"));
  m_dlgBase->m_genre->insertItem(i18n("Gothic Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Gothic"));
  m_dlgBase->m_genre->insertItem(i18n("Grunge"));
  m_dlgBase->m_genre->insertItem(i18n("Hardcore"));
  m_dlgBase->m_genre->insertItem(i18n("Hard Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Heavy Metal"));
  m_dlgBase->m_genre->insertItem(i18n("Hip-Hop"));
  m_dlgBase->m_genre->insertItem(i18n("House"));
  m_dlgBase->m_genre->insertItem(i18n("Humor"));
  m_dlgBase->m_genre->insertItem(i18n("Indie"));
  m_dlgBase->m_genre->insertItem(i18n("Industrial"));
  m_dlgBase->m_genre->insertItem(i18n("Instrumental Pop"));
  m_dlgBase->m_genre->insertItem(i18n("Instrumental Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Instrumental"));
  m_dlgBase->m_genre->insertItem(i18n("Jazz+Funk"));
  m_dlgBase->m_genre->insertItem(i18n("Jazz"));
  m_dlgBase->m_genre->insertItem(i18n("JPop"));
  m_dlgBase->m_genre->insertItem(i18n("Jungle"));
  m_dlgBase->m_genre->insertItem(i18n("Latin"));
  m_dlgBase->m_genre->insertItem(i18n("Lo-Fi"));
  m_dlgBase->m_genre->insertItem(i18n("Meditative"));
  m_dlgBase->m_genre->insertItem(i18n("Merengue"));
  m_dlgBase->m_genre->insertItem(i18n("Metal"));
  m_dlgBase->m_genre->insertItem(i18n("Musical"));
  m_dlgBase->m_genre->insertItem(i18n("National Folk"));
  m_dlgBase->m_genre->insertItem(i18n("Native American"));
  m_dlgBase->m_genre->insertItem(i18n("Negerpunk"));
  m_dlgBase->m_genre->insertItem(i18n("New Age"));
  m_dlgBase->m_genre->insertItem(i18n("New Wave"));
  m_dlgBase->m_genre->insertItem(i18n("Noise"));
  m_dlgBase->m_genre->insertItem(i18n("Oldies"));
  m_dlgBase->m_genre->insertItem(i18n("Opera"));
  m_dlgBase->m_genre->insertItem(i18n("Other"));
  m_dlgBase->m_genre->insertItem(i18n("Polka"));
  m_dlgBase->m_genre->insertItem(i18n("Polsk Punk"));
  m_dlgBase->m_genre->insertItem(i18n("Pop-Funk"));
  m_dlgBase->m_genre->insertItem(i18n("Pop/Funk"));
  m_dlgBase->m_genre->insertItem(i18n("Pop"));
  m_dlgBase->m_genre->insertItem(i18n("Porn Groove"));
  m_dlgBase->m_genre->insertItem(i18n("Power Ballad"));
  m_dlgBase->m_genre->insertItem(i18n("Pranks"));
  m_dlgBase->m_genre->insertItem(i18n("Primus"));
  m_dlgBase->m_genre->insertItem(i18n("Progressive Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Psychedelic Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Psychedelic"));
  m_dlgBase->m_genre->insertItem(i18n("Punk Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Punk"));
  m_dlgBase->m_genre->insertItem(i18n("R&B"));
  m_dlgBase->m_genre->insertItem(i18n("Rap"));
  m_dlgBase->m_genre->insertItem(i18n("Rave"));
  m_dlgBase->m_genre->insertItem(i18n("Reggae"));
  m_dlgBase->m_genre->insertItem(i18n("Retro"));
  m_dlgBase->m_genre->insertItem(i18n("Revival"));
  m_dlgBase->m_genre->insertItem(i18n("Rhythmic Soul"));
  m_dlgBase->m_genre->insertItem(i18n("Rock & Roll"));
  m_dlgBase->m_genre->insertItem(i18n("Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Salsa"));
  m_dlgBase->m_genre->insertItem(i18n("Samba"));
  m_dlgBase->m_genre->insertItem(i18n("Satire"));
  m_dlgBase->m_genre->insertItem(i18n("Showtunes"));
  m_dlgBase->m_genre->insertItem(i18n("Ska"));
  m_dlgBase->m_genre->insertItem(i18n("Slow Jam"));
  m_dlgBase->m_genre->insertItem(i18n("Slow Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Sonata"));
  m_dlgBase->m_genre->insertItem(i18n("Soul"));
  m_dlgBase->m_genre->insertItem(i18n("Sound Clip"));
  m_dlgBase->m_genre->insertItem(i18n("Soundtrack"));
  m_dlgBase->m_genre->insertItem(i18n("Southern Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Space"));
  m_dlgBase->m_genre->insertItem(i18n("Speech"));
  m_dlgBase->m_genre->insertItem(i18n("Swing"));
  m_dlgBase->m_genre->insertItem(i18n("Symphonic Rock"));
  m_dlgBase->m_genre->insertItem(i18n("Symphony"));
  m_dlgBase->m_genre->insertItem(i18n("Synthpop"));
  m_dlgBase->m_genre->insertItem(i18n("Tango"));
  m_dlgBase->m_genre->insertItem(i18n("Techno-Industrial"));
  m_dlgBase->m_genre->insertItem(i18n("Techno"));
  m_dlgBase->m_genre->insertItem(i18n("Terror"));
  m_dlgBase->m_genre->insertItem(i18n("Thrash Metal"));
  m_dlgBase->m_genre->insertItem(i18n("Top 40"));
  m_dlgBase->m_genre->insertItem(i18n("Trailer"));
  m_dlgBase->m_genre->insertItem(i18n("Trance"));
  m_dlgBase->m_genre->insertItem(i18n("Tribal"));
  m_dlgBase->m_genre->insertItem(i18n("Trip-Hop"));
  m_dlgBase->m_genre->insertItem(i18n("Vocal"));

  m_dlgBase->m_trackList->setRenameable(TRACK_NUMBER, false);
  m_dlgBase->m_trackList->setRenameable(TRACK_TITLE, true);
  m_dlgBase->m_trackList->setRenameable(TRACK_COMMENT, true);
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
    m_dlgBase->m_trackList->clear();
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
  m_dlgBase->m_category->setCurrentText(cddbInfo.category.stripWhiteSpace());
  m_dlgBase->m_genre->setCurrentText(cddbInfo.genre.stripWhiteSpace());
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

  m_dlgBase->m_trackList->clear();

  for(unsigned i = 1; i <= ntracks; i++)
  {
    dl = framestoTime(cddbtoc[i].absframe-cddbtoc[i-1].absframe);

    QListViewItem * item = new QListViewItem( m_dlgBase->m_trackList, 0 );
    item->setText(TRACK_NUMBER, QString().sprintf("%02d",i) );
    if (dl.hour() > 0)
      item->setText(TRACK_TIME, dl.toString("hh:mm:ss"));
    else
      item->setText(TRACK_TIME, dl.toString("mm:ss"));

    item->setText(TRACK_TITLE, cddbInfo.trackInfoList[i-1].title);
    item->setText(TRACK_COMMENT, cddbInfo.trackInfoList[i-1].extt);
  }

  m_dlgBase->le_playOrder->setText( playlist.join( "," ) );
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
  if (!updateFromDialog())
    return;

  // Create a copy with a bumped revision number.
  KCDDB::CDInfo copyInfo = cddbInfo;
  copyInfo.revision++;

  KCDDB::TrackOffsetList offsetList;

  for( int i = 0; i < cd->ntracks + 1; i++)
  {
    if (i < cd->ntracks)
      offsetList.append(cddbtoc[i].absframe);
  }
  offsetList.append(cddbtoc[0].absframe);
  offsetList.append(cddbtoc[ntracks].absframe);

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

  bool have_nonempty_title = false;
  for (QListViewItem *item = m_dlgBase->m_trackList->firstChild(); item; item=item->nextSibling())
  {
      QString songTitle = item->text(TRACK_TITLE).stripWhiteSpace();
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

  // Playorder...
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

  // The information all passed our checks. Update the stored values from the dialog.
  cddbInfo.title = title;
  cddbInfo.artist = artist;
  cddbInfo.category = m_dlgBase->m_category->currentText();
  cddbInfo.extd = m_dlgBase->m_discComment->text().stripWhiteSpace();
  for (QListViewItem *item = m_dlgBase->m_trackList->firstChild(); item; item=item->nextSibling())
  {
    unsigned trackNumber = item->text(TRACK_NUMBER).toUInt();
    cddbInfo.trackInfoList[trackNumber - 1].title = item->text(TRACK_TITLE).stripWhiteSpace();
    cddbInfo.trackInfoList[trackNumber - 1].extt = item->text(TRACK_COMMENT).stripWhiteSpace();
    kdWarning() << "track " << trackNumber << "=" << cddbInfo.trackInfoList[trackNumber - 1].title<<endl;
  }
  return true;
} // updateFromDialog

#include "cddbdlg.moc"
