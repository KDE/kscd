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

#include "panel.h"
#include <kdebug.h>
#include <QTimer>

Panel::Panel(QWidget * parent):QGroupBox(parent)
{
	vbl_layout = new QVBoxLayout;
	timer = new QTimer ();
	timer->setSingleShot(false);
	index=0;
	l_title = new QLabel("title");
	QString str = l_title->text();
	if(str.count() >53)
	{
		str = str+"     ";
	}
	l_title->setText(str);
	vbl_layout->addWidget(l_title);
	setLayout(vbl_layout);
	connect(timer,SIGNAL(timeout()),this,SLOT(update_panel_label()));
	timer->start(1);
}

void Panel::update_panel_label(){

	QString str = l_title->text();
	int nb =  str.count();
	// to stop the timer if the size of the title is lower than the panel
	if( nb <= 53)
	{
		index++;
		if( index == 53)
		{
			timer->stop();
		}
	}
	
	// if the size is lower than the size of the panel
	while(nb< 53)
	{
		//add  " " to have the same size that the panel
		str = str+" ";
		nb++;
	}
	
	//recup the first letter
	QChar c = str.data()[0];
	
	//create a new data
	QString data;
	for(int i = 1; i < nb; i++)
	{
		data = data+str.data()[i];
	}

	
	//add the last letter
	data = data+c;
	l_title->setText(data);
}

Panel::~Panel()
{
	delete this;
}


