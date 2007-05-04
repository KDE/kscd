
/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Richard Moore (moorer@cs.man.ac.uk)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef LED_LAMP_H
#define LED_LAMP_H

#include <QFrame>


class LedLamp : public QFrame
{
  Q_OBJECT
public:

  enum Type { Rect, Loop };

  explicit LedLamp(QWidget *parent=0, Type t=Rect);

  enum State { On, Off };


  State state() const { return s; }


  void setState(State state) { s= state; repaint(); }


  void toggleState() { if (s == On) s= Off; else if (s == Off) s= On; repaint(); }
public slots:
  void toggle() { toggleState(); }
  void on() { setState(On); }
  void off() { setState(Off); }
protected:
  void drawContents(QPainter *);
private:
  const int w;
  const int h;
  const int dx;
  State s;
  int ledtype;
};


#endif
