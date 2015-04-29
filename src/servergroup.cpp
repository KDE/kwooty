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


#include "servergroup.h"

#include "servermanager.h"
#include "serverspeedmanager.h"
#include "segmentmanager.h"
#include "segmentbuffer.h"
#include "core.h"
#include "clientmanagerconn.h"
#include "nntpclient.h"
#include "observers/clientsperserverobserver.h"
#include "preferences/kconfiggrouphandler.h"
#include "kwootysettings.h"

#include "kwooty_debug.h"

ServerGroup::ServerGroup(ServerManager* parent, Core* core, int serverGroupId) : QObject(parent) {

    this->mServerManager = parent;
    this->mCore = core;
    this->mServerGroupId = serverGroupId;
    this->mServerAvailable = true;
    this->mPendingSegments = false;
    this->mStabilityCounter = 0;

    // retrieve server settings *without* password (if stored by kwallet it may be asked to access wallet, so ask question only when first server connection is required) :
    this->mServerData = KConfigGroupHandler::getInstance()->readServerSettings(this->mServerGroupId, KConfigGroupHandler::DoNotReadPasswordData);
    this->mPasswordRetrieved = false;

    // create observer for clients specific to this server instance :
    this->mClientsPerServerObserver = new ClientsPerServerObserver(this);

    // create download speed manager for this server group :
    this->mServerSpeedManager = new ServerSpeedManager(this);

    // connect clients :
    this->createNntpClients();

    // check server availabilty every 500 ms :
    this->mClientsAvailableTimer = new QTimer(this);
    this->mClientsAvailableTimer->start(500);

    // check server stability every minutes :
    this->mStabilityTimer = new QTimer(this);
    this->mStabilityTimer->start(1 * UtilityNamespace::MINUTES_TO_MILLISECONDS);

    this->setupConnections();

}



void ServerGroup::setupConnections() {

    // check that server is available or not :
    connect(mClientsAvailableTimer, SIGNAL(timeout()), this, SLOT(checkServerAvailabilitySlot()));

    // check if pending segments are ready for the current server :
    connect(mClientsAvailableTimer, SIGNAL(timeout()), this, SLOT(downloadPendingSegmentsSlot()));

    // check server stability :
    connect(mStabilityTimer, SIGNAL(timeout()), this, SLOT(checkServerStabilitySlot()));

}


int ServerGroup::getRealServerGroupId() const {
    // used for debugging puropses only :
    return this->mServerGroupId;
}

int ServerGroup::getServerGroupId() const {

    // default server group id :
    int currentServerGroupId = this->mServerGroupId;

    // if server is now configured in Active mode, return MasterServer groupId
    // in order be considered as another master server and to be able to download
    // pending segment targeted with MasterServer Id :
    if (this->isActiveBackupServer()) {
        currentServerGroupId = ActiveBackupServer;
    }

    // if backup server is in Failover mode and that master server is down
    // the current backup server may be considered as master server :
    else if (this->isActiveFailover()) {
        currentServerGroupId = MasterServer;
    }

    return currentServerGroupId;
}


ServerGroup* ServerGroup::getNextTargetServer() {

    return this->getServerManager()->getNextTargetServer(this);

}

int ServerGroup::saveSegment(const SegmentData& segmentData) {

    return this->getServerManager()->getSegmentBuffer()->segmentSavingQueued(segmentData);
}

bool ServerGroup::isBufferFull() {

    return this->getServerManager()->getSegmentBuffer()->isBufferFull();
}


void ServerGroup::readDataWithPassword() {

    if (!this->mPasswordRetrieved) {

        this->mServerData = KConfigGroupHandler::getInstance()->readServerSettings(this->mServerGroupId);
        this->mPasswordRetrieved = true;

    }

}


bool ServerGroup::canDownload(int serverGroupTargetId) const {

    bool segmentMatch = false;


    // if this is the Master server or an activeFailOver server that supersedes it :
    if (this->isMasterServer() || this->isActiveFailover()) {

        // allow to download segments only targeted for MasterServer :
        if (serverGroupTargetId == MasterServer) {
            segmentMatch = true;
        }

    }
    // if this is a passiveBackupServer
    else if (this->isPassiveBackupServer() || this->isPassiveFailover()) {

        // check that the current target corresponds to the proper server group id :
        if (serverGroupTargetId == this->mServerGroupId) {
            segmentMatch = true;
        }

    }
    // if current serverGroup is an ActiveBackupServer :
    else if (this->isActiveBackupServer()) {

        // servergroup will download segments targeted for Master server and also for itself :
        if (serverGroupTargetId == MasterServer ||
            serverGroupTargetId == ActiveBackupServer) {

            segmentMatch = true;

        }

    }
    // unhandled case, should not happen but try to download segment anyway :
    else {
        segmentMatch = true;
    }



    return segmentMatch;

}



Core* ServerGroup::getCore() {
    return this->mCore;
}

ServerManager* ServerGroup::getServerManager() {
    return this->mServerManager;
}

ClientsPerServerObserver* ServerGroup::getClientsPerServerObserver() {
    return this->mClientsPerServerObserver;
}

ServerSpeedManager* ServerGroup::getServerSpeedManager() {
    return this->mServerSpeedManager;
}


ServerData ServerGroup::getServerData() const {
    return this->mServerData;
}


QList<ClientManagerConn*> ServerGroup::getClientManagerConnList() {
    return this->mClientManagerConnList;
}


bool ServerGroup::isMasterServer() const {
    return (this->mServerGroupId == MasterServer);
}


bool ServerGroup::isDisabledBackupServer() const {
    return (this->mServerData.getServerModeIndex() == UtilityNamespace::DisabledServer);
}


bool ServerGroup::isPassiveBackupServer() const {

    bool passiveServer = false;

    // current server is in passive mode :
    if (this->mServerData.getServerModeIndex() == UtilityNamespace::PassiveServer) {
        passiveServer = true;
    }
    // else if it is in failover mode, it will works as passive if it is not currently replacing a down master server :
    else if (this->isPassiveFailover()) {
        passiveServer = true;
    }

    return passiveServer;
}

bool ServerGroup::isActiveBackupServer() const {
    return (this->mServerData.getServerModeIndex() == UtilityNamespace::ActiveServer);
}

bool ServerGroup::isFailoverBackupServer() const {
    return (this->mServerData.getServerModeIndex() == UtilityNamespace::FailoverServer);
}

bool ServerGroup::isActiveFailover() const {
    return (this->isFailoverBackupServer() && this->mServerManager->currentIsFirstMasterAvailable(this));
}

bool ServerGroup::isPassiveFailover() const {
    return (this->isFailoverBackupServer() && !this->mServerManager->currentIsFirstMasterAvailable(this));
}


bool ServerGroup::isServerAvailable() const {
    return this->mServerAvailable;
}



void ServerGroup::createNntpClients() {

    // create the nntp clients thread manager :
    int connectionNumber = KConfigGroupHandler::getInstance()->serverConnectionNumber(this->mServerGroupId);

    // set a delay of +100 ms between each nntp client instance :
    int connectionDelay = 0;

    for (int i = 0; i < connectionNumber; ++i) {
        this->mClientManagerConnList.append(new ClientManagerConn(this, i, connectionDelay));
        connectionDelay += 100;
    }

}



void ServerGroup::disconnectAllClients() {

    // stop timer that notify if clients are available or not :
    this->mClientsAvailableTimer->stop();

    // disconnect all clients :
    emit disconnectRequestSignal();
}


void ServerGroup::connectAllClients() {

    // connect all clients :
    emit connectRequestSignal();

    // restart timer that notify if clients are available :
    this->mStabilityCounter = 0;
    this->mServerAvailable = true;
    QTimer::singleShot(500 * this->mServerGroupId, this, SLOT(startTimerSlot()));

}




void ServerGroup::resetAllClientsConnection() {
    emit resetConnectionSignal();
}


void ServerGroup::assignDownloadToReadyClients() {

    // do not hammer backup servers that new segments are available,
    // it will be done asynchronously every 500ms :
    this->mPendingSegments = true;
}



void ServerGroup::checkServerStabilitySlot() {

    if (this->mStabilityCounter > MAX_SERVER_DOWN_PER_MINUTE) {

        // stop timer availability checking :
        this->mClientsAvailableTimer->stop();

        // set server unavailable for 5 minutes :
        this->mServerAvailable = false;
        this->serverSwitchIfFailure();

        QTimer::singleShot(5 * UtilityNamespace::MINUTES_TO_MILLISECONDS, this, SLOT(startTimerSlot()));

        qCDebug(KWOOTY_LOG) << "server stability issues, forced to unavailable during 5 minutes, group :" << this->mServerGroupId;
    }

    // reset counter :
    this->mStabilityCounter = 0;
}



void ServerGroup::serverSwitchIfFailure() {

    // availability of **master server** (master or active failover) has changed, notify server manager :
    if (this->isMasterServer() || this->isActiveFailover()) {

        qCDebug(KWOOTY_LOG) << "Master server group id : " << this->mServerGroupId << "available : " << this->mServerAvailable;
        this->mServerManager->masterServerAvailabilityChanges();

    }
    // availability of **backup server** has changed :
    else {

        // if backup server is now unavailable :
        if (!this->mServerAvailable) {

            qCDebug(KWOOTY_LOG) << "Backup server group id : " << this->mServerGroupId << "available : " << this->mServerAvailable;

            // current backup server is down, try to download pending downloads with another backup server if any :
            this->mServerManager->downloadWithAnotherBackupServer(this);
        }

    }

    // check that current server is not available / unavailable too frequently :
    this->mStabilityCounter++;

}







//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void ServerGroup::startTimerSlot() {
    this->mClientsAvailableTimer->start();
}


void ServerGroup::downloadPendingSegmentsSlot() {

    if (this->mPendingSegments) {

        // notify only nntpclients ready that pending data are waiting :
        foreach (ClientManagerConn* clientManagerConn, this->mClientManagerConnList) {

            if (clientManagerConn->isClientReady()) {
                clientManagerConn->dataHasArrivedSlot();
            }
        }

        // clients have been notified :
        this->mPendingSegments = false;
    }

}


void ServerGroup::checkServerAvailabilitySlot() {

    bool serverAvailableOld = this->mServerAvailable;

    int clientsNotReady = 0;

    // count the number of clients not ready to download :
    foreach (ClientManagerConn* clientManagerConn, this->mClientManagerConnList) {

        if (!clientManagerConn->isClientReady()) {
            clientsNotReady++;
        }
    }

    // if all clients are not ready, the server is unavailable :
    if (clientsNotReady == this->mClientManagerConnList.size()) {
        this->mServerAvailable = false;
    }
    else {
        this->mServerAvailable = true;
    }

    // server has been disabled in settings, consider it as unavailable :
    if (this->isDisabledBackupServer()) {
        this->mServerAvailable = false;
    }

    // server availabilty has changed :
    if (this->mServerAvailable != serverAvailableOld) {
        this->serverSwitchIfFailure();
    }


}



bool ServerGroup::settingsServerChangedSlot() {

    // 1. ajust connection objects according to value set in settings :
    // if more nntp connections are requested :
    int connectionNumber = KConfigGroupHandler::getInstance()->serverConnectionNumber(this->mServerGroupId);

    if (connectionNumber > this->mClientManagerConnList.size()) {

        int connectionDelay = 0;
        for (int i = this->mClientManagerConnList.size(); i < connectionNumber; ++i){

            this->mClientManagerConnList.append(new ClientManagerConn(this, i, connectionDelay));

            //set a delay of 100ms between each new connection :
            connectionDelay += 100;

        }
    }

    // if less nntp connections are requested :
    if (connectionNumber < this->mClientManagerConnList.size()) {

        while (this->mClientManagerConnList.size() > connectionNumber) {
            this->mClientManagerConnList.takeLast()->deleteLater();
        }

    }

    // read new server config :
    ServerData newServerData = KConfigGroupHandler::getInstance()->readServerSettings(this->mServerGroupId);

    bool serverSettingsChanged = false;

    // if config changed :
    if (this->mServerData != newServerData) {

        // update new settings right now as they will used by nntpclients :
        this->mServerData = newServerData;

        // reset stability counter :
        this->mStabilityCounter = 0;

        // notity manager that some settings have changed :
        serverSettingsChanged = true;

    }


    // name of server may have changed, update it in order to synchronize tab name in side bar ;
    this->mServerData.setServerName(newServerData.getServerName());


    return serverSettingsChanged;

}



