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

#include "servermanager.h"
#include "centralwidget.h"
#include "servergroup.h"
#include "segmentmanager.h"
#include "preferences/kconfiggrouphandler.h"


ServerManager::ServerManager(CentralWidget* parent) : QObject(parent) {

    this->parent = parent;

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    // create all nntp clients for all servers (master + backups) :
    for (int serverGroupId = 0; serverGroupId < serverNumber; serverGroupId++) {
        this->idServerGroupMap.insert(serverGroupId, new ServerGroup(this, parent, serverGroupId));
    }

    // by default, the master server is always the first one :
    this->currentMasterServer = this->idServerGroupMap.value(MasterServer);

    this->setupConnections();


}


void ServerManager::setupConnections() {

    // parent notify that settings have been changed :
    connect (this->parent,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));

}



int ServerManager::getNextTargetServer(const int& currentTargetServer) {

    int nextTargetServer = UtilityNamespace::NoTargetServer;

    // if a next backup server exists :
    if (this->idServerGroupMap.size() > currentTargetServer + 1) {

        // look for next available backup server with passive mode only
        // - a backup server with failover mode whose master server is available act as being in passive mode
        // - a backup server with active mode is considered as another master server :
        foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values().mid(currentTargetServer + 1)) {

            if (nextServerGroup->isServerAvailable() &&
                nextServerGroup->isPassiveBackupServer()) {

                nextTargetServer = nextServerGroup->getServerGroupId();
                break;
            }

        }

    }

    return nextTargetServer;
}




void ServerManager::downloadWithAnotherBackupServer(const int& serverGroupId) {

    // get next server group id :
    int nextServerId = this->getNextTargetServer(serverGroupId);

    // update pending segments with this new server group id in order to be downloaded by
    // this server group (if NoTargetServer pending segments are considered as download finish) :
    this->parent->getSegmentManager()->updatePendingSegmentsToTargetServer(serverGroupId, nextServerId);

    if (nextServerId != NoTargetServer) {
        this->tryDownloadWithServer(nextServerId);
    }

}



void ServerManager::tryDownloadWithServer(const int& nextTargetServer) {

    ServerGroup* nextServerGroup = this->idServerGroupMap.value(nextTargetServer);

    if (nextServerGroup && nextServerGroup->isServerAvailable()) {

        // notify server group that pending segments wait for it :
        nextServerGroup->assignDownloadToReadyClients();
    }

}



void ServerManager::masterServerAvailabilityChanges() {

    ServerGroup* newMasterServer = 0;

    // current master server availability has changed ,
    foreach (ServerGroup* currentServerGroup, this->idServerGroupMap.values()) {

        // look for the first available master server substitute :
        if (currentServerGroup->isServerAvailable() &&
            ( currentServerGroup->isMasterServer() ||
              currentServerGroup->isFailoverBackupServer() ) ) {

            newMasterServer = currentServerGroup;
            break;
        }
    }

    // if a new master server has been found, store it :
    if (newMasterServer &&
        this->currentMasterServer != newMasterServer) {

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


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void ServerManager::settingsChangedSlot() {

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    // 1.a if new backup servers are requested :
    if (serverNumber > this->idServerGroupMap.size()) {

        for (int serverGroupId = this->idServerGroupMap.size(); serverGroupId < serverNumber; serverGroupId++){
            this->idServerGroupMap.insert(serverGroupId, new ServerGroup(this, parent, serverGroupId));
        }
    }


    // 1.b if less backup servers are requested :
    if (serverNumber < this->idServerGroupMap.size()) {

        while (this->idServerGroupMap.size() > serverNumber) {

            this->idServerGroupMap.take(this->idServerGroupMap.size() - 1)->deleteLater();

        }

    }


    // 2.a notify servergroups that some settings changed :
    bool serverSettingsChanged = false;
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

        // reconnect all clients 100 ms later :
        QTimer::singleShot(100, this, SLOT(requestClientConnectionSlot()));

    }

}



void ServerManager::requestClientConnectionSlot() {

    foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values()) {
        nextServerGroup->connectAllClients();
    }

}


