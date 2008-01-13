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

#include "cddbdlg.h"

#include <klocale.h>
#include <kmessagebox.h>

CDDBDlg::CDDBDlg(QWidget* parent):CDInfoDialog(parent)
{
	setCaption( i18n("CD Editor") );
	setModal( true );
	
	setButtons( KDialog::Ok|KDialog::Cancel|KDialog::User1|KDialog::User2|KDialog::User3 );
	setDefaultButton(KDialog::Ok);
	setButtonText( User1, i18n("Upload") );
	setButtonText( User2, i18n("Download") );
	setButtonText( User3, i18n("Test") );

	connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
	connect( this, SIGNAL( user1Clicked() ), SLOT( upload() ) );
	connect( this, SIGNAL( user2Clicked() ), parent, SLOT(lookupCDDB()) );
	connect( this, SIGNAL( user3Clicked() ), SLOT( fillDlg() ) );

// Don't understand the meaning of this line
//	connect( this, SIGNAL( play( int ) ), SIGNAL( play( int ) ) );
	
	m_cddbClient = new KCDDB::Client();
	m_cddbClient->setBlockingMode(false);
	connect( m_cddbClient, SIGNAL( finished(KCDDB::Result) ), SLOT( submitFinished(KCDDB::Result) ) );
}

CDDBDlg::~CDDBDlg()
{
	delete m_cddbClient;
}

void CDDBDlg::setData(const KCDDB::CDInfo &_cddbInfo,const KCDDB::TrackOffsetList &_trackStartFrames)
{
	// Let's make a deep copy of the cd struct info so that the data won't
	// change the cd changes while we are playing with the dialog.
	m_cddbInfo = _cddbInfo;
	m_trackStartFrames = _trackStartFrames;
	
	// Write the complete record to the dialog.
	setInfo(m_cddbInfo, m_trackStartFrames);
} // setData

KCDDB::TrackOffsetList CDDBDlg::getTrackStartFrames()
{
	return m_trackStartFrames ;
}

void CDDBDlg::submitFinished(KCDDB::Result r)
{
	if (r == KCDDB::Success)
	{
		KMessageBox::information(this, i18n("Record submitted successfully."), i18n("Record Submission"));
	}
	else
	{
		QString str = i18n("Error sending record.\n\n%1", KCDDB::resultToString(r));
		KMessageBox::error(this, str, i18n("Record Submission"));
	}
} // submitFinished()

void CDDBDlg::fillDlg()
{
	setData(m_cddbInfo, m_trackStartFrames) ;
}

void CDDBDlg::upload()
{
	if (!validInfo())
		return;
	
	// Create a copy with a bumped revision number.
	KCDDB::CDInfo copyInfo = m_cddbInfo;
	copyInfo.set("revision",copyInfo.get("revision").toInt()+1);
	m_cddbClient->submit(copyInfo, m_trackStartFrames);
} // upload

void CDDBDlg::save()
{
//	m_cddbClient->store(m_cddbInfo, m_trackStartFrames);

	emit newCDInfoStored(m_cddbInfo);
} // save

bool CDDBDlg::validInfo()
{
	KCDDB::CDInfo copy = info();
	
	if (copy.get(KCDDB::Artist).toString().isEmpty())
	{
		KMessageBox::sorry(this, i18n("The artist name of the disc has to be entered.\nPlease correct the entry and try again."), i18n("Invalid Database Entry"));
		return false;
	}
	
	if (copy.get(KCDDB::Title).toString().isEmpty())
	{
		KMessageBox::sorry(this, i18n("The title of the disc has to be entered.\nPlease correct the entry and try again."), i18n("Invalid Database Entry"));
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
		KMessageBox::sorry(this, i18n("At least one track title must be entered.\nPlease correct the entry and try again."), i18n("Invalid Database Entry"));
		return false;
	}
	
	return true;
}

#include "cddbdlg.moc"
