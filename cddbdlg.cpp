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
  // To cope with preexisting records which don't match an entry, we will
  // add one later if needed.
  m_dlgBase->m_genre->insertItem("Unknown");
  m_dlgBase->m_genre->insertItem("A Cappella");
  m_dlgBase->m_genre->insertItem("Acid Jazz");
  m_dlgBase->m_genre->insertItem("Acid Punk");
  m_dlgBase->m_genre->insertItem("Acid");
  m_dlgBase->m_genre->insertItem("Acoustic");
  m_dlgBase->m_genre->insertItem("Alternative");
  m_dlgBase->m_genre->insertItem("Alt. Rock");
  m_dlgBase->m_genre->insertItem("Ambient");
  m_dlgBase->m_genre->insertItem("Anime");
  m_dlgBase->m_genre->insertItem("Avantgarde");
  m_dlgBase->m_genre->insertItem("Ballad");
  m_dlgBase->m_genre->insertItem("Bass");
  m_dlgBase->m_genre->insertItem("Beat");
  m_dlgBase->m_genre->insertItem("Bebop");
  m_dlgBase->m_genre->insertItem("Big Band");
  m_dlgBase->m_genre->insertItem("Black Metal");
  m_dlgBase->m_genre->insertItem("Bluegrass");
  m_dlgBase->m_genre->insertItem("Blues");
  m_dlgBase->m_genre->insertItem("Booty Bass");
  m_dlgBase->m_genre->insertItem("BritPop");
  m_dlgBase->m_genre->insertItem("Cabaret");
  m_dlgBase->m_genre->insertItem("Celtic");
  m_dlgBase->m_genre->insertItem("Chamber Music");
  m_dlgBase->m_genre->insertItem("Chanson");
  m_dlgBase->m_genre->insertItem("Chorus");
  m_dlgBase->m_genre->insertItem("Christian Gangsta Rap");
  m_dlgBase->m_genre->insertItem("Christian Rap");
  m_dlgBase->m_genre->insertItem("Christian Rock");
  m_dlgBase->m_genre->insertItem("Classical");
  m_dlgBase->m_genre->insertItem("Classic Rock");
  m_dlgBase->m_genre->insertItem("Club-house");
  m_dlgBase->m_genre->insertItem("Club");
  m_dlgBase->m_genre->insertItem("Comedy");
  m_dlgBase->m_genre->insertItem("Contemporary Christian");
  m_dlgBase->m_genre->insertItem("Country");
  m_dlgBase->m_genre->insertItem("Crossover");
  m_dlgBase->m_genre->insertItem("Cult");
  m_dlgBase->m_genre->insertItem("Dance Hall");
  m_dlgBase->m_genre->insertItem("Dance");
  m_dlgBase->m_genre->insertItem("Darkwave");
  m_dlgBase->m_genre->insertItem("Death Metal");
  m_dlgBase->m_genre->insertItem("Disco");
  m_dlgBase->m_genre->insertItem("Dream");
  m_dlgBase->m_genre->insertItem("Drum & Bass");
  m_dlgBase->m_genre->insertItem("Drum Solo");
  m_dlgBase->m_genre->insertItem("Duet");
  m_dlgBase->m_genre->insertItem("Easy Listening");
  m_dlgBase->m_genre->insertItem("Electronic");
  m_dlgBase->m_genre->insertItem("Ethnic");
  m_dlgBase->m_genre->insertItem("Eurodance");
  m_dlgBase->m_genre->insertItem("Euro-House");
  m_dlgBase->m_genre->insertItem("Euro-Techno");
  m_dlgBase->m_genre->insertItem("Fast-Fusion");
  m_dlgBase->m_genre->insertItem("Folklore");
  m_dlgBase->m_genre->insertItem("Folk/Rock");
  m_dlgBase->m_genre->insertItem("Folk");
  m_dlgBase->m_genre->insertItem("Freestyle");
  m_dlgBase->m_genre->insertItem("Funk");
  m_dlgBase->m_genre->insertItem("Fusion");
  m_dlgBase->m_genre->insertItem("Game");
  m_dlgBase->m_genre->insertItem("Gangsta Rap");
  m_dlgBase->m_genre->insertItem("Goa");
  m_dlgBase->m_genre->insertItem("Gospel");
  m_dlgBase->m_genre->insertItem("Gothic Rock");
  m_dlgBase->m_genre->insertItem("Gothic");
  m_dlgBase->m_genre->insertItem("Grunge");
  m_dlgBase->m_genre->insertItem("Hardcore");
  m_dlgBase->m_genre->insertItem("Hard Rock");
  m_dlgBase->m_genre->insertItem("Heavy Metal");
  m_dlgBase->m_genre->insertItem("Hip-Hop");
  m_dlgBase->m_genre->insertItem("House");
  m_dlgBase->m_genre->insertItem("Humor");
  m_dlgBase->m_genre->insertItem("Indie");
  m_dlgBase->m_genre->insertItem("Industrial");
  m_dlgBase->m_genre->insertItem("Instrumental Pop");
  m_dlgBase->m_genre->insertItem("Instrumental Rock");
  m_dlgBase->m_genre->insertItem("Instrumental");
  m_dlgBase->m_genre->insertItem("Jazz+Funk");
  m_dlgBase->m_genre->insertItem("Jazz");
  m_dlgBase->m_genre->insertItem("JPop");
  m_dlgBase->m_genre->insertItem("Jungle");
  m_dlgBase->m_genre->insertItem("Latin");
  m_dlgBase->m_genre->insertItem("Lo-Fi");
  m_dlgBase->m_genre->insertItem("Meditative");
  m_dlgBase->m_genre->insertItem("Merengue");
  m_dlgBase->m_genre->insertItem("Metal");
  m_dlgBase->m_genre->insertItem("Musical");
  m_dlgBase->m_genre->insertItem("National Folk");
  m_dlgBase->m_genre->insertItem("Native American");
  m_dlgBase->m_genre->insertItem("Negerpunk");
  m_dlgBase->m_genre->insertItem("New Age");
  m_dlgBase->m_genre->insertItem("New Wave");
  m_dlgBase->m_genre->insertItem("Noise");
  m_dlgBase->m_genre->insertItem("Oldies");
  m_dlgBase->m_genre->insertItem("Opera");
  m_dlgBase->m_genre->insertItem("Other");
  m_dlgBase->m_genre->insertItem("Polka");
  m_dlgBase->m_genre->insertItem("Polsk Punk");
  m_dlgBase->m_genre->insertItem("Pop-Funk");
  m_dlgBase->m_genre->insertItem("Pop/Funk");
  m_dlgBase->m_genre->insertItem("Pop");
  m_dlgBase->m_genre->insertItem("Porn Groove");
  m_dlgBase->m_genre->insertItem("Power Ballad");
  m_dlgBase->m_genre->insertItem("Pranks");
  m_dlgBase->m_genre->insertItem("Primus");
  m_dlgBase->m_genre->insertItem("Progressive Rock");
  m_dlgBase->m_genre->insertItem("Psychedelic Rock");
  m_dlgBase->m_genre->insertItem("Psychedelic");
  m_dlgBase->m_genre->insertItem("Punk Rock");
  m_dlgBase->m_genre->insertItem("Punk");
  m_dlgBase->m_genre->insertItem("R&B");
  m_dlgBase->m_genre->insertItem("Rap");
  m_dlgBase->m_genre->insertItem("Rave");
  m_dlgBase->m_genre->insertItem("Reggae");
  m_dlgBase->m_genre->insertItem("Retro");
  m_dlgBase->m_genre->insertItem("Revival");
  m_dlgBase->m_genre->insertItem("Rhythmic Soul");
  m_dlgBase->m_genre->insertItem("Rock & Roll");
  m_dlgBase->m_genre->insertItem("Rock");
  m_dlgBase->m_genre->insertItem("Salsa");
  m_dlgBase->m_genre->insertItem("Samba");
  m_dlgBase->m_genre->insertItem("Satire");
  m_dlgBase->m_genre->insertItem("Showtunes");
  m_dlgBase->m_genre->insertItem("Ska");
  m_dlgBase->m_genre->insertItem("Slow Jam");
  m_dlgBase->m_genre->insertItem("Slow Rock");
  m_dlgBase->m_genre->insertItem("Sonata");
  m_dlgBase->m_genre->insertItem("Soul");
  m_dlgBase->m_genre->insertItem("Sound Clip");
  m_dlgBase->m_genre->insertItem("Soundtrack");
  m_dlgBase->m_genre->insertItem("Southern Rock");
  m_dlgBase->m_genre->insertItem("Space");
  m_dlgBase->m_genre->insertItem("Speech");
  m_dlgBase->m_genre->insertItem("Swing");
  m_dlgBase->m_genre->insertItem("Symphonic Rock");
  m_dlgBase->m_genre->insertItem("Symphony");
  m_dlgBase->m_genre->insertItem("Synthpop");
  m_dlgBase->m_genre->insertItem("Tango");
  m_dlgBase->m_genre->insertItem("Techno-Industrial");
  m_dlgBase->m_genre->insertItem("Techno");
  m_dlgBase->m_genre->insertItem("Terror");
  m_dlgBase->m_genre->insertItem("Thrash Metal");
  m_dlgBase->m_genre->insertItem("Top 40");
  m_dlgBase->m_genre->insertItem("Trailer");
  m_dlgBase->m_genre->insertItem("Trance");
  m_dlgBase->m_genre->insertItem("Tribal");
  m_dlgBase->m_genre->insertItem("Trip-Hop");
  m_dlgBase->m_genre->insertItem("Vocal");

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

  // Disable changes to category if the version number indicates that a record
  // is already in the database.
  m_dlgBase->m_category->setEnabled(cddbInfo.revision < 1);

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
    kdDebug() << "track " << trackNumber << "=" << cddbInfo.trackInfoList[trackNumber - 1].title<<endl;
  }
  return true;
} // updateFromDialog

#include "cddbdlg.moc"
