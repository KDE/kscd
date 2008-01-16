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
 * Bouchikhi Mohamed-Amine <bouchikhi.amine@gmail.com>
 * Gastellu Sylvain
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
#ifndef __HWCONTROLER__
#define __HWCONTROLER__

#include <QObject>
#include <solid/device.h>
#include <solid/opticaldisc.h>

#include <phonon/mediasource.h>
#include <phonon/mediaobject.h>
#include <phonon/mediacontroller.h>
#include <phonon/audiooutput.h>
#include <phonon/volumeslider.h>


#include "audiocd.h"

enum LoopMode
{
	NoLoop = 0,
	LoopOne = 1,
	LoopAll = 2
};

class HWControler : public QObject
{

	Q_OBJECT

	private:

	// List of Cds inserted
		QList<AudioCD> cdIn;
	// Selected Cd to read
		int selectedCd;
	// List of detected Audio Output on the system
		//QList<Phonon::AudioOutput> speakers;
		Phonon::AudioOutput * speakers;
	// Selected Output to use
		int selectedS;
	// Control play activity
		Phonon::MediaObject *media;
	// Control Next/Preview functions
		Phonon::MediaController * mc;

	// Loop Mode
		LoopMode loopState;

		Phonon::Path path;
		
	public:
		HWControler();
		~HWControler();
		void selectCd(int cdNum);
		void selectSpeaker(int sNum);
		void eject();
		void play();
		void nextTrack();
		void prevTrack();
		void stop();
		void pause();
		void mute(bool mute);
		qint64 getTotalTime ();
		qint64 getRemainingTime ();
		qreal getVolume();
		void setVolume(qreal vol);
		Phonon::State getState();
		void configMedia();
		void setLoopMode(LoopMode lm);
		Phonon::MediaObject * getMedia();
		Phonon::AudioOutput * getAudioOutPut();
		AudioCD getCD();

	private slots:
		void catchCurrentTime(qint64 pos);

	public slots:
		void replayTrack(qint64 pos);
		void replayDisk();

	signals:
		void currentTime (qint64 pos);

};

#endif
