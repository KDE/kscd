/*
 * Kscd - A simple cd player for the KDE Project
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
#ifndef __AUDIOCD__
#define __AUDIOCD__

#include <solid/opticaldisc.h>
#include <solid/device.h>
#include <solid/opticaldrive.h>
#include <solid/block.h>
#include <solid/devicenotifier.h>
#include <QString>
#include <phonon/mediasource.h>

#include <QString>
#include <QObject>


class AudioCD: public QObject
{
	Q_OBJECT

	private:
		Solid::DeviceNotifier * bell;
		Solid::Device odsign;
		Solid::OpticalDrive *cdDrive;
		Solid::OpticalDisc *cd;
		Solid::Block *block;
		Phonon::MediaSource *src;

	public:
		AudioCD();
		AudioCD(Solid::Device aCd);
		~AudioCD();
		Solid::OpticalDrive * getCdDrive() const;
		Solid::OpticalDisc * getCd() const;
		Phonon::MediaSource * getMediaSource() const;
		QString getCdPath() const ;
		bool isCdInserted();
		QString signature() const;

	public slots:
		void catchEjectPressed();
		void reloadCD();

	signals:
		void discChanged ();
};

#endif
