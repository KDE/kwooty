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

#include <KDebug>
#include <QTimer>
#include <QSslCipher>
#include <QFile>

#include "settings.h"
#include "clientmanagerconn.h"



NntpClient::NntpClient(ClientManagerConn* parent) : QObject (parent)
{
    this->parent = parent;

    // instantiate socket :
    tcpSocket = new QSslSocket(parent);

    // set a timer to reconnect to host after 10 seconds if disconnection occurs :
    tryToReconnectTimer = new QTimer(this);
    tryToReconnectTimer->setInterval(10000);

    // set a timer to disconnect from host after idle activity :
    idleTimeOutTimer = new QTimer(this);
    idleTimeOutTimer->setInterval(Settings::disconnectTimeout() * MINUTES_TO_MILLISECONDS);

    // set a timer to check that stream communication is not stuck,
    // disconnect from host after 20 seconds with no answer from host :
    serverAnswerTimer = new QTimer(this);
    serverAnswerTimer->setInterval(20000);
    serverAnswerTimer->setSingleShot(true);


    this->authenticationDenied = false;
    this->nntpError = NoError;

    // set up connections with tcpSocket :
    this->setupConnections();

    this->connectToHost();

    // set client status to IdleStatus by default :
    this->setConnectedClientStatus(ClientIdle);

    // notify status bar that SSL is disabled by default :
    emit encryptionStatusSignal(false);

}


NntpClient::NntpClient()
{
}


NntpClient::~NntpClient()
{
    // stop all timers :
    idleTimeOutTimer->stop();
    tryToReconnectTimer->stop();
    serverAnswerTimer->stop();

    // quit :
    this->sendQuitCommandToServer();
    this->segmentDataRollBack();
    tcpSocket->abort();

}


void NntpClient::setupConnections() {

    connect (tcpSocket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    connect (tcpSocket, SIGNAL(connected()), this, SLOT(connectedSlot()));
    connect (tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
    connect (tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSlot(QAbstractSocket::SocketError)));
    connect (tcpSocket, SIGNAL(encrypted()), this, SLOT(socketEncryptedSlot()));

    // timer connections :
    connect(tryToReconnectTimer, SIGNAL(timeout()), this, SLOT(tryToReconnectSlot()));
    connect(idleTimeOutTimer, SIGNAL(timeout()), this, SLOT(idleTimeOutSlot()));
    connect(serverAnswerTimer, SIGNAL(timeout()), this, SLOT(answerTimeOutSlot()));

}


void NntpClient::connectToHost() {

    // set nntpError do noError by default before connection process :
    this->nntpError = NoError;

    //kDebug() << "client ID : " << parent->getClientId() << "disconnectTimeout : " << Settings::disconnectTimeout();
    idleTimeOutTimer->stop();
    idleTimeOutTimer->setInterval(Settings::disconnectTimeout() * MINUTES_TO_MILLISECONDS);

    QString hostName = Settings::hostName();
    int port = Settings::port();

    if (Settings::enableSSL()) {
        tcpSocket->connectToHostEncrypted(hostName, port);
    }
    else {
        tcpSocket->connectToHost(hostName, port);
        // SSL is disabled :
        emit encryptionStatusSignal(false);
    }



}


void NntpClient::getAnswerFromServer() {

    // get answer from server :
    int answer = tcpSocket->readLine().left(3).toInt();

    switch (answer) {

    case ServerIsReadyPosting: case ServerIsReadyNoPosting: {
            this->setConnectedClientStatus(ClientIdle);
            break;
        }

    case AuthenticationRequired: {
            // authentication name requested :
            serverAnswerTimer->stop();
            if (Settings::groupBoxAuthentication()) {
                this->sendUserCommandToServer();
            }
            else{
                // group box is uncheked but authentication needed, inform the user :
                this->sendQuitCommandToServer();
                nntpError = AuthenticationNeeded;
            }
            break;
        }

    case PasswordRequested: {
            // authentication password requested :
            serverAnswerTimer->stop();
            if (Settings::groupBoxAuthentication()) {
                this->sendPasswordCommandToServer();
            }
            else{
                // group box is uncheked but authentication needed, inform the user :
                this->sendQuitCommandToServer();
                nntpError = AuthenticationNeeded;
            }
            break;
        }

    case AuthenticationAccepted: {
            //kDebug() << "answer from server : " << answer;
            this->authenticationDenied = false;
            this->sendBodyCommandToServer();
            break;
        }


    case AuthenticationDenied: case AuthenticationRejected: {
            //kDebug() << "answer from server : " << answer;

            // stop reconnect timer if authentication has been rejected :
            serverAnswerTimer->stop();
            this->authenticationDenied = true;

            // set type of error in order to notify status bar :
            nntpError = AuthenticationFailed;

            // disconnect from host :
            this->sendQuitCommandToServer();
            tcpSocket->abort();

            // In case authentication denied, try to reconnect to host in 30 seconds :
            QTimer::singleShot(30000, this, SLOT(answerTimeOutSlot()) );
            kDebug() << "AuthenticationDenied,  try to reconnect in 30 seconds";
            break;
        }

    case BodyArticleFollows: {
            // init before download :
            this->setConnectedClientStatus(ClientDownload);
            this->segmentByteArray.clear();

            // set file identifier in order to delete incomplete downloads at next launch :
            this->segmentByteArray.append(applicationFileOwner);

            // notify segment is being downloaded :
            this->currentSegmentData.setProgress(PROGRESS_INIT);
            this->currentSegmentData.setStatus(DownloadStatus);
            emit updateDownloadSegmentSignal(currentSegmentData);

            // manage download segment :
            this->downloadSegmentFromServer();
            break;
        }


    case IdleTimeout: {
            this->setConnectedClientStatus(ClientIdle);
            break;
        }

    case NoSuchArticle: {
            //kDebug() << "NO SUCH ARTICLE" << "client ID : " << parent->getClientId();
            this->postDownloadProcess(NotPresent);
            break;
        }

    default: {
            kDebug() << "Answer from host : " << answer << " not handled !";
            break;
        }


    }

}




void NntpClient::downloadSegmentFromServer(){

    // answer received before time-out : OK
    serverAnswerTimer->stop();

    //kDebug() << "bytes available : " << tcpSocket->bytesAvailable();

    // read available data:
    QByteArray chunckData = tcpSocket->readAll();
    this->segmentByteArray.append(chunckData);

    // send size of downloaded data to the status bar :
    emit speedSignal(chunckData.size());

    // if end of download has been reached :
    if (chunckData.endsWith("\r\n.\r\n")) {

        this->postDownloadProcess(Present);

    }
    // data are pending, next readyRead signal is expected before time-out :
    else {
        serverAnswerTimer->start();
    }

}



void NntpClient::postDownloadProcess(const UtilityNamespace::Article articlePresence){

    //kDebug();

    if (serverAnswerTimer->isActive()) {
        serverAnswerTimer->stop();
    }

    // consider that data will be corrected saved by default :
    bool isSaved = true;

    // save data only if article has been found on server :
    if (articlePresence == Present) {

        // replace new lines starting with double periods by a simple one (RFC 977)
        this->segmentByteArray.replace("\r\n..", "\r\n.");

        // save segment :
        isSaved = Utility::saveData(currentSegmentData.getFileSavePath(), currentSegmentData.getPart(), this->segmentByteArray);

        // if file can not be saved, inform the user and stop downloads :
        if (!isSaved) {

            // segment has been downloaded but not saved, proceed to roll back :
            this->segmentDataRollBack();
            // send save error notification :
            emit saveFileErrorSignal(DuringDownload);

            // set client in Idle status for the moment :
            this->setConnectedClientStatus(ClientIdle);

        }
    }

    // if article has been saved, set status to DownloadFinishStatus and update segment
    if (isSaved) {

        this->currentSegmentData.setProgress(PROGRESS_COMPLETE);
        this->currentSegmentData.setStatus(DownloadFinishStatus);
        this->currentSegmentData.setArticlePresenceOnServer(articlePresence);

        // update segmentData :
        emit updateDownloadSegmentSignal(currentSegmentData);

        // set client ready for next receiving segment :
        this->setConnectedClientStatus(ClientSegmentRequest);

        //kDebug() << " getNextSegmentSignal : client ID :" << parent->getClientId();
        emit getNextSegmentSignal(parent);

    }


}





void NntpClient::downloadNextSegment(const SegmentData currentSegmentData){

    this->currentSegmentData = currentSegmentData;

    // should not occur but due to asynchronous call, check that the
    // socket is in connected state before further processing :
    if ((tcpSocket->state() == QAbstractSocket::ConnectedState) ) {
        // get body message from server :
        this->sendBodyCommandToServer();
    }
    else {
        this->segmentDataRollBack();
    }

}





void NntpClient::setConnectedClientStatus(const NntpClient::NntpClientStatus status) {

    this->clientStatus = status;

    // start disconnect timeout if idle :
    if (this->clientStatus == ClientIdle) {
        idleTimeOutTimer->start();
    }
    else {
        // client is connected and working , stop timers :
        if (idleTimeOutTimer->isActive()) {
            idleTimeOutTimer->stop();
        }
        if (tryToReconnectTimer->isActive()) {
            tryToReconnectTimer->stop();
        }
    }
}




void NntpClient::segmentDataRollBack(){

    if (currentSegmentData.getStatus() == DownloadStatus) {

        //kDebug() << "segmentData roll back effective !";
        currentSegmentData.setStatus(IdleStatus);
        currentSegmentData.setProgress(PROGRESS_INIT);
        // update segment data status :
        emit updateDownloadSegmentSignal(currentSegmentData);

    }

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void NntpClient::readyReadSlot(){

    switch (this->clientStatus) {

    case ClientIdle: case ClientSegmentRequest:  {

            this->getAnswerFromServer();
            break;
        }

    case ClientDownload: {
            this->downloadSegmentFromServer();
            break;
        }
    }

}



void NntpClient::dataHasArrivedSlot() {

    //kDebug() << "client state : "  << this->clientStatus << " client ID :" << parent->getClientId() << "connectedState :" << tcpSocket->state();

    // try to connect if disconnected from server :
    if (tcpSocket->state() == QAbstractSocket::UnconnectedState) {
        this->connectToHost();
    }

    // if the client is not currently being downloading, request another segment
    // only if socket is in connected state :
    if ( (this->clientStatus == ClientIdle) &&
         (tcpSocket->state() == QAbstractSocket::ConnectedState) ) {

        emit getNextSegmentSignal(parent);
        this->setConnectedClientStatus(ClientSegmentRequest);
    }

}




void NntpClient::connectedSlot(){

    //kDebug() << "client ID : " << parent->getClientId();

    emit connectionStatusSignal(Connected);

    // if reconnection succeeded by timer :
    if (tryToReconnectTimer->isActive()) {
        tryToReconnectTimer->stop();
    }

    // fetch new item :
    dataHasArrivedSlot();
}


void NntpClient::disconnectedSlot(){

    //kDebug() << "client ID : " << parent->getClientId();

    this->clientStatus = ClientIdle;

    this->segmentDataRollBack();

    // if disconnection comes after an socket error, notify type of error in status bar :
    // (if disconnection is normal, nntpError = NoError) :
    emit nntpErrorSignal(nntpError);

    emit connectionStatusSignal(Disconnected);


    // try to reconnect only if login and password are ok :
    if (!this->authenticationDenied) {

        if (idleTimeOutTimer->isActive()) {
            idleTimeOutTimer->stop();
        }
        else if (!tryToReconnectTimer->isActive()) {
            tryToReconnectTimer->start();
        }
    }

}


void NntpClient::errorSlot(QAbstractSocket::SocketError socketError){

    this->clientStatus = ClientIdle;
    this->segmentDataRollBack();

    //kDebug() << socketError << "client ID : " << parent->getClientId();

    if (socketError == QAbstractSocket::HostNotFoundError){
        // connection failed, notify error now :
        emit nntpErrorSignal(HostNotFound);
    }

    if (socketError == QAbstractSocket::ConnectionRefusedError){
        // connection failed, notify error now :
        emit nntpErrorSignal(ConnectionRefused);
    }

    if (socketError == QAbstractSocket::RemoteHostClosedError){
        // disconnection will occur after this slot, notify error only when disconnect occurs :
        nntpError = RemoteHostClosed;
    }

    if (socketError == QAbstractSocket::SslHandshakeFailedError){
        // disconnection will occur after this slot, notify error only when disconnect occurs :
        nntpError = SslHandshakeFailed;
    }


}



void NntpClient::idleTimeOutSlot(){

    this->sendQuitCommandToServer();
    tcpSocket->disconnectFromHost();

}



void NntpClient::answerTimeOutSlot(){

    //kDebug() << "Host answer time out, reconnecting...";
    serverAnswerTimer->stop();

    // anticipate socket error notification -> reconnect immediately :
    this->sendQuitCommandToServer();
    tcpSocket->abort();

    //tcpSocket->abort();
    this->segmentDataRollBack();
    this->dataHasArrivedSlot();
}




void NntpClient::tryToReconnectSlot(){

    // try to connect, be sure to be unconnected before :
    if (tcpSocket->state() == QAbstractSocket::UnconnectedState) {
        this->connectToHost();
    }

}


void NntpClient::socketEncryptedSlot(){
    // SSL connection is active, send also encryption method used by host :
    emit encryptionStatusSignal(true, tcpSocket->sessionCipher().encryptionMethod());
}






//============================================================================================================//
//                                            host commands                                                   //
//============================================================================================================//


void NntpClient::sendBodyCommandToServer(){
    QString commandStr("BODY <" + currentSegmentData.getPart() + ">\r\n");
    tcpSocket->write( commandStr.toLatin1(), commandStr.size());

    serverAnswerTimer->start();
}

void NntpClient::sendQuitCommandToServer(){
    QString commandStr("QUIT\r\n");
    tcpSocket->write( commandStr.toLatin1(), commandStr.size());
}

void NntpClient::sendUserCommandToServer(){
    QString commandStr("AUTHINFO USER " + Settings::login() + "\r\n");
    tcpSocket->write( commandStr.toLatin1(), commandStr.size());
}

void NntpClient::sendPasswordCommandToServer(){
    QString commandStr("AUTHINFO PASS " + Settings::password() + "\r\n");
    tcpSocket->write( commandStr.toLatin1(), commandStr.size());
}

