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
#ifndef SEEKCURSOR_H
#define SEEKCURSOR_H

#include <QWidget>
#include <QString>
#include <kdebug.h>
#include "kscdwidget.h"
#include <math.h>
#define SECOND_IN_MILLI	1000

class SeekCursor:public KscdWidget
{
	Q_OBJECT
public:
	SeekCursor(QWidget * parent=0, const QString& sName="seekCursor");
	virtual ~SeekCursor();

	/**
	 * Initialize the cursor to his initial position
	 */
	void init();

	int getStep() const;
	void setStep(qint64,int);
private:
	/**
	 * Initial x position of the cursor
	 */
	int m_posInit;
	/**
	 * Current x position of the cursor
	 */
	int m_posCurrent;
	/**
	 * The step to move the cursor
	 */
	int m_step;
public slots:
	/**
	 * Move the cursor
	 * @param: qreal
	 */
	void moveCursor(qreal);
};

#endif /*SEEKSLIDER_H_*/
