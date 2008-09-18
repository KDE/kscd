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
#include "kscd.h"
#include <klocale.h>

class KSCD;

CDDBManager::CDDBManager(KSCD *parent)
{
	// CDDB initialization
	m_cddbInfo.clear(); // The first freedb revision is "0"
	m_cddbClient = new KCDDB::Client();
	
	// KsCD pointer
	pKscd = parent;
//	m_cddialog = 0L;
	

// TODO inactivate CDDB options if no disc
// TODO erase that !
//	if( pKscd->getDevices()->getCD()->isCdInserted() && pKscd->getDevices()->isDiscValid() )
	{
		for (int i=0; i<pKscd->getDevices()->getTotalTrack(); i++)
		{
			m_cddbInfo.track(i).set(KCDDB::Title, QString(i18n("Unknown")) ) ;
		}
	}
	
	connect(m_cddbClient, SIGNAL(finished(KCDDB::Result)), this, SLOT(lookupCDDBDone(KCDDB::Result)));
}

CDDBManager::~CDDBManager()
{
	delete m_cddialog;
	delete m_cddbClient;
}

// Fonction to be called whenever a new Disc is inserted
void CDDBManager::setupCDDB(int nbTrack, KCDDB::TrackOffsetList signature)
{
	m_cdSignature = signature;
	
	for (int i=0; i<nbTrack; i++)
	{
		m_cddbInfo.track(i).set(KCDDB::Title, QString("unknown") ) ;
	}
}

void CDDBManager::CDDialogSelected()
{	
	kDebug() << "CDDialogSelected1" ;
	
	//Create CDDB Window
	m_cddialog = new QDialog(0, Qt::Window);
	
	//m_cddialog->raise();// Puts the window on top

	m_cddialog->setWindowTitle(i18n("CDDB Manager"));
	m_cddialog->resize(400, 600);
	m_cddialog->setMinimumSize(300,500);
//	m_cddialog->setDefault(true);
	
	
	QGridLayout * mainLayout = new QGridLayout;

	// Album Info
	QGridLayout * albumLayout = new QGridLayout;

	// Album Title
	QLabel* albumTitleLabel = new QLabel(i18n("Album Title"));
	QLineEdit* albumTitlelineEdit = new QLineEdit;
	albumTitlelineEdit->insert (m_cddbInfo.get(Title).toString());
	//albumTitleLabel->setBuddy(albumTitlelineEdit);
	albumLayout->addWidget(albumTitleLabel, 0, 0);
	albumLayout->addWidget(albumTitlelineEdit, 0, 1);

	// Artist
	QLabel* albumArtistLabel = new QLabel(i18n("Artist"));
	QLineEdit* albumArtistlineEdit = new QLineEdit;
	albumArtistlineEdit->insert (m_cddbInfo.get(KCDDB::Artist).toString());
	//albumArtistLabel->setBuddy(albumArtistlineEdit);
	albumLayout->addWidget(albumArtistLabel, 1, 0);
	albumLayout->addWidget(albumArtistlineEdit, 1, 1);

	// Year
	QLabel* albumYearLabel = new QLabel(i18n("Year"));
	QLineEdit* albumYearlineEdit = new QLineEdit;
	albumYearlineEdit->insert (m_cddbInfo.get(Year).toString());
	//albumYearLabel->setBuddy(albumYearlineEdit);
	albumLayout->addWidget(albumYearLabel, 2, 0);
	albumLayout->addWidget(albumYearlineEdit, 2, 1);
	
	
	// Genre
	QLabel* albumGenreLabel = new QLabel(i18n("Genre"));
	QLineEdit* albumGenrelineEdit = new QLineEdit;
	albumGenrelineEdit->insert (m_cddbInfo.get(Genre).toString());
	//albumGenreLabel->setBuddy(albumGenrelineEdit);
	albumLayout->addWidget(albumGenreLabel, 3, 0);
	albumLayout->addWidget(albumGenrelineEdit, 3, 1);

	// Category
	QLabel* albumCategoryLabel = new QLabel(i18n("Category"));
	QLineEdit* albumCategorylineEdit = new QLineEdit;
	albumCategorylineEdit->insert (m_cddbInfo.get(Category).toString());
	//albumCategoryLabel->setBuddy(albumCategorylineEdit);
	albumLayout->addWidget(albumCategoryLabel, 4, 0);
	albumLayout->addWidget(albumCategorylineEdit, 4, 1);

	// Length
	QLabel* albumLengthLabel = new QLabel(i18n("Length"));
	QLineEdit* albumLengthlineEdit = new QLineEdit;
	albumLengthlineEdit->insert(m_cddbInfo.get(Length).toString());
	//albumLengthLabel->setBuddy(albumLengthlineEdit);
	albumLayout->addWidget(albumLengthLabel, 5, 0);
	albumLayout->addWidget(albumLengthlineEdit, 5, 1);
	
	// Comment
	QLabel* albumCommentLabel = new QLabel(i18n("Comment"));
	QTextEdit* albumCommenttextEdit = new QTextEdit;
	albumCommenttextEdit->append(m_cddbInfo.get(Comment).toString());
	//albumCommentLabel->setBuddy(albumCommentlineEdit);
	albumLayout->addWidget(albumCommentLabel, 6, 0, Qt::AlignTop);
	albumLayout->addWidget(albumCommenttextEdit, 6, 1);

	mainLayout->addLayout(albumLayout, 0, 0);

	//Tracks	
	QTableWidget* tracksTable = new QTableWidget(0, 7);
	QStringList tracklabels;
	tracklabels << i18n("Title") << i18n("Artist") << i18n("Genre") << i18n("Category") << i18n("Year") << i18n("Length") << i18n("Comment");
	tracksTable->setHorizontalHeaderLabels(tracklabels);
	//tracksTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
	//tracksTable->verticalHeader()->hide();
	int taille = sizeof(tracklabels);
	int row = tracksTable->rowCount();
// 	tracksTable->insertRow(row);
	for (int i=0; i < pKscd->getDevices()->getTotalTrack(); i++) {
		
		row = tracksTable->rowCount();
		tracksTable->insertRow(row);
		
		for (int j=0; j<taille+3; j++) {
			
			QTableWidgetItem* infoItem = new QTableWidgetItem(m_cddbInfo.track(i).get(tracklabels[j]).toString());
			tracksTable->setItem(i, j, infoItem);
		}
		
	}
	
	tracksTable->setShowGrid(false);
	mainLayout->addWidget(tracksTable, 1, 0);

	// Buttons
	QGridLayout * buttonsLayout = new QGridLayout;

	// Save Button
	QPushButton *savebutton = new QPushButton(i18n("Save"));
	buttonsLayout->addWidget(savebutton, 0, 0);

	//Cancel Button
	QPushButton *cancelbutton = new QPushButton(i18n("Cancel"));
	connect(cancelbutton,SIGNAL(clicked()),this,SLOT(CDDialogDone()));
	buttonsLayout->addWidget(cancelbutton, 0, 1);

	mainLayout->addLayout(buttonsLayout, 2, 0);

	m_cddialog->setLayout(mainLayout);

	m_cddialog->show();

}

void CDDBManager::CDDialogDone()
{
	kDebug() << "close CDDB Manager" ;
	m_cddialog->hide();
	/*
	m_cddialog->delayedDestruct();
	m_cddialog = 0L;*/
}

void CDDBManager::refreshCDDB()
{
	/*
	kDebug() << "refreshCDDB" ;
	if (pKscd->getDevices()->getCD()->isCdInserted())
	{
		setupCDDB(pKscd->getDevices()->getTotalTrack(), pKscd->getCd()->discSignature() );
	}
	*/
}

// TODO move this function to CDDBManager and add an signature attribute
void CDDBManager::lookupCDDB()
{
	kDebug() << "Lookup CDDB" ;
	if(!pKscd->getDevices()->getCD()->isCdInserted())
	{
		showArtistLabel(i18n("No Disc"));
		QTimer::singleShot(2000, this, SLOT(restoreArtistLabel()));
	}
	else
	{
		if(!pKscd->getDevices()->isDiscValid())
		{
			showArtistLabel(i18n("Invalid disc"));
			QTimer::singleShot(2000, this, SLOT(restoreArtistLabel()));
		}
		else
		{
			showArtistLabel(i18n("Start freedb lookup."));
		
			getCddbClient()->config().reparse();
			getCddbClient()->setBlockingMode(false);
			
	//TODO get CD signature through Solid
			//getCddbClient()->lookup( pKscd->getCd()->discSignature() );
	
//			kDebug() << pKscd->getCd()->discSignature() ;
		}
	}
}
void CDDBManager::lookupCDDBDone(KCDDB::Result result)
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
			list.append( QString("%1, %2, %3, %4").arg((*it).get(KCDDB::Artist).toString()).arg((*it).get(KCDDB::Title).toString()).arg((*it).get(KCDDB::Genre).toString()).arg((*it).get(KCDDB::Category).toString()));
		}
	
		bool ok(false);
		QString res = KInputDialog::getItem(i18n("Select CDDB Entry"),
				i18n("Select a CDDB entry:"), list, 0, false, &ok);
		if(ok)
		{
			// The user selected and item and pressed OK
			int c = 0;
			for(QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
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
	
	emit restoreArtistLabel();
	emit restoreTrackinfoLabel();
}

QList <CDDBTrack> CDDBManager::getTrackList()
{
	QList<CDDBTrack> list;
	CDDBTrack track;
	for (int i=0; i<m_cddbInfo.numberOfTracks(); i++)
	{
		track.Title = m_cddbInfo.track(i).get(KCDDB::Title).toString();
		track.Comment = m_cddbInfo.track(i).get(KCDDB::Comment).toString();
		track.Artist = m_cddbInfo.track(i).get(KCDDB::Artist).toString();
		track.Genre = m_cddbInfo.track(i).get(KCDDB::Genre).toString();
		track.Year = m_cddbInfo.track(i).get(KCDDB::Year).toString();
		track.Length = m_cddbInfo.track(i).get(KCDDB::Length).toString();
		track.Category = m_cddbInfo.track(i).get(KCDDB::Category).toString();
		
		list << track;
	}
	return list;
}

QString CDDBManager::getDiscTitle()
{
	return m_cddbInfo.get(KCDDB::Title).toString();
}

/*
void CDDBManager::populateSongList()
{
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

}
*/
