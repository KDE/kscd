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


Panel::Panel(QWidget * parent):QWidget(parent)
{

	setAutoFillBackground(true); 
	p_panelColor= new QPalette(Qt::black);
	setPalette(*p_panelColor);
	vbl_layout = new QVBoxLayout;

	index=0;
	l_title = new QLabel("WELCOME!");

	vbl_layout->addWidget(l_title);
	l_album = new QLabel();
	vbl_layout->addWidget(l_album);
	l_author = new QLabel("                                                     ");
	vbl_layout->addWidget(l_author);
	l_volume = new QLabel();
	vbl_layout->addWidget(l_volume);
	l_time = new QLabel();
	vbl_layout->addWidget(l_time);

	setLayout(vbl_layout);


	QTimer * timer = new QTimer ();
	timer->setSingleShot(false);
	connect(timer,SIGNAL(timeout()),this,SLOT(update_panel_label()));
	timer->start(150);
}

void Panel::update_panel_label(){


	
	// if the size is lower than the size of the panel
	while(l_title->text().count()< 53)
	{
		//add  " " to have the same size that the panel
		l_title->setText(l_title->text()+" ");
	}
	
	//recup the first letter
	QChar c = l_title->text().data()[0];
	
	//create a new data
	QString data;
	for(int i = 1; i <l_title->text().count(); i++)
	{
		data = data+l_title->text().data()[i];
	}

	
	//add the last letter
	data = data+c;
	l_title->setText(data);
}

Panel::~Panel()
{
	delete 	timer;
	delete vbl_layout;
	delete l_title;
	delete p_panelColor;

	delete l_author;
	delete l_album;
	delete l_playing_state;
	delete l_volume;
	delete l_time;
}

QString Panel::getTitle()
{
	return l_title->text();
}
QString Panel::getAlbum()
{
	return l_album->text();

}
QString Panel::getAuthor()
{
	return l_author->text();

}
QString Panel::getVolume()
{
	return l_volume->text();

}
void Panel::setTitle(QString * title)
{
	l_title->setText(*title);
}
void Panel::setAuthor(QString * author)
{
	l_title->setText(*author);

}
void Panel::setAlbum(QString * album)
{
	l_title->setText(*album);

}
void Panel::setVolume(QString * volume)
{
	l_title->setText(*volume);

}
void Panel::setTime(QString * time)
{
	l_time->setText(*time);

}
void Panel::setPanelColor(QColor c){
	const QPalette *p = new QPalette(c);
	setPalette(*p);


	//p_panelColor->setColor(QPalette::Background,c);
}
void Panel::setTextColor(QColor c){

	QPalette p(c);
	p.setColor(QPalette::Text,c);
	setPalette(p);
	//p_panelColor->setColor(QPalette::Text,c);

}