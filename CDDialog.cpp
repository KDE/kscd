/***********************************************************************
 *
 * I can't get no dlgedit running. So I put this note here:
 *
 *
 *      --- This file has been manually midified ---
 *
 *
 * $Id$
 *
 ***********************************************************************/

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

#include <klocale.h>
#include <kdebug.h>
#include <ktempfile.h>

#include <stdio.h>
#include <math.h>

#include <kapplication.h>
#include <kmessagebox.h>

#include "CDDialog.h"
#include "CDDialog.moc"
#include "CDDialogData.moc"
#include "inexact.h"
#include "version.h"
#include "kscd.h"

extern "C" {
#include "libwm/include/workman.h"
}

QTime framestoTime(int frames);

CDDialog::CDDialog
(
	QWidget* parent,
	const char* name
)
	:
	CDDialogData( parent, name )
{
    cdinfo.magicID = QString::null;	/*cddb magic disk id BERND*/
    cdinfo.ntracks = 0;	/* Number of tracks on the disc */
    cdinfo.length  = 0;	/* Total running time in seconds */
    cdinfo.cddbtoc = 0L;

    connect(tracksList,     SIGNAL(selectionChanged(QListViewItem *)),this,SLOT(titleselected(QListViewItem *)));
    connect(tracksList,     SIGNAL(selectionChanged(QListViewItem *)),this,SLOT(play(QListViewItem *)));
    connect(trackEdit,   SIGNAL(textChanged( const QString & )) ,this,SLOT(trackchanged( const QString & )));
    connect(trackEdit,   SIGNAL( returnPressed ()) ,this,SLOT(nextTrack()));
    connect(ok_button, SIGNAL(clicked())       ,this,SLOT(save()));
    connect(upload_button, SIGNAL(clicked())       ,this,SLOT(upload()));
    connect(cancel_button, SIGNAL(clicked())       ,this,SLOT(cancel()));
    connect(load_button, SIGNAL(clicked())       ,this,SLOT(load_cddb()));
    connect(ext_info_title_button, SIGNAL(clicked()) ,this,SLOT(extITB()));
    connect(ext_info_button, SIGNAL(clicked())       ,this,SLOT(extIB()));
    connect(titleEdit,   SIGNAL(textChanged(const QString &)), this,SLOT(titlechanged()));
    connect(artistEdit,   SIGNAL(textChanged(const QString &)), this,SLOT(titlechanged()));
    ext_info_button->setEnabled(false);

    catlist.append("rock");
    catlist.append("classical");
    catlist.append("jazz");
    catlist.append("soundtrack");
    catlist.append("newage");
    catlist.append("blues");
    catlist.append("folk");
    catlist.append("country");
    catlist.append("reggae");
    catlist.append("misc");
    catlist.append("data");

    cddbClient = new KCDDB::Client();
    cddbClient->setBlockingMode(false);
    connect (cddbClient, SIGNAL(finished(CDDB::Result)),
             this, SLOT(submitFinished(CDDB::Result)));
} // CDDialog


CDDialog::~CDDialog()
{
    if(cdinfo.cddbtoc)
        delete [] cdinfo.cddbtoc;
    delete cddbClient;
} // ~CDDialog

void
CDDialog::closeEvent(QCloseEvent*)
{
    emit dialog_done();
} // closeEvent

void
CDDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->state() == 0 && e->key() == Key_Escape)
        emit dialog_done();
} // keyPressEvent

void
CDDialog::cancel()
{
  emit dialog_done();
} // ok

void
CDDialog::play(QListViewItem *item)
{
  emit play_signal(item->text(0).toInt() - 1);
} // play


void
CDDialog::setData(
        struct wm_cdinfo *cd,
        QStringList& tracktitlelist,
        QStringList& extlist,
        QString& _xmcd_data,
        QString& cat,
	QString& _genre,
        int rev,
	int _year,
        QStringList& _playlist,
        QStringList& _pathlist
        )
{
    int ntr;

    ext_list 	= extlist;
    track_list 	= tracktitlelist;
    xmcd_data   = _xmcd_data.copy();
    category 	= cat.copy();
    genre       = _genre.copy();
    revision    = rev;
    year        = _year;
    playlist	= _playlist;
    pathlist	= _pathlist;

    ntr = track_list.count();

    // Let's make a deep copy of the cd struct info so that the data won't
    // change the cd changes while we are playing with the dialog.

    // put one of these into the destructor too..
    if(cdinfo.cddbtoc)
      delete [] cdinfo.cddbtoc;
    if(!cd)
      return;
    cdinfo.cddbtoc =  new struct mytoc [cd->ntracks + 2];
    /*
     * Avoid people who need to edit titles of "no discs" to crash kscd.
     */
    if( cd->ntracks == 0 )
      {
        cdinfo.magicID = QString::null;
        cdinfo.ntracks = 0;
        cdinfo.length = 0;
        titleEdit->setText("No Disc");
        disc_id_label->clear();
        tracksList->clear();
        return;
      }

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
    cdinfo.magicID = KCDDB::CDDB::trackOffsetListToId(offsetList); /* cddb magic disk id */

    // some sanity provisions

    if((int)track_list.count() < cdinfo.ntracks + 1)
      {
	int k = track_list.count();
	for(int i = 0 ; i < (int)( cdinfo.ntracks + 1 - k) ; i ++)
	  {
	    track_list.append("");
	  }
      }

    if((int)ext_list.count() < cdinfo.ntracks + 1)
      {
	int l = ext_list.count();
	for(int i = 0 ; i < (int) ( cdinfo.ntracks + 1 - l) ; i ++)
	  {
	    ext_list.append("");
	  }
      }

    while((int)track_list.count() > cdinfo.ntracks + 1)
      {
	track_list.remove(track_list.at(track_list.count() - 1));
      }

    while((int)ext_list.count() > cdinfo.ntracks + 1)
      {
	ext_list.remove(ext_list.at(ext_list.count() - 1));
      }

    artistEdit->setText((track_list.first().section('/',0,0)).stripWhiteSpace());
    titleEdit->setText((track_list.first().section('/',1,1)).stripWhiteSpace());

    QString idstr;
    idstr = category + "  " + cdinfo.magicID;

    disc_id_label->setText(idstr.stripWhiteSpace());

    QTime   dl;
    dl 	=  dl.addSecs(cdinfo.length);

    QString temp2;
    if(dl.hour() > 0)
      temp2 = dl.toString("hh:mm:ss:");
    else
      temp2 = dl.toString("mm:ss");
    total_time_label->setText(temp2);

    QString 	fmt;

    tracksList->clear();

    for(int i = 1; i <= cdinfo.ntracks; i++)
      {
	dl = framestoTime(cdinfo.cddbtoc[i].absframe - cdinfo.cddbtoc[i-1].absframe);

        QListViewItem * item = new QListViewItem( tracksList, 0 );
        item->setText( 0, QString().sprintf("%02d",i) );
        if (dl.hour() > 0)
            item->setText( 1, dl.toString("hh:mm:ss"));
        else
            item->setText( 1, dl.toString("mm:ss"));

	if((ntr >=  i) && (ntr > 0))
            item->setText( 2,  *(track_list.at(i)));
      }

    QString str;
    progseq_edit->setText(str);
} // setData

void
CDDialog::extIB()
{
  if (!tracksList->currentItem())
  {
    return;
  }

  int trackNumber = tracksList->currentItem()->text(0).toInt();
  QString trackTitle = tracksList->currentItem()->text(2);
  QString dialogTitle;

  if (!trackTitle.isNull())
  {
    dialogTitle = i18n("Use this editor to annotate track #%1: %2").arg(trackNumber).arg(trackTitle);
  }
  else
  {
    dialogTitle = i18n("Use this editor to annotate track #%1.").arg(trackNumber);
  }

  InexactDialog *dialog = new InexactDialog(this,"dialog",false);
  dialog->setTitle(dialogTitle);

  dialog->insertText(*ext_list.at(trackNumber));

  if(dialog->exec() == QDialog::Accepted)
  {
    QString text;
    text = dialog->selection();

    *ext_list.at(trackNumber) = text;
  }
  delete dialog;
} // extIB

void
CDDialog::extITB()
{
  InexactDialog *dialog = new InexactDialog(this,"dialog",false);
  dialog->insertText(ext_list.first());
  dialog->setTitle(i18n("Enter annotation for this album:"));

  if(dialog->exec() == QDialog::Accepted)
  {
    QString text;
    text = dialog->selection();

    *ext_list.at(0) = text;
  }
  delete dialog;
} // extITB

void CDDialog::titleselected(QListViewItem *item)
{
    ext_info_button->setEnabled(true);
    trackEdit->setEnabled(true);
    if(item->text(0).toInt() <  (int)track_list.count()) {
        trackEdit->setText(item->text(2));
        trackEdit->setFocus();
    }

} // titleselected

void CDDialog::trackchanged( const QString &text ) {
    int i = tracksList->currentItem()->text(0).toInt();
    track_list.insert(track_list.at(i),text);
    track_list.remove(track_list.at(i+1));

    tracksList->currentItem()->setText( 2,  text);
} // trackchanged

void CDDialog::nextTrack() {
    int i = tracksList->currentItem()->text(0).toInt();
    QListViewItem  *item = tracksList->findItem ( QString().sprintf("%02d",i+1), 0,ExactMatch);
    tracksList->setSelected(item, true);
    tracksList->ensureItemVisible(item);
}

void
CDDialog::submitFinished(KCDDB::CDDB::Result r)
{
  if (r == KCDDB::CDDB::Success)
  {
    KMessageBox::information(this, i18n("Record submitted successfully"),
         i18n("Record Submission"));
    upload_button->setDisabled(true);
  }
  else
  {
    QString str = i18n("Error sending message via SMTP.\n\n%1")
      .arg(KCDDB::CDDB::resultToString(r));
    KMessageBox::error(this, str, i18n("Record Submission"));
  }
} // submitFinished()

QTime
framestoTime(int _frames)
{
  QTime 	dml;
  double 	frames;
  double 	dsecs;
  int 		secs;
  double 	ip;

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

void CDDialog::titlechanged() {
    QString title = titleEdit->text().stripWhiteSpace();
    QString artist = artistEdit->text().stripWhiteSpace();
    if(title.isEmpty() || artist.isEmpty()) return;
    title = QString("%1 / %2").arg(artist).arg(title);
    *track_list.begin() = title;
} // titlechanged

QString submitcat;

void
CDDialog::upload()
{
  if(!checkit())
    return;

  InexactDialog *dialog;
  dialog = new InexactDialog(this,"Dialog",true);

  if( category.length() < 1 )
	{
	  dialog->insertList(catlist);
	  dialog->setErrorString(i18n("Please select a category or press Cancel"));
	  dialog->setTitle(i18n("To Which Category Does CD Belong?"));
	  if(dialog->exec() != QDialog::Accepted)
		{
		  delete dialog;
		  return;
		}

	  submitcat = dialog->selection();
	} else {
	  submitcat = category.copy();
	}
  delete dialog;
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

void
CDDialog::setCdInfo(KCDDB::CDInfo &info, const QString& category)
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

void
CDDialog::save()
{
  if(!checkit())
  {
    emit dialog_done();
    return;
  }

  QString savecat;

  if( category.length() < 1 )
  {
    InexactDialog *dialog;
    dialog = new InexactDialog(0,"Dialog",true);
    dialog->insertList(catlist);
    dialog->setErrorString(i18n("Please select a category or press Cancel"));
    dialog->setTitle(i18n("Under Which Category Would you Like to Store This Disc's Information?"));

    if(dialog->exec() != QDialog::Accepted)
    {
      delete dialog;
      emit dialog_done();
      return;
    }

    savecat = dialog->selection();
    delete dialog;
  }
  else
  {
    savecat = category.copy();
  }

  KCDDB::CDInfo info;
  setCdInfo(info, savecat);
  // Playorder...

  KCDDB::Cache::store(info);

  load_cddb();
  emit dialog_done();
} // save

bool
CDDialog::checkit()
{
  QString title = titleEdit->text().stripWhiteSpace();
  if(title.isEmpty())
    {
      KMessageBox::sorry(this,
			 i18n("The title of the disc has to be entered.\n"
			      "Please correct the entry and try again."),
			 i18n("Invalid Database Entry"));
      return false;
    }

  QString artist = artistEdit->text().stripWhiteSpace();
  if(artist.isEmpty())
    {
      KMessageBox::sorry(this,
			 i18n("The artist name of the disc has to be entered.\n"
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
      if(!title.isEmpty()){
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

  QString str;
  QStringList strlist;
  str = progseq_edit->text();

  bool ret  = true;

  QString teststr;
  bool ok;
  int  num;

  for ( QStringList::Iterator it = strlist.begin();
        it != strlist.end();
        ++it )
    {
      teststr = *it;
      num = teststr.toInt(&ok);

      if( num > cdinfo.ntracks || !ok)
	ret = false;
    }

  if(!ret)
    {
      KMessageBox::sorry(this,
			 i18n("Invalid Playlist\nPlease use track numbers only, separated by commas."));
      return false;
    }

  return true;
} // checkit

void
CDDialog::load_cddb()
{
  emit cddb_query_signal(true);
} // load

