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

/**********************************************************************

	--- Dlgedit generated file ---

	File: CDDialog.cpp
	Last generated: Sun Dec 28 19:48:13 1997

 *********************************************************************/

#include "CDDialog.h"
#include "CDDialog.moc"
#include "CDDialogData.moc"
#include "inexact.h"
#include "version.h"
#include "smtp.h"
#include "smtpconfig.h"
extern "C" {
#include "libwm/include/workman.h"
}

#include <unistd.h>
#include <klocale.h>

#include <qkeycode.h>
#include <qregexp.h> 
#include <qdatetime.h> 
#include <qtextstream.h> 
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h> 

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <math.h>

#include <kapp.h>
#include <kmessagebox.h>

#define Inherited CDDialogData

QTime framestoTime(int frames);
void  mimetranslate(QString& s);
extern void cddb_decode(QString& str);
extern void cddb_encode(QString& str, QStrList &returnlist);
extern void cddb_playlist_encode(QStrList& list,QString& playstr);
extern bool cddb_playlist_decode(QStrList& list, QString& str);

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
	connect(load_button, SIGNAL(clicked())       ,this,SLOT(load()));
	connect(ext_info_title_button, SIGNAL(clicked()) ,this,SLOT(extITB()));
	connect(ext_info_button, SIGNAL(clicked())       ,this,SLOT(extIB()));
	connect(titleedit,   SIGNAL(textChanged(const QString &)),
		             this,SLOT(titlechanged(const QString &)));
	ext_info_button->setEnabled(false);

        setFixedSize(width(),height());

	
} // CDDialog


CDDialog::~CDDialog()
{
    if(cdinfo.cddbtoc)
      delete [] cdinfo.cddbtoc;
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
		  QStrList& tracktitlelist,
		  QStrList& extlist,
		  QStrList& discidl,
		  QString& _xmcd_data,
		  QString& cat,
		  int& rev,
		  QStrList& _playlist,
		  QStrList& _pathlist,
		  QString& _mailcmd,
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
    mailcmd	= _mailcmd.copy();
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
	track_list.remove(track_list.count() - 1);
      }
    
    while((int)ext_list.count() > cdinfo.ntracks + 1)
      {
	ext_list.remove(ext_list.count() - 1);
      }
    

    titleedit->setText(track_list.at(0));

    QString idstr;
    idstr.sprintf("%08lx",cddb_discid());
    idstr = category + (QString("\n") + idstr);

    if(cdinfo.ntracks > 0)
      disc_id_label->setText(idstr);
    else
      disc_id_label->setText("");

    QTime   dl;
    dl 	=  dl.addSecs(cdinfo.length);

    QString temp2;
    if(dl.hour() > 0)
      temp2.sprintf(i18n("Total Time:\n%02d:%02d:%02d").ascii(),dl.hour(),dl.minute(),dl.second());
    else
      temp2.sprintf(i18n("Total Time:\n %02d:%02d").ascii(),dl.minute(),dl.second());
    total_time_label->setText(temp2);

    QString 	fmt;
    QTime 	dml;

    listbox->setAutoUpdate(false);
    listbox->clear();

    for(int i = 1; i <= cdinfo.ntracks; i++)
      {
	dml = framestoTime(cdinfo.cddbtoc[i].absframe - cdinfo.cddbtoc[i-1].absframe);
	
	if((ntr >=  i) && (ntr > 0))
	  {
	    fmt.sprintf("%02d   %02d:%02d   %s",i,
			dml.minute(),dml.second(),track_list.at(i));
	  } else {
	    fmt.sprintf("%02d   %02d:%02d",i,dml.minute(),dml.second());
	  }
	listbox->insertItem(fmt,-1);
      }
    
    listbox->setAutoUpdate(true);
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
  dialog->setTitle(i18n("Use this Editor to annotate this track"));

  dialog->insertText(ext_list.at(item + 1));
  
  if(dialog->exec() != QDialog::Accepted)
    {
      delete dialog;
      return;
    }
  
  QString text;
  dialog->getSelection(text);

//  ext_list.insert( item, text );
  ext_list.remove(item + 1);
  ext_list.insert(item + 1, text.ascii());

  delete dialog;
} // extIB

void 
CDDialog::extITB()
{
  InexactDialog *dialog;
  dialog = new InexactDialog(0,"dialog",false);
  dialog->insertText(ext_list.at(0));
  dialog->setTitle(i18n("Use this Editor to annotate the title"));
  
  if(dialog->exec() != QDialog::Accepted)
    {
      delete dialog;
      return;
    }
  
  QString text;
  dialog->getSelection(text);

  ext_list.insert( 0 , text.ascii() );
  ext_list.remove( 1 );

  delete dialog;
} // extITB

void CDDialog::titleselected(int i)
{
  ext_info_button->setEnabled(true);
  if(i + 1 < (int)track_list.count())
    trackedit->setText(track_list.at(i+1));
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

  fmt.sprintf("%02d   %02d:%02d   %s",i+1,dml.minute(),dml.second(),trackedit->text().ascii());

  track_list.insert(i+1,trackedit->text().ascii());
  track_list.remove(i+2);

  listbox->setAutoUpdate(false);

  listbox->insertItem(fmt, i);
  listbox->removeItem(i+1);
  listbox->setAutoUpdate(true);
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
  track_list.remove((uint)0);
  track_list.insert(0, t.ascii());
} // titlechanged

QString submitcat;

void 
CDDialog::upload()
{
  if(!checkit())
    return;

  InexactDialog *dialog;

  dialog = new InexactDialog(0,"Dialog",true);

  QStrList catlist;

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

  dialog->insertList(catlist);
  dialog->setErrorString(i18n("Please select a Category or press Cancel"));
  dialog->setTitle(i18n("To which category does the CD belong?"));
  if(dialog->exec() != QDialog::Accepted)
    {
      delete dialog;
      return;
    }

  dialog->getSelection(submitcat);
  delete dialog;

  QString tempfile;
  tempfile = tmpnam(0L);

  save_cddb_entry(tempfile,true);

  kapp->processEvents();
  kapp->flushX();


  if(smtpConfigData->enabled)
    {
      debug("Submitting cddb entry via SMTP...\n");
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
      smtpMailer->setRecipientAddress(submitaddress);
      
      subject.sprintf("cddb %s %08lx", submitcat.data(), cdinfo.magicID);
      smtpMailer->setMessageSubject(subject);
      smtpMailer->setMessageBody(s);

      smtpMailer->sendMessage();
      
      return;
    }

      
  QString cmd;

  cmd = "sendmail -tU";

  debug("Submitting cddb entry: %s\n",cmd.ascii());
  
  FILE* mailpipe;
  mailpipe = popen(cmd.data(),"w");

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

  unlink(tempfile.data());
  debug("DONE SENDING\n");
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

  QString path;

  InexactDialog *dialog;

  dialog = new InexactDialog(0,"Dialog",true);

  // Let's get rid of some ugly double slashes such as in 
  // /usr/local/kde/share/apps/kscd/cddb//rock 
  
  for(int i = 0; i < (int)pathlist.count();i++)
    {
      QString temp = pathlist.at(i);
      temp = temp.replace( QRegExp("//"), "/" );
      pathlist.insert(i,temp.ascii());
      pathlist.remove(i+1);
    }

  dialog->insertList(pathlist);
  dialog->setErrorString(i18n("Please select a Category or press Cancel"));
  dialog->setTitle(i18n("Under which category would you like to store this CDDB entry?"));

  if(dialog->exec() != QDialog::Accepted)
    {
      delete dialog;
      return;
    }

  dialog->getSelection(path);
  QString mag;
  mag.sprintf("%s/%08lx",path.data(),cdinfo.magicID);

  save_cddb_entry(mag,false);
  load();
  delete dialog;
} // save

void 
CDDialog::save_cddb_entry(QString& path,bool upload)
{
  QString magic;
  magic.sprintf("%08lx",cdinfo.magicID);
  bool have_magic_already = false;

  debug("::save_cddb_entry(): path: %s upload = %d\n", path.data(), upload);
  // Steve and Ti contacted me and said they have changed the cddb upload specs
  // Now, an uploaded entry must only contain one DISCID namely the one corresponding
  // to the CD the user actually owns.
  if( !upload )
    {
      for(int i = 0 ; i < (int)discidlist.count();i ++)
	{
	  if(magic == (QString)discidlist.at(i))
	    {
	      have_magic_already = true;
	      break;
	    }
	}
      
      if(!have_magic_already)
	discidlist.insert(0,magic.data());
    } else { // uploading 
      discidlist.clear();
      discidlist.insert(0,magic.data());
    }

  QFile file(path.data());


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
      subject.sprintf("cddb %s %08lx", submitcat.data(), cdinfo.magicID);
      
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
    t << "# Copyright (C) 2000 Dirk Foersterling.\n";
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
  if(upload)
    tmp = QString("# Revision: %1\n").arg(++revision);
  else
    tmp = QString("# Revision: %1\n").arg(revision);
  t << tmp;
  t << "# Submitted via: Kscd "KSCDVERSION"\n";
  t << "#\n";


  tmp = "DISCID=";
  int counter = 0;

  for(int i = 0 ; i < (int)discidlist.count();i ++)
    {
      
      tmp += discidlist.at(i);
      
      if( i < (int) discidlist.count() - 1)
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

  QStrList returnlist;
  QString tmp2;

  tmp2 = track_list.at(0);
  cddb_encode(tmp2,returnlist);  

  if(returnlist.count() == 0)
    {
      // sanity provision
      tmp = QString("DTITLE=%1\n").arg("");
      t << tmp;
    } else {
      for(int i = 0; i < (int) returnlist.count();i++)
	{
	  tmp = QString("DTITLE=%1\n").arg(returnlist.at(i));
	  t << tmp;
	}
    }

  for(int i = 1 ; i < (int)track_list.count();i ++)
    {
      tmp2 = track_list.at(i);
      cddb_encode(tmp2,returnlist);  
      
      if(returnlist.count() == 0)
	{
	  // sanity provision
	  tmp = QString("TTITLE%1=%2\n").arg(i-1).arg("");
	  t << tmp;
	} else {
	  for(int j = 0; j < (int) returnlist.count();j++)
	    {
	      tmp = QString("TTITLE%1=%2\n").arg(i-1).arg(returnlist.at(j));
	      t << tmp;
	    }
	}
    }
  
  tmp2 = ext_list.at(0);
  cddb_encode(tmp2,returnlist);  

  if(returnlist.count() == 0)
    {
      // sanity provision
      tmp = tmp.sprintf("EXTD=%s\n","");
      t << tmp;
    } else {
      for(int i = 0; i < (int) returnlist.count();i++)
	{
	  tmp = QString("EXTD=%1\n").arg(returnlist.at(i));
	  t << tmp;
	}
    }

  for(int i = 1 ; i < (int)ext_list.count();i ++)
    {
      tmp2 = ext_list.at(i);
      cddb_encode(tmp2,returnlist);  
      
      if(returnlist.count() == 0)
	{
	  // sanity provision
	  tmp = tmp.sprintf("EXTT%d=%s\n",i-1,"");
	  t << tmp;
	} else {
	  for(int j = 0; j < (int) returnlist.count();j++)
	    {
	      tmp = tmp.sprintf("EXTT%d=%s\n",i-1,returnlist.at(j));
	      t << tmp;
	    }
	}
    }
  
  if(!upload)
    {
      cddb_encode(playorder,returnlist);  
      
      for(int i = 0; i < (int) returnlist.count();i++)
	{
	  tmp = tmp.sprintf("PLAYORDER=%s\n",returnlist.at(i));
	  t << tmp;
	}
    } else {
      tmp = tmp.sprintf("PLAYORDER=\n");
      t << tmp;
    }

  t << "\n";

  file.close();
  chmod(file.name().ascii(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
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
  pos = title.find("/",0,true);
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
  for(int i = 1; i < (int)track_list.count(); i++)
    {
      title = track_list.at(i);
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
  QStrList strlist;
  str = progseq_edit->text();

  bool ret;
  ret = cddb_playlist_decode(strlist, str);
  
  QString teststr;
  bool ok;
  int  num;

  for(uint i = 0; i < strlist.count();i++)
    {
      teststr = strlist.at(i);
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
CDDialog::load()
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
      if (((s.data()[i] >= 32) && (s.data()[i] <= 60)) || 
	  ((s.data()[i] >= 62) && (s.data()[i] <= 126))) 
	{
	  
	  q += s.at(i);
	} else {
	  
	  hex = hex.sprintf("=%02X", (unsigned char)s.data()[i]);
	  q += hex; 
	}
    }

  //  printf("%s\n",q.data());
  s = q.copy();

} // mimetranslate

#undef Inherited
