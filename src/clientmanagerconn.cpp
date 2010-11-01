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

#include <KDebug>
#include <KGlobal>

#include "kwootysettings.h"
#include "centralwidget.h"
#include "servergroup.h"
#include "segmentmanager.h"
#include "servermanager.h"
#include "nntpclient.h"
#include "data/segmentinfodata.h"
#include "observers/clientsperserverobserver.h"
#include "preferences/kconfiggrouphandler.h"
#include "utility.h"
using namespace UtilityNamespace;



ClientManagerConn::ClientManagerConn() : QObject(){}


ClientManagerConn::ClientManagerConn(ServerGroup* parent, int clientId, int connectionDelay) : QObject(parent)
{

    this->nntpClient = 0;
    this->parent = parent;

    // client identifier :
    this->clientId = clientId;

    // start each nntpclient instance with a delay in order to not hammer the host :
    this->connectionDelay = connectionDelay;
    QTimer::singleShot(this->connectionDelay, this, SLOT(initSlot()) );

}


ClientManagerConn::~ClientManagerConn()
{
}

ServerGroup* ClientManagerConn::getServerGroup() {
    return this->parent;
}



void ClientManagerConn::initSlot()
{

    CentralWidget* centralWidget = this->parent->getCentralWidget();

    // create nntp socket :
    this->nntpClient = new NntpClient(this);

    // ServerGroup notify all nntpClient instances that connection settings changed :
    connect (centralWidget,
             SIGNAL(dataHasArrivedSignal()),
             this,
             SLOT(dataHasArrivedSlot()));


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


    // ask to segment manager to send a new segment to download :
    connect (this->nntpClient,
             SIGNAL(getNextSegmentSignal(ClientManagerConn*)),
             centralWidget->getSegmentManager(),
             SLOT(getNextSegmentSlot(ClientManagerConn*)));

    // send to centralWidget segment data update :
    qRegisterMetaType<SegmentData>("SegmentData");
    connect (this->nntpClient,
             SIGNAL(updateDownloadSegmentSignal(SegmentData)),
             centralWidget->getSegmentManager(),
             SLOT(updateDownloadSegmentSlot(SegmentData)));

    // notify centralWidget that error occured during file save process :
    connect (this->nntpClient,
             SIGNAL(saveFileErrorSignal(const int)),
             centralWidget,
             SLOT(saveFileErrorSlot(const int)));


    // send connection status (connected, deconnected) to client observer for the current server :
    connect (this->nntpClient,
             SIGNAL(connectionStatusPerServerSignal(const int)),
             this->parent->getClientsPerServerObserver(),
             SLOT(connectionStatusPerServerSlot(const int)));


    // send type of encryption used by host with ssl connection to client observer for the current server :
    connect (this->nntpClient,
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
    return clientId;
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




bool ClientManagerConn::isClientReady() const {

    bool clientReady = false;

    if (this->nntpClient &&
        this->nntpClient->isClientReady()) {

        clientReady = true;
    }

    return clientReady;

}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ClientManagerConn::dataHasArrivedSlot() {
    // due to delay instanciation,
    // be sure that the instance has been really created before requesting segments to download :
    if (this->nntpClient) {
        this->nntpClient->dataHasArrivedSlot();
    }
}



void ClientManagerConn::disconnectRequestSlot() {
    this->nntpClient->disconnectRequestByManager();
}

void ClientManagerConn::connectRequestSlot() {
    this->nntpClient->connectRequestByManager();
}
