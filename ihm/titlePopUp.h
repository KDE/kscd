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

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QWidget>


class TitlePopUp : public QWidget
{
   public:
       QLabel *lengthLbl;
       QLabel *titleLbl;

      explicit TitlePopUp( QWidget *parent ) : QWidget( parent ) {
        setObjectName(QString::fromUtf8("titlePopUp"));
	setWindowFlags ( Qt::FramelessWindowHint );
        resize(296, 64);
        setCursor(QCursor(Qt::PointingHandCursor));
        setContextMenuPolicy(Qt::NoContextMenu);
        setAutoFillBackground(false);
        lengthLbl = new QLabel(this);
        lengthLbl->setObjectName(QString::fromUtf8("lengthLbl"));
        lengthLbl->setGeometry(QRect(20, 30, 241, 22));
        titleLbl = new QLabel(this);
        titleLbl->setObjectName(QString::fromUtf8("titleLbl"));
        titleLbl->setGeometry(QRect(20, 10, 201, 22));

      }
};


#endif // TITLEPOPUP_H
