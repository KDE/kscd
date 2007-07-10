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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <qnamespace.h>
#include <QDateTime>
#include <qtextstream.h>
#include <QFile>
#include <QDir>
#include <qfileinfo.h>
#include <k3listview.h>
#include <klineedit.h>
#include <knuminput.h>

#include <kglobal.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kmessagebox.h>

#include <stdio.h>
#include <math.h>

#include "version.h"
#include "kscd.h"
#include "cddbdlg.h"

CDDBDlg::CDDBDlg( QWidget* parent )
    : CDInfoDialog( parent)
{
  setCaption( i18n( "CD Editor" ) );
  setModal( true );

  setButtons( KDialog::Ok|KDialog::Cancel|KDialog::User1|KDialog::User2 );
  setDefaultButton( KDialog::Ok );
  setButtonText( User1, i18n( "Upload" ) );
  setButtonText( User2, i18n( "Fetch Info" ) );

  connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( upload() ) );
  connect( this, SIGNAL( user2Clicked() ), SIGNAL( cddbQuery() ) );
  connect( this, SIGNAL( play( int ) ), SIGNAL( play( int ) ) );

  cddbClient = new KCDDB::Client();
  cddbClient->setBlockingMode(false);
  connect (cddbClient, SIGNAL(finished(KCDDB::Result)),
                       SLOT(submitFinished(KCDDB::Result)));
}


CDDBDlg::~CDDBDlg()
{
  delete cddbClient;
}

void CDDBDlg::setData(
  const KCDDB::CDInfo &_cddbInfo,
  const KCDDB::TrackOffsetList &_trackStartFrames)
{
    // Let's make a deep copy of the cd struct info so that the data won't
    // change the cd changes while we are playing with the dialog.
    cddbInfo = _cddbInfo;
    trackStartFrames = _trackStartFrames;

    // Write the complete record to the dialog.
    setInfo(cddbInfo, trackStartFrames);
} // setData

void CDDBDlg::submitFinished(KCDDB::Result r)
{
  if (r == KCDDB::Success)
  {
    KMessageBox::information(this, i18n("Record submitted successfully."),
         i18n("Record Submission"));
  }
  else
  {
    QString str = i18n("Error sending record.\n\n%1",
       KCDDB::resultToString(r));
    KMessageBox::error(this, str, i18n("Record Submission"));
  }
} // submitFinished()

void CDDBDlg::upload()
{
    if (!validInfo())
        return;

     // Create a copy with a bumped revision number.
    KCDDB::CDInfo copyInfo = cddbInfo;
    copyInfo.set("revision",copyInfo.get("revision").toInt()+1);
    cddbClient->submit(copyInfo, trackStartFrames);
} // upload

void CDDBDlg::save()
{
    cddbClient->store(cddbInfo);

    emit newCDInfoStored(cddbInfo);
} // save

bool CDDBDlg::validInfo()
{
  KCDDB::CDInfo copy = info();

  if (copy.get(KCDDB::Artist).toString().isEmpty())
  {
    KMessageBox::sorry(this,
        i18n("The artist name of the disc has to be entered.\n"
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  if (copy.get(KCDDB::Title).toString().isEmpty())
  {
    KMessageBox::sorry(this,
        i18n("The title of the disc has to be entered.\n"
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  bool have_nonempty_title = false;
  for (int i = 0; i < copy.numberOfTracks(); i++)
  {
      if (!copy.track(i).get(KCDDB::Title).toString().isEmpty())
      {
          have_nonempty_title = true;
          break;
      }
  }

  if (!have_nonempty_title)
  {
    KMessageBox::sorry(this,
        i18n("At least one track title must be entered.\n"\
             "Please correct the entry and try again."),
        i18n("Invalid Database Entry"));
    return false;
  }

  return true;
}

#include "cddbdlg.moc"
