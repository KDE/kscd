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
#include <qregexp.h> 
#include <qdatetime.h> 
#include <qtextstream.h> 
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h> 

#include <klocale.h>
#include <kdebug.h>
#include <ktempfile.h>

#include <stdio.h>
#include <math.h>

#include <kapp.h>
#include <kmessagebox.h>

#include "CDDialog.h"
#include "CDDialog.moc"
#include "CDDialogData.moc"
#include "inexact.h"
#include "version.h"
#include "smtp.h"
#include "smtpconfig.h"
#include "kscd.h"

extern "C" {
#include "libwm/include/workman.h"
}

#define Inherited CDDialogData

QTime framestoTime(int frames);
void  mimetranslate(QString& s);
extern void cddb_decode(QString& str);
extern void cddb_encode(QString& str, QStringList &returnlist);
extern void cddb_playlist_encode(QStringList& list,QString& playstr);
extern bool cddb_playlist_decode(QStringList& list, QString& str);

extern SMTP *smtpMailer;

CDDialog::CDDialog
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{
	setCaption( i18n("CD Database Editor") );

	cdinfo.magicID = 0;	/*cddb magic disk id BERND*/
	cdinfo.ntracks = 0;	/* Number of tracks on the disc */
	cdinfo.length  = 0;	/* Total running time in seconds */
	cdinfo.cddbtoc = 0L;

	connect(listbox,     SIGNAL(highlighted(int)),this,SLOT(titleselected(int)));
	connect(listbox,     SIGNAL(selected(int)),this,SLOT(play(int)));
	connect(trackedit,   SIGNAL(returnPressed()) ,this,SLOT(trackchanged()));
	connect(save_button, SIGNAL(clicked())       ,this,SLOT(save()));
	connect(upload_button, SIGNAL(clicked())       ,this,SLOT(upload()));
	connect(ok_button, SIGNAL(clicked())       ,this,SLOT(ok()));
	connect(load_button, SIGNAL(clicked())       ,this,SLOT(load_cddb()));
	connect(ext_info_title_button, SIGNAL(clicked()) ,this,SLOT(extITB()));
	connect(ext_info_button, SIGNAL(clicked())       ,this,SLOT(extIB()));
	connect(titleedit,   SIGNAL(textChanged(const QString &)),
		             this,SLOT(titlechanged(const QString &)));
	ext_info_button->setEnabled(false);
	
	setFixedSize(width(),height());


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

} // CDDialog


CDDialog::~CDDialog()
{
    if(cdinfo.cddbtoc)
      delete [] cdinfo.cddbtoc;
} // ~CDDialog

void 
CDDialog::closeEvent(QCloseEvent*)
{
    kdDebug() << "emmitting done()" << endl;
    emit dialog_done();
} // closeEvent

void 
CDDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->state() == 0 && e->key() == Key_Escape)
        emit dialog_done();
} // keyPressEvent

void 
CDDialog::ok()
{
  emit dialog_done();
} // ok

void 
CDDialog::play(int i)
{
  emit play_signal(i);
} // play


void 
CDDialog::setData(
		  struct wm_cdinfo *cd,
		  QStringList& tracktitlelist,
		  QStringList& extlist,
		  QStringList& discidl,
		  QString& _xmcd_data,
		  QString& cat,
		  int& rev,
		  QStringList& _playlist,
		  QStringList& _pathlist,
		  QString& _cddbbasedir,
		  QString& _submitaddress,
		  SMTPConfig::SMTPConfigData *_smtpConfigData
		  )
{
    int ntr, etr;

    ext_list 	= extlist;
    track_list 	= tracktitlelist;
    xmcd_data   = _xmcd_data.copy();
    category 	= cat.copy(); 
    discidlist  = discidl;
    revision    = rev;
    playlist	= _playlist;
    pathlist	= _pathlist;
	cddbbasedir = _cddbbasedir.copy();
    submitaddress = _submitaddress.copy();
    smtpConfigData = _smtpConfigData;
    
    ntr = track_list.count();
    etr = ext_list.count();

    // Let's make a deep copy of the cd struct info so that the data won't
    // change the cd changes while we are playing with the dialog.

    // put one of these into the destructor too..
    if(cdinfo.cddbtoc)
      delete [] cdinfo.cddbtoc;

    
    cdinfo.cddbtoc =  new struct mytoc [cd->ntracks + 2];

    /*
     * Avoid people who need to edit titles of "no discs" to crash kscd.
     */
    if( cd->ntracks == 0 ) 
      {
        cdinfo.magicID = 0;
        cdinfo.ntracks = 0;
        cdinfo.length = 0;
        titleedit->setText("No Disc");      
        disc_id_label->clear();
        listbox->clear();
        listbox->repaint();
        return;
      }

    cdinfo.magicID = cddb_discid();	/* cddb magic disk id            */
    cdinfo.ntracks = cd->ntracks;	/* Number of tracks on the disc  */
    cdinfo.length  = cd->length;	/* Total running time in seconds */


    for( int i = 0; i < cd->ntracks + 1; i++)
      {
        cdinfo.cddbtoc[i].min = cd->trk[i].start / 4500;
        cdinfo.cddbtoc[i].sec = (cd->trk[i].start % 4500) / 75;
        cdinfo.cddbtoc[i].frame = cd->trk[i].start - ((cd->trk[i].start / 75)*75);
        cdinfo.cddbtoc[i].absframe = cd->trk[i].start;
      }

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
    

    titleedit->setText(track_list.first());

    QString idstr;
    idstr.sprintf("%08lx",cddb_discid());
    idstr = category + "  " + idstr;

    disc_id_label->setText(idstr.stripWhiteSpace());

    QTime   dl;
    dl 	=  dl.addSecs(cdinfo.length);

    QString temp2;
    if(dl.hour() > 0)
      temp2 = QString::fromUtf8( QCString().sprintf("%02d:%02d:%02d",dl.hour(),dl.minute(),dl.second()) );
    else
      temp2 = QString::fromUtf8( QCString().sprintf("%02d:%02d",dl.minute(),dl.second()) );
    total_time_label->setText(temp2);

    QString 	fmt;
    QTime 	dml;

    listbox->clear();

    for(int i = 1; i <= cdinfo.ntracks; i++)
      {
	dml = framestoTime(cdinfo.cddbtoc[i].absframe - cdinfo.cddbtoc[i-1].absframe);
	
	if((ntr >=  i) && (ntr > 0))
	  {
	    fmt.sprintf("%02d   %02d:%02d   %s",i, dml.minute(),dml.second(),(*track_list.at(i)).utf8().data());
	  } else {
	    fmt.sprintf("%02d   %02d:%02d",i,dml.minute(),dml.second());
	  }
	listbox->insertItem(fmt,-1);
      }
    
    listbox->repaint();
    
    QString str;
    cddb_playlist_encode(playlist,str);
    progseq_edit->setText(str);
} // setData

void 
CDDialog::extIB()
{
  
  int item;
  item = listbox->currentItem();
  if(item == -1)
    return;

  InexactDialog *dialog;
  dialog = new InexactDialog(0,"dialog",false);
  dialog->setTitle(i18n("Use this editor to annotate this track"));

  dialog->insertText(*ext_list.at(item + 1));
  
  if(dialog->exec() != QDialog::Accepted)
    {
      delete dialog;
      return;
    }
  
  QString text;
  dialog->getSelection(text);

  *ext_list.at(item + 1) = text;
  //ext_list.remove( ext_list.at(item + 1) );
  //ext_list.insert( ext_list.at(item + 1) , text);

  delete dialog;
} // extIB

void 
CDDialog::extITB()
{
  InexactDialog *dialog;
  dialog = new InexactDialog(0,"dialog",false);
  dialog->insertText(ext_list.first());
  dialog->setTitle(i18n("Use this editor to annotate the title"));
  
  if(dialog->exec() != QDialog::Accepted)
    {
      delete dialog;
      return;
    }
  
  QString text;
  dialog->getSelection(text);

  *ext_list.at(0) = text;
  //ext_list.insert( 0 , text );
  //ext_list.remove( 1 );

  delete dialog;
} // extITB

void CDDialog::titleselected(int i)
{
  ext_info_button->setEnabled(true);
  if(i + 1 < (int)track_list.count())
    trackedit->setText(*track_list.at(i+1));
} // titleselected

void 
CDDialog::trackchanged()
{
  int i;

  i = listbox->currentItem();
  if (i == -1)
    return;
  
  QTime dml = framestoTime(cdinfo.cddbtoc[i+1].absframe - cdinfo.cddbtoc[i].absframe);

  QString fmt;

  fmt.sprintf("%02d   %02d:%02d   %s",i+1,dml.minute(),dml.second(),trackedit->text().utf8().data());

  //  *track_list.at(i+1) = trackedit->text();
  track_list.insert(track_list.at(i+1),trackedit->text());
  track_list.remove(track_list.at(i+2));

  listbox->insertItem(fmt, i);
  listbox->removeItem(i+1);
  listbox->repaint();
  if ( i <(int) listbox->count() -1 )
    {
      listbox->setCurrentItem(i+1);
      listbox->centerCurrentItem();
    }
} // trackchanged


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

void 
CDDialog::titlechanged(const QString &t)
{
  *track_list.begin() = t;
  //track_list.remove(track_list.begin());
  //track_list.insert(track_list.begin(), t);
} // titlechanged

QString submitcat;

void 
CDDialog::upload()
{
  if(!checkit())
    return;

  InexactDialog *dialog;
  dialog = new InexactDialog(0,"Dialog",true);

  if( category.length() < 1 )
	{
	  dialog->insertList(catlist);
	  dialog->setErrorString(i18n("Please select a category or press Cancel"));
	  dialog->setTitle(i18n("To which category does the CD belong?"));
	  if(dialog->exec() != QDialog::Accepted)
		{
		  delete dialog;
		  return;
		}
	  
	  dialog->getSelection(submitcat);
	  delete dialog;
	} else {
	  submitcat = category.copy();
	}

  KTempFile tmpFile;
  tmpFile.setAutoDelete(true); // delete file when we are done.
  QString tempfile = tmpFile.name();

  save_cddb_entry(tempfile,true);

  kapp->processEvents();
  kapp->flushX();


  if(smtpConfigData->enabled)
    {
      kdDebug() << "Submitting freedb entry via SMTP...\n" << endl;
      QFile file(tempfile);
      
      file.open(IO_ReadOnly);
      QTextStream ti(&file);

      QString s;
      QString subject;
      
      while (!ti.eof())
	{
          s += ti.readLine() + "\r\n";
	}

      smtpMailer->setServerHost(smtpConfigData->serverHost);
      smtpMailer->setPort(smtpConfigData->serverPort.toUInt());
      
      smtpMailer->setSenderAddress(smtpConfigData->senderAddress);
      smtpMailer->setSenderReplyTo(smtpConfigData->senderReplyTo);
      smtpMailer->setRecipientAddress(submitaddress);
      
      subject.sprintf("cddb %s %08lx", submitcat.utf8().data(), cdinfo.magicID);
      smtpMailer->setMessageSubject(subject);
      smtpMailer->setMessageBody(s);

      smtpMailer->sendMessage();
      
      return;
    }

      
  QString cmd;

  cmd = "sendmail -tU";

  kdDebug() << "Submitting freedb entry: " << cmd << "\n" << endl;
  
  FILE* mailpipe;
  mailpipe = popen(QFile::encodeName(cmd),"w");

  if(mailpipe == NULL){
    QString str;
    str = i18n("Could not pipe contents into:\n %1").arg(cmd);

    KMessageBox::error(this, str);
    pclose(mailpipe);
    return;
    
  }
  
  QFile file(tempfile);

  file.open(IO_ReadOnly);

  QTextStream ti(&file);


  QTextStream to(mailpipe,IO_WriteOnly );

  QString s;

  //to << "Content-Transfer-Encoding: quoted-printable\n";

  while ( !ti.eof() ) 
    {
      s = ti.readLine();
      if(!ti.eof())
	{
          //  mimetranslate(s);
          to << s << '\n';
	}
    }

  pclose(mailpipe);

  file.close();
  //  file2.close();   // *****

  kdDebug() << "DONE SENDING\n" << endl;
} // upload

void 
CDDialog::getCategoryFromPathName(char* pathname, QString& _category)
{
  QString path = pathname;
  path = path.stripWhiteSpace();

  while(path.right(1) == QString("/"))
    {
      path = path.left(path.length() - 1);
    }
  
  int pos = 0;
  pos  = path.findRev("/",-1,true);
  if(pos == -1)
    _category = path.copy();
  else
    _category = path.mid(pos+1,path.length());
} // getCategoryFromPathName

void 
CDDialog::save()
{
  if(!checkit())
    return;

  QString savecat;

  InexactDialog *dialog;
  dialog = new InexactDialog(0,"Dialog",true);

  if( category.length() < 1 )
	{
	  dialog->insertList(catlist);
	  dialog->setErrorString(i18n("Please select a category or press Cancel"));
	  dialog->setTitle(i18n("Under which category would you like to store this disc's information?"));
	  
	  if(dialog->exec() != QDialog::Accepted)
		{
		  delete dialog;
		  return;
		}
	  
	  dialog->getSelection(savecat);
	} else {
	  savecat = category.copy();
	}

  QString mag;
  mag.sprintf("%s%s/%08lx",cddbbasedir.utf8().data(),savecat.utf8().data(),cdinfo.magicID);

  save_cddb_entry(mag,false);
  load_cddb();
  delete dialog;
  emit dialog_done();
} // save

void 
CDDialog::save_cddb_entry(QString& path,bool upload)
{
  QString magic;
  magic.sprintf("%08lx",cdinfo.magicID);
  bool have_magic_already = false;

  kdDebug() << "::save_cddb_entry(): path: " << path << " upload = " << upload << "\n" << endl;

  if( !upload )
    {
      for ( QStringList::Iterator it = discidlist.begin();
            it != discidlist.end();
            ++it )
	{
	  if (magic == *it)
	    {
	      have_magic_already = true;
	      break;
	    }
	}
      
      if(!have_magic_already)
	discidlist.insert(discidlist.begin(), magic);
    } else { // uploading 
      discidlist.clear();
      discidlist.insert(discidlist.begin(), magic);
    }

  QFile file(path);


  if( !file.open( IO_WriteOnly  )) 
    {
      QString str = i18n("Unable to write to file:\n%1\nPlease check "
			 "your permissions and make your category directories exist.")
	.arg(path);

      KMessageBox::error(this, str);
      return;
    }

  QString tmp;
  QTextStream t(&file);


  if(upload && !smtpConfigData->enabled)
    {
      QString subject;
      subject.sprintf("cddb %s %08lx", submitcat.utf8().data(), cdinfo.magicID);
      
      t << "To: " + submitaddress + "\n";
      tmp = QString("Subject: %1\n").arg(subject);
      t << tmp;
    }

  t << "# xmcd CD database file\n";
  
  QString datestr;
  datestr = QDateTime::currentDateTime().toString();
  tmp = QString("# Generated: %1 by KSCD\n").arg(datestr);
  t << tmp;

  // Waste some disk space
  if(!upload) {
    t << "# Copyright (C) 1997-1999 Bernd Johannes Wuebben.\n";
    t << "# Copyright (C) 1999-2001 Dirk Foersterling.\n";
  }
  

  t << "# \n";
  t << "# Track frame offsets:\n";

  for(int i = 0 ; i < cdinfo.ntracks;i ++)
    {
      tmp = QString("#       %1\n").arg(cdinfo.cddbtoc[i].absframe);
      t << tmp;
    }

  t << "#\n";
  tmp = QString("# Disc length: %1 seconds\n").arg(cdinfo.length);
  t << tmp;
  t << "#\n";
  // FIXME: Only increase if the entry was received from freedb
  if(upload)
    tmp = QString("# Revision: %1\n").arg(++revision);
  else
    tmp = QString("# Revision: %1\n").arg(revision);

  t << tmp;
  t << "# Submitted via: Kscd "KSCDVERSION"\n";
  t << "#\n";


  tmp = "DISCID=";
  int counter = 0;

  int num = 0;
  for ( QStringList::Iterator it = discidlist.begin();
        it != discidlist.end();
        ++it, ++num )
    {
      
      tmp += *it;
      
      if( num < (int) discidlist.count() - 1)
	{
	  if( counter++ == 3 )
	    {
	      tmp += "\nDISCID=";
	      counter = 0;
	    } else {
	      tmp += ",";
	    }
	}
    }

  tmp += "\n";
  t << tmp;

  QStringList returnlist;
  QString tmp2;

  tmp2 = *track_list.at(0);
  cddb_encode(tmp2,returnlist);  

  if(returnlist.count() == 0)
    {
      // sanity provision
      tmp = QString("DTITLE=%1\n").arg("");
      t << tmp;
    } else {
      for ( QStringList::Iterator it = returnlist.begin();
            it != returnlist.end(); 
            ++it )
	{
	  tmp = QString("DTITLE=%1\n").arg(*it);
	  t << tmp;
	}
    }

  num = 1;
  for ( QStringList::Iterator it = track_list.begin();
        it != track_list.end();
        ++it)
    {
      tmp2 = *it;
      cddb_encode(tmp2,returnlist);  
     
      // no perfect solution, but it's working so far.
      if( it != track_list.begin() ) {
        if(returnlist.isEmpty())
          {
            // sanity provision
            tmp = QString("TTITLE%1=%2\n").arg(num-1).arg("");
            t << tmp;
          } else {
            tmp = QString("TTITLE%1=%2\n").arg(num-1).arg(*it);
            t << tmp;
          }
        num++;
      }
    }
  
  tmp2 = ext_list.first();
  cddb_encode(tmp2,returnlist);  

  if(returnlist.isEmpty())
    {
      // sanity provision
      tmp = tmp.sprintf("EXTD=%s\n","");
      t << tmp;
    } else {
      for ( QStringList::Iterator it = returnlist.begin();
            it != returnlist.end();
            ++it )
	{
	  tmp = QString("EXTD=%1\n").arg(*it);
	  t << tmp;
	}
    }

  int i = 1;
  for ( QStringList::Iterator it = ext_list.at(1);
        it != ext_list.end(); 
        ++it, i++ )
    {
      tmp2 = *it;
      cddb_encode(tmp2,returnlist);  
      
      if(returnlist.count() == 0)
	{
	  // sanity provision
	  tmp = tmp.sprintf("EXTT%d=%s\n",i-1,"");
	  t << tmp;
	} else {
	  for(int j = 0; j < (int) returnlist.count();j++)
	    {
	      tmp = tmp.sprintf("EXTT%d=%s\n",i-1,(*returnlist.at(j)).utf8().data());
	      t << tmp;
	    }
	}
    }
  
  if(!upload)
    {
      cddb_encode(playorder,returnlist);  
      
      for(int i = 0; i < (int) returnlist.count();i++)
	{
	  tmp = tmp.sprintf("PLAYORDER=%s\n", (*returnlist.at(i)).utf8().data());
	  t << tmp;
	}
    } else {
      tmp = tmp.sprintf("PLAYORDER=\n");
      t << tmp;
    }

  t << "\n";

  file.close();
  chmod(QFile::encodeName(file.name()), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
  return;
} // save_cddb_entry


bool 
CDDialog::checkit()
{
  QString title = titleedit->text();
  title = title.stripWhiteSpace();
  if(title.isEmpty())
    {
      KMessageBox::sorry(this,
			 i18n("The Disc Artist / Title field is not filled in.\n"\
			      "Please correct the entry and try again."),
			 i18n("Invalid Database Entry"));
      return false;
    }
  
  int pos;
  pos = title.find('/',0,true);
  if(pos == -1)
    {
      KMessageBox::sorry(this,
			 i18n("The Disc Artist / Title field is not filled in correctly.\n"\
			      "Please separate the artist from the title of the CD with \n"\
			      "a forward slash, such as in: Peter Gabriel / Greatest Hits\n"),
			 i18n("Invalid Database Entry"));
      return false;
    }

  

  if(track_list.count() < 2)
    {
      KMessageBox::sorry(this,
			 i18n("Not all track titles can be empty.\n"\
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
			 i18n("Not all track titles can be empty.\n"\
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

  bool ret;
  ret = cddb_playlist_decode(strlist, str);
  
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
			 i18n("Invalid Playlist\n"));
      return false;
    }
  
  cddb_playlist_encode(strlist,playorder);
  return true;
} // checkit


void 
CDDialog::load_cddb()
{
  emit cddb_query_signal(true);
} // load

// simplyfied quoted printable mime encoding that should be good enough 
// for our purposed. The encoding differs from the 'real' encoding in
// that we don't need to worry about trailing \n, \t or lines exeeding the
// spec length.

void 
mimetranslate(QString& s)
{
  QString q;
  QString hex;
  
  s = s.stripWhiteSpace(); // there is no harm in doing this and it
  // will simplify the quoted printable mime encoding.
  
  for(uint i = 0 ; i < s.length(); i++)
    {
      if (((s[i] >= 32) && (s[i] <= 60)) || 
	  ((s[i] >= 62) && (s[i] <= 126))) 
	{
	  
	  q += s.at(i);
	} else {
	  
	  hex = hex.sprintf("=%02X", (unsigned char)s[i].latin1());
	  q += hex; 
	}
    }

  //  printf("%s\n",q.data());
  s = q.copy();

} // mimetranslate

#undef Inherited
