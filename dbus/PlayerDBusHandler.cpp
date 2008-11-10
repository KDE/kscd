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


#include "PlayerDBusHandler.h"
#include "../kscd.h"
#include <kdebug.h>
#include <QObject>
#include <QVariantMap>
#include <QDBusArgument>

#include "PlayerAdaptor.h"

// Marshall the DBusStatus data into a D-BUS argument
QDBusArgument &operator<<(QDBusArgument &argument, const DBusStatus &status)
{
    argument.beginStructure();
    argument << status.Play;
    argument << status.Random;
    argument << status.Repeat;
    argument << status.RepeatPlaylist;
    argument.endStructure();
    return argument;
}

// Retrieve the DBusStatus data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusStatus &status)
{
    argument.beginStructure();
    argument >> status.Play;
    argument >> status.Random;
    argument >> status.Repeat;
    argument >> status.RepeatPlaylist;
    argument.endStructure();
    return argument;
}


namespace KsCD
{

    PlayerDBusHandler *PlayerDBusHandler::s_instance = 0;

    PlayerDBusHandler::PlayerDBusHandler(KSCD* kscd)
        : QObject()
    {
    	player = kscd;
    	kDebug() << "**** Launching Player Handler ****";
    	new PlayerAdaptor( this );

    		QDBusConnection::sessionBus().registerObject("/Player", this);
    		
        s_instance = this;
        setObjectName("PlayerDBusHandler");

        //QDBusConnection::sessionBus().registerObject("/Player", this);

        connect( this, SIGNAL( StatusChange( DBusStatus ) ), this, SLOT( slotCapsChange() ) );
    }

    DBusStatus PlayerDBusHandler::GetStatus()
    {
    	kDebug() << "**** Dbus -> GetStatus() ****";
        struct DBusStatus s;
        memset(&s, 0, sizeof(struct DBusStatus));
        return s;
    }

    void PlayerDBusHandler::Pause()
    {
    	kDebug() << "**** Dbus -> Pause() ****";
    	player->getDevices()->pause() ;
    }

    void PlayerDBusHandler::Play()
    {
    	kDebug() << "**** Dbus -> Play() ****";
    	player->getDevices()->play();
    }

    void PlayerDBusHandler::Next()
    {
    	kDebug() << "**** Dbus -> Next() ****";
    	player->getDevices()->nextTrack();
    }

    void PlayerDBusHandler::Prev()
    {
    	kDebug() << "**** Dbus -> Prev() ****";
    	player->getDevices()->prevTrack();
    }

    void PlayerDBusHandler::Repeat( bool on )
    {
    	kDebug() << "**** Dbus -> Repeat() ****";
    	
    }

    //position is specified in milliseconds
    int PlayerDBusHandler::PositionGet()
    {
    	kDebug() << "**** Dbus -> PositionGet() ****";
    	return player->getDevices()->getTotalTime() - player->getDevices()->getRemainingTime();
    }

    void PlayerDBusHandler::PositionSet( int time )
    {
    	kDebug() << "**** Dbus -> Position set() = "<<time<<" ****";
    	
    }

    void PlayerDBusHandler::Stop()
    {
    	kDebug() << "**** Dbus -> Stop() ****";
    	player->getDevices()->stop();
    }

    int PlayerDBusHandler::VolumeGet()
    {
    	kDebug() << "**** Dbus -> VolumeGet() ****";
    	return player->getDevices()->getVolume();
    }

    void PlayerDBusHandler::VolumeSet( int vol )
    {
    	kDebug() << "**** Dbus -> VolumeSet() ****";
    	player->getDevices()->setVolume(vol);
    }

    int PlayerDBusHandler::GetCaps()
    {
    	kDebug() << "**** Dbus -> GetCaps() ****";
    	return 0;
    }

    void PlayerDBusHandler::slotCapsChange()
    {
    	kDebug() << "**** Dbus -> slotCapsChange() ****";
    }

    void PlayerDBusHandler::slotTrackChange()
    {
    	kDebug() << "**** Dbus -> slotTrackChange() ****";
    }

    void PlayerDBusHandler::slotStatusChange()
    {
    	kDebug() << "**** Dbus -> slotStatusChange() ****";
    }

} // namespace KsCD

#include "PlayerDBusHandler.moc"
