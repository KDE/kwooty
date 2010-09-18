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

ServerData::ServerData() {

}


void ServerData::setServerId(const int& serverId) {
    this->serverId = serverId;
}
int ServerData::getServerId() const {
    return this->serverId;
}

void ServerData::setServerName(const QString& serverName) {
    this->serverName = serverName;
}
QString ServerData::getServerName() const {
    return this->serverName;
}

void ServerData::setHostName(const QString& hostName) {
    this->hostName = hostName;
}
QString ServerData::getHostName() const {
    return this->hostName;
}

void ServerData::setLogin(const QString& login) {
    this->login = login;
}
QString ServerData::getLogin() const {
    return this->login;
}


void ServerData::setPassword(const QString& password) {
    this->password = password;
}
QString ServerData::getPassword() const {
    return this->password;
}


void ServerData::setPort(const uint& port) {
    this->port = port;
}
uint ServerData::getPort() const {
    return this->port;
}


void ServerData::setDisconnectTimeout(const uint& disconnectTimeout) {
    this->disconnectTimeout = disconnectTimeout;
}
uint ServerData::getDisconnectTimeout() const {
    return this->disconnectTimeout;
}


void ServerData::setConnectionDelay(const uint& connectionDelay) {
    this->connectionDelay = connectionDelay;
}
uint ServerData::getConnectionDelay() const {
    return this->connectionDelay;
}


void ServerData::setAuthentication(const bool& authentication) {
    this->authentication = authentication;
}
bool ServerData::isAuthentication() const {
    return this->authentication;
}


void ServerData::setEnableSSL(const bool& enableSSL) {
    this->enableSSL = enableSSL;
}
bool ServerData::isEnableSSL() const {
    return this->enableSSL;
}


void ServerData::setConnectionNumber(const uint& connectionNumber) {
    this->connectionNumber = connectionNumber;
}
uint ServerData::getConnectionNumber() const {
    return this->connectionNumber;
}


bool ServerData::operator!=(const ServerData& serverDataToCompare) {

    bool different = false;

    if ( (serverId          != serverDataToCompare.getServerId())           ||
         (hostName          != serverDataToCompare.getHostName())           ||
         (login             != serverDataToCompare.getLogin())              ||
         (password          != serverDataToCompare.getPassword())           ||
         (port              != serverDataToCompare.getPort())               ||
         (disconnectTimeout != serverDataToCompare.getDisconnectTimeout())  ||
         (connectionDelay   != serverDataToCompare.getConnectionDelay())    ||
         (authentication    != serverDataToCompare.isAuthentication())      ||
         (enableSSL         != serverDataToCompare.isEnableSSL())           ||
         (connectionNumber  != serverDataToCompare.getConnectionNumber()) ) {

        different = true;
    }

    return different;

}


