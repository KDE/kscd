/*
 * Kscd - A simple cd player for the KDE Project

   $Id: kscd.h 764078 2008-01-20 23:18:46Z bouchikhi $

 * Copyright (c) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
 * Copyright (c) 2002 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>

 --------------
 ISI KsCD Team :
 --------------
 * Audureau Jérôme
 * Bouchikhi Mohamed-Amine <bouchikhi.amine@gmail.com>
 * Gastellu Sylvain<sylvain.gastellu@gmail.com>
 * KRZYWDA Stanislas <stanislas.krzywda@gmail.com>
 * Rapicault Emilie
 * Var Sovanramy <mastasushi@gmail.com>
 * ------------------------------------------------------

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __PANEL__
#define __PANEL__

#include <QLabel>
#include <QWidget>
#include <QObject>


class Panel:public QObject
{
	Q_OBJECT

	private:
		
	// Title of the track
		QLabel * lTrackTitle;
	// Author name
		QLabel * lAuthor;
	// Album title
		QLabel * lAlbumTitle;
	// Volume text
		QLabel * lVolume;
	// TrackState (play,pause...)
		QLabel * lTrackState;
	// Loop mode activated
		QLabel * lLoopMode;
	// Random activated
		QLabel * lRandomMode;
	// Time Status
		QLabel * lTime;

	public:
		Panel(QWidget *parent = 0);
		~Panel();
}

#endif
