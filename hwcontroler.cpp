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


using namespace Phonon;

HWControler :: HWControler ()
{
	selectedCd=-1;
	selectedS=-1;

	QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDrive, QString());

	if (devList.isEmpty())
	{
		kDebug() << "No Optical Drive detected!";
		selectedCd = -1;

	}
	else
	{
		for (int i = 0; i < devList.size();i++)
		{
			cdIn.append(AudioCD(devList[i]));
			selectedCd = 0;
		}
		/*connect(cdIn[selectedCd].getCdDrive(), SIGNAL(ejectDone(Solid::ErrorType,
					QVariant, const QString)), this, SLOT(reloadSystem()));
			kDebug()<<"connecting ejectdone with reloadSystem!";*/
	}
	//TODO: Load ALL audio output
	speakers = new Phonon::AudioOutput ( MusicCategory, this );
	selectedS=0;
	media = new Phonon::MediaObject(this);
	configMedia();
}

void HWControler :: selectCd(int cdNum)
{

}
void HWControler :: selectSpeaker(int sNum)
{

}
void HWControler :: eject()
{
	if(!(selectedCd==-1))
	{
		cdIn[selectedCd].getCdDrive()->eject();
	}
}
void HWControler :: play()
{
kDebug() << selectedCd;
	if((selectedCd!=-1))
	{
		if(cdIn[selectedCd].isCdInserted())
		{
			media->play();
			kDebug() << cdIn[selectedCd].getMediaSource()->fileName();
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

}
void HWControler :: prevTrack()
{

}
void HWControler :: stop()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd].isCdInserted())
		{
			media->stop();
		}
	}
}
void HWControler :: pause()
{
	if(!(selectedCd==-1))
	{
		if(cdIn[selectedCd].isCdInserted())
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

qint64 HWControler :: getCurrentTime ()
{
	if(!(selectedCd==-1))
	{
		return -1;
	}
	else
	{
		if(cdIn[selectedCd].isCdInserted())
		{
			return media->currentTime();
		}
		else
		{
			return -1;
		}
	}
}
qint64 HWControler :: getTotalTime ()
{
	if(!(selectedCd==-1))
	{
		return -1;
	}
	else
	{
		if(cdIn[selectedCd].isCdInserted())
		{
			return media->totalTime();
		}
		else
		{
			return -1;
		}
	}
}
qint64 HWControler :: getRemainingTime ()
{
	if(!(selectedCd==-1))
	{
		return -1;
	}
	else
	{
		if(cdIn[selectedCd].isCdInserted())
		{
			return media->remainingTime();
		}
		else
		{
			return -1;
		}
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
Phonon::State HWControler ::getState()
{
	if(selectedCd==-1)
	{
		return ErrorState;
	}
	else
	{
		if(cdIn[selectedCd].isCdInserted())
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
		if(cdIn[selectedCd].isCdInserted())
		{
			media->setCurrentSource(*cdIn[selectedCd].getMediaSource());
			path = Phonon::createPath(media, speakers);
			kDebug()<< "Phonon Loaded";
		}
	}
}


/*

Solid::OpticalDrive* HWcontroler ::getSelectedOpticalDrive()
{
	return CDDrive[ODactual];
}
void HWcontroler ::setSelectedOpticalDrive(int num)
{
	if (num >= CDDrive.size())
	{
		kDebug() << "Invalid Parameter!";
	}
	else
	{
		ODactual=num;
	}
}
void HWcontroler ::loadCDDevices()
{
//	----	Loading Optical Drives	----	//

	kDebug() << "Loading Optical Drives";

	// list all Optical Drives of the system
	CDDriveList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDrive, QString());

	
	if (CDDriveList.size()<1)
	{
		kDebug() << "No Optical detected!";
	}
	else
	{
		for (int i = 0; i < CDDriveList.size();i++)
		{
			kDebug() << "Optical Disc detected: " << CDDriveList[i].udi();
			kDebug() << "Optical Disc detected: " << CDDriveList[i].parentUdi();
			// Devices to OpticalDrive
			CDDrive.append(CDDriveList[i].as<Solid::OpticalDrive>());
		}
	}
	kDebug() << "bus: "<< CDDrive[ODactual]->bus();
}
void HWcontroler ::loadReading() // Not finished
{
	Solid::Block * b = CDDriveList[ODactual].as<Solid::Block>();
	kDebug()<<b->device();
	MS = new Phonon::MediaSource(Cd,b->device());
	media = new Phonon::MediaObject();
	//connect(media, SIGNAL(finished()), SLOT(slotFinished()));
	media->setCurrentSource(MS->fileName());
}
void HWcontroler ::playSelectedOpticalDrive() // Not finished
{
	media->play();
}
void HWcontroler::ejectSelectedOpticalDrive()
{
	CDDrive[ODactual]->eject();
	kDebug() << CDDriveList[ODactual].udi() << " ejected!";
}
*/
