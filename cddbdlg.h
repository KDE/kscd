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

class CDDBDlg : public KDialogBase
{
  Q_OBJECT

  public:
    CDDBDlg(QWidget* parent, const char* name = 0);
    ~CDDBDlg();

    void setData(
      struct wm_cdinfo *cd,
      const KCDDB::CDInfo &_cddbInfo,
      const QStringList  &_playlist
    );

    bool checkit();
    void setCdInfo(KCDDB::CDInfo &info, const QString& category);

  public slots:
    void save();
    void extITB( int trackNum );
    void extIB();
    void upload();
    void submitFinished(CDDB::Result);

  signals:
    void cddbQuery();
    void play(int i);

  private:
    void updateTrackList();

    CDDBDlgBase *m_dlgBase;
    QStringList catlist;
    KCDDB::CDInfo cddbInfo;
    QStringList playlist;
    unsigned ntracks;   /* Number of tracks on the disc */
    int length;         /* Total running time in seconds */
    struct mytoc *cddbtoc;
    KCDDB::Client *cddbClient;
};
#endif // CDDBDLG_H
