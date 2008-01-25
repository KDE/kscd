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
#include "volumebutton.h"

VolumeButton::VolumeButton(QWidget * parent,QString sName,qreal value):KscdWidget(sName,parent)
{
  	m_region = new QRegion(x(),y(),x()+width(),y()+height(),QRegion::Ellipse);

	m_vValue = value;
	m_angle = valueToAngle(m_vValue);
	m_centerX = width()/2;
	m_centerY = height()/2;
	m_move = false;
	kDebug()<<"start m_value:"<<m_vValue;
	kDebug()<<"start m_angle:"<<m_angle;
	kDebug()<<"start m_centreX:"<<m_centerX;
	kDebug()<<"start m_centreY:"<<m_centerY;
// 	setEnabled(true);
}

VolumeButton::~VolumeButton()
{
}

// void VolumeButton :: mousePressEvent(QMouseEvent *event)
// {
// 	if(m_region->contains(event->pos()))
// 	{
// 		event->accept();
//  		m_posX = event->x();
//  		m_posY = event->y();
// 	kDebug()<<"press m_posX:"<<m_posX;
// 	kDebug()<<"press m_posY:"<<m_posY;
// 	kDebug()<<"press m_angle:"<<m_angle;
// 		grabMouse(Qt::ClosedHandCursor);
// 		m_move =true;
// 	}
// 	else
// 	{
// 		event->ignore();
// 	}
// }
// 
// void VolumeButton :: mouseMoveEvent(QMouseEvent *event)
// {
// 	if(m_region->contains(event->pos()) && m_move == true)
// 	{
// 		event->accept();
// 	kDebug()<<"move m_posX:"<<m_posX;
// 	kDebug()<<"move m_posY:"<<m_posY;
// 	kDebug()<<"move eventX:"<<event->x();
// 	kDebug()<<"mvoe eventY:"<<event->y();
// 	kDebug()<<"move m_angle:"<<m_angle;
// 		m_deplacement = m_posY - event->y();
// 
// 	kDebug()<<"move m_deplacement:"<<(m_posY - event->y());
// 		if(m_deplacement == 0)
// 		{
// 			m_deplacement = m_posX - event->x();
// 			if(m_posX < m_centerX)
// 			{
// 				
// 				kDebug()<<"move depX:"<<(m_posX - event->x());
// 				if((m_angle + (qreal)m_deplacement) < 0.0)
// 				{
// 					rotation(m_angle - (0.0 - m_angle));
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else if ((m_angle + (qreal)m_deplacement) > 270.0)
// 				{
// 					rotation(m_angle  - (270.0 - m_angle));
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else
// 				{
// 					rotation(m_angle + (qreal)m_deplacement);
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 			}
// 			else
// 			{
// 				if((m_angle + (qreal)m_deplacement) < 0.0)
// 				{
// 					rotation(m_angle - 0.0 - m_angle);
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else if ((m_angle + (qreal)m_deplacement) > 270.0)
// 				{
// 					rotation(m_angle  - (270.0 - m_angle));
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else
// 				{
// 					rotation(m_angle - (qreal)m_deplacement);
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 			}
// 		}
// 		else
// 		{
// 	kDebug()<<"move m_deplacement:"<<(m_posY - event->y());
// 			if(m_posX < m_centerX)
// 			{
// 				
// 				if((m_angle + (qreal)m_deplacement) < 0.0)
// 				{
// 					rotation(m_angle - 0.0 - m_angle);
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else if ((m_angle + (qreal)m_deplacement) > 270.0)
// 				{
// 					rotation(m_angle  - (270.0 - m_angle));
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else
// 				{
// 					rotation(m_angle + (qreal)m_deplacement);
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 			}
// 			else
// 			{
// 				if((m_angle + (qreal)m_deplacement) < 0.0)
// 				{
// 					rotation(m_angle - 0.0 - m_angle);
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else if ((m_angle + (qreal)m_deplacement) > 270.0)
// 				{
// 					rotation(m_angle  - (270.0 - m_angle));
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 				else
// 				{
// 					rotation(m_angle - (qreal)m_deplacement);
// 					emit(volChanged(angleToValue(m_angle)));
// 				}
// 			}
// 		}
// 	}
// 		if (posToAngle(event->x(),event->y()) > m_angle)
// 		{
		/* angle between press position and center axe*/
// 		qreal teta1 = posToAngle(m_posX,m_posY);
		/* angle between move position and center axe*/
// 		qreal teta2 = posToAngle(event->x(),event->y());
		/* move angle */
// 		qreal teta = teta2 - teta1;
// 		kDebug()<<"move teta1:"<<teta1;
// 		kDebug()<<"move teta2:"<<teta2;
// 		kDebug()<<"move teta:"<<teta;
// 		kDebug()<<"move rotation:"<<(180 * (teta))/3.1415;
// 		rotation(m_angle - (180 * (teta))/3.1415);

// 		}
// 	else
// 	{
// 		event->ignore();
// 	}
// }

// void VolumeButton :: mouseReleaseEvent(QMouseEvent *event)
// {
// 	releaseMouse();
// 	m_move = false;
// }

void VolumeButton :: wheelEvent(QWheelEvent *event)
{		
	int pas = (324 / event->delta());
	if((m_angle + pas)>=0 && (m_angle + pas)<=270)
	{
		event->accept();
		m_angle += pas;
		emit(volumeChange(angleToValue(m_angle)));
		rotation(m_angle);
		kDebug()<<"wheel value:"<<angleToValue(m_angle);
	}
	else
	{
		event->ignore();
	}
}


void VolumeButton::rotation (int angle)
{
	m_angle = angle;
	emit(update());
}

qreal VolumeButton::angleToValue(int angle)
{
 	m_vValue = ((qreal)(m_angle)/2.7);
 	return m_vValue;
}

int VolumeButton::valueToAngle(qreal value)
{
 	m_angle = (qreal)(m_vValue * 2.7);
 	return m_angle;
}
// qreal VolumeButton::posToAngle(int x, int y)
// {
// 			kDebug()<<"posToAngle x:"<<x;
// 		kDebug()<<"posToAngle y:"<<y;
// 			kDebug()<<"posToAngle m_centerx:"<<m_centerX;
// 		kDebug()<<"posToAngle m_centery:"<<m_centerY;
// 	qreal adj = abs(x - m_centerX);
// 		kDebug()<<"posToAngle adj:"<<adj;
// 
// 	qreal opp = abs(y- m_centerY);
// 		kDebug()<<"posToAngle opp:"<<opp;
// 
// 	qreal hyp = sqrt(adj*adj + opp*opp);
// 		kDebug()<<"posToAngle hyp:"<<hyp;
// 		kDebug()<<"posToAngle adj/hyp:"<<adj/hyp;
// 	qreal angle = acos(adj/hyp);
// 		kDebug()<<"posToAngle angle:"<<angle;
// 		
// 	return angle;
// }

void VolumeButton :: paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.translate((m_renderer->boundsOnElement(m_id).width())/2,
				(m_renderer->boundsOnElement(m_id).height())/2);
	painter.rotate((qreal)m_angle);
	painter.translate(-(m_renderer->boundsOnElement(m_id).width())/2,
				-(m_renderer->boundsOnElement(m_id).height())/2);
	m_renderer->render(&painter,m_id);
}
