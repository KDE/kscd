
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

#ifndef __INEXACT_DIALOG__
#define __INEXACT_DIALOG__

#include <stdlib.h>

#include <qdialog.h>
#include <qfont.h>
#include <qstring.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qstrlist.h>

class InexactDialog : public QDialog {

Q_OBJECT

public:
  InexactDialog(QWidget *parent=0, const char *name=0,bool listbox = true);
  ~InexactDialog();

  void insertList(const QStringList& list);
  void insertText(const QString& str);
  void getSelection(QString& string);

  void setTitle(const QString& t);
  void setErrorString(const QString& t);

private slots:

  void setStatusBar(int i);
  void checkit();

private:
    bool            listbox;
    QPushButton     *ok_button;
    QPushButton     *cancel_button;
    QListBox 	    *list_box;
    QMultiLineEdit  *edit;
    QLabel 	    *statuslabel;
    QLabel 	    *text;
    QString 	    returnstring;
    QString 	    titlestring;
    QString	    errorstring;
};

#endif
