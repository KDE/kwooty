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

    this->totalConnections = 0;

    this->createNntpClients();


    this->clientsAvailableTimer = new QTimer();
    this->clientsAvailableTimer->start(500);

    this->setupConnections();

}


int ServerGroup::getServerGroupId() const {
    return this->serverGroupId;
}


CentralWidget* ServerGroup::getCentralWidget() {
    return this->centralWidget;
}

ServerManager* ServerGroup::getServerManager() {
    return this->serverManager;
}


void ServerGroup::setServerGroupId(const int serverGroupId) {
    this->serverGroupId = serverGroupId;
}


//
//void ServerGroup::addServerFromUnavailableSet(const int serverGroupId)  {
//    this->unavailableServerSet.insert(serverGroupId);
//}
//
//void ServerGroup::removeServerFromUnavailableSet(const int serverGroupId)  {
//    this->unavailableServerSet.remove(serverGroupId);
//}
//
//QSet<int> ServerGroup::getUnavailableSet() const {
//    return this->unavailableServerSet;
//}






void ServerGroup::setupConnections() {

    // parent notify that settings have been changed :
    connect (this->centralWidget,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));


    connect(clientsAvailableTimer, SIGNAL(timeout()), this, SLOT(checkServerAvailabilitySlot()));

    connect(clientsAvailableTimer, SIGNAL(timeout()), this, SLOT(downloadPendingSegmentsSlot()));

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



void ServerGroup::settingsChangedSlot() {
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


}


void ServerGroup::assignDownloadToReadyClients() {

    // do not hammer backup servers that new segments are available,
    // it will be done asynchronously every 500ms :
    this->pendingSegments = true;
}



void ServerGroup::downloadPendingSegmentsSlot() {

    if (this->pendingSegments) {

        foreach (ClientManagerConn* clientManagerConn, this->clientManagerConnList) {

            if (clientManagerConn->isClientReady()) {
                clientManagerConn->getNntpClient()->dataHasArrivedSlot();
                //break;
            }
        }

        this->pendingSegments = false;
    }

}





void ServerGroup::checkServerAvailabilitySlot() {

    if ( (this->serverGroupId != MasterServer)) {

        bool serverAvailableOld = this->serverAvailable;

        int clientsNotReady = 0;

        foreach (ClientManagerConn* clientManagerConn, this->clientManagerConnList) {

            if (!clientManagerConn->isClientReady()) {
                clientsNotReady++;
            }

        }

        if (clientsNotReady == this->clientManagerConnList.size()) {
            this->serverAvailable = false;
        }
        else {
            this->serverAvailable = true;
        }



        // if availability of the server has changed and this is not the master server :
        if ( (this->serverAvailable != serverAvailableOld) &&
             (this->serverGroupId != MasterServer) )  {

            kDebug() << "Is server now available ? :" << this->serverGroupId << this->serverAvailable;

            // current backup server is down, try to download pending downloads with
            // another backup server if any :
            this->serverManager->downloadWithAnotherBackupServer(this->serverGroupId);

        }

    }

}


bool ServerGroup::isServerAvailable() {
    return this->serverAvailable;
}







