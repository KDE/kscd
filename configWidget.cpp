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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "configWidget.h"
#include "kscd.h"

#include <kaccel.h>
#include <kcolorbutton.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a configWidget which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
configWidget::configWidget(KSCD* player, QWidget* parent, const char* name)
    : QWidget(parent, name),
      mPlayer(player)
{
    if (!name)
    {
    	setName("configWidget");
    }

    resize(425, 297); 
    configWidgetLayout = new QGridLayout(this, 1, 1, 6, 5, "configWidgetLayout"); 

    GroupBox1 = new QGroupBox(this, "GroupBox1");
    GroupBox1->setFrameShape(QGroupBox::Box);
    GroupBox1->setFrameShadow(QGroupBox::Sunken);
    GroupBox1->setTitle(i18n("Interface"));
    GroupBox1->setColumnLayout(0, Qt::Vertical);
    GroupBox1->layout()->setSpacing(KDialog::spacingHint());
    GroupBox1->layout()->setMargin(KDialog::marginHint());
    GroupBox1Layout = new QGridLayout(GroupBox1->layout());
    GroupBox1Layout->setAlignment(Qt::AlignTop);

    bgColorBtn = new KColorButton(GroupBox1, "bgColorBtn");
    bgColorBtn->setColor(mPlayer->bgColor());

    GroupBox1Layout->addWidget(bgColorBtn, 1, 1);

    ledColorBtn = new KColorButton(GroupBox1, "ledColorBtn");
    ledColorBtn->setColor(mPlayer->ledColor());

    GroupBox1Layout->addWidget(ledColorBtn, 0, 1);

    TextLabel2 = new QLabel(GroupBox1, "TextLabel2");
    TextLabel2->setText(i18n("&Background color:"));
    TextLabel2->setBuddy(bgColorBtn);
    GroupBox1Layout->addWidget(TextLabel2, 1, 0);

    TextLabel1 = new QLabel(GroupBox1, "TextLabel1");
    TextLabel1->setText(i18n("&LCD color:"));
    TextLabel1->setBuddy(ledColorBtn);
 
    GroupBox1Layout->addWidget(TextLabel1, 0, 0);

    showToolTips = new QCheckBox(GroupBox1, "showToolTips");
    showToolTips->setText(i18n("Show &tool tips"));
    showToolTips->setChecked(mPlayer->toolTips());

    GroupBox1Layout->addMultiCellWidget(showToolTips, 2, 2, 0, 1);

    systrayChckbx = new QCheckBox(GroupBox1, "systrayChckbx");
    systrayChckbx->setText(i18n("Show icon in &system tray"));
    systrayChckbx->setChecked(mPlayer->dock());

    GroupBox1Layout->addMultiCellWidget(systrayChckbx, 3, 3, 0, 1);

    configWidgetLayout->addWidget(GroupBox1, 0, 0);

    GroupBox3 = new QGroupBox(this, "GroupBox3");
    GroupBox3->setTitle(i18n("Play Options"));
    GroupBox3->setColumnLayout(0, Qt::Vertical);
    GroupBox3->layout()->setSpacing(KDialog::spacingHint());
    GroupBox3->layout()->setMargin(KDialog::marginHint());
    GroupBox3Layout = new QVBoxLayout(GroupBox3->layout());
    GroupBox3Layout->setAlignment(Qt::AlignTop);

    Layout1 = new QHBoxLayout(0, 0, 6, "Layout1"); 

    TextLabel1_2 = new QLabel(GroupBox3, "TextLabel1_2");
    TextLabel1_2->setText(i18n("Skip &interval:"));
    Layout1->addWidget(TextLabel1_2);

    skipInterval = new QSpinBox(GroupBox3, "skipInterval");
    skipInterval->setSuffix(i18n(" sec"));
    skipInterval->setMaxValue(1000);
    skipInterval->setMinValue(1);
    skipInterval->setValue(mPlayer->skipInterval());
    Layout1->addWidget(skipInterval);
    QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    Layout1->addItem(spacer);
    GroupBox3Layout->addLayout(Layout1);
    
    TextLabel1_2->setBuddy(skipInterval);

    autoplayChkbx = new QCheckBox(GroupBox3, "autoplayChkbx");
    autoplayChkbx->setText(i18n("Auto&play when CD inserted"));
    autoplayChkbx->setChecked(mPlayer->autoPlay());
    GroupBox3Layout->addWidget(autoplayChkbx);

    ejectChkbx = new QCheckBox(GroupBox3, "ejectChkbx");
    ejectChkbx->setText(i18n("&Eject CD when finished playing"));
    ejectChkbx->setChecked(mPlayer->ejectOnFinish());
    GroupBox3Layout->addWidget(ejectChkbx);

    stopOnExitChckbx = new QCheckBox(GroupBox3, "stopOnExitChckbx");
    stopOnExitChckbx->setText(i18n("Stop playing on e&xit"));
    stopOnExitChckbx->setChecked(mPlayer->stopOnExit());
    GroupBox3Layout->addWidget(stopOnExitChckbx);

    configWidgetLayout->addWidget(GroupBox3, 0, 1);
    
    ButtonGroup1 = new QButtonGroup(this, "ButtonGroup1");
    ButtonGroup1->setTitle(i18n("Random Play Mode"));
    ButtonGroup1->setColumnLayout(0, Qt::Vertical);
    ButtonGroup1->layout()->setSpacing(KDialog::spacingHint());
    ButtonGroup1->layout()->setMargin(KDialog::marginHint());
    ButtonGroup1Layout = new QVBoxLayout(ButtonGroup1->layout());
    ButtonGroup1Layout->setAlignment(Qt::AlignTop);

    randomShuffleRadio = new QRadioButton(ButtonGroup1, "randomShuffleRadio");
    randomShuffleRadio->setText(i18n("Shu&ffle order and play each song only once"));
    randomShuffleRadio->setChecked(mPlayer->randomOnce());
    ButtonGroup1Layout->addWidget(randomShuffleRadio);

    randomSelectRadio = new QRadioButton(ButtonGroup1, "randomSelectRadio");
    randomSelectRadio->setText(i18n("&Randomly select songs (may repeat)"));
    randomSelectRadio->setChecked(!mPlayer->randomOnce());
    ButtonGroup1Layout->addWidget(randomSelectRadio);

    configWidgetLayout->addMultiCellWidget(ButtonGroup1, 1, 1, 0, 1);

    GroupBox2 = new QGroupBox(this, "GroupBox2");
    GroupBox2->setTitle(i18n("CD-ROM &Device"));
    GroupBox2->setColumnLayout(0, Qt::Vertical);
    GroupBox2->layout()->setSpacing(KDialog::spacingHint());
    GroupBox2->layout()->setMargin(KDialog::marginHint());
    GroupBox2Layout = new QVBoxLayout(GroupBox2->layout());
    cdDevice = new KURLRequester(mPlayer->devicePath(), GroupBox2);
    GroupBox2Layout->addWidget(cdDevice);

    configWidgetLayout->addMultiCellWidget(GroupBox2, 2, 2, 0, 1);

    spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    configWidgetLayout->addItem(spacer, 3, 1);
}

configWidget::~configWidget()
{
}

void configWidget::apply()
{
    mPlayer->setColors(ledColorBtn->color(), bgColorBtn->color());
    mPlayer->setToolTips(showToolTips->isChecked());
    mPlayer->setDocking(systrayChckbx->isChecked());
    mPlayer->setSkipInterval(skipInterval->value());
    mPlayer->setAutoplay(autoplayChkbx->isChecked());
    mPlayer->setEjectOnFinish(ejectChkbx->isChecked());
    mPlayer->setStopOnExit(stopOnExitChckbx->isChecked());
    mPlayer->setRandomOnce(randomShuffleRadio->isChecked());
    
    if (cdDevice->lineEdit()->edited())
    {
        mPlayer->setDevicePath(cdDevice->lineEdit()->text());
    }
}
