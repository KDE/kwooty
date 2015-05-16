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

ClientManagerConn::ClientManagerConn(ServerGroup *parent, int clientId, int connectionDelay) : QObject(parent)
{

    this->mNntpClient = 0;
    this->mParent = parent;
    this->mBandwidthClientMode = BandwidthFull;

    // client identifier :
    this->mClientId = clientId;

    // start each nntpclient instance with a delay in order to not hammer the host,
    // moreover the first instance will be created *after first event loop* to be sure that the whole application
    // has been fully started (i.e : connections with all signals/slots are ok) :
    this->mConnectionDelay = connectionDelay;

    // create nntp socket :
    this->mNntpClient = new NntpClient(this);

    this->initSlot();

}

ClientManagerConn::~ClientManagerConn()
{
}

ServerGroup *ClientManagerConn::getServerGroup()
{
    return this->mParent;
}

void ClientManagerConn::initSlot()
{

    Core *core = this->mParent->getCore();

    connect(core->getMainWindow(),
            SIGNAL(startupCompleteSignal()),
            this,
            SLOT(startupCompleteSlot()));

    // ServerGroup notify all nntpClient instances that connection settings changed :
    connect(core,
            SIGNAL(dataHasArrivedSignal()),
            this,
            SLOT(dataHasArrivedSlot()));

    // ServerGroup asked for connection reset :
    connect(this->mParent,
            SIGNAL(resetConnectionSignal()),
            this,
            SLOT(resetConnectionSlot()));

    // ServerGroup asked for clients disconnection :
    connect(this->mParent,
            SIGNAL(disconnectRequestSignal()),
            this,
            SLOT(disconnectRequestSlot()));

    // ServerGroup asked for clients connection :
    connect(this->mParent,
            SIGNAL(connectRequestSignal()),
            this,
            SLOT(connectRequestSlot()));

    // manage bandwidth download speed:
    connect(this->mParent->getServerSpeedManager(),
            SIGNAL(limitDownloadSpeedSignal(BandwidthClientMode)),
            this,
            SLOT(limitDownloadSpeedSlot(BandwidthClientMode)));

    // ask to segment manager to send a new segment to download :
    connect(this->mNntpClient,
            SIGNAL(getNextSegmentSignal(ClientManagerConn*)),
            core->getSegmentManager(),
            SLOT(getNextSegmentSlot(ClientManagerConn*)));

    // send to segmentManager segment data update :
    qRegisterMetaType<SegmentData>("SegmentData");
    connect(this->mNntpClient,
            SIGNAL(updateDownloadSegmentSignal(SegmentData)),
            core->getSegmentManager(),
            SLOT(updateDownloadSegmentSlot(SegmentData)));

    // send connection status (connected, deconnected) to client observer for the current server :
    connect(this->mNntpClient,
            SIGNAL(connectionStatusPerServerSignal(int)),
            this->mParent->getClientsPerServerObserver(),
            SLOT(connectionStatusPerServerSlot(int)));

    // send type of encryption used by host with ssl connection to client observer for the current server :
    connect(this->mNntpClient->getTcpSocket(),
            SIGNAL(encryptionStatusPerServerSignal(bool,QString,bool,QString,QStringList)),
            this->mParent->getClientsPerServerObserver(),
            SLOT(encryptionStatusPerServerSlot(bool,QString,bool,QString,QStringList)));

    // send eventual socket error to client observer for the current server :
    connect(this->mNntpClient,
            SIGNAL(nntpErrorPerServerSignal(int)),
            this->mParent->getClientsPerServerObserver(),
            SLOT(nntpErrorPerServerSlot(int)));

    // send bytes downloaded to client observer for the current server :
    qRegisterMetaType<SegmentInfoData>("SegmentInfoData");
    connect(this->mNntpClient,
            SIGNAL(speedPerServerSignal(SegmentInfoData)),
            this->mParent->getClientsPerServerObserver(),
            SLOT(nntpClientSpeedPerServerSlot(SegmentInfoData)));

}

void ClientManagerConn::noSegmentAvailable()
{
    // if the getNextSegmentSignal returns this method, there is no item to download
    // set the client to Idle Status :
    this->mNntpClient->noSegmentAvailable();

}

void ClientManagerConn::processNextSegment(const SegmentData &inCurrentSegmentData)
{
    this->mNntpClient->downloadNextSegment(inCurrentSegmentData);

}

NntpClient *ClientManagerConn::getNntpClient()
{
    return this->mNntpClient;
}

int ClientManagerConn::getClientId() const
{
    return this->mClientId;
}

int ClientManagerConn::getConnectionDelay() const
{
    return this->mConnectionDelay;
}

ServerData ClientManagerConn::getServerData() const
{
    return this->mParent->getServerData();
}

bool ClientManagerConn::isMasterServer() const
{
    return this->mParent->isMasterServer();
}

bool ClientManagerConn::isDisabledBackupServer() const
{
    return this->mParent->isDisabledBackupServer();
}

void ClientManagerConn::handleFirstConnection()
{

    // read data with password in order to open kwallet dialog box (if needed)
    // only when the first connection to server is performed :
    this->mParent->readDataWithPassword();
}

bool ClientManagerConn::isClientReady() const
{

    bool clientReady = false;

    if (this->mNntpClient &&
            this->mNntpClient->isClientReady()) {

        clientReady = true;
    }

    return clientReady;

}

void ClientManagerConn::setBandwidthMode(const BandwidthClientMode &bandwidthClientMode)
{

    BandwidthClientMode bandwidthClientModeOld = this->mBandwidthClientMode;
    this->mBandwidthClientMode = bandwidthClientMode;

    if (this->mBandwidthClientMode != bandwidthClientModeOld) {

        this->dataHasArrivedSlot();

    }

}

bool ClientManagerConn::isBandwidthNotNeeded() const
{
    return this->mBandwidthClientMode == BandwidthNotNeeded;
}

bool ClientManagerConn::isBandwidthLimited() const
{
    return this->mBandwidthClientMode == BandwidthLimited;
}

bool ClientManagerConn::isBandwidthFull() const
{
    return this->mBandwidthClientMode == BandwidthFull;
}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ClientManagerConn::startupCompleteSlot()
{

    // when startup is complete, do not connect to servers if session has been restored
    // and there is nothing to download :
    if (!this->mParent->getServerManager()->isSessionRestoredNoJobs()) {
        QTimer::singleShot(this->mConnectionDelay, this, SLOT(connectRequestSlot()));
    }

}

void ClientManagerConn::dataHasArrivedSlot()
{

    // this slot is called each time segments have been set pending for backup servers by ServerGroup,
    // if client is currently not used for limit speed purposes (disconnected), do not go further
    // as calling dataHasArrivedSlot() will have the effect to reconnect current client again :
    if (!this->isBandwidthNotNeeded()) {
        this->mNntpClient->dataHasArrivedSlot();
    }

}

void ClientManagerConn::resetConnectionSlot()
{

    // in case of retry action, reset connection between client and server to ensure that
    // the connection was not broken :
    if (this->mNntpClient->getTcpSocket()->isSocketConnected()) {

        // disconnect :
        this->disconnectRequestSlot();
        // reconnect :
        QTimer::singleShot(this->mConnectionDelay, this, SLOT(connectRequestSlot()));

    } else {
        this->connectRequestSlot();
    }

}

void ClientManagerConn::disconnectRequestSlot()
{
    this->mNntpClient->disconnectRequestByManager();
}

void ClientManagerConn::connectRequestSlot()
{
    this->mNntpClient->connectRequestByManager();
}

void ClientManagerConn::limitDownloadSpeedSlot(BandwidthClientMode bandwidthClientMode)
{
    this->setBandwidthMode(bandwidthClientMode);
}

