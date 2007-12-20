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
 * Stanislas KRZYWDA <stanislas.krzywda@gmail.com>
 * Sovanramy Var <mastasushi@gmail.com>
 * Bouchikhi Mohamed-Amine <bouchikhi.amine@gmail.com>
 * Gastellu Sylvain<sylvain.gastellu@gmail.com>
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
#include "kscdwidget.h"

KscdWidget::KscdWidget(QString sName,QWidget * parent):QSvgWidget(parent)
{
	m_state = Default;
	m_name = sName;

	m_path = "./skin/default/" ;
	
	connect(this,SIGNAL(changePicture(QString)),this,SLOT(load(QString)));
	setMouseTracking ( true );
	emit(changePicture(m_path+ sName+"_n.svg"));
	show();
}

KscdWidget::~KscdWidget()
{
}

QString nameToString(NameButton sName)
{
	QString name;
	switch(sName)
	{
		case Play: 
				name = "play";
				break;
		case Pause:
				name = "pause";
				break;
		case Stop:
				name =  "stop";
				break;
		case Previous: 
				name =  "previous";
				break;
		case Next: 
				name =  "next";
				break;
		case Eject: 
				name =  "eject";
				break;
		case Mute: 
				name =  "mute";
				break;
		case Unmute: 
				name =  "unmute";
				break;
		case Loop: 
				name =  "loop";
				break;
		case LoopTrack: 
				name =  "loopTrack";
				break;
		case LoopDisc: 
				name =  "loopDisc";
				break;
		case Random: 
				name =  "random";
				break;
		case TrackList: 
				name =  "trackList";
				break;
	}
	return name;
}
