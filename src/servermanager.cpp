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

#include <KDebug>
#include <KApplication>

#include "servermanager.h"
#include "core.h"
#include "servergroup.h"
#include "serverspeedmanager.h"
#include "sidebar.h"
#include "standarditemmodelquery.h"
#include "segmentmanager.h"
#include "segmentbuffer.h"
#include "mainwindow.h"
#include "observers/clientsperserverobserver.h"
#include "preferences/kconfiggrouphandler.h"


ServerManager::ServerManager(Core* parent) : QObject(parent) {

    this->parent = parent;

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    // create segment buffer to store downloaded segments ready to decoded by dedicated server :
    this->segmentBuffer = new SegmentBuffer(this, parent);


    // create all nntp clients for all servers (master + backups) :
    for (int serverGroupId = 0; serverGroupId < serverNumber; serverGroupId++) {
        this->idServerGroupMap.insert(serverGroupId, new ServerGroup(this, parent, serverGroupId));
    }

    // by default, the master server is always the first one :
    this->currentMasterServer = this->idServerGroupMap.value(MasterServer);

    this->setupConnections();

    // notify sidebar that servergroups have been created :
    emit serverManagerSettingsChangedSignal();


}


void ServerManager::setupConnections() {

    // parent notify that settings have been changed :
    connect (this->parent,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));

}



int ServerManager::getServerNumber() const {
    return this->idServerGroupMap.size();
}

ServerGroup* ServerManager::getServerGroupById(const int& index) {
    return this->idServerGroupMap.value(index);
}

SegmentBuffer* ServerManager::getSegmentBuffer() {
    return this->segmentBuffer;
}


bool ServerManager::isSessionRestoredNoJobs() const {
    return (this->parent->getModelQuery()->isRootModelEmpty() && kapp->isSessionRestored());
}


ServerGroup* ServerManager::getNextTargetServer(ServerGroup* currentServerGroup) {

    ServerGroup* nextServerGroupTarget = 0;

    // if current server is the MasterServer :
    if (currentServerGroup->isMasterServer()) {

        // search any other Active servers in order to download missing segments from MasterServer with them :
        foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values()) {

            if ( nextServerGroup->isActiveBackupServer() &&
                 nextServerGroup->isServerAvailable() ) {

                nextServerGroupTarget = nextServerGroup;
                break;
            }

        }
    }

    // if current server is not the master server or no active server have been found :
    if (!nextServerGroupTarget) {

        int currentTargetServer = currentServerGroup->getServerGroupId();

        // current Active server could be at any position, in order to not skip any
        // eventual backup server at a lower position set currentTargetServer as first position :
        if (currentServerGroup->isActiveBackupServer()) {
            currentTargetServer = MasterServer;
        }

        // if a next backup server exists :
        if (this->idServerGroupMap.size() > currentTargetServer + 1) {

            // look for next available backup server with passive mode only
            // - a backup server with failover mode whose master server is available act as being in passive mode
            // - a backup server with active mode is considered as another master server :
            foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values().mid(currentTargetServer + 1)) {

                if ( nextServerGroup->isPassiveBackupServer() &&
                     nextServerGroup->isServerAvailable() ) {

                    nextServerGroupTarget = nextServerGroup;
                    break;
                }

            }

        }
    }

    return nextServerGroupTarget;
}




void ServerManager::downloadWithAnotherBackupServer(ServerGroup* currentServerGroup) {

    // check that there is at least one pending download in order to not connect clients uselessly :
    if ( !this->parent->getModelQuery()->isRootModelEmpty() ) {

        // get next server group :
        ServerGroup* nextServerGroup = this->getNextTargetServer(currentServerGroup);

        // get corresponding server group id :
        int nextServerId = UtilityNamespace::NoTargetServer;

        if (nextServerGroup) {
            nextServerId = nextServerGroup->getServerGroupId();
        }

        // update pending segments with this new server group id in order to be downloaded by
        // this server group (if NoTargetServer pending segments are considered as download finish) :
        this->parent->getSegmentManager()->updatePendingSegmentsToTargetServer(currentServerGroup->getServerGroupId(), nextServerId);


        // notify server group that pending segments wait for it :
        if (nextServerGroup && nextServerGroup->isServerAvailable()) {
            nextServerGroup->assignDownloadToReadyClients();
        }

    }

}



bool ServerManager::areAllServersEncrypted() const {

    bool allServersEncrypted = true;

    // current master server availability has changed ,
    foreach (ServerGroup* currentServerGroup, this->idServerGroupMap.values()) {

        if ( currentServerGroup->getClientsPerServerObserver()->isConnected() &&
             !currentServerGroup->getClientsPerServerObserver()->isSslActive() ) {

            allServersEncrypted = false;
            break;
        }
    }

    return allServersEncrypted;

}


quint64 ServerManager::retrieveCumulatedDownloadSpeed(const int& nzbDownloadRowPos) const {

    quint64 cumulatedDownloadSpeed = 0;

    // look for every servers currently downloading the same nzb item (at nzbDownloadRowPos in model)
    // in order to compute the current nzb item cumulated download speed :
    foreach (ServerGroup* currentServerGroup, this->idServerGroupMap.values()) {

        ClientsPerServerObserver* clientsPerServerObserver = currentServerGroup->getClientsPerServerObserver();
        SegmentInfoData segmentInfoData = clientsPerServerObserver->getSegmentInfoData();

        // if current server group is downloading the current nzb item, add it to the cumulatedDownloadSpeed :
        if (segmentInfoData.getNzbRowModelPosition() == nzbDownloadRowPos) {

            cumulatedDownloadSpeed += clientsPerServerObserver->getDownloadSpeed();

        }

    }

    return cumulatedDownloadSpeed;

}


quint64 ServerManager::retrieveServerDownloadSpeed(const int& currentServer) const {

    // retrieve average download speed for the current server :
    ServerGroup* currentServerGroup = this->idServerGroupMap.value(currentServer);
    return currentServerGroup->getClientsPerServerObserver()->getDownloadSpeed();

}



void ServerManager::setBandwidthMode(const BandwidthClientMode& bandwidthClientMode) {

    // retrieve average download speed for the current server :
    foreach (ServerGroup* currentServerGroup, this->idServerGroupMap.values()) {
        currentServerGroup->getServerSpeedManager()->setBandwidthMode(bandwidthClientMode);
    }
}


void ServerManager::setLimitServerDownloadSpeed(const int& currentServer, const qint64& serverGroupLimitSpeedInBytes) {

    // retrieve average download speed for the current server :
    ServerGroup* currentServerGroup = this->idServerGroupMap.value(currentServer);
    currentServerGroup->getServerSpeedManager()->setDownloadSpeedLimitInBytes(serverGroupLimitSpeedInBytes);

}



void ServerManager::masterServerAvailabilityChanges() {

    ServerGroup* newMasterServer = 0;

    // current master server availability has changed ,
    foreach (ServerGroup* currentServerGroup, this->idServerGroupMap.values()) {

        // look for the first available master server substitute :
        if ( currentServerGroup->isServerAvailable() &&
             ( currentServerGroup->isMasterServer() ||
               currentServerGroup->isFailoverBackupServer() ) ) {

            newMasterServer = currentServerGroup;
            break;
        }
    }

    // if a new master server has been found, store it :
    if ( newMasterServer &&
         this->currentMasterServer != newMasterServer ) {

        this->currentMasterServer = newMasterServer;

        // retry to download pending segments with new master server first :
        this->parent->getSegmentManager()->updatePendingSegmentsToTargetServer(MasterServer, MasterServer, SegmentManager::ResetSegments);

        // notify it that pending segments wait for it :
        this->currentMasterServer->assignDownloadToReadyClients();

    }

}



bool ServerManager::currentIsFirstMasterAvailable(const ServerGroup* currentServerGroup) const {
    return (this->currentMasterServer == currentServerGroup);
}


void ServerManager::resetAllServerConnection() {

    // disconnect and reconnect all clients from all servers :
    foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values()) {
        nextServerGroup->resetAllClientsConnection();
    }

}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void ServerManager::settingsChangedSlot() {

    bool serverSettingsChanged = false;

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    // 1.a if new backup servers are requested :
    if (serverNumber > this->idServerGroupMap.size()) {

        for (int serverGroupId = this->idServerGroupMap.size(); serverGroupId < serverNumber; serverGroupId++){
            this->idServerGroupMap.insert(serverGroupId, new ServerGroup(this, parent, serverGroupId));
        }
    }

    // 1.b if less backup servers are requested :
    if (serverNumber < this->idServerGroupMap.size()) {
        serverSettingsChanged = true;
    }


    // 2.a notify servergroups that some settings changed :
    foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values()) {

        bool currentServerSettingsChanged = nextServerGroup->settingsServerChangedSlot();

        if (currentServerSettingsChanged) {
            serverSettingsChanged = true;
        }
    }

    // 2.b if one or several server(s) settings have changed :
    if (serverSettingsChanged) {

        // disconnect all clients from all servers :
        foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values()) {
            nextServerGroup->disconnectAllClients();
        }

        // retry to download pending segments with master server first, then first backup server, then second one...
        // with new servers settings :
        this->parent->getSegmentManager()->updatePendingSegmentsToTargetServer(MasterServer, MasterServer, SegmentManager::ResetSegments);

        // by default, the master server is always the first one :
        this->currentMasterServer = this->idServerGroupMap.value(MasterServer);


        // remove servers right now after disconnection done :
        while (this->idServerGroupMap.size() > serverNumber) {
            this->idServerGroupMap.take(this->idServerGroupMap.size() - 1)->deleteLater();
        }


        // reconnect all clients 100 ms later :
        QTimer::singleShot(100, this, SLOT(requestClientConnectionSlot()));


    }


    emit serverManagerSettingsChangedSignal();

}



void ServerManager::requestClientConnectionSlot() {

    foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values()) {
        nextServerGroup->connectAllClients();
    }

}


