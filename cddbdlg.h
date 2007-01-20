/*
   Copyright (c) 2006 Alexander Kern <alex.kern@gmx.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CDDBDLG_H
#define CDDBDLG_H

#include <kdialog.h>

#include "libkcddb/cdinfo.h"
#include "libkcddb/cddb.h"
#include "libkcddb/client.h"
#include "libkcddb/cdinfodialog.h"

class CDDBDlg : public KCDDB::CDInfoDialog
{
  Q_OBJECT

  public:
    explicit CDDBDlg(QWidget* parent, const char* name = 0);
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

    KCDDB::CDInfo cddbInfo;
    KCDDB::TrackOffsetList trackStartFrames;
    QStringList playlist;
    KCDDB::Client *cddbClient;
};
#endif // CDDBDLG_H
