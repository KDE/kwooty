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


#ifndef SERVERGROUP_H
#define SERVERGROUP_H

#include <QObject>
#include <QSet>
#include <QTimer>

#include "data/serverdata.h"
#include "utility.h"
using namespace UtilityNamespace;

class ClientManagerConn;
class ServerManager;
class CentralWidget;

class ServerGroup : public QObject {

    Q_OBJECT

public:
    ServerGroup(ServerManager*, CentralWidget*, int);
    int getServerGroupId() const;
    CentralWidget* getCentralWidget();
    ServerManager* getServerManager();
    bool isServerAvailable();
    QSet<int> getUnavailableSet() const;
    void assignDownloadToReadyClients();
    void disconnectAllClients();
    void connectAllClients();
    ServerData getServerData() const;
    bool isMasterServer() const;
    bool isDisabledBackupServer() const;
    bool isFailOverBackupServer() const;
    bool isLoadBalancingBackupServer() const;

private:

    int serverGroupId;
    QList<ClientManagerConn*> clientManagerConnList;
    CentralWidget* centralWidget;
    ServerManager* serverManager;
    int totalConnections;
    int nttpErrorStatus;
    bool serverAvailable;
    bool pendingSegments;
    ServerData serverData;
    QSet<int> unavailableServerSet;
    QTimer* clientsAvailableTimer;


    void createNntpClients();
    void setupConnections();


signals:
    void dataHasArrivedClientReadySignal();
    void disconnectRequestSignal();
    void connectRequestSignal();


public slots:
    bool settingsServerChangedSlot();
    void downloadPendingSegmentsSlot();


private slots:
    void checkServerAvailabilitySlot();
    void startTimerSlot();



};

#endif // SERVERGROUP_H
