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
 * Dembele Karim <>
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

#ifndef TITLEPOPUP_H
#define TITLEPOPUP_H

#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "kscdwidget.h"
#include <kdebug.h>


class TitlePopUp : public KscdWidget
{
private:
	QGridLayout* m_layout;
	QLabel* m_lengthLbl;
	QLabel* m_titleLbl;

public:
	TitlePopUp(QWidget *parent=0, const QString& sName=QLatin1String( "popup" ));
	~TitlePopUp();

	void enterEvent (QEvent * event){event->ignore();}
	void leaveEvent (QEvent * event){event->ignore();}
	void mousePressEvent(QMouseEvent * event){event->ignore();}
	void mouseReleaseEvent(QMouseEvent * event){event->ignore();}

public slots:
	void showTitlePopUp(const QString& , const QString&);
	void hideTitlePopUp();
};

#endif // TITLEPOPUP_H
