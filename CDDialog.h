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
#include "libkcddb/cdinfo.h"
#include "libkcddb/cddb.h"
#include "libkcddb/client.h"

using KCDDB::CDDB;

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
  QString magicID;	/*cddb magic disk id BERND*/
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
			   const QStringList& tracktitlelist,
			   const QStringList& extlist,
			   const QString& xmcddata,
			   const QString& cat,
			   const QString& _genre,
			   int revision,
			   int _year,
			   const QStringList& playlist,
			   const QStringList& pathlist
			   );
  
  bool checkit();
  void save_cddb_entry(QString& path,bool upload);
  void setCdInfo(KCDDB::CDInfo &info, const QString& category);
  
 public slots:
  void titleselected(QListViewItem *);
  void titlechanged();
  void trackchanged(const QString &);
  void save();
  void extITB();
  void extIB();
  void load_cddb();
  void upload();
  void play(QListViewItem *);
  void nextTrack();
  void submitFinished(CDDB::Result);

signals:

  void cddb_query_signal(bool);
  void play_signal(int i);
  
private:
  QStringList 	ext_list;
  QStringList 	track_list;
  QStringList pathlist;
  QStringList catlist;
  QStringList	playlist;
  QString  	xmcd_data;
  int      	revision;
  int           year;
  QString 	category;
  QString     playorder;
  QString     genre;
  struct dialogcdinfo cdinfo;
  bool            messageNotSent;
  KCDDB::Client *cddbClient;
};
#endif // CDDialog_included
