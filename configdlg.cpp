
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



#include "configdlg.h"
#include <klocale.h>

#include <qlayout.h>
#include <qfontmetrics.h>

ConfigDlg::ConfigDlg(QWidget *parent, struct configstruct *data,const char *name)
  : QDialog(parent, name)
{

  configdata.background_color = black;
  configdata.led_color = green;
  configdata.tooltips = true;
  configdata.cd_device = QString::fromLatin1("/dev/cdrom");
  configdata.mailcmd = QString::fromLatin1("mail -s \"%s\" ");
  configdata.docking = true;
  configdata.autoplay = false;
  configdata.stopexit = true;
  configdata.ejectonfinish = false;

  if(data)
    {
      configdata.background_color = data->background_color;
      configdata.randomonce = true;
      configdata.led_color = data->led_color;
      configdata.tooltips = data->tooltips;
      configdata.cd_device = data->cd_device;
      configdata.mailcmd = data->mailcmd;
      configdata.browsercmd = data->browsercmd;
      configdata.use_kfm = data->use_kfm;
      configdata.docking = data->docking;
      configdata.autoplay = data->autoplay;
      configdata.stopexit = data->stopexit;
      configdata.ejectonfinish = data->ejectonfinish;
    }

  colors_changed = false;

  configdata.randomonce = data->randomonce;
  setCaption(i18n("Configure kscd"));

  QVBoxLayout * lay1 = new QVBoxLayout ( this, 10 );
  box = new QGroupBox(this, "box");
  lay1->addWidget ( box );
  
  QFontMetrics fm ( font() );
  
  QHBoxLayout * lay2 = new QHBoxLayout ( box, 10 );
  QVBoxLayout * lay3 = new QVBoxLayout ( lay2 );
  QGroupBox * cpbox = new QGroupBox ( i18n("Colors and paths"), box );
  lay3->addWidget ( cpbox );
  QGridLayout * cplay = new QGridLayout ( cpbox, 5, 3, 10 );
  cplay->addRowSpacing ( 0, fm.lineSpacing() );

  label1 = new QLabel(i18n("LED Color:"), cpbox);
  cplay->addWidget ( label1, 1, 0 );
  qframe1 = new QFrame(cpbox);
  qframe1->setFixedSize(30,25);
  cplay->addWidget ( qframe1, 1, 1, AlignLeft );
  qframe1->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe1->setBackgroundColor(configdata.led_color);
  button1 = new QPushButton(i18n("Change"),cpbox);
  cplay->addWidget ( button1, 1, 2 );
  connect(button1,SIGNAL(clicked()),this,SLOT(set_led_color()));

  label2 = new QLabel(i18n("Background Color:"),cpbox);
  cplay->addWidget ( label2, 2, 0 );
  qframe2 = new QFrame(cpbox);
  qframe2->setFixedSize(30,25);
  cplay->addWidget ( qframe2, 2, 1, AlignLeft );
  qframe2->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe2->setBackgroundColor(configdata.background_color);
  button2 = new QPushButton(i18n("Change"),cpbox);
  cplay->addWidget ( button2, 2, 2 );
  connect(button2,SIGNAL(clicked()),this,SLOT(set_background_color()));

  label5 = new QLabel(i18n("CDROM Device:"), cpbox);
  cplay->addWidget ( label5, 3, 0 );
  cd_device_edit = new QLineEdit(cpbox);
  cplay->addMultiCellWidget ( cd_device_edit, 3,3, 1,2 );
  cd_device_edit->setText(configdata.cd_device);
  connect(cd_device_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(device_changed(const QString &)));

#if defined(sun) || defined(__sun__) || defined(__osf__) || defined(ultrix) || defined(__ultrix)

  label5->hide();
  cd_device_edit->hide();

#endif

  label6 = new QLabel(i18n("Unix mail command:"),cpbox);
  cplay->addWidget ( label6, 4, 0 );
  mail_edit = new QLineEdit(cpbox);
  cplay->addMultiCellWidget ( mail_edit, 4,4, 1,2 );
  mail_edit->setText(configdata.mailcmd);
  connect(mail_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(mail_changed(const QString &)));

  browserbox = new  QButtonGroup(i18n("WWW Browser"),box,"wwwbox");
  lay3->addWidget ( browserbox );
  QVBoxLayout * lay4 = new QVBoxLayout ( browserbox, 10 );
  lay4->addSpacing ( fm.lineSpacing() );

  kfmbutton = new QRadioButton(i18n("Use Konqueror as Browser"),
			       browserbox,"kfmbutton");
  lay4->addWidget ( kfmbutton );
  kfmbutton->setChecked(configdata.use_kfm);
  connect(kfmbutton,SIGNAL(clicked()),this,SLOT(kfmbutton_clicked()));

  custombutton = new QRadioButton(i18n("Use Custom Browser:"),
				  browserbox,"custombutton");
  lay4->addWidget ( custombutton );
  custombutton->setChecked(!configdata.use_kfm);
  connect(custombutton,SIGNAL(clicked()),this,SLOT(custombutton_clicked()));

  QBoxLayout * lay5 = new QHBoxLayout ( lay4 );
  lay5->addSpacing ( 20 );
  custom_edit = new QLineEdit(browserbox,"customedit");
  custom_edit->setText(data->browsercmd);
  custom_edit->setEnabled(!configdata.use_kfm);
  lay5->addWidget ( custom_edit, 1 );

  QBoxLayout * lay6 = new QVBoxLayout ( lay2 );
  QGroupBox * miscbox = new QGroupBox ( 1, Horizontal, i18n("Misc"), box );
  lay6->addWidget ( miscbox );

  ttcheckbox = new QCheckBox(i18n("Show Tool Tips"),
			     miscbox, "tooltipscheckbox");
  ttcheckbox->setChecked(configdata.tooltips);
  connect(ttcheckbox,SIGNAL(clicked()),this,SLOT(ttclicked()));

  dockcheckbox = new QCheckBox(i18n("Enable KPanel Docking"),
			       miscbox, "dockcheckbox");
  dockcheckbox->setChecked(configdata.docking);
  connect(dockcheckbox,SIGNAL(clicked()),this,SLOT(dockclicked()));

  cdAutoPlayCB = new QCheckBox(i18n("Play on Tray Close"),
                               miscbox, "cdAutoPlayCB");
  cdAutoPlayCB->setChecked(configdata.autoplay);
  connect(cdAutoPlayCB, SIGNAL(clicked()), this, SLOT(autoPlayClicked()));

  stopOnExitCB = new QCheckBox(i18n("Stop Playing on Exit"),
                               miscbox, "stopOnExitCB");
  stopOnExitCB->setChecked(configdata.stopexit);
  connect(stopOnExitCB, SIGNAL(clicked()), this, SLOT(stopOnExitClicked()));

  ejectOnFinishCB = new QCheckBox(i18n("Eject on Finish"),
                                  miscbox, "ejectOnFinishCB");
  ejectOnFinishCB->setChecked(configdata.ejectonfinish);
  connect(ejectOnFinishCB, SIGNAL(clicked()), this, SLOT(ejectOnFinishClicked()));


  /* koz: Added a configure option to select the unique random play mode, */
  /* or the traditional random mode */
  randomOnceCB = new QCheckBox(i18n("Random is Shuffle"),
			     miscbox, "randomOnceCB");
  randomOnceCB->setChecked(configdata.randomonce);
  connect(randomOnceCB,SIGNAL(clicked()),this,SLOT(randomOnceClicked()));

  button3 = new QPushButton(i18n("Help"),box);
  lay6->addWidget ( button3, 0, AlignRight );
  connect(button3,SIGNAL(clicked()),this,SLOT(help()));

  lay1->addStretch ( 1 );
}


void 
ConfigDlg::custombutton_clicked()
{
    configdata.use_kfm = false;
    custom_edit->setEnabled(!configdata.use_kfm);
} // custombutton_clicked


void 
ConfigDlg::kfmbutton_clicked()
{
    configdata.use_kfm = true;
    custom_edit->setEnabled(!configdata.use_kfm);
} // kfmbutton_clicked


void 
ConfigDlg::okbutton() 
{
} // okbutton

void 
ConfigDlg::device_changed(const QString &dev) {

  configdata.cd_device = dev;
} // device_changed

void 
ConfigDlg::mail_changed(const QString &dev) {

  configdata.mailcmd = dev;
} // mail_changed

void 
ConfigDlg::ttclicked(){

  if(ttcheckbox->isChecked())
    configdata.tooltips = TRUE;
  else
    configdata.tooltips = FALSE;
} // ttclicked

void 
ConfigDlg::dockclicked(){

    if(dockcheckbox->isChecked()){
        configdata.docking = TRUE;
    }else{
        configdata.docking = FALSE;
    }
} // dockclicked

void 
ConfigDlg::autoPlayClicked(){
    if(cdAutoPlayCB->isChecked())
        configdata.autoplay = TRUE;
    else
        configdata.autoplay = FALSE;
} // autoPlayClicked


void 
ConfigDlg::stopOnExitClicked()
{
    if(stopOnExitCB->isChecked())
        configdata.stopexit = TRUE;
    else
        configdata.stopexit = FALSE;
} // stopOnExitClicked

void 
ConfigDlg::ejectOnFinishClicked()
{
    if(ejectOnFinishCB->isChecked())
        configdata.ejectonfinish = TRUE;
    else
        configdata.ejectonfinish = FALSE;
} // ejectOnFinishClicked

void 
ConfigDlg::help()
{
    kapp->invokeHTMLHelp("kscd/kscd.html","");
} // help

void 
ConfigDlg::cancelbutton() 
{
  reject();
} // cancelbutton

void 
ConfigDlg::set_led_color()
{
  KColorDialog::getColor(configdata.led_color);
  qframe1->setBackgroundColor(configdata.led_color);
} // set_led_color

void 
ConfigDlg::set_background_color()
{

  KColorDialog::getColor(configdata.background_color);
  qframe2->setBackgroundColor(configdata.background_color);
} // set_background_color

void 
ConfigDlg::randomOnceClicked()
{
  if(randomOnceCB->isChecked())
    configdata.randomonce = TRUE;
  else
    configdata.randomonce = FALSE;
} // randomOnceClicked

struct configstruct * 
ConfigDlg::getData()
{
  configdata.browsercmd = custom_edit->text();
  return &configdata;
} // getData

#include "configdlg.moc"




