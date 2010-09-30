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

#include <ktoolinvocation.h>

#include <musicbrainz3/musicbrainz.h>
#include <musicbrainz3/query.h>

MBManager::MBManager():m_validInfo(true)
{

}

MBManager::~MBManager()
{

}

void MBManager::discLookup()
{
	m_validInfo = true;

	MusicBrainz::WebService* ws = new MusicBrainz::WebService();

	// Set the proper server to use. Defaults to mm.musicbrainz.org:80
	if (!qgetenv("MB_SERVER").isNull())
	{
		std::string server(qgetenv("MB_SERVER"));
		ws->setHost(server);
		//kDebug() << "!! set server !!" ;
	}
	else
	{
		//kDebug() << "no server";
	}

	// If you need to use a proxy, uncomment/edit the following line
	// as appropriate
	//ws->setProxyHost("proxy.mydomain.com");
	//ws->setProxyPort(80);

	try
	{
		// FIXME Uses the disc found by musicbrainz, not
		// necessarily the one used by KsCD
		MusicBrainz::Disc *disc = MusicBrainz::readDisc();
		std::string discId = disc->getId();

		MusicBrainz::Query q(ws);
		MusicBrainz::ReleaseResultList results;

		try
		{
			MusicBrainz::ReleaseFilter f = MusicBrainz::ReleaseFilter().discId(discId);
			results = q.getReleases(&f);

			// Check to see how many items were returned from the server
			if (!results.empty())
			{
				// TODO if multiple entries found
				if (results.size() > 1)
				{
					kDebug() << results.size() << " entries found";
				}

				MusicBrainz::ReleaseResult *result = results.front();
				MusicBrainz::Release *release = q.getReleaseById(result->getRelease()->getId(),
								&MusicBrainz::ReleaseIncludes().tracks().artist());
				// Sets info
				m_discInfo.Title = QString::fromUtf8(release->getTitle().c_str());
				m_discInfo.Artist = QString::fromUtf8(release->getArtist()->getName().c_str());

				m_trackList.clear();
				MBTrackInfo track;
				for (MusicBrainz::TrackList::iterator j = release->getTracks().begin();
								j != release->getTracks().end(); j++)
				{
					MusicBrainz::Track *t = *j;
					MusicBrainz::Artist *artist = t->getArtist();
					if (!artist)
						artist = release->getArtist();

					track.Title = QString::fromUtf8(t->getTitle().c_str());
					track.Artist = QString::fromUtf8(artist->getName().c_str());
					track.Duration = t->getDuration();

					m_trackList << track;
				}
			}
			else
			{
				kDebug() << "This CD was not found.";
				m_validInfo = false;
			}


		}
		// FIXME Doesn't seem to get caught, why?
		// catch (MusicBrainz::WebServiceError &e)
		catch (...)
		{
			//kDebug() << "Error: " << e.what();
			m_validInfo = false;
		}

		if (!m_validInfo)
		{
			// If invalid data, fill the information with something
			// Sets info
			m_discInfo.Title = i18n("Unknown album");
			m_discInfo.Artist = i18n("Unknown artist");

			m_trackList.clear();
			MBTrackInfo track;
			for (MusicBrainz::Disc::TrackList::iterator j = disc->getTracks().begin(); j != disc->getTracks().end(); j++)
			{
				track.Title = i18n("Unknown title");
				track.Artist = m_discInfo.Artist;
				// time from mb library in sectors, 75 sectors = 1 second
				track.Duration = (*j).second*1000/75;

				m_trackList << track;
			}
		}
		delete disc;
	}
	// FIXME Doesn't seem to get caught, why?
	//catch (MusicBrainz::DiscError &e)
	catch(...)
	{
		//kDebug() << "Error: " << e.what();
		m_discInfo.Title = i18n("Unknown album");
		m_discInfo.Artist = i18n("Unknown artist");
		m_discInfo.Artist = i18n( "No Disc" );
		m_trackList.clear();
	}
}

void MBManager::discUpload()
{
	showArtistLabel(m_discInfo.Artist);

	try
	{
		MusicBrainz::Disc *disc = MusicBrainz::readDisc();
		std::string url = MusicBrainz::getSubmissionUrl(disc);
		delete disc;

		KToolInvocation::invokeBrowser(QString::fromUtf8(url.c_str()));
	}
	catch (MusicBrainz::DiscError &e)
	{
		kDebug() << "Error: " << e.what();
	}
}

