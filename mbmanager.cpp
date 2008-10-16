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
#include "mbmanager.h"

MBManager::MBManager():m_validInfo(true)
{

}

MBManager::~MBManager()
{

}

void MBManager::discLookup()
{
	MusicBrainz MB;
	string      error, data;
	bool        ret;
	int         numTracks = 0, trackNum = 1;

	m_validInfo = true;

	// Set the proper server to use. Defaults to mm.musicbrainz.org:80
	if (!qgetenv("MB_SERVER").isNull())
	{
		string server(qgetenv("MB_SERVER"));
		MB.SetServer(server, 80);
		//kDebug() << "!! set server !!" ;
	}
	else
	{
		//kDebug() << "no server";
	}

	// Check to see if the debug env var has been set
	if (!qgetenv("MB_DEBUG").isNull())
	{
		MB.SetDebug(qgetenv("MB_DEBUG").toInt() );
		//kDebug() << "!! set debug !!" ;
	}
	else
	{
		//kDebug() << "no debug";
	}

	// If you need to use a proxy, uncomment/edit the following line
	// as appropriate
	//MB.SetProxy("proxy.mydomain.com", 80);

	// Tell the client library to return data in UTF-8
	MB.UseUTF8(false);

	// Execute the GetCDInfo query, which pulls the TOC from the
	// audio CD in the cd-rom drive, calculates the disk id and
	// requests the data from the server
	ret = MB.Query(string(MBQ_GetCDInfo));

	//kDebug() << "query passed";
	if (!ret)
	{
		MB.GetQueryError(error);
		printf("Query failed: %s\n", error.c_str());
		m_validInfo = false;
	}

	// Check to see how many items were returned from the server
	if (MB.DataInt(MBE_GetNumAlbums) < 1)
	{
		printf("This CD was not found.\n");
		m_validInfo = false;
	}

	// TODO if multiple entries found
	if (MB.DataInt(MBE_GetNumAlbums) > 1)
	{
		kDebug() << MB.DataInt(MBE_GetNumAlbums) << " entries found";
	}

	// TODO manage multiple entries
	// Select the first album
	MB.Select(MBS_SelectAlbum, 1);

	// Get the number of tracks
	numTracks = MB.DataInt(MBE_AlbumGetNumTracks);
	//kDebug() << "NumTracks: " << numTracks << endl;

	if (m_validInfo == true)
	{
		// Sets info
		m_discInfo.Title = MB.Data(MBE_AlbumGetAlbumName).c_str();
		m_discInfo.Artist = MB.Data(MBE_AlbumGetAlbumArtistName).c_str();

		m_trackList.clear();
		MBTrackInfo track;
		for(int i = 1; i <= numTracks; i++)
		{
			track.Title = MB.Data(MBE_AlbumGetTrackName, i).c_str();
			track.Artist = MB.Data(MBE_AlbumGetArtistName, i).c_str();
			track.Duration = MB.Data(MBE_AlbumGetTrackDuration, i).c_str();

			m_trackList << track;
		}
	}
	else
	{
		// If invalid data, fill the information with something
		// Sets info
		m_discInfo.Title = i18n("Unknown album");
		m_discInfo.Artist = i18n("Unknown artist");

		m_trackList.clear();
		MBTrackInfo track;
		for(int i = 1; i <= numTracks; i++)
		{

			track.Title = i18n("Unknown title");
			track.Artist = m_discInfo.Artist;
			track.Duration = MB.Data(MBE_AlbumGetTrackDuration, i).c_str();

			m_trackList << track;
		}
	}
}

void MBManager::discUpload()
{
	showArtistLabel(m_discInfo.Artist);

	musicbrainz_t o;
	char          url[1025];

    // Create the musicbrainz object, which will be needed for subsequent calls
	o = mb_New();

    // Set the proper server to use. Defaults to mm.musicbrainz.org:80
	if (!qgetenv("MB_SERVER").isNull())
		mb_SetServer(o, qgetenv("MB_SERVER").data(), 80);

    // Check to see if the debug env var has been set
	if (!qgetenv("MB_DEBUG").isNull())
		mb_SetDebug(o, qgetenv("MB_DEBUG").toInt());

    // Tell the server to only return 2 levels of data, unless the MB_DEPTH env var is set
	if (!qgetenv("MB_DEPTH").isNull())
		mb_SetDepth(o, qgetenv("MB_DEPTH").toInt());
	else
		mb_SetDepth(o, 2);

// 	m_browser = "firefox";
	m_browser = "konqueror";

    // Tell the client library to return data in ISO8859-1 and not UTF-8
	mb_UseUTF8(o, 0);

    // Now get the web submit url
	if (mb_GetWebSubmitURL(o, url, 1024))
	{
		int ret;

		printf("URL: %s\n", url);

		if (!m_browser)
			m_browser = "konqueror";

		ret = LaunchBrowser(url, m_browser);
		if (ret == 0)
			printf("Could not launch browser. (%s)\n", m_browser);
	}
	else
		printf("Could not read CD-ROM parameters. Is there a CD in the drive?\n");

// and clean up the musicbrainz object
	mb_Delete(o);
}

