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
#ifndef CONFIGWINDOW_H_
#define CONFIGWINDOW_H_

#include <QWidget>
#include <QMainWindow>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QList>
#include <QColor>

#include <kcolorbutton.h>
#include <kdebug.h>


class ConfigWindow:public QMainWindow
{
	Q_OBJECT

	QWidget * confPage;
	QVBoxLayout * lConfPage;

	QTabWidget * tab;
	QWidget * hwPage;
	QWidget * panelPage;
	QWidget * shortcutsPage;

	QGridLayout * hwGrid;
	QGridLayout * panelGrid;
	QGridLayout * scGrid;

	QWidget * wButtons;
	QHBoxLayout * lButtons;

	QPushButton * bOk;
	QPushButton * bApply;
	QPushButton * bCancel;

// Hardware Configuration

	QCheckBox * cbEject;
	QLabel * lEject;

// Panel Configuration
	QLabel * lPanelColor;
	KColorButton * cbPanel;

	QLabel * lTextColor;
	KColorButton * cbText;

	QList<int> actionsCalled;

enum actions{
	Eject = 1,
	PanelColor = 2,
	TextColor = 3
};


public:
	ConfigWindow(QWidget * parent=0);
	virtual ~ConfigWindow();

private:
	void setPanelConf();
	void setHardConfig();
	void setSCConfig();
	void applyAction(actions a);

signals:
	void ejectChanged(bool b);
	void panelColorChanged(QColor c);
	void textColorChanged(QColor c);

private slots:
	void apply();
	void ok();
	void catchCBEject();
	void catchPanelColor();
	void catchTextColor();

};

#endif /*EJECTBUTTON_H_*/
