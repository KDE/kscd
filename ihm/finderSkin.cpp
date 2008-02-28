/*
 * finderSkin - the dialog page for skins change
 *
 * $Id:
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

#include "finderSkin.h"

/*
 *  Constructs a finderSkin which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FinderSkin::FinderSkin(QWidget* parent):finderSkinUI(parent)
{
	kDebug()<<"creation finderSkin";

	newSkin= new QString();
	skinFound=false;

	clearButton->setEnabled(false);
	connect(buttonBox,SIGNAL(accepted()), SLOT(accept())); 
	connect(buttonBox,SIGNAL(rejected()), SLOT(reject()));
	connect(browserButton,SIGNAL(clicked()), SLOT(showBrowser()));
	connect(clearButton,SIGNAL(clicked()),SLOT(clearBrowser()));
}

FinderSkin::~FinderSkin()
{
	delete newSkin;
}

void FinderSkin::accept(){
	if(skinFound){
		emit(pathSkinChanged(*newSkin));
	}
	skinFound=false;
	newSkin->clear();
	lTitleSkin->clear();
	lTitleSkin->setText("Choose a new skin");
	this->hide();
	clearButton->setEnabled(false);
}

void FinderSkin::reject(){
	kDebug()<<"cancel clicked";
	skinFound=false;
	newSkin->clear();
	lTitleSkin->clear();
	lTitleSkin->setText("Choose a new skin");
	this->hide();
	clearButton->setEnabled(false);
}

void FinderSkin::showBrowser(){
	kDebug()<<"browser";
	QFileDialog fileDlg(this,"Find a new skin", "/home", NULL);
	fileDlg.setFileMode(QFileDialog::ExistingFile);
	fileDlg.setFilter(tr("SVG Files (*.svg)"));
	fileDlg.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if(fileDlg.exec()) fileNames= fileDlg.selectedFiles();
	kDebug()<<"Noms choisis:"<<fileNames;
 	

	if(!fileNames.empty()){
		skinFound=true;
		delete(newSkin);
		newSkin=new QString(fileNames.first());	
		lTitleSkin->setText(((fileNames.first()).split("/")).back());
		clearButton->setEnabled(true);
	}else{
		kDebug()<<"Aucun file choisi";
	}
}

void FinderSkin::clearBrowser(){
	kDebug()<<"clearBrowserclicked";
	skinFound=false;
	newSkin->clear();
	lTitleSkin->clear();
	lTitleSkin->setText("Choose a new skin");
	clearButton->setEnabled(false);
}