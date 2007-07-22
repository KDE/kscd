/*  This file is part of the KDE libraries
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

#include "ledlamp.h"

#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <QColor>

LedLamp::LedLamp(QWidget *parent, Type t) : QFrame(parent), 
  w( 10 ), h( 7 ), dx( 4 )
{
  // Make sure we're in a sane state
  s = Off;

  // Set the frame style
  //  setFrameStyle(Sunken | Box);
  //setGeometry(0,0,h+1,w+1);
  ledtype = t;
} // LedLamp

void 
LedLamp::drawContents(QPainter *painter)
{
  //QBrush lightBrush(palette().color(foregroundRole()));
  //QBrush darkBrush(palette().color(backgroundRole()));

  //QPen darkPen(palette().color(backgroundRole()), 1);
  //QPen lightPen(palette().color(foregroundRole()), 1);

  QBrush lightBrush(palette().color(backgroundRole()));
  QBrush darkBrush(palette().color(foregroundRole()));

  QPen darkPen(palette().color(foregroundRole()), 1);
  QPen lightPen(palette().color(backgroundRole()), 1);

    switch(s) {
    case On:
        painter->setBrush(lightBrush);
        switch (ledtype) {
        case Rect:
            painter->drawRect(1,1,w-3, h-2);
            break;
        case Loop:
            painter->setBrush(lightBrush);
            painter->setPen(lightPen);

            painter->drawLine(0, 2, 0, h-2); //  |
            painter->drawLine(1, 1, w-4, 1); // ~
            painter->drawLine(w-3, 2, w-3, h-2); // |
            painter->drawLine(2, h-2, w-3, h-2); //_
            painter->drawLine(w-6,0,w-6,2); // ---+
            painter->drawLine(3,h-2,3,h); // +---
            break;
        }
        break;
    case Off:
        painter->setBrush(darkBrush);
        switch (ledtype) {
        case Rect:
            painter->drawRect(1,1,w-3, h-2);
            break;
        case Loop:
            painter->setBrush(darkBrush);
            painter->setPen(darkPen);

            painter->fillRect(0,0,w,h, darkBrush);
            /*
            painter->drawLine(0, 2, 0, h-2); //  |
            painter->drawLine(1, 1, w-4, 1); // ~
            painter->drawLine(w-3, 2, w-3, h-2); // |
            painter->drawLine(2, h-2, w-3, h-2); //_
            painter->drawLine(w-6,0,w-6,2); // ---+
            painter->drawLine(3,h-2,3,h); // +---
            */
            break;
        }
        break;
    default:
        fprintf(stderr, "LedLamp: INVALID State (%d)\n", s);
    }
} // drawContents

#include "ledlamp.moc"
