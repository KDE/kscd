

/*
   Kscd - A simple cd player for the KDE Project

   $Id$

   Copyright (c) 1997 Bernd Johannes Wuebben math.cornell.edu

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


 */

#include <stdio.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "inexact.h"

#include <qlayout.h>


InexactDialog::InexactDialog(QWidget *parent, const char *name,bool _listbox)
  : QDialog(parent, name, TRUE)
{

  setCaption("Kscd");

  QBoxLayout * lay1 = new QVBoxLayout ( this, 10 );
  text = new QLabel(this,"textlabel");
//  text->setAlignment(WordBreak|AlignCenter);
  lay1->addWidget ( text );

  listbox = _listbox;
  if(listbox)
    {
      list_box = new QListBox(this,"debugwindow");
      list_box->setColumnMode(QListBox::FitToWidth);
      lay1->addWidget ( list_box );
      connect(list_box,SIGNAL(highlighted(int)),SLOT(setStatusBar(int)));
    } else {
      edit = new QMultiLineEdit(this,"debugwindow");
      lay1->addWidget ( edit );
   }



  text->setText(i18n("No exact match or multiple exact matches found.\nPlease select the appropriate"\
                     " CD from the list of choices presented below."));
  errorstring = i18n("Please select a Disk Title or press Cancel");


  statuslabel = new QLabel( this, "statuslabel" );
  lay1->addWidget ( statuslabel );
  statuslabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statuslabel->setText( "" );
  statuslabel->setAlignment( AlignCenter );
  //statusPageLabel->setFont( QFont("helvetica",12,QFont::Normal) );

  QBoxLayout * lay2 = new QHBoxLayout ( lay1 );
  lay2->addStretch ( 1 );
  ok_button = new QPushButton(i18n("OK"),this,"ok_button");
  lay2->addWidget ( ok_button );
  lay2->addStretch ( 1 );
  cancel_button = new QPushButton(i18n("Cancel"),this,"cancel_button");
  lay2->addWidget ( cancel_button );
  lay2->addStretch ( 1 );

  if(listbox)
    list_box->setFocus();
  else
    edit->setFocus();
  
  connect(ok_button,SIGNAL(clicked()),SLOT(checkit()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));

  returnstring = "";
} // InexactDialog()


InexactDialog::~InexactDialog()
{
}

void
InexactDialog::setTitle(const QString& t)
{
  titlestring = t;
  text->setText(t);
}

void
InexactDialog::setErrorString(const QString& t)
{
  errorstring = t;
}

void
InexactDialog::checkit()
{
  if(listbox)
    {
      if(list_box->currentItem() == -1)
        {
          KMessageBox::information(this, errorstring);
          return;
        }
      returnstring = list_box->text(list_box->currentItem());
    } else {
      returnstring = edit->text();
    }
  accept();
}

void
InexactDialog::getSelection(QString& string)
{
  string = returnstring;
}


void
InexactDialog::insertList(const QStringList& stringlist)
{
  if(listbox)
    {
      list_box->insertStringList(stringlist,-1);
    }
}

void
InexactDialog::insertText(const QString& str)
{
  if(!listbox)
    {
      edit->setAutoUpdate(FALSE);
      edit->setText(str);
      edit->setAutoUpdate(TRUE);
    }
}

void
InexactDialog::setStatusBar(int i)
{
  returnstring = list_box->text(i);
  statuslabel->setText(returnstring);
}

#include "inexact.moc"
