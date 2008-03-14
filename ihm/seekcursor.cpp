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
#include "seekcursor.h"

SeekCursor::SeekCursor(QWidget * parent,QString sName):KscdWidget(sName,parent)
{
	m_bounds = new QRegion((m_renderer->boundsOnElement(getId())).toRect(),QRegion::Ellipse);
	move((m_bounds->boundingRect()).x(),(m_bounds->boundingRect()).y());
	m_posInit = x();
	init();
}

SeekCursor::~SeekCursor()
{
}

int SeekCursor :: getStep()
{
	return m_step;
}

void SeekCursor :: setStep(qint64 time,int lenght)
{

	m_step = ceil((SECOND_IN_MILLI * (float)lenght)/(float)time);
	kDebug()<<"m_step:"<<m_step;
}

void SeekCursor :: init()
{
	m_posCurrent = m_posInit;
	move(m_posCurrent,y());
}

void SeekCursor :: moveCursor(qreal pos)
{
	m_posCurrent = m_posCurrent + getStep();
	move(m_posCurrent,y());
}
