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
#ifndef KSCDWIDGET_H
#define KSCDWIDGET_H

#include <QSvgRenderer>
#include <QWidget>
#include <QRegion>
#include <QMouseEvent>
#include <QString>
#include <kdebug.h>
#include "prefs.h"


class KscdWidget:public QWidget
{
	Q_OBJECT
public:

	explicit KscdWidget(const QString& sName,QWidget * parent=0);
	virtual ~KscdWidget();
	void setName(const QString &);
	QString getName() const;
	QString getState() const;
	void setId(const QString &,const QString &);
	QString getId() const;
	void loadPicture(const QString &,const QString &);
	void enterEvent (QEvent * event);
	void leaveEvent (QEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	QString getPath() const;
	QRegion* bounds() const;
	QPixmap getPix() const;
	void rotation(qreal);

protected:
	void paintEvent(QPaintEvent *event);

public:
	void loadSkin(const QString &);

signals:
	void needRepaint();
	void changePicture();
	void buttonClicked(const QString &);

protected:
    QRegion *m_bounds;
	QString m_state;
	QString m_name;
	QString m_id;
	QSvgRenderer *m_renderer;
private:
	QPixmap pix;
	QString m_file;
	QString m_path;
	QString m_baseName;

};

#endif /*KSCDWIDGET_H_*/
