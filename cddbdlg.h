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

class CDInfoDialogBase;

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

  private slots:
    void save();
    void upload();
    void submitFinished(CDDB::Result);

  signals:
    void cddbQuery();
    void play(int i);

  private:
    bool updateFromDialog();
    QString framesTime(unsigned frames);

    CDInfoDialogBase *m_dlgBase;
    KCDDB::CDInfo cddbInfo;
    QStringList playlist;
    unsigned ntracks;   /* Number of tracks on the disc */
    QValueList<unsigned> trackStartFrames;
    KCDDB::Client *cddbClient;
};
#endif // CDDBDLG_H
