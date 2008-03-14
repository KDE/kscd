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
#include "seekslider.h"

SeekSlider::SeekSlider(QWidget * parent):QWidget(parent)
{
	m_bar = new SeekBar(parent);
	m_cursor = new SeekCursor(parent);
	setTime(1000);
	m_timeL = new QTimeLine(m_time,parent);
	kDebug()<<"duration"<<m_timeL->duration ();
	setStep(m_time,m_bar->width());
	connect(m_timeL,SIGNAL(valueChanged(qreal)),m_cursor,SLOT(moveCursor(qreal)));
	connect(m_timeL,SIGNAL(stateChanged(QTimeLine::State)),this,SLOT(resume(QTimeLine::State)));
}

SeekSlider::~SeekSlider()
{
}

SeekCursor* SeekSlider :: cursor()
{
	return m_cursor;
}

SeekBar* SeekSlider :: bar()
{
	return m_bar;
}

void SeekSlider :: init(qint64 time)
{
	setTime(time);
	m_timeL->setDuration((int)time);
	m_timeL->setFrameRange (0,m_bar->width());
	m_timeL->setUpdateInterval(1000);
	m_cursor->setStep(m_time,m_bar->width());
	kDebug()<<"m_step"<<m_cursor->getStep();
	kDebug()<<"m_time"<<getTime();
	kDebug()<<"width bar"<<m_bar->width();
	kDebug()<<"duration"<<m_timeL->duration ();
	kDebug()<<"current time"<<m_timeL->currentTime();
	kDebug()<<"update"<<m_timeL->updateInterval();
/*	setStep(m_time,m_bar->width());
	kDebug()<<"step slider"<<getStep();*/
}

void SeekSlider :: start(qint64 time)
{
	init(time);
	m_timeL->start();
	m_state = m_timeL->state();
}

void SeekSlider :: stop()
{
	m_timeL->stop();
	m_state = m_timeL->state();
	m_cursor->init();
}

void SeekSlider :: pause()
{
	m_timeL->setPaused(true);
	m_state = m_timeL->state();
}

void SeekSlider :: resume(QTimeLine::State state)
{	
	kDebug()<<"m_state"<<m_state;
	kDebug()<<"state"<<state;
	if(m_state==QTimeLine::Paused && state==QTimeLine::Running)
	{
		m_timeL->setPaused(false);
		m_state = m_timeL->state();
	}
}
	
void SeekSlider :: setTime(qint64 time)
{
	m_time = time;
}

qint64 SeekSlider :: getTime()
{
	return m_time;
}

void  SeekSlider :: setStep(qint64 time,int length)
{
	m_step = (1000*length)/time;
}

int SeekSlider :: getStep()
{
	return m_step;
}
