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
#include "kscdwindow.h"

KscdWindow::KscdWindow(QString skinPath):QWidget()
{
 	//setMaximumSize ( 650,200 );
 	m_layout = new QGridLayout;
	m_layout->setSizeConstraint(QLayout::SetFixedSize);

	m_stopB = new StopButton(this);
	m_playB = new PlayButton(this);
	m_nextB = new NextButton(this);
	m_prevB = new PreviousButton(this);
	m_ejectB = new EjectButton(this);
	m_muteB = new MuteButton(this);
	m_randB = new RandomButton(this);
	m_loopB = new LoopButton(this);
	m_trackB = new TrackListButton(this);

 	m_layout->addWidget(m_ejectB, 0, 1);
 	m_layout->addWidget(m_prevB, 1, 0);
 	m_layout->addWidget(m_playB, 1, 1);
 	m_layout->addWidget(m_nextB, 1, 2);
 	m_layout->addWidget(m_stopB, 2, 1);
	m_layout->addWidget(m_randB, 3, 0,Qt::AlignCenter);
	m_layout->addWidget(m_loopB, 3, 2,Qt::AlignCenter);
	m_layout->addWidget(m_muteB, 3, 1,Qt::AlignCenter);
	m_layout->addWidget(m_trackB, 3, 3,Qt::AlignCenter);
 	setLayout(m_layout);

	show();
}

KscdWindow::~KscdWindow()
{
	delete m_playB;
	delete m_stopB;
	delete m_nextB;
	delete m_prevB;
	delete m_ejectB;
	delete m_muteB;
	delete m_layout;
}

QString KscdWindow::getSkinPath()
{
	return m_skinPath;
}
void KscdWindow::setSkinPath(QString sPath)
{
	m_skinPath = sPath;
}
