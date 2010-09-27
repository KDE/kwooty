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

#include <QTimer>
#include "kwootysettings.h"
#include "centralwidget.h"
#include "servergroup.h"
#include "segmentmanager.h"
#include "servermanager.h"
#include "clientsobserver.h"
#include "nntpclient.h"
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
    QTimer::singleShot(connectionDelay, this, SLOT(initSlot()) );

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
             nntpClient,
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
    connect (nntpClient,
             SIGNAL(getNextSegmentSignal(ClientManagerConn*)),
             centralWidget->getSegmentManager(),
             SLOT(getNextSegmentSlot(ClientManagerConn*)));

    // send to centralWidget segment data update :
    qRegisterMetaType<SegmentData>("SegmentData");
    connect (nntpClient,
             SIGNAL(updateDownloadSegmentSignal(SegmentData)),
             centralWidget->getSegmentManager(),
             SLOT(updateDownloadSegmentSlot(SegmentData)));

    // notify centralWidget that error occured during file save process :
    connect (nntpClient,
             SIGNAL(saveFileErrorSignal(int)),
             centralWidget,
             SLOT(saveFileErrorSlot(int)));


    // send connection status (connected, deconnected) to status bar :
    connect (nntpClient,
             SIGNAL(connectionStatusSignal(const int)),
             centralWidget->getClientsObserver(),
             SLOT(connectionStatusSlot(const int)));



    // send type of encryption used by host with ssl connection to status bar :
    connect (nntpClient,
             SIGNAL(encryptionStatusSignal(const bool, const QString, const bool, const QString, const QStringList)),
             centralWidget->getClientsObserver(),
             SLOT(encryptionStatusSlot(const bool, const QString, const bool, const QString, const QStringList)));

    // send eventual socket error to status bar :
    connect (nntpClient,
             SIGNAL(nntpErrorSignal(const int)),
             centralWidget->getClientsObserver(),
             SLOT(nntpErrorSlot(const int)));

    // send bytes downloaded to info collector dispatcher :
    connect (nntpClient,
             SIGNAL(speedSignal(const int)),
             centralWidget->getClientsObserver(),
             SLOT(nntpClientSpeedSlot(const int)));


}



void ClientManagerConn::noSegmentAvailable() {
    // if the getNextSegmentSignal returns this method, there is no item to download
    // set the client to Idle Status :
    nntpClient->noSegmentAvailable();

}



void ClientManagerConn::processNextSegment(SegmentData inCurrentSegmentData){
    nntpClient->downloadNextSegment(inCurrentSegmentData);
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




bool ClientManagerConn::isClientReady() {

    bool clientReady = false;

    if (this->nntpClient &&
        this->nntpClient->isClientReady()) {

        clientReady = true;
    }

    return clientReady;

}



void ClientManagerConn::disconnectRequestSlot() {
    this->nntpClient->disconnectRequestByManager();
}

void ClientManagerConn::connectRequestSlot() {
    this->nntpClient->connectRequestByManager();
}
