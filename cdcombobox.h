/*
 * cCDComboBox
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

#ifndef CDCOMBOBOX_H
#define CDCOMBOBOX_H

#include <kcombobox.h>

class CDComboBox : public KComboBox
{
    Q_OBJECT
    Q_PROPERTY( QString currentDevice READ currentDevice WRITE setCurrentDevice USER true )

    public:
        explicit CDComboBox(QWidget *parent = 0);
        ~CDComboBox();
        QString currentDevice(void) const;
        void addDevices(const QStringList &);

Q_SIGNALS:
        void currentDeviceChanged(const QString &);

public Q_SLOTS:
        void setCurrentDevice(const QString &);

};

#endif // CDCOMBOBOX_H
