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
#include <solid/opticaldrive.h>

#include <phonon/mediasource.h>
#include <phonon/mediaobject.h>


class HWcontroler : public QObject
{

	private:

// Contains all Optical Drive of the system, but in a Solid::Device list
	QList<Solid::Device> CDDriveList;

// Contains all Optical Drive of the system, but in a Solid::OpticalDrive list
	QList<Solid::OpticalDrive*> CDDrive;

// Number of the main Optical Drive (as default)
	int ODactual;

// Actual Media source
	Phonon::MediaSource *MS;

// Selected Optical Drive is manipulated with media
	Phonon::MediaObject *media;
	

	public:
		HWcontroler ();
		~HWcontroler ();

	// return the Solid::OpticalDrive of the selected Optical Drive
		Solid::OpticalDrive* getSelectedOpticalDrive();

	// num selected an optical drive in the CDDrive list and put it as the main
		void setSelectedOpticalDrive(int num);

	// load all Optical discs of the system
		void loadCDDevices();

	// Load the Optical Drive controler
		void loadReading();

	// eject the main Optical Drive
		void ejectSelectedOpticalDrive();

	// play the main Optical Drive --- WARNING: Not finished!
		void playSelectedOpticalDrive();
};

#endif
