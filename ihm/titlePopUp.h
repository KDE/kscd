/*
 * configWidget - the config dialog page for KSCD settings
 *
 * Copyright (c) 2002 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef TITLEPOPUP_H
#define TITLEPOPUP_H

#include <QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

#include <kdebug.h>


class TitlePopUp : public QWidget
{
private:
	QGridLayout* m_layout;
	QLabel* m_lengthLbl;
	QLabel* m_titleLbl;

public:
	explicit TitlePopUp(QWidget *parent=0);
	~TitlePopUp();

public slots:
	void showTitlePopUp(QString , QString);
	
};

#endif // TITLEPOPUP_H
