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
#include "background.h"

BackGround::BackGround(QWidget * parent,QString sName):KscdWidget(sName,parent)

{
// 	kDebug()<<"default size:"<<m_renderer->defaultSize();
// 	kDebug()<<"background size:"<<(m_renderer->boundsOnElement(getId())).toRect().size();
// 	pix = ((m_renderer->boundsOnElement(getId())).toRect().size());
// 	
// // 	pix.setMask(pix.createHeuristicMask());
// 	QPainter painter(&pix);
// 	painter.setBackgroundMode(Qt::OpaqueMode);
// // pix.setMask(pix.createMaskFromColor(Qt::lightGray,Qt::MaskOutColor));
// //  	kDebug()<<"background size:"<<(m_renderer->boundsOnElement(getId())).toRect().size();
// // 	QBitmap pix((m_renderer->boundsOnElement(m_id)).toRect().size());
// // 	QPixmap pix((m_renderer->boundsOnElement(getId())).toRect().size());
// // // 	m_bitmap = QBitmap((m_renderer->boundsOnElement(getId())).toRect().size());
// // // 	kDebug()<<"bitmap size:"<<m_bitmap.size();
// // 	QPainter painter(&pix);
// 	m_renderer->render(&painter,getId());
// 	pix.setMask(pix.createHeuristicMask());
//  	m_bounds = new QRegion(&pix);
// 	setMask(pix);
// 	m_bounds = new QRegion(pix);
	m_bounds = new QRegion((m_renderer->boundsOnElement(getId())).toRect(),QRegion::Ellipse);
	move((m_bounds->boundingRect()).x(),(m_bounds->boundingRect()).y());
	m_move = false;
// 	update();
}

BackGround::~BackGround()
{
}

void BackGround :: mousePressEvent(QMouseEvent *event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()) 
		&& event->button() == Qt::LeftButton)
	{
		event->accept();
		mousePosition = event->pos();
		m_move =true;
		grabMouse(Qt::SizeAllCursor);
	}
	else
	{
		event->ignore();
	}
}

void BackGround :: mouseReleaseEvent(QMouseEvent *event)
{
	releaseMouse();
	m_move = false;
}

void BackGround :: mouseMoveEvent(QMouseEvent * event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()) && m_move == true)
	{
		event->accept();
		emit(moveValue(event->globalPos() - mousePosition));
	}
	else
	{
		event->ignore();
	}
}

void BackGround :: enterEvent (QEvent * event)
{
	event->ignore();
}

void BackGround :: leaveEvent (QEvent * event)
{
	event->ignore();
}

