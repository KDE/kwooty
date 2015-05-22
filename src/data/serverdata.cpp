/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
 *   xavier.kwooty@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "serverdata.h"

ServerData::ServerData()
{

}

void ServerData::setServerId(const int &serverId)
{
    mServerId = serverId;
}
int ServerData::getServerId() const
{
    return mServerId;
}

void ServerData::setServerName(const QString &serverName)
{
    mServerName = serverName;
}
QString ServerData::getServerName() const
{
    return mServerName;
}

void ServerData::setHostName(const QString &hostName)
{
    mHostName = hostName;
}
QString ServerData::getHostName() const
{
    return mHostName;
}

void ServerData::setLogin(const QString &login)
{
    mLogin = login;
}
QString ServerData::getLogin() const
{
    return mLogin;
}

void ServerData::setPassword(const QString &password)
{
    mPassword = password;
}
QString ServerData::getPassword() const
{
    return mPassword;
}

void ServerData::setPort(const uint &port)
{
    mPort = port;
}
uint ServerData::getPort() const
{
    return mPort;
}

void ServerData::setDisconnectTimeout(const uint &disconnectTimeout)
{
    mDisconnectTimeout = disconnectTimeout;
}
uint ServerData::getDisconnectTimeout() const
{
    return mDisconnectTimeout;
}

void ServerData::setAuthentication(const bool &authentication)
{
    mAuthentication = authentication;
}
bool ServerData::isAuthentication() const
{
    return mAuthentication;
}

void ServerData::setEnableSSL(const bool &enableSSL)
{
    mEnableSSL = enableSSL;
}
bool ServerData::isEnableSSL() const
{
    return mEnableSSL;
}

void ServerData::setConnectionNumber(const uint &connectionNumber)
{
    mConnectionNumber = connectionNumber;
}
uint ServerData::getConnectionNumber() const
{
    return mConnectionNumber;
}

void ServerData::setServerModeIndex(const uint &serverModeIndex)
{
    mServerModeIndex = serverModeIndex;
}

uint ServerData::getServerModeIndex() const
{
    return mServerModeIndex;
}

bool ServerData::operator!=(const ServerData &serverDataToCompare)
{

    bool different = false;

    if ((mServerId          != serverDataToCompare.getServerId())           ||
            (mHostName          != serverDataToCompare.getHostName())           ||
            (mLogin             != serverDataToCompare.getLogin())              ||
            (mPassword          != serverDataToCompare.getPassword())           ||
            (mPort              != serverDataToCompare.getPort())               ||
            (mDisconnectTimeout != serverDataToCompare.getDisconnectTimeout())  ||
            (mAuthentication    != serverDataToCompare.isAuthentication())      ||
            (mEnableSSL         != serverDataToCompare.isEnableSSL())           ||
            (mConnectionNumber  != serverDataToCompare.getConnectionNumber())   ||
            (mServerModeIndex   != serverDataToCompare.getServerModeIndex())) {

        different = true;
    }

    return different;

}


