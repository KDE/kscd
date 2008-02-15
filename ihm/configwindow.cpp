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
#include "configwindow.h"
#include <kconfigdialog.h>

ConfigWindow::ConfigWindow(QWidget * parent):QMainWindow(parent)
{

	confPage = new QWidget(this);
	lConfPage = new QVBoxLayout(this);

	confPage->setLayout(lConfPage);

	tab = new QTabWidget(this);


	wButtons= new QWidget(this) ;
	lButtons= new QHBoxLayout(this);
	wButtons->setLayout(lButtons);

	lConfPage->addWidget(tab);
	lConfPage->addWidget(wButtons);


	resize(500,600);
	hwPage = new QWidget(tab);
	panelPage = new QWidget(tab);
	shortcutsPage = new QWidget(tab);

	setHardConfig();
	setPanelConf();

	tab->addTab(hwPage,"Hardware");
	tab->addTab(panelPage,"Panel");
	tab->addTab(shortcutsPage,"Short Nuts");

	setCentralWidget(confPage);



	bOk = new QPushButton("Ok",this);
	bApply = new QPushButton("Apply",this);
	bCancel = new QPushButton("Cancel",this);

	lButtons->addWidget(bOk);
	connect(bOk,SIGNAL(clicked()),this,SLOT(ok()));
	lButtons->addWidget(bApply);
	connect(bApply,SIGNAL(clicked()),this,SLOT(apply()));
	lButtons->addWidget(bCancel);

	

}

ConfigWindow::~ConfigWindow(){
	delete confPage;
	delete lConfPage;
	delete tab;
	delete hwPage;
	delete panelPage;
	delete shortcutsPage;
	delete hwGrid;
	delete panelGrid;
	delete scGrid;
	delete wButtons;
	delete lButtons;
	delete bOk;
	delete bApply;
	delete bCancel;
	delete cbEject;
	delete lEject;
	delete lPanelColor;
	delete cbPanel;
	delete lTextColor;
	delete cbText;
}

void ConfigWindow::setPanelConf(){
	panelGrid = new QGridLayout(panelPage);
	panelPage->setLayout(panelGrid);

	cbPanel = new KColorButton(QColor(Qt::black),this);
	lPanelColor = new QLabel("Choose Panel Color : ",this);

	panelGrid->addWidget(lPanelColor, 0, 0);
	panelGrid->addWidget(cbPanel, 0, 1);

	cbText = new KColorButton(QColor(Qt::white),this);
	lTextColor = new QLabel("Choose Text Color : ",this);

	panelGrid->addWidget(lTextColor, 1, 0);
	panelGrid->addWidget(cbText, 1, 1);

	connect(cbPanel,SIGNAL(changed ( const QColor )),this,SLOT(catchPanelColor()));
	connect(cbText,SIGNAL(changed ( const QColor )),this,SLOT(catchTextColor()));

}
void ConfigWindow::setHardConfig(){
	hwGrid = new QGridLayout(hwPage);
	hwPage->setLayout(hwGrid);

	cbEject = new QCheckBox(this);
	lEject = new QLabel("Eject the CD at the end of the disc",this);

	hwGrid->addWidget(cbEject, 0, 0);
	hwGrid->addWidget(lEject, 0, 1);

	connect(cbEject,SIGNAL(stateChanged ( int )),this,SLOT(catchCBEject()));
	

}
void ConfigWindow::setSCConfig(){

}
void ConfigWindow::apply(){
	for(int index = 0; index < actionsCalled.size(); index++){
		applyAction((actions)actionsCalled[index]);

	}
	actionsCalled.clear();
}
void ConfigWindow::applyAction(actions a){
	switch(a){
		case Eject:
			emit(ejectChanged(cbEject->isChecked())); break;
		case PanelColor:
			emit(panelColorChanged(cbPanel->color()));
			break;
		case TextColor:
			emit(textColorChanged(cbText->color()));
			break;
	}

}
void ConfigWindow::catchCBEject(){
	actionsCalled.append(Eject);
	kDebug()<<"check baby check!";
}
void ConfigWindow::ok(){
	apply();
	hide();
}
void ConfigWindow::catchPanelColor(){
	actionsCalled.append(PanelColor);
	kDebug()<<"user has chosen a new panel color";
}
void ConfigWindow::catchTextColor(){
	actionsCalled.append(TextColor);
	kDebug()<<"user has chosen a new text color";
}
