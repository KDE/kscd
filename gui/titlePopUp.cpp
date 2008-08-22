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
 * Gastellu Sylvain <sylvain.gastellu@gmail.com>
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

#include "titlePopUp.h"
#include <QTimer>

TitlePopUp::TitlePopUp(QWidget *parent, const QString& sName) : KscdWidget(sName,parent)
{
	move(parent->width()/2,parent->height()/2);

	m_layout = new QGridLayout;
/*	resize(300,64);*/

// 	kDebug() << "size : " << size();
//
// 	move(600,400);
	setAutoFillBackground(false);
	m_lengthLbl = new QLabel(this);
//	m_lengthLbl->setGeometry(QRect(20, 30, 241, 22));
	m_titleLbl = new QLabel(this);
//	m_titleLbl->setGeometry(QRect(20, 10, 201, 22));

	m_layout->addWidget(m_titleLbl, 0, 0, Qt::AlignVCenter);
	m_layout->addWidget(m_lengthLbl, 1, 0, Qt::AlignVCenter);


	setLayout(m_layout);
	raise();

}

TitlePopUp::~TitlePopUp()
{
	delete m_layout;
	delete m_lengthLbl;
	delete m_titleLbl;
}

/**
* show a popup containning current track title and his length
*/
void TitlePopUp::showTitlePopUp(const QString& trackTitle, const QString& trackLength)
{
	QTimer::singleShot(5000, this, SLOT(hideTitlePopUp()));
	m_lengthLbl->setText(trackLength);
	m_titleLbl->setText(trackTitle);
	show();
}

void TitlePopUp::hideTitlePopUp()
{
	hide();
}

