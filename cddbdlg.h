#ifndef CDDBDLG_H
#define CDDBDLG_H

#include <kdialogbase.h>

#include "libkcddb/cdinfo.h"
#include "libkcddb/cddb.h"
#include "libkcddb/client.h"

using KCDDB::CDDB;

extern "C" {
#include "libwm/include/wm_cdinfo.h"
}

class CDDBDlgBase;

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

class CDDBDlg : public KDialogBase
{
  Q_OBJECT

  public:
    CDDBDlg(QWidget* parent, const char* name = 0);
    ~CDDBDlg();

    void setData(
      struct wm_cdinfo *cd,
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
    void save();
    void extITB( int trackNum );
    void extIB();
    void load_cddb();
    void upload();
    void submitFinished(CDDB::Result);

  signals:
    void cddbQuery(bool);
    void play(int i);

  private:
    void updateTrackList();

    CDDBDlgBase *m_dlgBase;
    QStringList ext_list;
    QStringList track_list;
    QStringList pathlist;
    QStringList catlist;
    QStringList playlist;
    QString xmcd_data;
    int revision;
    int year;
    QString category;
    QString playorder;
    QString genre;
    struct dialogcdinfo cdinfo;
    bool messageNotSent;
    KCDDB::Client *cddbClient;
};
#endif // CDDBDLG_H
