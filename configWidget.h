/*
 * configWidget - the config dialog page for KSCD settings
 *
 * $Id:
 *
 * Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include "configWidgetUI.h"

class KSCD;

class configWidget : public configWidgetUI
{
   public:
        configWidget(KSCD* player, QWidget* parent = 0, const char* name = 0);
        ~configWidget();

        void apply();

   protected:
        KSCD* mPlayer;

   public slots:
        virtual void digitalPlaybackChckbx_toggled(bool);
};

#endif // CONFIGWIDGET_H
