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
#include "HWcontroler.h"
#include <solid/opticaldisc.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <kdebug.h>

HWcontroler :: HWcontroler ()
{
	CDList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());

	if (CDList.size()<1)
	{
		kDebug() << "No Audio CD inserted";
	}
	else
	{
		for (int i = 0; i < CDList.size();i++)
		{
			kDebug() << CDList[i].udi();
		}
	}
//	Solid::OpticalDisc *disc = device.as<Solid::Processor>();
//	Solid::OpticalDisc * disc = new Solid::OpticalDisc::OpticalDisc();
//	kDebug() << disc->availableContent();
}
HWcontroler :: ~HWcontroler ()
{

}
