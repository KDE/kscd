/*
 *  KCompactDisc - A CD drive interface for the KDE Project.
 *
 *  Copyright (c) 2005 Shaheedur R. Haque <srhaque@iee.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef KCOMPACTDISC_H
#define KCOMPACTDISC_H

#include <qobject.h>
#include <qtimer.h>
#include <qvaluelist.h>

/**
 *  KCompactDisc - A CD drive interface for the KDE Project.
 *
 *  The disc lifecycle is modelled by these signals:
 *
 * @see #trayClosing(): A disc is being inserted.
 * @see #discChanged(): A disc was inserted or removed.
 * @see #trayOpening(): A disc is being removed.
 *
 *  The progress of playout is modelled by these signals:
 *
 * @see #trackPlaying(): A track started playing, or is still playing.
 * @see #trackPaused(): A track was paused.
 * @see #discStopped(): The disc stopped.
 *
 *  All times in this interface are is milliseconds.
 */
class KCompactDisc :
    public QObject
{
    Q_OBJECT
public:
    KCompactDisc();
    virtual ~KCompactDisc();

    /**
     * Open/close tray.
     */
    void eject();

    /**
     * Start playout at given position of track.
     */
    void play(unsigned startTrack = 0, unsigned startTrackPosition = 0, unsigned endTrack = 0);

    /**
     * Pause/resume playout.
     */
    void pause();

    /**
     * @param device Name of CD device, e.g. /dev/cdrom.
     * @param digitalPlayback Select digial or analogue playback.
     * @param audioSystem For analogue playback, system to use, e.g. "arts".
     * @param audioDevice For analogue playback, device to use.
     */
    void setDevice(
        const QString &device = defaultDevice,
        unsigned volume = 50,
        bool digitalPlayback = true,
        const QString &audioSystem = "",
        const QString &audioDevice = "");

    void setVolume(unsigned volume);

    /**
     * Stop playout.
     */
    void stop();

    /**
     * The default CD for this system.
     */
    static const QString defaultDevice;

    /**
     * Current device.
     */
    const QString &device() const;

    /**
     * Current disc, zero if no disc.
     */
    unsigned discId() const { return m_discId; }

    const QString &discArtist() const { return m_artist; }
    const QString &discTitle() const { return m_title; }
    const QString &trackArtist(unsigned track) const { return m_trackArtists[track - 1]; }
    const QString &trackTitle(unsigned track) const { return m_trackTitles[track - 1]; }
    const QValueList<unsigned> &cddbSignature() const { return m_trackStartFrames; }

    /**
     * Length of disc.
     *
     * @return Disc length in milliseconds.
     */
    unsigned discLength() const;

    /**
     * Position in disc.
     *
     * @return Position in milliseconds.
     */
    unsigned discPosition() const;

    /**
     * Current track.
     *
     * @return Track number.
     */
    unsigned track() const;

    /**
     * Number of tracks.
     */
    unsigned tracks() const;

    /**
     * Length of current track.
     *
     * @return Track length in milliseconds.
     */
    unsigned trackLength() const;

    /**
     * Length of given track.
     *
     * @param track Track number.
     * @return Track length in milliseconds.
     */
    unsigned trackLength(unsigned track) const;

    /**
     * Position in current track.
     *
     * @return Position in milliseconds.
     */
    unsigned trackPosition() const;

    bool isPaused() const;

    bool isPlaying() const;

signals:

    /**
     * A disc is being inserted.
     */
    void trayClosing();

    /**
     * A disc is being removed.
     */
    void trayOpening();

    /**
     * A disc was inserted or removed.
     *
     * @param discId Zero if there is no (audio) disc.
     * @param artist Empty string if unknown.
     * @param album Empty string if unknown.
     * @param trackNames Array of track names, each item empty string if unknown.
     * @param trackStartTimes CDDB-compatible array of track start times (i.e. two
     *        more entries than tracks).
     */
    void discChanged(unsigned discId);

    /**
     * Disc stopped. See @see #trackPaused.
     */
    void discStopped();

    /**
     * The current track changed.
     *
     * @param track Track number.
     * @param trackLength Length within track in milliseconds.
     */
    void trackChanged(unsigned track, unsigned trackLength);

    /**
     * A track started playing, or is still playing. This signal is delivered at
     * approximately 1 second intervals while a track is playing. At first sight,
     * this might seem overzealous, but its likely that any CD player UI will use
     * this to track the second-by-second position, so we may as well do it for
     * them.
     *
     * @param track Track number.
     * @param trackPosition Position within track in milliseconds.
     */
    void trackPlaying(unsigned track, unsigned trackPosition);

    /**
     * A track paused playing.
     *
     * @param track Track number.
     * @param trackPosition Position within track in milliseconds.
     */
    void trackPaused(unsigned track, unsigned trackPosition);

private:
    QTimer timer;
    QString m_device;
    int m_status;
    int m_previousStatus;
    unsigned m_discId;
    unsigned m_previousDiscId;
    QString m_artist;
    QString m_title;
    unsigned m_tracks;
    QValueList<unsigned> m_trackStartFrames;
    QStringList m_trackArtists;
    QStringList m_trackTitles;
    unsigned m_track;
    unsigned m_previousTrack;
    void checkDeviceStatus();
    QString discStatus(int status);

private slots:
    void timerExpired();
};

#endif
