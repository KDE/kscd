/**********************************************************************

	--- Dlgedit generated file ---

	File: CDDialog.h
	Last generated: Sun Dec 28 19:48:13 1997

 *********************************************************************/

#ifndef CDDialog_included
#define CDDialog_included

#include <qstrlist.h>
#include <qdatetime.h>

#include "CDDialogData.h"
#include "smtpconfig.h"
#include "libkcddb/cdinfo.h"

extern "C" {
#include "libwm/include/wm_cdinfo.h"
}

struct mytoc 
{
  int	min;
  int	sec;
  int	frame;
  int     absframe;
};


struct dialogcdinfo 
{
  unsigned long magicID;	/*cddb magic disk id BERND*/
  int	ntracks;	/* Number of tracks on the disc */
  int	length;		/* Total running time in seconds */
  struct mytoc *cddbtoc;
};

class CDDialog : public CDDialogData
{
  Q_OBJECT
	
	public:
  
  CDDialog(QWidget* parent = 0, const char* name = 0);
  
  virtual ~CDDialog();
  
  
  void setData(
#ifdef OLD_WM_CODE
			   struct cdinfo_wm *cd,
#else
			   struct wm_cdinfo *cd,
#endif
			   QStringList& tracktitlelist,
			   QStringList& extlist,
			   QStringList& discidlist,
			   QString& xmcddata,
			   QString& cat,
			   int& revision,
			   QStringList& playlist,
			   QStringList& pathlist,
			   QString& cddbbasedir,
			   QString& _submitaddress,
			   SMTPConfigData *_smtpConfigData
			   );
  
  bool checkit();
  void save_cddb_entry(QString& path,bool upload);
  void getCategoryFromPathName(char* pathname, QString& _category);
  void setCdInfo(KCDDB::CDInfo &info, const QString& category);
  
 protected:
  void closeEvent(QCloseEvent* e);
  void keyPressEvent(QKeyEvent* e);
 public slots:
  void titleselected(QListViewItem *);
  void titlechanged();
  void trackchanged(const QString &);
  void save();
  void extITB();
  void extIB();
  void load_cddb();
  void upload();
  void cancel();
  void play(QListViewItem *);
  void nextTrack();

signals:

  void cddb_query_signal(bool);
  void dialog_done();
  void play_signal(int i);
  
private:
  SMTPConfigData *smtpConfigData;
  QStringList 	ext_list;
  QStringList 	track_list;
  QStringList pathlist;
  QStringList catlist;
  QString cddbbasedir;
  QStringList	playlist;
  QString  	xmcd_data;
  QStringList 	discidlist;
  int      	revision;
  QString     submitaddress;
  QString 	category;
  QString     playorder;
  struct dialogcdinfo cdinfo;
  bool            messageNotSent;
};
#endif // CDDialog_included
