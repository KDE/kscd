/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
 * Copyright (c) 2003-2006 Richard Lärkäng <nouseforaname@home.se>
 *
 * --------------
 * ISI KsCD Team :
 * --------------
 * Stanislas KRZYWDA <stanislas.krzywda@gmail.com>
 * Sovanramy Var <mastasushi@gmail.com>
 * Bouchikhi Mohamed-Amine <bouchikhi.amine@gmail.com>
 * Gastellu Sylvain<sylvain.gastellu@gmail.com>
 * -----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "cddbmanager.h"

class KSCD;

CDDBManager::CDDBManager()
{
	// CDDB initialization
	m_cddbInfo.clear(); // The first freedb revision is "0"
	m_cddbClient = new KCDDB::Client();
//	m_cddialog = 0L;
	
	connect(m_cddbClient, SIGNAL(finished(KCDDB::Result)), this, SLOT(lookupCDDBDone(KCDDB::Result)));

// TODO inactivate CDDB options if no disc
// TODO erase that !
	for (int i=0; i<20; i++)
	{
		kDebug() << i << "!!!!!!!!!!!Changed" ;
		m_cddbInfo.track(i).set(KCDDB::Title, QString("unknown") ) ;
	}
}

CDDBManager::~CDDBManager()
{
//	delete m_cddialog;
	delete m_cddbClient;
}

// Fonction to be called whenever a new Disc is inserted
void CDDBManager::setup(int nbTrack, KCDDB::TrackOffsetList signature)
{
	m_cdSignature = signature;
	
	for (int i=0; i<nbTrack; i++)
	{
		m_cddbInfo.track(i).set(KCDDB::Title, QString("unknown") ) ;
	}
}

void CDDBManager::CDDialogSelected()
{	
	kDebug() << "CDDialogSelected" ;
/*
	if (!m_cddialog)
	{
		kDebug() << "Create a CDDB Dialog" ;
		// CDDB Dialog initialization
		m_cddialog = new CDDBDlg(this);
		connect(m_cddialog,SIGNAL(newCDInfoStored(KCDDB::CDInfo)), SLOT(setCDInfo(KCDDB::CDInfo)));
		connect(m_cddialog,SIGNAL(finished()),SLOT(CDDialogDone()));
// 		connect(cddialog,SIGNAL(play(int)),cddialog,SLOT(slotTrackSelected(int)));
	}

	m_cddialog->show();
//	m_cddialog->raise(); // Puts the window on top
*/
}

void CDDBManager::CDDialogDone()
{
	kDebug() << "test CDDB" ;
	/*
	m_cddialog->delayedDestruct();
	m_cddialog = 0L;*/
}

void CDDBManager::lookupCDDBDone(Result result)
{
	if (result != KCDDB::Success)
	{
		if(result == KCDDB::NoRecordFound)
			emit showArtistLabel(i18n("No matching freedb entry found."));
		else
			emit showArtistLabel(i18n("Error getting freedb entry."));

		QTimer::singleShot(3000, this, SIGNAL(restoreArtistLabel()));
		return;
	}

	KCDDB::CDInfo info = m_cddbClient->lookupResponse().first();

	// If there is more than 1 result display a choice window
	if(m_cddbClient->lookupResponse().count() > 1) {
		CDInfoList cddb_info = m_cddbClient->lookupResponse();
		QStringList list;
		CDInfoList::iterator it;
		for ( it = cddb_info.begin(); it != cddb_info.end(); ++it  )
		{
			list.append( QString("%1, %2, %3, %4").arg((*it).get(Artist).toString()).arg((*it).get(Title).toString()).arg((*it).get(Genre).toString()).arg((*it).get(KCDDB::Category).toString()));
		}
	
		bool ok(false);
		QString res = KInputDialog::getItem(i18n("Select CDDB Entry"),
				i18n("Select a CDDB entry:"), list, 0, false, &ok);
		if(ok)
		{
			// The user selected and item and pressed OK
			int c = 0;
			for(QStringList::Iterator it = list.begin(); it != list.end(); ++it )
			{
				if( *it == res) break;
				c++;
			}
			// Ensure that the selected item is within the range of the number of CDDB entries found
			if( c < cddb_info.size() )
			{
				info = cddb_info[c];
			}
		}
		else
		{
			return;
			// user pressed Cancel
		}
	}
	setCDInfo(info);
}

void CDDBManager::setCDInfo(KCDDB::CDInfo info)
{
	// Some sanity provisions to ensure that the number of records matches what
	// the CD actually contains.
	// TODO Q_ASSERT(info.numberOfTracks() == cd->tracks());

	m_cddbInfo = info;
	
	kDebug() << m_cddbInfo.get(Title).toString() ;
	kDebug() << m_cddbInfo.get(Comment).toString() ;
	kDebug() << m_cddbInfo.get(Artist).toString() ;
	kDebug() << m_cddbInfo.get(Genre).toString() ;
	kDebug() << m_cddbInfo.get(Year).toString() ;
	kDebug() << m_cddbInfo.get(Length).toString() ;
	kDebug() << m_cddbInfo.get(Category).toString() ;
	
	// Trace
	kDebug() << m_cddbInfo.toString() ;
	
	for (int i=0; i<m_cddbInfo.numberOfTracks (); i++)
	{
		kDebug() << i << " " << m_cddbInfo.track(i).get(KCDDB::Title).toString() ;
		kDebug() << i << " " << m_cddbInfo.track(i).get(KCDDB::Artist).toString() ;
	}
//	populateSongList();
	emit restoreArtistLabel();
	emit restoreTrackinfoLabel();
}

void CDDBManager::populateSongList()
{
/*
	songListCB->clear();
	for (uint i = 1; i <= m_cd->tracks(); ++i)
	{
	unsigned tmp = m_cd->trackLength(i);
	unsigned mymin;
	unsigned mysec;
	mymin = tmp / 60;
	mysec = (tmp % 60);
	QString str1;
	str1.sprintf("%02u: ", i);

	QString str2;
	str2.sprintf(" (%02u:%02u) ", mymin,  mysec);
	// see comment before restoreArtistLabel()
	if (cddbInfo.isValid() && cddbInfo.numberOfTracks() == m_cd->tracks()) {
	const KCDDB::TrackInfo& t = cddbInfo.track(i - 1);

	if (cddbInfo.get(KCDDB::Artist).toString() != t.get(KCDDB::Artist).toString())
	str1.append(t.get(KCDDB::Artist).toString()).append(" - ");
	str1.append(t.get(KCDDB::Title).toString());
} else {
	if (m_cd->discArtist() != m_cd->trackArtist(i))
	str1.append(m_cd->trackArtist(i)).append(" - ");
	str1.append(m_cd->trackTitle(i));
}
	str1.append(str2);
	songListCB->addItem(str1);
}
*/
}
