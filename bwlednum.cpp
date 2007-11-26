/*
   BW_LED_Number a very very primitive LED

   $Id$

   Copyright (c) Bernd Johannes Wuebben <wuebben@math.cornell.edu>

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "bwlednum.h"
#include "stdio.h"

BW_LED_Number::BW_LED_Number(QWidget *parent)
  : QLCDNumber(parent)
{
}

BW_LED_Number::~BW_LED_Number()
{
}

void BW_LED_Number::mouseReleaseEvent(QMouseEvent * e)
{
	emit(clicked());
}

#include "bwlednum.moc"
