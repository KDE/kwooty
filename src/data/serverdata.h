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


#ifndef SERVERDATA_H
#define SERVERDATA_H

#include <QString>

class ServerData {

public:
    ServerData();

    void setServerId(const int&);
    int getServerId() const;

    void setServerName(const QString&);
    QString getServerName() const;

    void setHostName(const QString&);
    QString getHostName() const;

    void setLogin(const QString&);
    QString getLogin() const;

    void setPassword(const QString&);
    QString getPassword() const;

    void setPort(const uint&);
    uint getPort() const;

    void setDisconnectTimeout(const uint&);
    uint getDisconnectTimeout() const;

    void setAuthentication(const bool&);
    bool isAuthentication() const;

    void setEnableSSL(const bool&);
    bool isEnableSSL() const;

    void setConnectionNumber(const uint&);
    uint getConnectionNumber() const;

    void setServerModeIndex(const uint&);
    uint getServerModeIndex() const;


    bool operator!=(const ServerData&);

private:

    int serverId;
    QString serverName;
    QString hostName;
    QString login;
    QString password;
    uint port;
    uint disconnectTimeout;
    bool authentication;
    bool enableSSL;
    uint connectionNumber;
    uint serverModeIndex;


};

#endif // SERVERDATA_H
