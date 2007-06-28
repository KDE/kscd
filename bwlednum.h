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

#ifndef BW_LED_NUM_H
#define BW_LED_NUM_H

#include <QLCDNumber>
#include <QMouseEvent>


class BW_LED_Number : public QLCDNumber
{
    Q_OBJECT

public:

    BW_LED_Number(QWidget *parent = 0);
   ~BW_LED_Number();

signals:
    void clicked();

protected:
    virtual void mouseReleaseEvent(QMouseEvent *);
};


#endif // BW_LED_NUM_H
