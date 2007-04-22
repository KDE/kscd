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

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include "ui_configWidgetUI.h"

class KSCD;

class configWidgetUI : public QWidget, public Ui::configWidgetUI
{
   public:
      explicit configWidgetUI( QWidget *parent ) : QWidget( parent ) {
         setupUi( this );
      }
};


class configWidget : public configWidgetUI
{
   Q_OBJECT

   public:
        explicit configWidget(KSCD* player, QWidget* parent = 0);
        ~configWidget();

   protected:
        KSCD* mPlayer;

   public slots:
       virtual void kcfg_DigitalPlayback_toggled(bool);
       virtual void kcfg_SelectEncoding_toggled(bool);
       virtual void defaults(void);
       virtual void save(void);
   private:
       void load(void);
};

#endif // CONFIGWIDGET_H
