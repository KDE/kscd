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

#include "audiocd.h"

#include <solid/opticaldisc.h>
#include <solid/device.h>
#include <solid/opticaldrive.h>
#include <solid/block.h>
#include <solid/devicenotifier.h>

#include <kdebug.h>

#include <phonon/mediasource.h>

#include <QString>

using namespace Phonon;
AudioCD::AudioCD()
{
    cdDrive = NULL;
    cd = NULL;
    block = NULL;
    src = NULL;
}

AudioCD::AudioCD(Solid::Device aCd)
{

	odsign=aCd;

	bell=Solid::DeviceNotifier::instance();

	// get the opticaldrive
	cdDrive=aCd.as<Solid::OpticalDrive>();

	connect(cdDrive,SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),this,SLOT(catchEjectPressed()));
	connect(bell,SIGNAL(deviceAdded(const QString)),this,SLOT(reloadCD()));

	cd = NULL;
	block = NULL;
	src = NULL;

	// look for an opticaldisc inserted in this drive
	QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());

	if (devList.isEmpty())
	{
		kDebug() << "No Optical Disc detected in the computer!";
		cd = NULL;
		block = NULL;
		src = NULL;

	}
	else
	{
		for (int i = 0; i < devList.size();i++)
		{
			if (devList[i].parentUdi()==odsign.udi())
			{
				cd = devList[i].as<Solid::OpticalDisc>();
				block = odsign.as<Solid::Block>();
				src = new MediaSource(Cd,block->device());
			}
		}
	}


/*
	cdDrive = Solid::Device(aCd.parentUdi()).as<Solid::OpticalDrive>();
	block = Solid::Device(aCd.parentUdi()).as<Solid::Block>();
	src = new MediaSource(Cd,block->device());
*/
}

AudioCD::~AudioCD()
{
}

Solid::OpticalDrive * AudioCD::getCdDrive() const
{
	return cdDrive;
}

Solid::OpticalDisc * AudioCD::getCd() const
{
	return cd;
}

Phonon::MediaSource * AudioCD::getMediaSource() const
{
	return src;
}

QString AudioCD::getCdPath() const
{
	return block->device();
}

bool AudioCD::isCdInserted() const
{
	return (cd!=NULL);
}

void AudioCD::catchEjectPressed()
{
	kDebug() << "#o#o#o#o#o#o#o#o#o#o#o#o#o#o#Eject Pressed!";

	cd = NULL;
	block = NULL;
	src = NULL;

	emit(discChanged ());
}

void AudioCD::reloadCD()
{
	// look for an opticaldisc inserted in this drive
	QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());

	if (devList.isEmpty())
	{
		kDebug() << "No Optical Disc detected in the computer!";
		cd = NULL;
		block = NULL;
		src = NULL;
		emit(discChanged ());

	}
	else
	{
		for (int i = 0; i < devList.size();i++)
		{
			if (devList[i].parentUdi()==odsign.udi())
			{
				kDebug() << "CD inserted!";
				cd = devList[i].as<Solid::OpticalDisc>();
				block = odsign.as<Solid::Block>();
				src = new MediaSource(Cd,block->device());
				emit(discChanged ());
			}
		}
	}

}

QString AudioCD::signature() const{
	return odsign.udi();
}
