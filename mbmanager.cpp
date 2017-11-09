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

#include <musicbrainz5/Query.h>
#include <musicbrainz5/Artist.h>
#include <musicbrainz5/ArtistCredit.h>
#include <musicbrainz5/Disc.h>
#include <musicbrainz5/HTTPFetch.h>
#include <musicbrainz5/Medium.h>
#include <musicbrainz5/NameCredit.h>
#include <musicbrainz5/NameCreditList.h>
#include <musicbrainz5/Recording.h>
#include <musicbrainz5/Release.h>
#include <musicbrainz5/Track.h>

MBManager::MBManager():m_validInfo(true)
{
	m_discid = discid_new();
}

MBManager::~MBManager()
{
	discid_free(m_discid);
}

static QString getTitle(MusicBrainz5::CRelease *release, MusicBrainz5::CMedium *medium)
{
	QString title;
	if (!release)
	{
		return title;
	}

	title = QString::fromUtf8(release->Title().c_str());
	if (medium && release->MediumList()->NumItems() > 1)
	{
		title = i18n("%1 (disc %2)", title, medium->Position());
	}

	return title;
}

static QString getArtistFromArtistCredit(MusicBrainz5::CArtistCredit *artistCredit)
{
	QString artist;
	MusicBrainz5::CNameCreditList *artistList = artistCredit->NameCreditList();

	if (!artistList)
	{
		return artist;
	}

	for (int i = 0; i < artistList->NumItems(); i++)
	{
		MusicBrainz5::CNameCredit* name = artistList->Item(i);
		MusicBrainz5::CArtist* itemArtist = name->Artist();

		if (!name->Name().empty())
		{
			artist += QString::fromUtf8(name->Name().c_str());
		}
		else
		{
			artist += QString::fromUtf8(itemArtist->Name().c_str());
		}

		artist += QString::fromUtf8(name->JoinPhrase().c_str());
	}

	return artist;
}

static QString getArtist(MusicBrainz5::CRelease *release)
{
	QString artist;
	if (!release)
	{
		return artist;
	}

	MusicBrainz5::CArtistCredit *artistCredit = release->ArtistCredit();
	return getArtistFromArtistCredit(artistCredit);
}

static QList<MBTrackInfo> unknownTracks(QString &discArtist, DiscId *m_discid)
{
	QList<MBTrackInfo> tracks;
	MBTrackInfo track;
	for (int j = 1; j < discid_get_first_track_num(m_discid); j++)
	{
		track.Title = i18n("Unknown title");
		track.Artist = discArtist;
		// Not an audio track
		track.Duration = 0;

		tracks << track;
	}
	for (int j = discid_get_first_track_num(m_discid); j <= discid_get_last_track_num(m_discid); j++)
	{
		track.Title = i18n("Unknown title");
		track.Artist = discArtist;
		// time from mb library in sectors, 75 sectors = 1 second
		track.Duration = discid_get_track_length(m_discid, j) * 1000 / 75;

		tracks << track;
	}

	return tracks;
}

static QList<MBTrackInfo> getTracks(MusicBrainz5::CMedium *medium, QString &discArtist, DiscId *m_discid)
{
	QList<MBTrackInfo> tracks;
	if (!medium)
	{
		return tracks;
	}

	MusicBrainz5::CTrackList *trackList = medium->TrackList();
	if (!trackList)
	{
		return unknownTracks(discArtist, m_discid);
	}

	MBTrackInfo track;
	for (int i = 0; i < trackList->NumItems(); i++)
	{
		MusicBrainz5::CTrack *itemTrack = trackList->Item(i);
		MusicBrainz5::CRecording *recording = itemTrack->Recording();
		if (recording && !itemTrack->ArtistCredit())
		{
			track.Artist = getArtistFromArtistCredit(recording->ArtistCredit());
		}
		else
		{
			track.Artist = getArtistFromArtistCredit(itemTrack->ArtistCredit());
		}

		if(recording && itemTrack->Title().empty())
		{
			track.Title = QString::fromUtf8(recording->Title().c_str());
		}
		else
		{
			track.Title = QString::fromUtf8(itemTrack->Title().c_str());
		}

		track.Duration = itemTrack->Length();

		tracks << track;
	}

	return tracks;
}

static MusicBrainz5::CRelease *getRelease(MusicBrainz5::CQuery &query, std::string &discId, MusicBrainz5::CMetadata &metadata, MusicBrainz5::CMetadata &fullMetadata)
{
	metadata = query.Query("discid", discId);
	// Check to see how many items were returned from the server
	if (!metadata.Disc() || !metadata.Disc()->ReleaseList())
	{
		return 0;
	}

	MusicBrainz5::CReleaseList *results = metadata.Disc()->ReleaseList();

	// TODO if multiple entries found
	if (results->NumItems() > 1)
	{
		kDebug() << results->NumItems() << " entries found";
	}

	MusicBrainz5::CRelease *release;
	for (int i = 0; i < results->NumItems(); i++)
	{
		MusicBrainz5::CRelease *result = results->Item(i);
		MusicBrainz5::CQuery::tParamMap params;
		params["inc"] = "artists labels recordings release-groups url-rels "
						"discids artist-credits";
		fullMetadata = query.Query("release", result->ID(), "", params);

		release = fullMetadata.Release();
		if (release)
		{
			break;
		}
	}

	return release;
}

static MusicBrainz5::CMedium *getMedium(MusicBrainz5::CRelease *release, std::string &discId, MusicBrainz5::CMediumList &mediaList)
{
	if (!release)
	{
		return 0;
	}

	// Find the specific media in the release
	mediaList = release->MediaMatchingDiscID(discId);
	MusicBrainz5::CMedium* medium = 0;

	for (int i = 0; i < mediaList.NumItems(); i++)
	{
		medium = mediaList.Item(i);
		if (medium)
		{
			break;
		}
	}

	return medium;
}

void MBManager::discLookup(const QString &device)
{
	m_validInfo = true;
	MusicBrainz5::CQuery query("kscd");
	int discid_ok = discid_read_sparse(m_discid, qPrintable(device), 0);
	if (discid_ok)
	{
		std::string discId(discid_get_id(m_discid));
		try
		{
			MusicBrainz5::CMetadata metadata, fullMetadata;
			MusicBrainz5::CMediumList mediaList;
			MusicBrainz5::CRelease *release = getRelease(query, discId, metadata, fullMetadata);
			MusicBrainz5::CMedium *medium = getMedium(release, discId, mediaList);

			if (release && medium)
			{
				// Sets info
				m_discInfo.Title = getTitle(release, medium);
				m_discInfo.Artist = getArtist(release);

				m_trackList = getTracks(medium, m_discInfo.Artist, m_discid);
			}
			else
			{
				kDebug() << "This CD was not found.";
				m_validInfo = false;
			}

		}
		catch (MusicBrainz5::CExceptionBase& error)
		{
			kDebug() << "Connection Exception: '" << error.what() << "'";
			kDebug() << "LastResult: " << query.LastResult();
			kDebug() << "LastHTTPCode: " << query.LastHTTPCode();
			kDebug() << "LastErrorMessage: " << QString::fromUtf8(query.LastErrorMessage().c_str());

			m_validInfo = false;
		}
		catch (...)
		{
			kDebug() << "Caught Unknown Exception:";
			m_validInfo = false;
		}
	}
	else
	{
		m_validInfo = false;
	}

	if (!m_validInfo)
	{
		// If invalid data, fill the information with something
		// Sets info
		m_discInfo.Title = i18n("Unknown album");
		m_discInfo.Artist = i18n("Unknown artist");

		m_trackList.clear();
		if (discid_ok)
		{
			m_trackList = unknownTracks(m_discInfo.Artist, m_discid);
		}
	}

	emit discLookupFinished();
}

void MBManager::discUpload(const QString &device)
{
	showArtistLabel(m_discInfo.Artist);
	const char *discid_device = device.isEmpty()? NULL : qPrintable(device);

	int ok = discid_read_sparse(m_discid, discid_device, 0);
	if (ok)
	{
		QString url = QString::fromUtf8(discid_get_submission_url(m_discid));
		KToolInvocation::invokeBrowser(url);
	}
	else
	{
		kDebug() << "Error: " << discid_get_error_msg(m_discid);
	}
}
