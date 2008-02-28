/*
 * finderSkin - the dialog page for skins change
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

#ifndef FINDERSKIN_H
#define FINDERSKIN_H

#include "ui_finderSkinUI.h"


#include <QDialog>
#include <QtGui>

#include <kdebug.h>

class finderSkinUI : public QDialog, public Ui::finderSkinUI
{
   public:
      finderSkinUI( QWidget *parent ) : QDialog( parent ) {
         setupUi( this );
      }
};

class FinderSkin : public finderSkinUI
{
   Q_OBJECT
   private:
	QString * newSkin;
	bool skinFound;


   public:
 	/**Create an instance of FinderSkin */
	FinderSkin(QWidget* parent = 0);

	/**Destroy an instance of FinderSkin */
        ~FinderSkin();


   public slots:
	/**ok action*/
	void accept();
	/** cancel action */
	void reject();
	/** open a new browser to find out new svg files on hard disc */
	void showBrowser();
	/** clear the new skin**/
	void clearBrowser();


   signals:
	/**Send new skin path*/ 
	void pathSkinChanged(QString);

};

#endif // FINDERSKIN_H
