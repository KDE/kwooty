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
#include <QTimer>

#include "data/serverdata.h"
#include "data/segmentdata.h"
#include "utility.h"
using namespace UtilityNamespace;

class ClientManagerConn;
class ServerManager;
class CentralWidget;
class ClientsPerServerObserver;
class ServerSpeedManager;

class ServerGroup : public QObject {

    Q_OBJECT

public:
    ServerGroup(ServerManager*, CentralWidget*, int);
    CentralWidget* getCentralWidget();
    ServerManager* getServerManager();
    ClientsPerServerObserver* getClientsPerServerObserver();
    ServerSpeedManager* getServerSpeedManager();
    ServerGroup* getNextTargetServer();
    ServerData getServerData() const;
    QList<ClientManagerConn*> getClientManagerConnList();
    int getServerGroupId() const;
    int getRealServerGroupId() const;
    int saveSegment(const SegmentData&);
    bool canDownload(const int&) const;
    bool isServerAvailable() const;
    bool isMasterServer() const;
    bool isDisabledBackupServer() const;
    bool isPassiveBackupServer() const;
    bool isActiveBackupServer() const;
    bool isFailoverBackupServer() const;
    bool isPassiveFailover() const;
    bool isActiveFailover() const;
    void assignDownloadToReadyClients();
    void disconnectAllClients();
    void connectAllClients();


private:
    static const int MAX_SERVER_DOWN_PER_MINUTE = 4;

    ServerData serverData;
    QTimer* clientsAvailableTimer;
    QTimer* stabilityTimer;
    QList<ClientManagerConn*> clientManagerConnList;
    CentralWidget* centralWidget;
    ServerManager* serverManager;
    ServerSpeedManager* serverSpeedManager;
    ClientsPerServerObserver* clientsPerServerObserver;
    int serverGroupId;
    int stabilityCounter;
    bool serverAvailable;
    bool pendingSegments;

    void createNntpClients();
    void setupConnections();
    void serverSwitchIfFailure();


signals:
    void dataHasArrivedClientReadySignal();
    void disconnectRequestSignal();
    void connectRequestSignal();


public slots:
    bool settingsServerChangedSlot();
    void downloadPendingSegmentsSlot();


private slots:
    void checkServerAvailabilitySlot();
    void checkServerStabilitySlot();
    void startTimerSlot();


};

#endif // SERVERGROUP_H
