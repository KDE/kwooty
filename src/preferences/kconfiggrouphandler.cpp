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


#include "kconfiggrouphandler.h"

#include <QApplication>
#include <KGlobal>
#include <KDebug>

#include "preferences/preferencesserver.h"


KConfigGroupHandler::KConfigGroupHandler() : QObject(qApp){}


KConfigGroupHandler::~KConfigGroupHandler() { kDebug();}


KConfigGroupHandler* KConfigGroupHandler::instance = 0;
KConfigGroupHandler* KConfigGroupHandler::getInstance() {

    if (!instance) {
        instance = new KConfigGroupHandler();
    }

    return instance;

}




ServerData KConfigGroupHandler::readServerSettings(const int& serverId) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));

    ServerData serverData;

    serverData.setServerId(configGroup.readEntry("serverId", serverId));
    serverData.setHostName(configGroup.readEntry("hostName", QString()));
    serverData.setPort(configGroup.readEntry("port", 119));
    serverData.setConnectionNumber(configGroup.readEntry("connectionNumber", 4));
    serverData.setAuthentication(configGroup.readEntry("authentication", false));
    serverData.setLogin(configGroup.readEntry("login", QString()));
    serverData.setPassword(configGroup.readEntry("password", QString()));
    serverData.setDisconnectTimeout(configGroup.readEntry("disconnectTimeout", 5));
    serverData.setEnableSSL(configGroup.readEntry("enableSSL", false));
    serverData.setServerModeIndex(configGroup.readEntry("serverModeIndex", 0));

    return serverData;

}



void KConfigGroupHandler::writeServerNumberSettings(const int& serverNumber) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("NumberOfServers"));
    configGroup.writeEntry("serverNumber", serverNumber);
    configGroup.sync();
}


int KConfigGroupHandler::readServerNumberSettings() {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("NumberOfServers"));
    int serverNumber = configGroup.readEntry("serverNumber", 1);

    return qMin(PreferencesServer::MAX_SERVERS, serverNumber);

}



void KConfigGroupHandler::writeServerSettings(const int& serverId, ServerData serverData) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));

    configGroup.writeEntry("serverId", serverData.getServerId());
    configGroup.writeEntry("serverName", serverData.getServerName());
    configGroup.writeEntry("hostName", serverData.getHostName());
    configGroup.writeEntry("port", serverData.getPort());
    configGroup.writeEntry("connectionNumber", serverData.getConnectionNumber());
    configGroup.writeEntry("authentication", serverData.isAuthentication());
    configGroup.writeEntry("login", serverData.getLogin());
    configGroup.writeEntry("password", serverData.getPassword());
    configGroup.writeEntry("disconnectTimeout", serverData.getDisconnectTimeout());
    configGroup.writeEntry("enableSSL", serverData.isEnableSSL());
    configGroup.writeEntry("serverModeIndex", serverData.getServerModeIndex());

    configGroup.sync();

}

void KConfigGroupHandler::removeServerSettings(const int& serverId) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));

    if (configGroup.exists()) {
        configGroup.deleteGroup();
    }

}




int KConfigGroupHandler::serverConnectionNumber(const int& serverId) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));
    return configGroup.readEntry("connectionNumber", 4);

}


QString KConfigGroupHandler::tabName(const int& serverId, const QString& tabText) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));
    return configGroup.readEntry("serverName", tabText);
}




