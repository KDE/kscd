/*
 * Kscd - A simple cd player for the KDE Project
 *
 * Copyright (c) 2008 Amine Bouchikhi <bouchikhi.amine@gmail.com>
 * Copyright (c) 2008 Stanislas Krzywda <stanislas.krzywda@gmail.com>
 *
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
#ifndef HWCONTROLER
#define HWCONTROLER

#include <QObject>
#include <QList>
#include <QString>
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
		QList<AudioCD *> cdIn;
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

		bool random;

		bool isEjectAtTheEndOfTheCdActivated;

		QList<int> playList;

		int posPlayList;

	public:
		HWControler();
		~HWControler();
		void selectCd(int cdNum);
		void selectSpeaker(int sNum);
		void play(int track);
		qint64 getTotalTime () const ;
		qint64 getRemainingTime ()const ;
		qreal getVolume()const ;
		void setVolume(qreal vol);
		Phonon::State getState()const ;
		void setLoopMode(LoopMode lm);
		Phonon::MediaObject * getMedia()const ;
		Phonon::AudioOutput * getAudioOutPut()const ;
		AudioCD* getCD()const ;
		QList<unsigned> &getDiscSignature()const ;
		int getCurrentTrack()const ;
		int getTotalTrack()const ;
		bool isDiscValid();
		bool isEjectActivated() const;
		int nbCdReader() const;
		QString getCdReader(int num)const ;
	private:
		void loadPlayList();
		int generateNumber(int inter);
		void playRand();

	private slots:
		void catchCurrentTime(qint64 pos);
		void catchTitleChanged();

	public slots:
		void replayTrack(qint64 pos);
		void replayDisk();
		void configMedia();

		void setEjectActivated(bool b);

		void eject();
		void play();
		void nextTrack();
		void prevTrack();
		void stop(bool restart=true); // @param restart, if true restart from the beginning of the disc
		void pause();
		void mute(bool mute);
		void setRandom(bool b);


	signals:
		void currentTime (qint64 pos);
		void totalTime(qint64 pos);
		void trackChanged();
		void cdLoaded();

};

#endif
