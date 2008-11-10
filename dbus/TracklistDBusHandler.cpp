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

#include "TracklistDBusHandler.h"
#include "TracklistAdaptor.h"
#include "dbus/PlayerDBusHandler.h"

#include <KUrl>
#include <KSelectAction>


namespace KsCD
{

    TracklistDBusHandler::TracklistDBusHandler(KSCD *kscd)
        : QObject()
    {
    	
    	player = kscd;
    	    	new TracklistAdaptor( this );

    	    		QDBusConnection::sessionBus().registerObject("/Player", this);

    	        setObjectName("TracklistDBusHandler");
    }

    int TracklistDBusHandler::AddTrack( const QString& url, bool playImmediately )
    {
        return 0;
    }

    void TracklistDBusHandler::DelTrack( int index )
    {
    }

    int TracklistDBusHandler::GetCurrentTrack()
    {
        return 0;
    }

    int TracklistDBusHandler::GetLength()
    {
        return 0;
    }

    QVariantMap TracklistDBusHandler::GetMetadata( int position )
    {
        return QVariantMap();
    }

    void TracklistDBusHandler::SetLoop(bool enable)
    {
    	
    }

    void TracklistDBusHandler::SetRandom( bool enable )
    {
    	
    }

    void TracklistDBusHandler::slotTrackListChange()
    {
    	
    }
}

#include "TracklistDBusHandler.moc"
