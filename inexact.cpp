

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


 */

#include <stdio.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "inexact.h"

#include <qlayout.h>


InexactDialog::InexactDialog(QWidget *parent, const char *name,bool _listbox)
  : KDialogBase(parent, name, true, "Kscd", Ok|Cancel),
    list_box( 0 ), edit( 0 ), statuslabel( 0 )
{
  QFrame *frame = makeMainWidget();
  QVBoxLayout * lay1 = new QVBoxLayout ( frame, 0, KDialog::spacingHint() );
  text = new QLabel(frame,"textlabel");
//  text->setAlignment(WordBreak|AlignCenter);
  lay1->addWidget ( text );

  if(_listbox)
    {
      list_box = new QListBox(frame,"debugwindow");
      list_box->setColumnMode(QListBox::FitToWidth);
      lay1->addWidget ( list_box );
      connect(list_box,SIGNAL(highlighted(int)),SLOT(setStatusBar(int)));
      connect(list_box,SIGNAL(selected(int)), SLOT(checkit()));
    } else {
      edit = new QMultiLineEdit(frame,"debugwindow");
      lay1->addWidget ( edit );
   }

  text->setText(i18n("No exact match or multiple exact matches found.\n"
        "Please select the appropriate CD from the list of choices "
        "presented below."));
  errorstring = i18n("Please select a Disk Title or press Cancel");

  if ( _listbox )
    {
      statuslabel = new QLabel( frame, "statuslabel" );
      lay1->addWidget ( statuslabel );
      statuslabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
      statuslabel->setText( "" );
      statuslabel->setAlignment( AlignCenter );
    }

  if(list_box)
    list_box->setFocus();
  else
    edit->setFocus();

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
  if(list_box)
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

QString
InexactDialog::selection()
{
  return returnstring;
}


void
InexactDialog::insertList(const QStringList& stringlist)
{
  if(list_box)
    {
      list_box->insertStringList(stringlist,-1);
    }
}

void
InexactDialog::insertText(const QString& str)
{
  if(!list_box)
    {
      edit->setAutoUpdate(false);
      edit->setText(str);
      edit->setAutoUpdate(true);
    }
}

void
InexactDialog::setStatusBar(int i)
{
  if ( list_box )
    {
      returnstring = list_box->text(i);
      statuslabel->setText(returnstring);
    }
}

#include "inexact.moc"
