
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include <stdio.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qcolor.h>
#include "ledlamp.h"
#include "ledlamp.moc"

LedLamp::LedLamp(QWidget *parent, Type t) : QFrame(parent),
  width( 10 ), height( 7 ), dx( 4 )
{
  // Make sure we're in a sane state
  s = Off;

  // Set the frame style
  //  setFrameStyle(Sunken | Box);
  setGeometry(0,0,height+1,width+1);
  ledtype = t;
} // LedLamp

void 
LedLamp::drawContents(QPainter *painter)
{

  QBrush lightBrush(this->foregroundColor());
  QBrush darkBrush(this->backgroundColor());

  //  QColor redColor(255,100,100);
  //  QBrush redBrush(redColor);

  QPen darkPen(this->backgroundColor(),1);
  QPen lightPen(this->foregroundColor(), 1);

  switch(s) 
    {
    case On:
      painter->setBrush(lightBrush);
      switch (ledtype)
	{
	case Rect:
	  painter->drawRect(1,1,width-3, height-2);
	  break;
	case Loop:
	  painter->setBrush(lightBrush);
	  painter->setPen(lightPen);

	  //	  painter->drawRect(0,0,width,height);

	  painter->drawLine(0, 2, 0, height-2); //  |
	  painter->drawLine(1, 1, width-4, 1); // ~
	  painter->drawLine(width-3, 2, width-3, height-2); // |
	  painter->drawLine(2, height-2, width-3, height-2); //_
	  painter->drawLine(width-6,0,width-6,2); // ---+
	  painter->drawLine(3,height-2,3,height); // +---
	  break;
	}
      break;
      
    case Off:
      painter->setBrush(darkBrush);
      switch (ledtype)
	{
	case Rect:
	  painter->drawRect(1,1,width-3, height-2);
	  break;
	case Loop:
	  painter->setBrush(darkBrush);
	  painter->setPen(darkPen);

	  painter->drawLine(0, 2, 0, height-2); //  |
	  painter->drawLine(1, 1, width-4, 1); // ~
	  painter->drawLine(width-3, 2, width-3, height-2); // |
	  painter->drawLine(2, height-2, width-3, height-2); //_
	  painter->drawLine(width-6,0,width-6,2); // ---+
	  painter->drawLine(3,height-2,3,height); // +---
	  break;
	}
      //      painter->setPen(pen);
      //    painter->drawLine(2,2,width-2, 2);
      //painter->drawLine(2,height-2,width-2,height-2);
      // Draw verticals
      //int i;
      //for (i= 2; i < width-1; i+= dx)
      //painter->drawLine(i,2,i,height-2);
      break;
      
    default:
    fprintf(stderr, "LedLamp: INVALID State (%d)\n", s);
    
    }
} // drawContents

