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
#ifndef PANEL_H_
#define PANEL_H_

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <QLCDNumber>
#include <QPalette>
#include "kscdwidget.h"

class Panel:public KscdWidget
{
	Q_OBJECT
private:

	QTimer * timer;
	QGridLayout * vbl_layout;
	QLabel * l_title;
	QLabel * l_test;
	int index;
	QColor color;
	const QPalette * p_panelColor;
	QLabel * l_author;
	QLabel * l_album;
	QLabel * l_playing_state;
	QLabel * l_volume;
	QLabel * l_time;
	QLabel * l_loop;
	QLabel * l_random;
	QLabel * l_info;
	QLabel * textSize;
public:
	Panel(QWidget * parent=0, QString sName="panel");
	virtual ~Panel();

	QString getTitle();
	QString getAlbum();
	QString getAuthor();
	QString getVolume();
	QString getLoop();
	QString getRandom();
	QString getInfo();
private:
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);


public slots:
	void setTime(qint64 pos);
	void update_panel_label();
	void setTextColor(QColor c);
	void setTitle(QString * title);
	void setAuthor(QString * author);
	void setAlbum(QString * album);
	void setVolume(QString * volume);
	void setLoop(QString loop);
	void setRandom(QString random);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void displayInfo(QString loop, QString random);
	void setTextSize(QString size);
	void setTextSizeFont(QFont font);
};
#endif 
