#ifndef CDDBDLG_H
#define CDDBDLG_H

#include <kdialogbase.h>

#include "libkcddb/cdinfo.h"
#include "libkcddb/cddb.h"
#include "libkcddb/client.h"

class CDInfoDialogBase;

class CDDBDlg : public KDialogBase
{
  Q_OBJECT

  public:
    CDDBDlg(QWidget* parent, const char* name = 0);
    ~CDDBDlg();

    void setData(
      const KCDDB::CDInfo &_cddbInfo,
      const KCDDB::TrackOffsetList &_trackStartFrames,
      const QStringList  &_playlist);

  private slots:
    void save();
    void upload();
    void submitFinished(CDDB::Result);

  signals:
    void cddbQuery();
    void newCDInfoStored(KCDDB::CDInfo);
    void play(int i);

  private:
    bool validInfo();
    void updateFromDialog();
    QString framesTime(unsigned frames);

    CDInfoDialogBase *m_dlgBase;
    KCDDB::CDInfo cddbInfo;
    KCDDB::TrackOffsetList trackStartFrames;
    QStringList playlist;
    KCDDB::Client *cddbClient;
};
#endif // CDDBDLG_H
