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
#include "hwcontroler.h"
#include "audiocd.h"
#include <solid/opticaldisc.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/block.h>

#include <kdebug.h>

#include <phonon/mediasource.h>
#include <phonon/mediaobject.h>
#include <phonon/phononnamespace.h>
#include <phonon/path.h>
#include <phonon/mediacontroller.h>

using namespace Phonon;

HWControler :: HWControler ()
{

	// in kscd starting, no loop option
	loopState = NoLoop;

	// init CD detection
	selectedCd=-1;

	// init Speakers detection
	selectedS=-1;

	// getting all optical disc driver
	QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDrive, QString());

	// if no optical drive detected
	if (devList.isEmpty())
	{
		kDebug() << "No Optical Drive detected!";
		selectedCd = -1;

	}

	// else loading  all optical drives
	else
	{
		for (int i = 0; i < devList.size();i++)
		{
			cdIn.append(new AudioCD(devList[i]));
			selectedCd = 0;
			connect(cdIn[selectedCd],SIGNAL(discChanged()),this,SLOT(configMedia()));
		}
	}

	//TODO: Load ALL audio output
	speakers = new Phonon::AudioOutput ( MusicCategory, this );
	selectedS=0;

	// init of media object
	media = new Phonon::MediaObject(this);

	// Multimedia configuration
	configMedia();
}

HWControler::~HWControler ()
{
	delete speakers;
	delete media;
	delete mc;
}

// TODO function to switch optical drive
void HWControler :: selectCd(int cdNum)
{

}

// TODO function to switch audio output
void HWControler :: selectSpeaker(int sNum)
{

}

void HWControler :: eject()
{
	// if optical drive detected with a cd inside
	if(!(selectedCd==-1))
	{
		cdIn[selectedCd]->getCdDrive()->eject();
	}
}

void HWControler :: play()
{
	if((selectedCd!=-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			mc->setAutoplayTitles(true);
			media->play();
			kDebug() << getCurrentTrack() <<"/"<< getTotalTrack();
		}
		else
		{
			kDebug() << "No CD detected";
		}
	}
	else
	{
		kDebug() << "No Drive detected!!!";
	}
}
void HWControler :: nextTrack()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			if(mc->currentTitle() == mc->availableTitles())
			{
				mc->setCurrentTitle(1);
			}
			else
			{
				mc->nextTitle();
				catchCurrentTime(0);
			}
		}
	}	

}
void HWControler :: prevTrack()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			if(mc->currentTitle() == 1)
			{
				mc->setCurrentTitle(mc->availableTitles());
			}
			else
			{
				mc->previousTitle();
				catchCurrentTime(0);
			}
		}
	}
}
void HWControler :: stop()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			mc->setAutoplayTitles(false);
			media->stop();
			mc->setCurrentTitle(1);
			catchCurrentTime(0);
		}
	}
}
void HWControler :: pause()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			media->pause();
		}
	}
}
void HWControler :: mute(bool mute)
{
	if((selectedS!=-1))
	{
		speakers->setMuted(mute);
	}
}

qint64 HWControler :: getTotalTime ()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			return media->totalTime();
		}
		else
		{
			return -1;
		}
	}

	else
	{
		return -1;
	}
}
qint64 HWControler :: getRemainingTime ()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			return media->remainingTime();
		}
		else
		{
			return -1;
		}
	}

	else
	{
		return -1;
	}
}
qreal HWControler :: getVolume()
{
	if (selectedS == -1)
	{
		return -1;
	}
	else
	{
		return speakers->volume();
	}
}
void HWControler :: setVolume(qreal vol)
{
	if (selectedS != -1)
	{
		speakers->setVolume(vol);
	}
}
Phonon::State HWControler ::getState()
{
	if(selectedCd==-1)
	{
		return ErrorState;
	}
	else
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			return media->state();
		}
		else
		{
			return ErrorState;
		}
	}
};

void HWControler ::configMedia()
{
	if(selectedCd!=-1)
	{
		kDebug()<< "#o#o#o# Loading Optical Drive";
		if(cdIn[selectedCd]->isCdInserted())
		{
			media->setCurrentSource(*cdIn[selectedCd]->getMediaSource());
			path = Phonon::createPath(media, speakers);
			kDebug()<< "Phonon Loaded";
			mc = new MediaController(media);
			mc->setAutoplayTitles(false);
			media->setTickInterval(100);
			connect(media,SIGNAL(tick(qint64)),this,SLOT(replayTrack(qint64)));
			connect(media,SIGNAL(tick(qint64)),this,SLOT(catchCurrentTime(qint64)));
			connect(media,SIGNAL(finished()),this,SLOT(replayDisk()));
			connect(mc,SIGNAL(titleChanged(int)),this,SLOT(catchTitleChanged()));
		}
		emit(cdLoaded());
	}
}

AudioCD * HWControler::getCD()
{
	if((selectedCd!=-1))
	{
		return cdIn[selectedCd];
	}
	return NULL;
}

void HWControler ::setLoopMode(LoopMode lm)
{
	loopState = lm;
	kDebug()<<"Loop Mode: "<<lm;
}
void HWControler ::replayTrack(qint64 pos)
{
	if(loopState==LoopOne)
	{
		
		if (getRemainingTime()<= 500)
		{
			kDebug()<<"End of this track!";
			stop();
			mc->setCurrentTitle(mc->currentTitle());
			play();
		}
	}
}
void HWControler ::replayDisk()
{
	if((selectedCd!=-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			if(loopState==LoopAll)
			{
				kDebug()<<"Replaying the disc!";
				mc->setCurrentTitle( 1 );
				play();
			}
		}
	}
}

Phonon::MediaObject * HWControler ::getMedia()
{
	if((selectedCd!=-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			return media;
		}
	}
	return NULL;
}
void HWControler ::catchCurrentTime(qint64 pos)
{
	emit(currentTime(pos));
}
Phonon::AudioOutput * HWControler ::getAudioOutPut()
{
	return speakers;
}
int HWControler ::getCurrentTrack()
{
	if((selectedCd!=-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			return mc->currentTitle();
		}
	}
	return 0;
}
int HWControler ::getTotalTrack()
{
	if((selectedCd!=-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			return mc->availableTitles();
		}
	}
	return 0;
}
void HWControler ::play(int track)
{
	if((selectedCd!=-1))
	{
		if(cdIn[selectedCd]->isCdInserted())
		{
			mc->setAutoplayTitles(true);
			mc->setCurrentTitle(track);
			media->play();
		}
	}

}
void HWControler ::catchTitleChanged()
{
	emit(trackChanged());
}

