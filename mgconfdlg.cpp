
/*
 *
 * kscd -- A simple CD player for the KDE project
 *
 * $Id$
 * 
 * Copyright (C) 1997 Bernd Johannes Wuebben 
 * wuebben@math.cornell.edu
 *
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include <qspinbox.h>

#include "mgconfdlg.h"
#include <klocale.h>


extern KApplication *mykapp;


MGConfigDlg::MGConfigDlg(QWidget *parent, 
			 struct mgconfigstruct *data,const char *name)
  : QDialog(parent, name)
{

  mgconfigdata.width = 320;
  mgconfigdata.height = 200;
  mgconfigdata.brightness = 10;
  QString temp;

  if(data){
    mgconfigdata.width = data->width;
    mgconfigdata.height = data->height;
    mgconfigdata.brightness = data->brightness;
  }

  setCaption(i18n("Magic Kscd"));

  box = new QGroupBox(this, "box");
  box->setGeometry(10,10,520,420);

  label1 = new QLabel(this);
  label1->setGeometry(20,25,155,25);
  label1->setText(i18n("Width of Magic Window:"));

  width_edit = new QLineEdit(this);
  width_edit->setGeometry(200,25,100,25);
  temp.setNum(mgconfigdata.width);
  width_edit->setText(temp);
  connect(width_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(width_changed(const QString &)));  


  label2 = new QLabel(this);
  label2->setGeometry(20,65,155,25);
  label2->setText(i18n("Height of Magic Window:"));

  height_edit = new QLineEdit(this);
  height_edit->setGeometry(200,65,100,25);
  temp.setNum(mgconfigdata.height);
  height_edit->setText(temp);
  connect(height_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(height_changed(const QString &)));  


  label3 = new QLabel(this);
  label3->setGeometry(20,110,155,25);
  label3->setText(i18n("MAGIC Brightness:"));

  bspin = new QSpinBox(0, 100, 1, this);
  bspin->setGeometry(200,110,50,25);
  
  bspin->setValue(mgconfigdata.brightness);
  connect(bspin,SIGNAL(valueChanged(int)),
	  this,SLOT(brightness_changed(int)));  
}




void MGConfigDlg::width_changed(const QString &width) {

  mgconfigdata.width = atoi(width);
}


void MGConfigDlg::height_changed(const QString &height) {

  mgconfigdata.height = atoi(height);
}


void MGConfigDlg::brightness_changed(int value) {

  mgconfigdata.brightness = value;
}


void MGConfigDlg::help(){

  if(mykapp)
    mykapp->invokeHTMLHelp("kscd/kscd.html","");
}


struct mgconfigstruct * MGConfigDlg::getData(){

  return &mgconfigdata;

}

#include "mgconfdlg.moc"



