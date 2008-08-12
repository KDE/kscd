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
#include "mutebutton.h"

MuteButton::MuteButton(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
// 	m_bounds = new QRegion((m_renderer->boundsOnElement(getId())).toRect(),QRegion::Ellipse);
// 	move((m_bounds->boundingRect()).x(),(m_bounds->boundingRect()).y());
// 	m_bounds=new QRegion(pix.mask());
}

MuteButton::~MuteButton()
{
}

void MuteButton :: mousePressEvent(QMouseEvent *event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()))
	{
		event->accept();
		m_state = "pressed";
		if(m_name== "mute")
		{
			m_id = m_name + "_" + m_state;
			emit(needRepaint());
		}
		else
		{
			m_id = m_name + "_" + m_state;
			emit(needRepaint());
		}
	}
	else
	{
		event->ignore();
	}
}

void MuteButton :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()))
	{
		event->accept();
		m_state = "over";
		if(m_name=="mute")
		{
			m_name = "unmute";
			m_id = m_name + "_" + m_state;
			emit(buttonClicked(m_name));
		}
		else
		{
			m_name = "mute";
			m_id = m_name + "_" + m_state;
			emit(buttonClicked(m_name));
		}
	}
}
