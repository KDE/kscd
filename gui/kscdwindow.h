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
#ifndef KSCDWINDOW_H
#define KSCDWINDOW_H

#include <QLabel>
#include <QWidget>
#include <QPalette>
#include <QTableWidget>
#include <QStringList>
#include <klocalizedstring.h>
#include <QList>
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
#include "volumebutton.h"
#include "tracklistdlg.h"
#include "closebutton.h"
#include "minimizebutton.h"
#include "background.h"
#include "panel.h"
#include "titlePopUp.h"
#include "seekslider.h"
#include "seekcursor.h"

// #include "cddbmanager.h"
#include "mbmanager.h"

#include <phonon/seekslider.h>
// #include <phonon/volumeslider.h>


#include "prefs.h"


#include <kdebug.h>

class KscdWindow:public QWidget
{
	Q_OBJECT

protected:

	KscdWidget *m_stopB;
	KscdWidget *m_playB;
	KscdWidget *m_prevB;
	KscdWidget *m_nextB;
	KscdWidget *m_ejectB;
	KscdWidget *m_muteB;
	KscdWidget *m_randB;
	KscdWidget *m_loopB;
	KscdWidget *m_trackB;
	VolumeButton *m_volumeB;
	KscdWidget *m_closeB;
	KscdWidget *m_backG;
	KscdWidget *m_miniB;
	KscdWidget *m_prefB;
// 	SeekSlider *m_slider;
	SeekBar *m_bar;
	Phonon::SeekSlider *sslider;
	Panel *m_panel;
	KscdWidget *m_popup;

	TrackListDlg *m_trackDlg;

	QPoint mousePosition;
	bool m_move;
	const QPalette * p_panelColor;

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent * event);
	 /**
	 * Create the track list dialog
	 */
	void createTrackDialog(QList<MBTrackInfo> & trackList,const QString & albumTitle);

	/** The dialog track state : true = visible / false = hide */
	bool m_stateTrackDialog;

	/** The state creation of the track dialog */
	bool m_trackDlgCreated;


public:
	/**
 	* Constructor
	*/
	KscdWindow(QWidget * parent = 0);

	/**
	 * Destructor
	 */
	virtual ~KscdWindow();

//	void paintEvent(QPaintEvent *event);

 	KscdWidget & getPanel() const;
	void setTime(qint64 pos);

public slots:
	void catchButton(QString &);
	void catchVolume(qreal);
	void changePicture(const QString &, const QString &);
	void doubleClickedEvent(int);
	void showArtistLabel(QString &);
	void showTrackinfoLabel(QString &);
	void panelInfo(const QString &);

	/**
	* Refresh skin
	*/
	void setNewSkin(QString &);

	void showArtistAlbum(QString &);
	//void setTime(qint64 pos);

    /**
	 * Close the track list dialog
	 */
	void closeTrackDialog();

signals:
	void actionClicked(const QString &);
	void actionVolume(qreal);
	void trackClicked(int);

};
#endif /*KSCDWINDOW_H_*/
