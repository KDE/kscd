/******************************************************************************
 * Copyright (C) 2008 Amine Bouchikhi <bouchikhi.amine@gmail.com>             *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "RootDBusHandler.h"
#include "RootAdaptor.h"
#include "kscd.h"

// Marshall the DBusVersion data into a D-BUS argument
QDBusArgument &operator<<(QDBusArgument &argument, const Version &version)
{
    argument.beginStructure();
    argument << version.major << version.minor;
    argument.endStructure();
    return argument;
}

// Retrieve the DBusVersion data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, Version &version)
{
    argument.beginStructure();
    argument >> version.major >> version.minor;
    argument.endStructure();
    return argument;
}


namespace KsCD
{

    RootDBusHandler::RootDBusHandler(KSCD * kscd)
        : QObject()
    {
    	
    	player = kscd;
    	    	new RootAdaptor( this );

    	    		QDBusConnection::sessionBus().registerObject( QLatin1String( "/Player" ), this);

    	        setObjectName( QLatin1String("RootDBusHandler" ));
    }

    QString RootDBusHandler::Identity()
    {
        return QString();
    }

    void RootDBusHandler::Quit()
    {

    }

    Version RootDBusHandler::MprisVersion()
    {
        struct Version version;
        version.major = 1;
        version.minor = 0;
        return version;
    }

}
