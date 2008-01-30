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
#ifndef KSCDWIDGET_H_
#define KSCDWIDGET_H_

#include <QSvgRenderer>
#include <QWidget>
#include <QRegion>
#include <QMouseEvent>
#include <KStandardDirs>
#include <QPainter>
#include <QString>
#include <kdebug.h>

class KscdWidget:public QWidget
{
	Q_OBJECT

protected:
	QRegion *m_region;
	QString m_state;
	QString m_name;
	QString m_file;
	QString m_path;
	QString m_id;
	QSvgRenderer *m_renderer;
	
	void paintEvent(QPaintEvent *event);

public:
	KscdWidget(QString sName,QWidget * parent=0);
	virtual ~KscdWidget();
	void setName(QString);
	QString getName();
	void setId(QString,QString);
	QString getId();
	void loadPicture(QString,QString);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
	
signals:
	void needRepaint();
	void changePicture();
	void buttonClicked(QString);
};

#endif /*KSCDWIDGET_H_*/
