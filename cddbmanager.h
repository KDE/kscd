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

#ifndef __CDDBMANAGER_H__
#define __CDDBMANAGER_H__

// CDDB support via libkcddb
#include <libkcddb/kcddb.h>
#include <libkcddb/client.h>
using namespace KCDDB;

#include <klocalizedstring.h>
#include <kinputdialog.h>
#include <kdebug.h>

#include <QTimer>
#include <QDialog>

#include <QLabel>
#include <QGridLayout>
#include <QLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableWidget>
#include <QStringList>
#include <QTableWidgetItem>
#include <QPushButton>
#include <KStandardAction>

class KSCD;

struct CDDBTrack
{
	QString Title;
	QString Comment;
	QString Artist;
	QString Genre;
	QString Year;
	QString Length;
	QString Category;
};

class CDDBManager : public QObject
{
	Q_OBJECT

public:
	CDDBManager(KSCD *parent = 0);
	~CDDBManager();

private:
	// Info from CDDB
	QDialog* m_cddialog;
	KCDDB::CDInfo m_cddbInfo;
	KCDDB::Client* m_cddbClient;
	KCDDB::TrackOffsetList m_cdSignature;
	bool infoSet;
	bool autoDownload;
	KSCD * pKscd;
	
public:
	KCDDB::Client* getCddbClient(){ return m_cddbClient; }
	KCDDB::CDInfo getCddbInfo(){ return m_cddbInfo; }
	KCDDB::TrackOffsetList getCdSignature(){ return m_cdSignature; }
	
	QList <CDDBTrack> getTrackList();
	QString getDiscTitle();
	
	void setupCDDB(int nbTrack, KCDDB::TrackOffsetList signature);

private slots:
	void CDDialogSelected();
	void CDDialogDone();
	void refreshCDDB();
	void lookupCDDB();
	void lookupCDDBDone(KCDDB::Result);
	void setCDInfo(KCDDB::CDInfo);

signals:
	void showArtistLabel(QString);
	void showTrackinfoLabel(QString);
	void restoreArtistLabel();
	void restoreTrackinfoLabel();
};

#endif
