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
#include <QPointer>
#include "settings.h"
#include "centralwidget.h"
#include "segmentmanager.h"
#include "mystatusbar.h"
#include "nntpclient.h"
#include "utility.h"
using namespace UtilityNamespace;



ClientManagerConn::ClientManagerConn() : QObject(){}


ClientManagerConn::ClientManagerConn(CentralWidget* inParent, int clientId, int connectionDelay) : QObject(inParent)
{

    this->parent = inParent;

    // client identifier :
    this->clientId = clientId;

    // start each nntpclient instance with a delay in order to not hammer the host :
    this->connectionDelay = connectionDelay;
    QTimer::singleShot(connectionDelay, this, SLOT(initSlot()) );

}


ClientManagerConn::~ClientManagerConn()
{
}


void ClientManagerConn::initSlot()
{

    // create nntp socket :
    this->nntpClient = new NntpClient(this);
    this->updateSettings();

    // parent notify all nntpClient instances that data to download is present :
    connect (parent,
             SIGNAL(dataHasArrivedSignal()),
             nntpClient,
             SLOT(dataHasArrivedSlot()));

    // parent notify all nntpClient instances that connection settings changed :
    connect (parent,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));

    // ask to segment manager to send a new segment to download :
    connect (nntpClient,
             SIGNAL(getNextSegmentSignal(ClientManagerConn*)),
             parent->getSegmentManager(),
             SLOT(getNextSegmentSlot(ClientManagerConn*)));

    // send to parent segment data update :
    qRegisterMetaType<SegmentData>("SegmentData");
    connect (nntpClient,
             SIGNAL(updateDownloadSegmentSignal(SegmentData)),
             parent->getSegmentManager(),
             SLOT(updateDownloadSegmentSlot(SegmentData)));

    // notify parent that error occured during file save process :
    connect (nntpClient,
             SIGNAL(saveFileErrorSignal(int)),
             parent,
             SLOT(saveFileErrorSlot(int)));


    // send connection status (connected, deconnected) to status bar :
    connect (nntpClient,
             SIGNAL(connectionStatusSignal(const int)),
             parent->getStatusBar(),
             SLOT(connectionStatusSlot(const int)));

    // send type of encryption used by host with ssl connection to status bar :
    connect (nntpClient,
             SIGNAL(encryptionStatusSignal(const bool, const QString, const bool, const QString)),
             parent->getStatusBar(),
             SLOT(encryptionStatusSlot(const bool, const QString, const bool, const QString)));

    // send eventual socket error to status bar :
    connect (nntpClient,
             SIGNAL(nntpErrorSignal(const int)),
             parent->getStatusBar(),
             SLOT(nntpErrorSlot(const int)));

    // send bytes downloaded to status bar :
    connect (nntpClient,
             SIGNAL(speedSignal(const int)),
             parent->getStatusBar(),
             SLOT(speedSlot(const int)));

}



void ClientManagerConn::noSegmentAvailable () {
    // if the getNextSegmentSignal return this slot, there is no item to download
    // set the client to Idle Status :
    nntpClient->setConnectedClientStatus(NntpClient::ClientIdle);
}




void ClientManagerConn::processNextSegment(SegmentData inCurrentSegmentData){
    nntpClient->downloadNextSegment(inCurrentSegmentData);
}




int ClientManagerConn::getClientId() const{
    return clientId;
}




void ClientManagerConn::settingsChangedSlot() {

    if ( (hostName != Settings::hostName()) ||
         (port != Settings::port()) ||
         (authentication != Settings::groupBoxAuthentication()) ||
         (login != Settings::login()) ||
         (password != Settings::password()) ||
         (disconnectTimeout != Settings::disconnectTimeout()) ||
         (enableSSL != Settings::enableSSL()) ) {

        // if connection settings changed, reconnect with new settings :
        QTimer::singleShot(connectionDelay, nntpClient, SLOT(answerTimeOutSlot()) );
        this->updateSettings();

    }
}


void ClientManagerConn::updateSettings() {

    // update settings :
    hostName = Settings::hostName();
    port = Settings::port();
    authentication = Settings::groupBoxAuthentication();
    login = Settings::login();
    password = Settings::password();
    disconnectTimeout = Settings::disconnectTimeout();
    enableSSL = Settings::enableSSL();
}

