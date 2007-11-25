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

using namespace Phonon;

HWControler :: HWControler ()
{
	mac = 0;
	loadAudioDiscs();
}
HWControler :: ~HWControler ()
{

}

// return the Solid::OpticalDrive of the selected Audio Disc
AudioCD* HWControler ::getAudioCD()
{

}

// set the Audio disc to listen in ce detected audio cds list
void HWControler ::setAudioDisc(int num)
{
	mac = num;
}

// load all Optical discs of the system
void HWControler ::loadAudioDiscs()
{
	kDebug() << "Loading Optical Drives";

	// list all Optical Drives of the system
	detectedDevices = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDrive, QString());

	
	if (detectedDevices.size()<1)
	{
		kDebug() << "No Optical detected!";
	}
	else
	{
		for (int i = 0; i < detectedDevices.size();i++)
		{
			kDebug() << "Optical Disc detected: " << detectedDevices[i].udi();
			kDebug() << "Optical Disc detected: " << detectedDevices[i].parentUdi();
			// Devices to OpticalDrive
			cds.append((AudioCD*)detectedDevices[i].as<Solid::OpticalDisc>());
		}
	}
}

// eject the main Optical Drive
void HWControler ::ejectAudioDisc()
{

}

// play the main Optical Drive --- WARNING: Not finished!
void HWControler ::playAudioCD()
{

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
