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

#include "clientmanagerconn.h"

#include "kwooty_debug.h"
#include <KGlobal>

#include "kwootysettings.h"
#include "core.h"
#include "mainwindow.h"
#include "servergroup.h"
#include "servermanager.h"
#include "segmentmanager.h"
#include "segmentbuffer.h"
#include "serverspeedmanager.h"
#include "nntpclient.h"
#include "nntpsocket.h"
#include "data/segmentinfodata.h"
#include "observers/clientsperserverobserver.h"
#include "preferences/kconfiggrouphandler.h"



ClientManagerConn::ClientManagerConn(ServerGroup* parent, int clientId, int connectionDelay) : QObject(parent) {

    this->nntpClient = 0;
    this->parent = parent;
    this->bandwidthClientMode = BandwidthFull;

    // client identifier :
    this->clientId = clientId;

    // start each nntpclient instance with a delay in order to not hammer the host,
    // moreover the first instance will be created *after first event loop* to be sure that the whole application
    // has been fully started (i.e : connections with all signals/slots are ok) :
    this->connectionDelay = connectionDelay;

    // create nntp socket :
    this->nntpClient = new NntpClient(this);

    this->initSlot();

}


ClientManagerConn::~ClientManagerConn()
{
}

ServerGroup* ClientManagerConn::getServerGroup() {
    return this->parent;
}



void ClientManagerConn::initSlot() {

    Core* core = this->parent->getCore();

    connect (core->getMainWindow(),
             SIGNAL(startupCompleteSignal()),
             this,
             SLOT(startupCompleteSlot()));

    // ServerGroup notify all nntpClient instances that connection settings changed :
    connect (core,
             SIGNAL(dataHasArrivedSignal()),
             this,
             SLOT(dataHasArrivedSlot()));

    // ServerGroup asked for connection reset :
    connect (this->parent,
             SIGNAL(resetConnectionSignal()),
             this,
             SLOT(resetConnectionSlot()));

    // ServerGroup asked for clients disconnection :
    connect (this->parent,
             SIGNAL(disconnectRequestSignal()),
             this,
             SLOT(disconnectRequestSlot()));

    // ServerGroup asked for clients connection :
    connect (this->parent,
             SIGNAL(connectRequestSignal()),
             this,
             SLOT(connectRequestSlot()));

    // manage bandwidth download speed:
    connect (this->parent->getServerSpeedManager(),
             SIGNAL(limitDownloadSpeedSignal(BandwidthClientMode)),
             this,
             SLOT(limitDownloadSpeedSlot(BandwidthClientMode)));

    // ask to segment manager to send a new segment to download :
    connect (this->nntpClient,
             SIGNAL(getNextSegmentSignal(ClientManagerConn*)),
             core->getSegmentManager(),
             SLOT(getNextSegmentSlot(ClientManagerConn*)));

    // send to segmentManager segment data update :
    qRegisterMetaType<SegmentData>("SegmentData");
    connect (this->nntpClient,
             SIGNAL(updateDownloadSegmentSignal(SegmentData)),
             core->getSegmentManager(),
             SLOT(updateDownloadSegmentSlot(SegmentData)));

    // send connection status (connected, deconnected) to client observer for the current server :
    connect (this->nntpClient,
             SIGNAL(connectionStatusPerServerSignal(const int)),
             this->parent->getClientsPerServerObserver(),
             SLOT(connectionStatusPerServerSlot(const int)));

    // send type of encryption used by host with ssl connection to client observer for the current server :
    connect (this->nntpClient->getTcpSocket(),
             SIGNAL(encryptionStatusPerServerSignal(const bool, const QString, const bool, const QString, const QStringList)),
             this->parent->getClientsPerServerObserver(),
             SLOT(encryptionStatusPerServerSlot(const bool, const QString, const bool, const QString, const QStringList)));

    // send eventual socket error to client observer for the current server :
    connect (this->nntpClient,
             SIGNAL(nntpErrorPerServerSignal(const int)),
             this->parent->getClientsPerServerObserver(),
             SLOT(nntpErrorPerServerSlot(const int)));

    // send bytes downloaded to client observer for the current server :
    qRegisterMetaType<SegmentInfoData>("SegmentInfoData");
    connect (this->nntpClient,
             SIGNAL(speedPerServerSignal(const SegmentInfoData)),
             this->parent->getClientsPerServerObserver(),
             SLOT(nntpClientSpeedPerServerSlot(const SegmentInfoData)));


}



void ClientManagerConn::noSegmentAvailable() {
    // if the getNextSegmentSignal returns this method, there is no item to download
    // set the client to Idle Status :
    this->nntpClient->noSegmentAvailable();

}


void ClientManagerConn::processNextSegment(const SegmentData& inCurrentSegmentData){
    this->nntpClient->downloadNextSegment(inCurrentSegmentData);

}


NntpClient* ClientManagerConn::getNntpClient() {
    return this->nntpClient;
}


int ClientManagerConn::getClientId() const {
    return this->clientId;
}


int ClientManagerConn::getConnectionDelay() const {
    return this->connectionDelay;
}

ServerData ClientManagerConn::getServerData() const {
    return this->parent->getServerData();
}


bool ClientManagerConn::isMasterServer() const {
    return this->parent->isMasterServer();
}

bool ClientManagerConn::isDisabledBackupServer() const {
    return this->parent->isDisabledBackupServer();
}


void ClientManagerConn::handleFirstConnection() {

    // read data with password in order to open kwallet dialog box (if needed)
    // only when the first connection to server is performed :
    this->parent->readDataWithPassword();
}


bool ClientManagerConn::isClientReady() const {

    bool clientReady = false;

    if (this->nntpClient &&
            this->nntpClient->isClientReady()) {

        clientReady = true;
    }

    return clientReady;

}


void ClientManagerConn::setBandwidthMode(const BandwidthClientMode& bandwidthClientMode) {

    BandwidthClientMode bandwidthClientModeOld = this->bandwidthClientMode;
    this->bandwidthClientMode = bandwidthClientMode;

    if (this->bandwidthClientMode != bandwidthClientModeOld) {

        this->dataHasArrivedSlot();

    }

}


bool ClientManagerConn::isBandwidthNotNeeded() const {
    return this->bandwidthClientMode == BandwidthNotNeeded;
}


bool ClientManagerConn::isBandwidthLimited() const {
    return this->bandwidthClientMode == BandwidthLimited;
}

bool ClientManagerConn::isBandwidthFull() const {
    return this->bandwidthClientMode == BandwidthFull;
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ClientManagerConn::startupCompleteSlot() {

    // when startup is complete, do not connect to servers if session has been restored
    // and there is nothing to download :
    if (!this->parent->getServerManager()->isSessionRestoredNoJobs()) {
        QTimer::singleShot(this->connectionDelay, this, SLOT(connectRequestSlot()));
    }

}

void ClientManagerConn::dataHasArrivedSlot() {

    // this slot is called each time segments have been set pending for backup servers by ServerGroup,
    // if client is currently not used for limit speed purposes (disconnected), do not go further
    // as calling dataHasArrivedSlot() will have the effect to reconnect current client again :
    if (!this->isBandwidthNotNeeded()) {
        this->nntpClient->dataHasArrivedSlot();
    }

}


void ClientManagerConn::resetConnectionSlot() {

    // in case of retry action, reset connection between client and server to ensure that
    // the connection was not broken :
    if (this->nntpClient->getTcpSocket()->isSocketConnected()) {

        // disconnect :
        this->disconnectRequestSlot();
        // reconnect :
        QTimer::singleShot(this->connectionDelay, this, SLOT(connectRequestSlot()));

    }
    else {
        this->connectRequestSlot();
    }

}


void ClientManagerConn::disconnectRequestSlot() {
    this->nntpClient->disconnectRequestByManager();
}

void ClientManagerConn::connectRequestSlot() {
    this->nntpClient->connectRequestByManager();
}


void ClientManagerConn::limitDownloadSpeedSlot(BandwidthClientMode bandwidthClientMode) {
    this->setBandwidthMode(bandwidthClientMode);
}

