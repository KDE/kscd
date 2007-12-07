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
#include <phonon/audiooutput.h>


#include "audiocd.h"

class HWControler : public QObject
{

	Q_OBJECT

	private:

	// List of Cds inserted
		QList<AudioCD> cdIn;
	// Selected Cd to read
		int selectedCd;
	// List of Audio Out Put detected on the system
		//QList<Phonon::AudioOutput> speakers;
		Phonon::AudioOutput * speakers;
	// Selected Out Put to use
		int selectedS;
	// Control play activity
		Phonon::MediaObject *media;

		Phonon::Path path;
		
	public:
		HWControler();
		void selectCd(int cdNum);
		void selectSpeaker(int sNum);
		void eject();
		void play();
		void nextTrack();
		void prevTrack();
		void stop();
		void pause();
		void mute(bool mute);
		qint64 getCurrentTime ();
		qint64 getTotalTime ();
		qint64 getRemainingTime ();
		qreal getVolume();
		Phonon::State getState();
		void configMedia();


};

#endif
