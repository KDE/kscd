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

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KColorButton;
class KURLRequester;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class QSpinBox;
class KSCD;

class configWidget : public QWidget
{ 
   public:
        configWidget(KSCD* player, QWidget* parent = 0, const char* name = 0);
        ~configWidget();

        void apply();

    protected:
        QGroupBox* GroupBox1;
        KColorButton* bgColorBtn;
        KColorButton* ledColorBtn;
        QLabel* TextLabel2;
        QLabel* TextLabel1;
        QCheckBox* showToolTips;
        QCheckBox* systrayChckbx;
        QGroupBox* GroupBox3;
        QLabel* TextLabel1_2;
        QSpinBox* skipInterval;
        QCheckBox* autoplayChkbx;
        QCheckBox* ejectChkbx;
        QCheckBox* stopOnExitChckbx;
        QGroupBox* GroupBox2;
        QButtonGroup* ButtonGroup1;
        QRadioButton* randomShuffleRadio;
        QRadioButton* randomSelectRadio;
        KURLRequester* cdDevice;

        QGridLayout* configWidgetLayout;
        QGridLayout* GroupBox1Layout;
        QVBoxLayout* GroupBox2Layout;
        QVBoxLayout* GroupBox3Layout;
        QHBoxLayout* Layout1;
        QVBoxLayout* ButtonGroup1Layout;
        
        KSCD* mPlayer;
};

#endif // CONFIGWIDGET_H
