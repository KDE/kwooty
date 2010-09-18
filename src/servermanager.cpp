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


    kDebug();

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    for (int serverGroupId = 0; serverGroupId < serverNumber; serverGroupId++) {
        this->idServerGroupMap.insert(serverGroupId, new ServerGroup(this, parent, serverGroupId));
    }

    this->setupConnections();


}


void ServerManager::setupConnections() {

    // parent notify that settings have been changed :
    connect (this->parent,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));

}




void ServerManager::settingsChangedSlot() {

    kDebug();

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    // if new backup servers are requested :
    if (serverNumber > this->idServerGroupMap.size()) {

        for (int serverGroupId = this->idServerGroupMap.size(); serverGroupId < serverNumber; serverGroupId++){
            this->idServerGroupMap.insert(serverGroupId, new ServerGroup(this, parent, serverGroupId));
        }
    }


    // if less backup servers are requested :
    if (serverNumber < this->idServerGroupMap.size()) {

        while (this->idServerGroupMap.size() > serverNumber) {

            this->idServerGroupMap.take(this->idServerGroupMap.size() - 1)->deleteLater();

        }

    }

}


int ServerManager::getNextTargetServer(int currentTargetServer) {

    ServerGroup* masterServerGroup = this->idServerGroupMap.value(MasterServer);

    int nextTargetServer = UtilityNamespace::NoTargetServer;

        if (this->idServerGroupMap.size() > currentTargetServer + 1) {

            foreach (ServerGroup* nextServerGroup, this->idServerGroupMap.values().mid(currentTargetServer + 1)) {

                if (nextServerGroup->isServerAvailable()) {

                    nextTargetServer = nextServerGroup->getServerGroupId();
                    break;
                }

            }

        }

    return nextTargetServer;

}





void ServerManager::downloadWithAnotherBackupServer(int serverGroupId) {


    int nextServerId = this->getNextTargetServer(serverGroupId);

    this->parent->getSegmentManager()->updatePendingSegmentsToTargetServer(serverGroupId, nextServerId);

    if (nextServerId != NoTargetServer) {
        this->tryDownloadWithServer(nextServerId);
    }

}





void ServerManager::tryDownloadWithServer(const int nextTargetServer) {

    ServerGroup* nextServerGroup = this->idServerGroupMap.value(nextTargetServer);

    if (nextServerGroup && nextServerGroup->isServerAvailable()) {

        nextServerGroup->assignDownloadToReadyClients();
    }

}






