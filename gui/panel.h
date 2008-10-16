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
#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include <QLabel>

#include <QPalette>
#include "kscdwidget.h"
#include <QGridLayout>

class Panel:public KscdWidget
{
	Q_OBJECT

public:
	explicit Panel(QWidget * parent=0, const QString& sName="panel");
	virtual ~Panel();
	QString getTitle() const;
	QString getAlbum() const;
	QString getAuthor() const;
	QString getVolume() const;
	QString getLoop() const;
	QString getRandom() const;
	QString getInfo() const;

private:
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void setTitleDisplay(const QString & title);


public slots:
	void setTime(qint64 pos);
	void update_panel_label();
	void setTextColor(const QColor& c);
	void setTitle(const QString & title);
	void setAuthor(const QString & author);
	void setAlbum(const QString & album);
	void setVolume(const QString & volume);
	void setLoop(const QString& loop);
	void setRandom(const QString& random);
	void displayInfo(const QString& loop, const QString& random);
	void setTextSizeFont(const QFont& font);
	void setEjectAct(bool b);
	void setVolumeDisplay(qreal volume);

private:
	QLabel * volumeDisplay;
	QGridLayout * vbl_layout;
	QGridLayout * vbl_layoutIntern;
	QLabel * l_title;
	QLabel * l_test;
	int index;
	int timerVolume;
	QColor color;
	QLabel * l_author;
	QLabel * titleTrack;
	QLabel * l_album;
	QLabel * l_playing_state;
	QLabel * l_volume;
	QLabel * l_time;
	QLabel * l_loop;
	QLabel * l_random;
	QLabel * l_info;
	QLabel * textSize;
	QLabel * ejectStatus;
};
#endif
