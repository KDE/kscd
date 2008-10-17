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

#include <QObject>
#include <QVariantMap>
#include <QDBusArgument>

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

    PlayerDBusHandler::PlayerDBusHandler()
        : QObject()
    {
        s_instance = this;
        setObjectName("PlayerDBusHandler");

        //QDBusConnection::sessionBus().registerObject("/Player", this);

        connect( this, SIGNAL( StatusChange( DBusStatus ) ), this, SLOT( slotCapsChange() ) );
    }

    DBusStatus PlayerDBusHandler::GetStatus()
    {

    }

    void PlayerDBusHandler::Pause()
    {

    }

    void PlayerDBusHandler::Play()
    {

    }

    void PlayerDBusHandler::Next()
    {

    }

    void PlayerDBusHandler::Prev()
    {

    }

    void PlayerDBusHandler::Repeat( bool on )
    {
    	
    }

    //position is specified in milliseconds
    int PlayerDBusHandler::PositionGet()
    {
    	
    }

    void PlayerDBusHandler::PositionSet( int time )
    {
    	
    }

    void PlayerDBusHandler::Stop()
    {
    	
    }

    int PlayerDBusHandler::VolumeGet()
    {
    	
    }

    void PlayerDBusHandler::VolumeSet( int vol )
    {
    	
    }

    int PlayerDBusHandler::GetCaps()
    {
        
    }

    void PlayerDBusHandler::slotCapsChange()
    {
        
    }

    void PlayerDBusHandler::slotTrackChange()
    {
        
    }

    void PlayerDBusHandler::slotStatusChange()
    {
        
    }

} // namespace KsCD

#include "PlayerDBusHandler.moc"
