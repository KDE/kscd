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
#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <QWidget>
#include <QTimeLine>
#include <kdebug.h>
// #include "kscdwidget.h"
#include "seekbar.h"
#include "seekcursor.h"

class SeekSlider:public QWidget
{
	Q_OBJECT

public:
	SeekSlider(QWidget * parent=0);
	virtual ~SeekSlider();

	SeekCursor* cursor() const;
	SeekBar* bar() const;

	void setTime(qint64);
	qint64 getTime() const;
	void setTotalTime(qint64);
	void moveC();
	/**
	 * Calculation of move to do in 1 second
	 */
	void  setStep();
	int  getStep() const;

	/**
	 * Initialize all attibutes
	 */
	void init(qint64);

	/**
	 * Start the qtimeline
	 */
	void start(qint64 time);

	/**
	 * Stop the qtimeline
	 */
	void stop();

	/**
	 * Paused the qtimeline
	 */
	void pause();

private:

	/**
	 * The bar of the slider
	 */
	SeekBar* m_bar;

	/**
	 * The cursor of the slider
	 */
	SeekCursor* m_cursor;

	/**
	 * A QTimeLine
	 */
	QTimeLine* m_timeL;

	/**
	 * Current state of the slider
	 */
	QTimeLine::State m_state;

	/**
	 * Current time of the current track
	 */
	qint64 m_time;

	/**
	 * Total time of the current track
	 */
	qint64 m_totalTime;

	/**
	 * Move to do in 1 second
	 */
	int m_step;
public slots:
	/**
	 * Restart the qtimeline after the pause mode
	 */
	void resume(QTimeLine::State state);
};

#endif /*SEEKSLIDER_H_*/
