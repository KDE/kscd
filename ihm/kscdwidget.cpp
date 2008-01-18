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

KscdWidget::KscdWidget(QString sName,QWidget * parent):QWidget(parent)
{
	m_state = "default";
 	m_name = sName;
	m_id = m_name + "_" + m_state;
	m_file = "default.svg";
	
	m_path = KStandardDirs::installPath("data") + "/kscd/skin/" + m_file;
	
	m_renderer = new QSvgRenderer(m_path,this);
	setFixedSize(m_renderer->boundsOnElement(m_id).width(),
			m_renderer->boundsOnElement(m_id).height());
 	
	connect(this, SIGNAL(needRepaint()),this, SLOT(repaint()));
	connect(this,SIGNAL(changePicture()),this,SLOT(update()));
	setMouseTracking ( true );
}

KscdWidget::~KscdWidget()
{
}

void KscdWidget :: setName(QString sName)
{
	m_name = sName;
}


QString KscdWidget :: getName()
{
	return m_name;
}

void KscdWidget :: setId(QString name,QString state)
{
	m_id = name + "_" + state;
}


QString KscdWidget :: getId()
 {
 	return m_id;
 }

void KscdWidget :: loadPicture(QString name,QString state)
{
	m_id= name + "_" + state;
	emit(changePicture());
	emit(needRepaint());
}

void KscdWidget :: paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	m_renderer->render(&painter,m_id);
}