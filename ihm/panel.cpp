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
 * it under the terms of the GNU General Public License as published byfor the time
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
#include <QTextEdit>


Panel::Panel(QWidget * parent, QString sName):KscdWidget(sName,parent)
{
	m_bounds = new QRegion((m_renderer->boundsOnElement(getId())).toRect(),QRegion::Rectangle);
	move((m_bounds->boundingRect()).x(),(m_bounds->boundingRect()).y());

	setAutoFillBackground(true); 
	p_panelColor= new QPalette(Qt::transparent);
	setPalette(*p_panelColor);
	vbl_layout = new QGridLayout();
	vbl_layout->setVerticalSpacing(1);

	index=0;
	ejectStatus = new QLabel("");
	titleTrack = new QLabel("");
	l_title = new QLabel("");
	vbl_layout->addWidget(l_title,4,0);
	l_album = new QLabel("<center>WELCOME!</center>");
	vbl_layout->addWidget(l_album,3,0);
	l_author = new QLabel("");
	vbl_layout->addWidget(l_author,2, 0);
	l_loop = new QLabel("");
	l_random = new QLabel("");
	l_info = new QLabel("");
	textSize = new QLabel("");
	vbl_layoutIntern = new QGridLayout();
	vbl_layout->addLayout(vbl_layoutIntern,0,0);
	vbl_layoutIntern->addWidget(l_loop,0,0);
	vbl_layoutIntern->addWidget(l_random,0,1);
	vbl_layoutIntern->addWidget(ejectStatus,0,2);
	l_time = new QLabel("<center><font size="+textSize->text()+"><b>00 : 00</b></font></center>");
	vbl_layout->addWidget(l_time, 6, 0);
	setLayout(vbl_layout);

	QTimer * timer = new QTimer ();
	timer->setSingleShot(false);
	connect(timer,SIGNAL(timeout()),this,SLOT(update_panel_label()));
	timer->start(150);
}

void Panel::update_panel_label(){


	if(l_title->text().count()>0)
	{
		QFont fontActually = l_title->font();
		int pointSize = fontActually.pointSize() ;
		int addSpace;
		switch (pointSize)
		{
			case 4:
				addSpace = 260;
				break;
			case 5:
				addSpace = 250;
				break;
			case 6:
				addSpace = 240;
				break;
			case 7:
				addSpace = 135;
				break;
			case 8:
				addSpace = 125;
				break;
			case 9:
				addSpace = 120;
				break;
			case 10:
				addSpace = 90;
				break;
			case 11:
				addSpace = 80;
				break;
			case 12:
				addSpace = 77;
				break;
			case 13:
				addSpace = 75;
				break;
			case 14:
				addSpace = 72;
				break;
			case 15:
				addSpace = 63;
				break;
			case 16:
				addSpace = 58;
				break;
			case 17:
				addSpace = 55;
				break;
			case 18:
				addSpace = 52;
				break;
			case 19:
				addSpace = 49;
				break;
			default:
				addSpace = 50;
		}
		if(l_title->text().count()!= addSpace)
		{
			l_title->setText(titleTrack->text());
		}
		
		// if the size is lower than the size of the panel
		while(l_title->text().count()< addSpace)
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
		data =data+c;
		setTitleDisplay(& data);
	}
}

Panel::~Panel()
{
	delete 	timer;
	delete vbl_layout;
	delete l_title;
	delete p_panelColor;
	delete ejectStatus;
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
QString Panel::getLoop()
{
	return l_loop->text();
}
QString Panel::getRandom()
{
	return l_random->text();
}
void Panel::setTitleDisplay(QString * title)
{
	l_title->setText(* title);
}
void Panel::setTitle(QString * title)
{
	l_title->setText(* title);
	titleTrack->setText(* title);
}
void Panel::setAuthor(QString * author)
{
	l_author->setText(* author);
}
void Panel::setAlbum(QString * album)
{

	QString mess = * album;
	l_album->setText(mess);

}
void Panel::setVolume(QString * volume)
{
	l_title->setText(*volume);

}

void Panel::setTextSizeFont(QFont font){
	l_author->setFont(font);
	l_title->setFont(font);
	l_album->setFont(font);
	l_time->setFont(font);
	l_loop->setFont(font);
	l_random->setFont(font);
	ejectStatus->setFont(font);
}

void Panel::setTextColor(QColor c){
	color = c;
	QColorGroup grp( QColor( c.red(), c.green(), c.blue() ), Qt::black, QColor( 128, 128, 128 ),
	QColor( 64, 64, 64 ), Qt::black, Qt::darkGreen, Qt::black );
	QPalette pal( grp, grp, grp );
	l_title->setPalette(pal);
	l_author->setPalette(pal);
	l_album->setPalette(pal);
	l_time->setPalette(pal);
	l_loop->setPalette(pal);
	l_random->setPalette(pal);
	ejectStatus->setPalette(pal);
}

void Panel::setEjectAct(bool b){
	if(b)
	{
		ejectStatus->setText("eject CD");
	}
	else
	{
		ejectStatus->setText("");
	} 
	
}

void Panel :: mousePressEvent(QMouseEvent *event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()))
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void Panel :: mouseReleaseEvent(QMouseEvent *event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()))
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void Panel :: enterEvent (QEvent * event)
{
	event->ignore();
}

void Panel :: leaveEvent (QEvent * event)
{
	event->ignore();
}

void Panel::setTime(qint64 pos)
{
	qint64 md = ((pos/1000)/60)/10;
	qint64 mu = ((pos/1000)/60)%10;
	qint64 sd = ((pos/1000)%60)/10;
	qint64 su = ((pos/1000)%60)%10;
	QString result;
	QTextStream(&result) << "<center><b>" << md << mu << " : " << sd << su << "</b></center>";
	l_time->setText(result);
}

void Panel::setLoop(QString loop)
{
	l_loop->setText(loop);
}
void Panel::setRandom(QString random)
{
	l_random->setText(random);
}

//concatenation display info random and loop panel
void Panel::displayInfo(QString loop, QString random)
{
	l_loop->setText(loop);
	l_random->setText(random);
}
QString Panel::getInfo()
{
	return l_info->text();
}