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
#ifndef VOLUMEBUTTON_H_
#define VOLUMEBUTTON_H_

#include <QWidget>
#include <QAbstractSlider>
#include <QString>
#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <QPainter>
#include <kdebug.h>
#include "kscdwidget.h"
#include <QWheelEvent>
#include <math.h>

class VolumeButton:public KscdWidget
{
	Q_OBJECT

private:
	qreal m_vValue;
	int m_angle;
	int m_posX;
	int m_posY;
	int m_centerX;
	int m_centerY;
	int m_deplacement;
	bool m_move;

private:
	qreal posToAngle(int x, int y);
	qreal angleToValue(int);
	int valueToAngle(qreal);
	void rotation(int);
public:
	VolumeButton(QWidget * parent=0, QString sName="volume", qreal value=50.0);
	virtual ~VolumeButton();
// 	void mousePressEvent(QMouseEvent * event);
// 	void mouseReleaseEvent(QMouseEvent * event);
// 	void mouseMoveEvent(QMouseEvent * event);
	void wheelEvent(QWheelEvent * event);
	void paintEvent(QPaintEvent* event);
signals:
	void volumeChange(qreal);
};

#endif /*VOLUMEBUTTON_H_*/
