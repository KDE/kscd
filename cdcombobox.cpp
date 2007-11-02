/*
 * CDComboBox
 *
 * $Id:
 *
 * Copyright (c) 2007 Alexander Kern <alex.kern@gmx.de>
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

#include "cdcombobox.h"
#include <kcombobox.h>

CDComboBox::CDComboBox(QWidget *parent) : KComboBox(parent)
{
    setDuplicatesEnabled(false);
    connect(this, SIGNAL(currentIndexChanged(const QString &)),
        this, SIGNAL(currentDeviceChanged(const QString &)));
}

CDComboBox::~CDComboBox()
{
}

QString CDComboBox::currentDevice(void) const
{
    return currentText();
}

void CDComboBox::addDevices(const QStringList &devs)
{
    addItems(devs);
}

void CDComboBox::setCurrentDevice(const QString &dev)
{
    int index;
    index = findText(dev);
    if(index == -1) {
        addItem(dev);
        index = count() - 1;
    }

    setCurrentIndex(index);
}
