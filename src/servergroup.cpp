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
#include "segmentmanager.h"
#include "centralwidget.h"
#include "clientmanagerconn.h"
#include "nntpclient.h"
#include "preferences/kconfiggrouphandler.h"
#include "kwootysettings.h"


ServerGroup::ServerGroup(ServerManager* parent, CentralWidget* centralWidget, int serverGroupId) : QObject(parent) {

    this->serverManager = parent;
    this->centralWidget = centralWidget;
    this->serverGroupId = serverGroupId;
    this->serverAvailable = true;
    this->pendingSegments = false;
    this->stabilityCounter = 0;

    // retrieve server settings :
    this->serverData = KConfigGroupHandler::getInstance()->readServerSettings(this->serverGroupId);

    // connect clients :
    this->createNntpClients();

    // check server availabilty every 500 ms :
    this->clientsAvailableTimer = new QTimer();
    this->clientsAvailableTimer->start(500);

    // check server stability every minutes :
    this->stabilityTimer = new QTimer();
    this->stabilityTimer->start(1 * UtilityNamespace::MINUTES_TO_MILLISECONDS);


    this->setupConnections();

}



void ServerGroup::setupConnections() {

    // check that server is available or not :
    connect(clientsAvailableTimer, SIGNAL(timeout()), this, SLOT(checkServerAvailabilitySlot()));

    // check if pending segments are ready for the current server :
    connect(clientsAvailableTimer, SIGNAL(timeout()), this, SLOT(downloadPendingSegmentsSlot()));

    // check server stability :
    connect(stabilityTimer, SIGNAL(timeout()), this, SLOT(checkServerStabilitySlot()));

}


int ServerGroup::getRealServerGroupId() const {
    // used for debugging puropses only :
    return this->serverGroupId;
}

int ServerGroup::getServerGroupId() const {

    // default server group id :
    int currentServerGroupId = this->serverGroupId;

    // if server is now configured in Active mode, return MasterServer groupId
    // in order be considered as another master server and to be able to download
    // pending segment targeted with MasterServer Id :
    if (this->isActiveBackupServer()) {
        currentServerGroupId = MasterServer;
    }

    // if backup server is in Failover mode and that master server is down
    // the current backup server may be considered as master server :
    else if (this->isActiveFailover()) {
        currentServerGroupId = MasterServer;
    }

    return currentServerGroupId;
}


CentralWidget* ServerGroup::getCentralWidget() {
    return this->centralWidget;
}

ServerManager* ServerGroup::getServerManager() {
    return this->serverManager;
}


ServerData ServerGroup::getServerData() const {
    return this->serverData;
}


bool ServerGroup::isMasterServer() const {
    return (this->serverGroupId == MasterServer);
}


bool ServerGroup::isDisabledBackupServer() const {
    return (this->serverData.getServerModeIndex() == UtilityNamespace::DisabledServer);
}


bool ServerGroup::isPassiveBackupServer() const {

    bool passiveServer = false;

    // current server is in passive mode :
    if (this->serverData.getServerModeIndex() == UtilityNamespace::PassiveServer) {
        passiveServer = true;
    }
    // else if it is in failover mode, it will works as passive if it is not currently replacing a down master server :
    else if (this->isPassiveFailover()) {
        passiveServer = true;
    }

    return passiveServer;
}

bool ServerGroup::isActiveBackupServer() const {
    return (this->serverData.getServerModeIndex() == UtilityNamespace::ActiveServer);
}

bool ServerGroup::isFailoverBackupServer() const {
    return (this->serverData.getServerModeIndex() == UtilityNamespace::FailoverServer);
}

bool ServerGroup::isActiveFailover() const {
    return (this->isFailoverBackupServer() && this->serverManager->currentIsFirstMasterAvailable(this));
}

bool ServerGroup::isPassiveFailover() const {
    return (this->isFailoverBackupServer() && !this->serverManager->currentIsFirstMasterAvailable(this));
}


bool ServerGroup::isServerAvailable() const {
    return this->serverAvailable;
}



void ServerGroup::createNntpClients() {

    // create the nntp clients thread manager :
    int connectionNumber = KConfigGroupHandler::getInstance()->serverConnectionNumber(this->serverGroupId);

    // set a delay of +100 ms between each nntp client instance :
    int connectionDelay = 0;

    for (int i = 0; i < connectionNumber; i++) {
        this->clientManagerConnList.append(new ClientManagerConn(this, i, connectionDelay));
        connectionDelay += 100;
    }

}



void ServerGroup::disconnectAllClients() {

    // stop timer that notify if clients are available or not :
    this->clientsAvailableTimer->stop();

    // disconnect all clients :
    emit disconnectRequestSignal();
}


void ServerGroup::connectAllClients() {

    // connect all clients :
    emit connectRequestSignal();

    // restart timer that notify if clients are available :
    this->stabilityCounter = 0;
    this->serverAvailable = true;
    QTimer::singleShot(500 * this->serverGroupId, this, SLOT(startTimerSlot()));

}


void ServerGroup::assignDownloadToReadyClients() {

    // do not hammer backup servers that new segments are available,
    // it will be done asynchronously every 500ms :
    this->pendingSegments = true;
}



void ServerGroup::checkServerStabilitySlot() {

    if (this->stabilityCounter > MAX_SERVER_DOWN_PER_MINUTE) {

        // stop timer availability checking :
        this->clientsAvailableTimer->stop();

        // set server unavailable for 5 minutes :
        this->serverAvailable = false;
        this->serverSwitchIfFailure();

        QTimer::singleShot(5 * UtilityNamespace::MINUTES_TO_MILLISECONDS, this, SLOT(startTimerSlot()));

        kDebug() << "server stability issues, forced to unavailable for 5 minutes, group :" << this->serverGroupId;
    }

    // reset counter :
    this->stabilityCounter = 0;
}



void ServerGroup::serverSwitchIfFailure() {

    // availability of **master server** (master or active failover) has changed, notify server manager :
    if (this->isMasterServer() || this->isActiveFailover()) {

        kDebug() << "Master server group id : " << this->serverGroupId << "available : " << this->serverAvailable;
        this->serverManager->masterServerAvailabilityChanges();

    }
    // availability of **backup server** has changed :
    else {

        // if backup server is now unavailable :
        if (!this->serverAvailable) {

            kDebug() << "Backup server group id : " << this->serverGroupId << "available : " << this->serverAvailable;

            // current backup server is down, try to download pending downloads with another backup server if any :
            this->serverManager->downloadWithAnotherBackupServer(this->serverGroupId);
        }

    }

    // check that current server is not available / unavailable too frequently :
    this->stabilityCounter++;

}





//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void ServerGroup::startTimerSlot() {
    this->clientsAvailableTimer->start();
}


void ServerGroup::downloadPendingSegmentsSlot() {

    if (this->pendingSegments) {

        // notify only nntpclients ready that pending data are waiting :
        foreach (ClientManagerConn* clientManagerConn, this->clientManagerConnList) {

            if (clientManagerConn->isClientReady()) {
                clientManagerConn->getNntpClient()->dataHasArrivedSlot();
            }
        }

        // clients have been notified :
        this->pendingSegments = false;
    }

}


void ServerGroup::checkServerAvailabilitySlot() {

    bool serverAvailableOld = this->serverAvailable;

    int clientsNotReady = 0;

    // count the number of clients not ready to download :
    foreach (ClientManagerConn* clientManagerConn, this->clientManagerConnList) {

        if (!clientManagerConn->isClientReady()) {
            clientsNotReady++;
        }
    }

    // if all clients are not ready, the server is unavailable :
    if (clientsNotReady == this->clientManagerConnList.size()) {
        this->serverAvailable = false;
    }
    else {
        this->serverAvailable = true;
    }

    // server has been disabled in settings, consider it as unavailable :
    if (this->isDisabledBackupServer()) {
        this->serverAvailable = false;
    }

    // server availabilty has changed :
    if (this->serverAvailable != serverAvailableOld) {
        this->serverSwitchIfFailure();
    }
}



bool ServerGroup::settingsServerChangedSlot() {

    // 1. ajust connection objects according to value set in settings :
    // if more nntp connections are requested :
    int connectionNumber = KConfigGroupHandler::getInstance()->serverConnectionNumber(this->serverGroupId);

    if (connectionNumber > this->clientManagerConnList.size()) {

        int connectionDelay = 0;
        for (int i = this->clientManagerConnList.size(); i < connectionNumber; i++){

            this->clientManagerConnList.append(new ClientManagerConn(this, i, connectionDelay));

            //set a delay of 100ms between each new connection :
            connectionDelay += 100;

        }
    }

    // if less nntp connections are requested :
    if (connectionNumber < this->clientManagerConnList.size()) {

        while (this->clientManagerConnList.size() > connectionNumber) {
            this->clientManagerConnList.takeLast()->deleteLater();
        }

    }

    // read new server config :
    ServerData newServerData = KConfigGroupHandler::getInstance()->readServerSettings(this->serverGroupId);

    bool serverSettingsChanged = false;

    // if config changed :
    if (this->serverData != newServerData) {

        // update new settings right now as they will used by nntpclients :
        this->serverData = newServerData;

        // reset stability counter :
        this->stabilityCounter = 0;

        // notity manager that some settings have changed :
        serverSettingsChanged = true;

    }

    return serverSettingsChanged;

}



