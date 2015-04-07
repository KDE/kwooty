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

#include "nntpclient.h"

#include "kwooty_debug.h"
#include <QTimer>
#include <QFile>
#include <QBuffer>

#include "servergroup.h"
#include "serverspeedmanager.h"
#include "kwootysettings.h"
#include "clientmanagerconn.h"
#include "nntpsocket.h"
#include "servermanager.h"
#include "data/serverdata.h"


NntpClient::NntpClient(ClientManagerConn* parent) : QObject (parent) {

    this->parent = parent;

    // instantiate socket :
    this->tcpSocket = new NntpSocket(parent);

    this->authenticationDenied = false;
    this->segmentProcessed = false;
    this->nntpError = NoError;
    this->updateServerAnswerStatus(ServerDisconnected);

    // set client status to IdleStatus by default :
    this->clientStatus = ClientIdle;

    // set up connections with tcpSocket :
    this->setupConnections();

}


NntpClient::~NntpClient() {

    // quit :
    this->segmentProcessed = true;
    this->tcpSocket->sendQuitCommandToServer();
    this->segmentDataRollBack();
    this->tcpSocket->abort();

}


void NntpClient::setupConnections() {

    connect (this->tcpSocket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    connect (this->tcpSocket, SIGNAL(connected()), this, SLOT(connectedSlot()));
    connect (this->tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
    connect (this->tcpSocket, SIGNAL(tryToReconnectSignal()), this, SLOT(connectToHostSlot()));
    connect (this->tcpSocket, SIGNAL(answerTimeOutSignal()), this, SLOT(answerTimeOutSlot()));
    connect (this->tcpSocket, SIGNAL(downloadSegmentFromServerSignal()), this, SLOT(downloadSegmentFromServerSlot()));
    connect (this->tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSlot(QAbstractSocket::SocketError)));

}

//-------------------------------------------------------------------------------------//
//                         connect and response management :                           //
//-------------------------------------------------------------------------------------//


void NntpClient::connectToHostSlot() {

    // connect if this client belongs to master server or if the backup server is not disabled :
    if ( this->parent->isMasterServer() ||
         !this->parent->isDisabledBackupServer() ) {

        this->parent->handleFirstConnection();

        // set nntpError to noError by default before connection process :
        this->updateServerAnswerStatus(ServerDisconnected);
        this->nntpError = NoError;
        this->connectingLoopCounter = 0;

        this->tcpSocket->connectToHost();

    }

}


void NntpClient::getAnswerFromServer() {

    // server answered :
    this->updateServerAnswerStatus(ServerFirstAnswerSent);

    // get answer from server :
    int answer = this->tcpSocket->readAnswer();

    switch (answer) {

    case ServerIsReadyPosting: case ServerIsReadyNoPosting: {

            this->setConnectedClientStatus(ClientIdle);

            // server returns posting ok, requesting a segment to download can be performed :
            this->updateServerAnswerStatus(ServerConnectedPostingOk);
            this->requestNewSegment();
            break;
        }

    case AuthenticationRequired: {
            // authentication name requested :

            if (this->parent->getServerData().isAuthentication()) {
                this->tcpSocket->sendUserCommandToServer(parent->getServerData().getLogin());
            }
            else{
                // group box is unchecked but authentication needed, inform the user :
                this->nntpError = AuthenticationNeeded;
                this->authenticationDenied = true;

                this->tcpSocket->sendQuitCommandToServer();
            }
            break;
        }

    case PasswordRequested: {
            // authentication password requested :

            if (this->parent->getServerData().isAuthentication()) {
                this->tcpSocket->sendPasswordCommandToServer(parent->getServerData().getPassword());
            }
            else{
                // group box is uncheked but authentication needed, inform the user :
                this->nntpError = AuthenticationNeeded;
                this->authenticationDenied = true;

                this->tcpSocket->sendQuitCommandToServer();
            }
            break;
        }

    case AuthenticationAccepted: {

            this->authenticationDenied = false;

            this->tcpSocket->sendBodyCommandToServer(currentSegmentData.getPart());
            break;
        }

    case AuthenticationDenied: case AuthenticationRejected: {

            this->authenticationDenied = true;

            // set type of error in order to notify status bar :
            this->nntpError = AuthenticationFailed;

            // In case of authentication denied, try to reconnect to host in 30 seconds :
            int reconnectSeconds = 30000;

            // if authentication denied on backup server, try to reconnect later :
            if (!this->parent->isMasterServer()) {
                reconnectSeconds = 3 * UtilityNamespace::MINUTES_TO_MILLISECONDS;
            }

            this->tcpSocket->quitAndReconnectInMs(reconnectSeconds);

            qCDebug(KWOOTY_LOG) << "AuthenticationDenied,  try to reconnect in " <<  reconnectSeconds << " seconds" << "group :" << parent->getServerGroup()->getRealServerGroupId();
            break;
        }

    case BodyArticleFollows: {

            // set status of client to ClientDownload :
            this->setConnectedClientStatus(ClientDownload);

            // set file identifier in order to delete incomplete downloads at next launch :
            this->segmentByteArray.clear();
            this->segmentByteArray.append(applicationFileOwner);

            // manage download segment :
            this->downloadSegmentFromServerSlot();
            break;
        }

    case NoSuchArticleMessageId: case NoSuchArticleNumber: {
            this->postDownloadProcess(NotPresent);
            break;
        }

    case IdleTimeout: case QuitFromServer: {
            this->setConnectedClientStatus(ClientIdle);
            break;
        }

    case CommandNotPerformed: case TransfertFailed: {
            // request new segment with short delay (10 seconds) :
            this->retryDownloadDelayed(10);
            break;
        }

    case AccessDenied: {
            // could occur if too many connection, bad credential or ssl connection is denied,
            // request new segment with long delay (1 minute) :
            this->retryDownloadDelayed(60);
            break;
        }

    default: {
            qCDebug(KWOOTY_LOG) << "Answer from host : " << answer << " not handled !" << "group :" << parent->getServerGroup()->getRealServerGroupId();
            // response not handled, consider that segment is not present :
            this->postDownloadProcess(NotPresent);
            break;
        }
    }
}


bool NntpClient::isClientReady() {

    bool clientReady = true;

    // authentication denied, client is not ready :
    if (this->authenticationDenied) {
        clientReady = false;
    }

    // client is connected :
    if (this->tcpSocket->isSocketConnected()) {

        // reset connecting loop counter :
        this->connectingLoopCounter = 0;

        // server did not answer yet, consider client as ready until first answer :
        if (this->serverAnswerStatus == ServerFirstAnswerNotSent) {
            clientReady = true;
        }
        // server answered but posting is not ok :
        else if (this->serverAnswerStatus != ServerConnectedPostingOk) {
            clientReady = false;
        }

    }
    // client is not connected :
    else if (this->tcpSocket->isSocketUnconnected()) {

        // reset connecting loop counter :
        this->connectingLoopCounter = 0;

        // if client is not connected to server due to errors and posting was not ok when client
        // was previously connected :
        if ( this->tcpSocket->error() != QAbstractSocket::UnknownSocketError &&
             this->serverAnswerStatus != ServerDisconnectedPostingOk ) {

            clientReady = false;
        }
    }
    // client is probably connecting, consider it as not ready after 5 consecutive probing :
    else {
        if (this->connectingLoopCounter > MAX_CONNECTING_LOOP) {
            clientReady = false;
        }
        else {
            this->connectingLoopCounter++;
        }
    }

    if (!clientReady) {
        this->setConnectedClientStatus(ClientIdle, DoNotTouchTimers);
    }

    //qCDebug(KWOOTY_LOG) << this->parent->getServerGroup()->getRealServerGroupId() << this->tcpSocket->state();

    return clientReady;
}


void NntpClient::updateServerAnswerStatus(const ServerAnswerStatus serverAnswerStatus) {

    switch (serverAnswerStatus) {

    case ServerDisconnected: {

            // if posting was ok when connected, set it as posting ok when disconnected
            // in order to consider this server as available even when disconnected (can happen after an idle timeout) :
            if (this->serverAnswerStatus == ServerConnectedPostingOk) {           
                this->serverAnswerStatus = ServerDisconnectedPostingOk;      
            }
            else {
                this->serverAnswerStatus = ServerDisconnected;
            }
            break;
        }

    case ServerFirstAnswerSent: {

            if (this->serverAnswerStatus == ServerFirstAnswerNotSent) {
                this->serverAnswerStatus = ServerFirstAnswerSent;
            }
            break;
        }

    default: {
            this->serverAnswerStatus = serverAnswerStatus;
            break;

        }

    }
}


void NntpClient::setConnectedClientStatus(const NntpClientStatus status, const TimerJob timerJob) {

    // update client status :
    this->clientStatus = status;

    if ( this->clientStatus != ClientDownload &&
         this->currentSegmentData.isInitialized() ) {

        this->postProcessIfBackupServer(DoNotRequestNewSegment);
        this->segmentDataRollBack();

    }

    this->tcpSocket->notifyClientStatus(this->clientStatus, timerJob);

}


void NntpClient::disconnectRequestByManager() {

    this->authenticationDenied = false;
    this->updateServerAnswerStatus(ServerDisconnected);

    this->segmentDataRollBack();

    this->tcpSocket->sendQuitCommandToServer();
    this->tcpSocket->abort();
}


void NntpClient::connectRequestByManager() {
    this->dataHasArrivedSlot();
}


NntpSocket* NntpClient::getTcpSocket() {
    return this->tcpSocket;
}


//-------------------------------------------------------------------------------------//
//                                download management :                                //
//-------------------------------------------------------------------------------------//

void NntpClient::requestNewSegment() {

    if (this->serverAnswerStatus == ServerConnectedPostingOk) {

        if (!this->parent->getServerGroup()->isBufferFull()) {

            // set client ready for receiving next segment :
            this->setConnectedClientStatus(ClientSegmentRequest);

            // request a new segment :
            emit getNextSegmentSignal(parent);

        }
        // buffer is full, lets time to decode pending segments,
        // retry to download a new segment in 10 seconds :
        else {
            this->retryDownloadDelayed(10);
        }

    }

}


void NntpClient::noSegmentAvailable() {
    this->setConnectedClientStatus(ClientIdle);
}


void NntpClient::retryDownloadDelayed(const int& seconds) {

    this->tcpSocket->retryDownloadDelayed();

    // rollback segment right now :
    this->segmentDataRollBack();
    // then set client as Idle (in that order it's sure that segment will be requested to be downloaded again) :
    this->setConnectedClientStatus(ClientIdle);

    QTimer::singleShot(seconds * 1000, this, SLOT(dataHasArrivedSlot()));

}


void NntpClient::downloadNextSegment(const SegmentData& currentSegmentData) {

    // ensure that previous segment has been proceeded :
    this->segmentDataRollBack();

    this->currentSegmentData = currentSegmentData;

    // a new segment has arrived, indicate that this current segment has not been proceeded yet :
    this->segmentProcessed = false;

    if (this->parent->isBandwidthNotNeeded()) {
        
        this->segmentDataRollBack();
        this->tcpSocket->sendQuitCommandToServer();

    }
    else {

        // should not occur but due to asynchronous call, check that the
        // socket is in connected state before further processing :
        if (this->tcpSocket->isSocketConnected()) {
            // get body message from server :
            this->tcpSocket->sendBodyCommandToServer(currentSegmentData.getPart());
        }
        else {
            this->postProcessIfBackupServer(DoNotRequestNewSegment);
            this->segmentDataRollBack();
        }
    }

}


void NntpClient::downloadSegmentFromServerSlot() {

    if (this->clientStatus == ClientDownload) {

        // read available data :
        QByteArray chunckData;

        ServerSpeedManager* serverSpeedManager = this->parent->getServerGroup()->getServerSpeedManager();
        qint64 speedLimitInBytes = serverSpeedManager->getDownloadSpeedLimitInBytes();

        // if bandwidth is limited, read proper byte size :
        if (!this->parent->isBandwidthFull() && speedLimitInBytes > 0) {

            chunckData = this->tcpSocket->readChunck(speedLimitInBytes, serverSpeedManager->getEnabledClientNumber());

        }
        // read all available data :
        else {
            chunckData = this->tcpSocket->readAll();
        }

        this->segmentByteArray.append(chunckData);

        // send size of downloaded data to status bar and side bar :
        SegmentInfoData segmentInfoData = this->currentSegmentData.getSegmentInfoData();
        segmentInfoData.setBytesDownloaded(chunckData.size());

        emit speedPerServerSignal(segmentInfoData);

        // if end of download has been reached :
        if (this->segmentByteArray.endsWith("\r\n.\r\n")) {

            this->postDownloadProcess(Present);

        }
        // data are pending, next readyRead signal is expected before time-out :
        else {

            this->tcpSocket->dataReadPending();

            if (this->segmentByteArray.size() > 10 * NBR_BYTES_IN_MB) {

                // request new segment with short delay (10 seconds) :
                this->segmentByteArray.clear();
                this->retryDownloadDelayed(10);
                qCDebug(KWOOTY_LOG) << "ooops, segment size probably too big : " << Utility::convertByteHumanReadable(this->segmentByteArray.size());
            }

        }

    }

}


void NntpClient::postDownloadProcess(const UtilityNamespace::Article articlePresence) {

    if (!this->segmentProcessed) {

        this->tcpSocket->dataReadComplete();

        // consider that data will be correctly saved by default :
        bool segmentDownloadFinished = true;

        // save data only if article has been found on server :
        if (articlePresence == Present) {

            // replace new lines starting with double periods by a simple one (RFC 977)
            this->segmentByteArray.replace("\r\n..", "\r\n.");

        }

        // article has not been found, try to download it with backup servers :
        else if (articlePresence == NotPresent) {

            // check if a backup server is available to download current segmentData
            bool nextServerFound = this->downloadSegmentWithBackupServer();

            // segment will be processed by another server :
            if (nextServerFound) {

                // request new pending segment for the current server :
                this->requestNewSegment();
                segmentDownloadFinished = false;
            }

        }

        // if article has been saved, set status to DownloadFinishStatus and update segment
        if (segmentDownloadFinished) {

            int downloadNextSegmentDelaySec = this->notifyDownloadHasFinished(articlePresence);

            // request new segment with short delay (variable) :
            if (downloadNextSegmentDelaySec > 0) {
                this->retryDownloadDelayed(downloadNextSegmentDelaySec);
            }
            // else request new pending segment right now :
            else {
                this->requestNewSegment();
            }

        }

    }

}


void NntpClient::segmentDataRollBack() {

    // in case of download errors it can happen that segmentDataRollBack()
    // is called several times with the same segmentData instance
    // check that roll back occurs only once per segmentData :
    if (!this->segmentProcessed) {

        if (this->currentSegmentData.getStatus() == DownloadStatus) {

            //qCDebug(KWOOTY_LOG) << "segmentData roll back effective !";

            this->currentSegmentData.setStatus(IdleStatus);
            this->currentSegmentData.setProgress(PROGRESS_INIT);

            // update segment data status :
            emit updateDownloadSegmentSignal(this->currentSegmentData);

            this->segmentProcessed = true;
        }
    }

}


void NntpClient::postProcessIfBackupServer(NewSegmentRequest newSegmentRequest) {

    if (!this->segmentProcessed) {

        // this can occur with a backup server with incorrect settings or access denied,
        // skip the current segment and try to download with another backup server if any :
        if (!this->parent->isMasterServer()) {

            bool nextServerFound = this->downloadSegmentWithBackupServer();

            if (!nextServerFound) {

                // consider current segment has not found as there no other available backup server :
                this->notifyDownloadHasFinished(NotPresent);
            }

            // request new segment only in case this method call does not come from
            // disconnectedSlot() or errorSlot() as next segment request must be done after connection process :
            if (newSegmentRequest == RequestNewSegment) {
                this->requestNewSegment();
            }
        }

    }
}


bool NntpClient::downloadSegmentWithBackupServer() {

    bool nextServerFound = false;

    // get server group of the next backup server :
    ServerGroup* nextServerGroup = this->parent->getServerGroup()->getNextTargetServer();

    // if another group of servers have been found :
    if (nextServerGroup) {

        this->currentSegmentData.setReadyForNewServer(nextServerGroup->getServerGroupId());

        // update segmentData :
        emit updateDownloadSegmentSignal(this->currentSegmentData);

        // tells clients connected to next server that this segment is pending :
        nextServerGroup->assignDownloadToReadyClients();

        this->segmentProcessed = true;
        nextServerFound = true;

    }

    return nextServerFound;
}


int NntpClient::notifyDownloadHasFinished(const UtilityNamespace::Article articlePresence) {

    int downloadNextSegmentDelaySec = 0;

    this->segmentProcessed = true;
    this->currentSegmentData.setDownloadFinished(articlePresence);

    // segment is present and download is complete :
    if (articlePresence == Present) {

        // set pointer to data to be decoded by segmentDecoderThread :
        QBuffer* buffer = new QBuffer();
        buffer->setData(this->segmentByteArray);

        this->currentSegmentData.setIoDevice(buffer);
        this->currentSegmentData.setDataSize(this->segmentByteArray.size());

        downloadNextSegmentDelaySec = this->parent->getServerGroup()->saveSegment(this->currentSegmentData);
    }
    // else update current segment status :
    else {
        emit updateDownloadSegmentSignal(this->currentSegmentData);
    }

    return downloadNextSegmentDelaySec;
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void NntpClient::readyReadSlot() {

    // check that socket contains some data :
    if (this->tcpSocket->bytesAvailable() > 0) {

        switch (this->clientStatus) {

        case ClientIdle: case ClientSegmentRequest: {

                this->getAnswerFromServer();
                break;
            }

        case ClientDownload: {

                this->tcpSocket->checkRateControlTimer();

                // if no download speed limit :
                if (this->parent->isBandwidthFull()) {
                    this->downloadSegmentFromServerSlot();
                }

                break;
            }
        }
    }
    // set client as Idle and eventually roll back current segment data :
    else {
        this->setConnectedClientStatus(ClientIdle);
    }

}


void NntpClient::dataHasArrivedSlot() {

    // try to connect if disconnected from server :
    if (this->tcpSocket->isSocketUnconnected()) {
        this->connectToHostSlot();
    }

    // if the client is not currently downloading, request another segment
    // only if socket is in connected state :
    if ( (this->clientStatus == ClientIdle) &&
         this->tcpSocket->isSocketConnected() ) {

        this->requestNewSegment();

    }

}


void NntpClient::answerTimeOutSlot() {

    this->setConnectedClientStatus(ClientIdle, DoNotTouchTimers);
    this->dataHasArrivedSlot();

}


void NntpClient::connectedSlot() {

    // client has just connected to server and did not answer yet :
    this->updateServerAnswerStatus(ServerFirstAnswerNotSent);

    emit connectionStatusPerServerSignal(Connected);

    // if reconnection succeeded by timer :
    this->tcpSocket->connected();

    // fetch new item :
    this->dataHasArrivedSlot();
}


void NntpClient::disconnectedSlot() {

    this->updateServerAnswerStatus(ServerDisconnected);

    this->setConnectedClientStatus(ClientIdle, DoNotTouchTimers);

    // if disconnection comes after a socket error, notify type of error in status bar :
    // (if disconnection is normal, nntpError = NoError) :
    emit nntpErrorPerServerSignal(this->nntpError);
    emit connectionStatusPerServerSignal(Disconnected);

    // try to reconnect only if login and password are ok :
    if (!this->authenticationDenied) {
        this->tcpSocket->tryToReconnect();
    }

}


void NntpClient::errorSlot(QAbstractSocket::SocketError socketError) {

    //qCDebug(KWOOTY_LOG) << this->parent->getServerGroup()->getRealServerGroupId() << socketError;

    this->setConnectedClientStatus(ClientIdle, DoNotTouchTimers);

    if (socketError == QAbstractSocket::HostNotFoundError) {
        // connection failed, notify error now :
        emit nntpErrorPerServerSignal(HostNotFound);
    }

    if (socketError == QAbstractSocket::ConnectionRefusedError) {
        // connection failed, notify error now :
        emit nntpErrorPerServerSignal(ConnectionRefused);
    }

    if (socketError == QAbstractSocket::RemoteHostClosedError) {
        // disconnection will occur after this slot, notify error only when disconnect occurs :
        this->nntpError = RemoteHostClosed;
    }

    if (socketError == QAbstractSocket::SslHandshakeFailedError) {
        // disconnection will occur after this slot, notify error only when disconnect occurs :
        this->nntpError = SslHandshakeFailed;
    }

}

