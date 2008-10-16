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

#ifndef MBMANAGER_H
#define MBMANAGER_H

// // MusicBrainz
#include <stdio.h>
#include <stdlib.h>
#include "musicbrainz/musicbrainz.h"
#include "musicbrainz/mb_c.h"
#include "musicbrainz/browser.h"

// KDE includes
#include <kdebug.h>
#include <klocalizedstring.h>

// QT includes
#include <qstring.h>
#include <qlist.h>

struct DiscInfo
{
	QString Title;
	QString Artist;
};

struct MBTrackInfo
{
	QString Title;
	QString Artist;
	QString Duration;
};

class MBManager : public QObject
{
	Q_OBJECT

private:
	DiscInfo m_discInfo;				/// Contains the album's information
	QList <MBTrackInfo> m_trackList;	/// List of tracks information

	char * m_browser;

	bool m_validInfo;					/// Tells whether the lookup query succeeded

public:
	MBManager();
	~MBManager();

/**
* Getters/Setters
*/
	/** Returns the disc information */
	DiscInfo getDiscInfo() const { return this->m_discInfo; }
	QList <MBTrackInfo> getTrackList() const { return this->m_trackList; }
	bool isValidInfo() const { return this->m_validInfo; }

public slots:
	/** Gets information about the disc inserted */
	void discLookup();

	/** Uploads information */
	void discUpload();

signals:
	void showArtistLabel(QString&);
	void showTrackinfoLabel(QString&);
	void restoreArtistLabel();
	void restoreTrackinfoLabel();
};

#endif

