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
#ifndef KSCDWINDOW_H_
#define KSCDWINDOW_H_

#include <QLabel>
#include <QWidget>
#include <QGridLayout>
#include <QLayout>
#include <QSlider>
#include <QLineEdit>
#include <QSizePolicy>
#include <QLabel>

#include "kscdwidget.h"
#include "stopbutton.h"
#include "playbutton.h"
#include "nextbutton.h"
#include "previousbutton.h"
#include "ejectbutton.h"
#include "mutebutton.h"
#include "randombutton.h"
#include "loopbutton.h"
#include "tracklistbutton.h"
#include "ihmnamespace.h"

#include <phonon/seekslider.h>

using namespace IHM;

class KscdWindow:public QWidget
{
	Q_OBJECT

private:
	QGridLayout *m_layout;
	KscdWidget *m_stopB;
	KscdWidget *m_playB;
	KscdWidget *m_prevB;
	KscdWidget *m_nextB;
	KscdWidget *m_ejectB;
	KscdWidget *m_muteB;
	KscdWidget *m_randB;
	KscdWidget *m_loopB;
	KscdWidget *m_trackB;
	
	QLabel *m_artistLabel ;

	QLabel * time;

	QString m_skinPath;
public:
	KscdWindow(QString sPath="./skin/default");
	virtual ~KscdWindow();
	QString getSkinPath();
	void setSkinPath(QString sPath);
	
	QLabel *getArtistLabel();
	void setArtistLabel(QString artist);

	// Sorry Stan it's for my tests :p
	void addSeekSlider(Phonon::SeekSlider *ss);

public slots:
	void catchButton(QString);
	void changePicture(QString,StateButton);
	void setTime(qint64 pos);
signals:
	void actionClicked(QString);

};

#endif /*KSCDWINDOW_H_*/
