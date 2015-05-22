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
    this->mServerId = serverId;
}
int ServerData::getServerId() const
{
    return this->mServerId;
}

void ServerData::setServerName(const QString &serverName)
{
    this->mServerName = serverName;
}
QString ServerData::getServerName() const
{
    return this->mServerName;
}

void ServerData::setHostName(const QString &hostName)
{
    this->mHostName = hostName;
}
QString ServerData::getHostName() const
{
    return this->mHostName;
}

void ServerData::setLogin(const QString &login)
{
    this->mLogin = login;
}
QString ServerData::getLogin() const
{
    return this->mLogin;
}

void ServerData::setPassword(const QString &password)
{
    this->mPassword = password;
}
QString ServerData::getPassword() const
{
    return this->mPassword;
}

void ServerData::setPort(const uint &port)
{
    this->mPort = port;
}
uint ServerData::getPort() const
{
    return this->mPort;
}

void ServerData::setDisconnectTimeout(const uint &disconnectTimeout)
{
    this->mDisconnectTimeout = disconnectTimeout;
}
uint ServerData::getDisconnectTimeout() const
{
    return this->mDisconnectTimeout;
}

void ServerData::setAuthentication(const bool &authentication)
{
    this->mAuthentication = authentication;
}
bool ServerData::isAuthentication() const
{
    return this->mAuthentication;
}

void ServerData::setEnableSSL(const bool &enableSSL)
{
    this->mEnableSSL = enableSSL;
}
bool ServerData::isEnableSSL() const
{
    return this->mEnableSSL;
}

void ServerData::setConnectionNumber(const uint &connectionNumber)
{
    this->mConnectionNumber = connectionNumber;
}
uint ServerData::getConnectionNumber() const
{
    return this->mConnectionNumber;
}

void ServerData::setServerModeIndex(const uint &serverModeIndex)
{
    this->mServerModeIndex = serverModeIndex;
}

uint ServerData::getServerModeIndex() const
{
    return this->mServerModeIndex;
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


