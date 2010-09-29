/***************************************************************************
 *   Copyright (C) 2009 by Xavier Lefage                                   *
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

#ifndef CLIENTMANAGERCONN_H
#define CLIENTMANAGERCONN_H

#include "data/segmentdata.h"
#include "data/serverdata.h"

class NntpClient;
class ServerGroup;


class ClientManagerConn : public QObject {

    Q_OBJECT

public:
    ClientManagerConn(ServerGroup*, int, int);
    ClientManagerConn();
    ~ClientManagerConn();
    NntpClient* getNntpClient();
    ServerGroup* getServerGroup();
    void processNextSegment(const SegmentData&);
    void noSegmentAvailable();
    ServerData getServerData() const;
    int getClientId() const;
    bool isClientReady() const;
    bool isMasterServer() const;
    bool isDisabledBackupServer() const;


private:
    NntpClient* nntpClient;
    ServerGroup* parent;
    int clientId;
    int connectionDelay;


signals:


public slots:
    void dataHasArrivedSlot();
    void disconnectRequestSlot();
    void connectRequestSlot();

private slots:
    void initSlot();


};

#endif // CLIENTMANAGERCONN_H

