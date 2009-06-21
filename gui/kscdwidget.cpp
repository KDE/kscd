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
#include "kscdwidget.h"
#include <QPainter>
#include <QRegion>
#include <QPixmap>
#include <QBitmap>
#include "klocale.h"
KscdWidget::KscdWidget(const QString& sName,QWidget * parent):QWidget(parent)
{
	m_state = "default";
 	m_name = sName;
	m_baseName = m_name;
	m_id = m_name + '_' + m_state;

	m_path = Prefs::url();

	m_renderer = new QSvgRenderer(this);
	loadSkin(m_path);
	if (m_renderer->elementExists(m_id))
	{
		setFixedSize(m_renderer->boundsOnElement(m_id).width(),
				m_renderer->boundsOnElement(m_id).height());

		connect(this, SIGNAL(needRepaint()),this, SLOT(repaint()));
		connect(this,SIGNAL(changePicture()),this,SLOT(update()));
		setMouseTracking ( true );
	}
}

KscdWidget::~KscdWidget()
{
    delete m_bounds;
	delete m_renderer;
}

void KscdWidget::setName(const QString & sName)
{
	m_name = sName;
}


QString  KscdWidget::getName() const
{
	return m_name;
}

QString  KscdWidget::getState() const
{
	return m_state;
}

void KscdWidget::setId(const QString & name,const QString & state)
{
	m_id = name + '_' + state;
}


QString KscdWidget::getId() const
 {
 	return m_id;
 }

void KscdWidget::loadPicture(const QString & name,const QString & state)
{
	m_id= name + '_' + state;
	emit(changePicture());
	emit(needRepaint());
}

void KscdWidget::paintEvent(QPaintEvent *event)
{

	QPainter painter(this);

	if (m_renderer->elementExists(m_id))
		m_renderer->render(&painter,m_id);
}

void KscdWidget::enterEvent (QEvent * event )
{
	event->accept();
	m_state = "over";
	m_id = m_name + '_' + m_state;
	emit(needRepaint());
	setToolTip( i18n( qPrintable( m_name ) ) );
}

void KscdWidget::leaveEvent (QEvent * event )
{
	event->accept();
	m_state = "default";
	m_id = m_name + '_' + m_state;
	emit(needRepaint());
}


void KscdWidget::mousePressEvent(QMouseEvent *event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()))
	{
		event->accept();
		kDebug() << "**** button name : " << m_name << " ****";
		m_state = "pressed";
		m_id = m_name + '_'+ m_state;
		emit(needRepaint());
	}
	else
	{
		event->ignore();
	}
}

void KscdWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(m_bounds->contains(event->pos()+(m_bounds->boundingRect()).topLeft()))
	{
		event->accept();
		m_state = "over";
		m_id = m_name + '_' + m_state;
		emit(buttonClicked(m_name));
		emit(needRepaint());
	}
	else
	{
		event->ignore();
	}
}

QString KscdWidget::getPath() const
{
	return m_path;
}

QRegion* KscdWidget::bounds() const
{
	return m_bounds;
}

QPixmap KscdWidget::getPix() const
{
	return pix;
}

void KscdWidget::loadSkin(const QString & skin)
{
	QString newId = m_baseName + "_default";
	m_path = skin;
	if (!m_renderer->load(skin))
	{
		// TODO We should make kconfig_compiler create a setDefaultUrl or something
		KConfigSkeletonItem *urlItem = Prefs::self()->findItem("url");
		if (urlItem)
		{
			urlItem->setDefault();
			m_path = Prefs::url();
			m_renderer->load(skin);
		}
	}
	if (m_renderer->elementExists(m_id)){
		QRectF rect = m_renderer->boundsOnElement(newId);
		resize(rect.width(),rect.height());
		pix = QPixmap(rect.toRect().size());
		pix.fill(QColor(Qt::transparent));
		QPainter p(&pix);
		m_renderer->render(&p,newId,rect);
		m_bounds = new QRegion(pix);
		//setMask( pix.mask() );
		move(rect.toRect().x(),rect.toRect().y());
	}
}
