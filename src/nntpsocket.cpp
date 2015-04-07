/***************************************************************************
 *   Copyright (C) 2013 by Xavier Lefage                                   *
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


#include "nntpsocket.h"

#include <KDebug>

#include <QSslCipher>
#include "clientmanagerconn.h"
#include "data/serverdata.h"


NntpSocket::NntpSocket(ClientManagerConn* parent) : QSslSocket(parent) {

    this->parent = parent;

    this->setPeerVerifyMode(QSslSocket::QueryPeer);

    // set a timer to reconnect to host after 20 seconds if disconnection occurs :
    this->tryToReconnectTimer = new QTimer(this);
    this->tryToReconnectTimer->setInterval(20000);

    // set a timer to disconnect from host after idle activity :
    this->idleTimeOutTimer = new QTimer(this);
    this->idleTimeOutTimer->setInterval(parent->getServerData().getDisconnectTimeout() * UtilityNamespace::MINUTES_TO_MILLISECONDS);

    // set a timer to check that stream communication is not stuck,
    // disconnect from host after 20 seconds with no answer from host :
    this->serverAnswerTimer = new QTimer(this);
    this->serverAnswerTimer->setInterval(20000);
    this->serverAnswerTimer->setSingleShot(true);

    this->rateControlTimer = new QTimer(this);
    this->rateControlTimer->setInterval(50);

    this->missingBytes = 0;
    this->certificateVerified = false;

    this->setupConnections();

    // notify status bar that SSL is disabled by default :
    emit encryptionStatusPerServerSignal(false);

}

NntpSocket::~NntpSocket() {

    this->stopAllTimers();
}



void NntpSocket::stopAllTimers() {

    // stop all timers :
    this->idleTimeOutTimer->stop();
    this->tryToReconnectTimer->stop();
    this->serverAnswerTimer->stop();

}


void NntpSocket::abort() {

    this->stopAllTimers();
    QSslSocket::abort();
}


void NntpSocket::quitAndReconnectInMs(const int& reconnectSeconds) {

    this->sendQuitCommandToServer();
    this->abort();
    QTimer::singleShot(reconnectSeconds, this, SLOT(answerTimeOutSlot()));
}


void NntpSocket::setupConnections() {

    // timer connections :
    connect(this->tryToReconnectTimer, SIGNAL(timeout()), this, SLOT(tryToReconnectSlot()));
    connect(this->idleTimeOutTimer, SIGNAL(timeout()), this, SLOT(idleTimeOutSlot()));
    connect(this->serverAnswerTimer, SIGNAL(timeout()), this, SLOT(answerTimeOutSlot()));
    connect(this->rateControlTimer, SIGNAL(timeout()), this, SLOT(rateControlSlot()));

    connect (this, SIGNAL(encrypted()), this, SLOT(socketEncryptedSlot()));
    connect (this, SIGNAL(peerVerifyError(const QSslError&)), this, SLOT(peerVerifyErrorSlot()));

}


bool NntpSocket::isSocketUnconnected() const {
    return (this->state() == QAbstractSocket::UnconnectedState);
}


bool NntpSocket::isSocketConnected() const {
    return (this->state() == QAbstractSocket::ConnectedState);
}


void NntpSocket::dataReadArrived() {

    // answer received before time-out : OK
    if (this->serverAnswerTimer->isActive()) {
        this->serverAnswerTimer->stop();
    }

}


void NntpSocket::dataReadPending() {

    this->serverAnswerTimer->start();

}


void NntpSocket::dataReadComplete() {

    // if segment data has been fully transferred and connection is encrypted :
    if (this->isEncrypted()) {

        // reset buffer settings :
        this->manageBuffer(NntpSocket::SegmentDownloadFinished);
    }

    // stop serverAnswerTimer :
    this->dataReadArrived();

}


int NntpSocket::readAnswer() {

    int answer = this->readLine().left(3).toInt();

    // stop reconnect timer if authentication has been rejected :
    if ( answer == NntpClient::AuthenticationRequired   ||
         answer == NntpClient::PasswordRequested        ||
         answer == NntpClient::AuthenticationDenied     ||
         answer == NntpClient::AuthenticationRejected ) {

        this->serverAnswerTimer->stop();
    }

    return answer;
}


void NntpSocket::connected() {

    this->tryToReconnectTimer->stop();
    this->idleTimeOutTimer->start();

}


void NntpSocket::tryToReconnect() {

    if (this->idleTimeOutTimer->isActive()) {
        this->idleTimeOutTimer->stop();
    }
    else if (!this->tryToReconnectTimer->isActive()) {
        this->tryToReconnectTimer->start();
    }

}


void NntpSocket::retryDownloadDelayed() {

    this->serverAnswerTimer->stop();
    this->idleTimeOutTimer->stop();
}



void NntpSocket::connectToHost() {

    if (this->isSocketUnconnected()) {

        this->idleTimeOutTimer->stop();
        this->idleTimeOutTimer->setInterval(this->parent->getServerData().getDisconnectTimeout() * UtilityNamespace::MINUTES_TO_MILLISECONDS);

        // try to reconnect if connection fails :
        this->tryToReconnectTimer->start();

        ServerData serverData = this->parent->getServerData();

        if (serverData.isEnableSSL()) {

            // by default, consider that certificate has been verified.
            // It could be set to false if peerVerifyErrorSlot() is raised :
            this->certificateVerified = true;
            QSslSocket::connectToHostEncrypted(serverData.getHostName(), serverData.getPort(), ReadWrite);

        }
        else {

            QSslSocket::connectToHost(serverData.getHostName(), serverData.getPort(), ReadWrite);
            // SSL is disabled :
            emit encryptionStatusPerServerSignal(false);

        }

    }
}



void NntpSocket::notifyClientStatus(NntpClient::NntpClientStatus nntpClientStatus, NntpClient::TimerJob timerJob) {

    // start/stop timers according to where this method has been called :
    if (timerJob == NntpClient::StartStopTimers) {

        // start disconnect timeout if idle :
        if (nntpClientStatus == NntpClient::ClientIdle) {

            if (!idleTimeOutTimer->isActive()) {
                idleTimeOutTimer->start();

            }
        }

        else {

            // client is connected and working, stop timers :
            if (idleTimeOutTimer->isActive()) {
                idleTimeOutTimer->stop();
            }

            if (tryToReconnectTimer->isActive()) {
                tryToReconnectTimer->stop();
            }
        }

    }

}


QByteArray NntpSocket::readAll() {

    this->dataReadArrived();

    return QSslSocket::readAll();

}


QByteArray NntpSocket::readChunck(const qint64& speedLimitInBytes, const int& enabledClientNumber) {

    this->dataReadArrived();

    this->manageBuffer(NntpSocket::SegmentDownloading);

    // compute maximum byte to fetch :
    qint64 maxReadbytes = ( speedLimitInBytes * this->rateControlTimer->interval() ) / ( enabledClientNumber * 1000 ) + this->missingBytes;
    QByteArray chunckData = this->read(maxReadbytes);

    this->missingBytes = qMax(maxReadbytes - chunckData.size(), static_cast<qint64>(0));

    return chunckData;
}


void NntpSocket::checkRateControlTimer() {

    // bandwidth control has been disabled :
    if ( this->parent->isBandwidthFull() &&
         this->rateControlTimer->isActive() ) {

        this->setReadBufferSize(0);
        this->rateControlTimer->stop();

    }
    // bandwidth control has been enabled :
    else if ( this->parent->isBandwidthLimited() &&
              !this->rateControlTimer->isActive() ) {

        this->missingBytes = 0;
        this->rateControlTimer->start();
    }

}


void NntpSocket::manageBuffer(const SegmentDownload& segmentDownload) {

    int readBufferSize = 0;

    if (this->isEncrypted()) {
        readBufferSize = 1024;
    }

    if (segmentDownload == SegmentDownloadFinished) {
        readBufferSize = 0;
        this->missingBytes = 0;
    }

    if (this->readBufferSize() != readBufferSize) {
        this->setReadBufferSize(readBufferSize);
    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void NntpSocket::tryToReconnectSlot() {

    // try to connect, be sure to be unconnected before :
    if (this->isSocketUnconnected()) {
        emit tryToReconnectSignal();
    }

}


void NntpSocket::idleTimeOutSlot() {

    this->sendQuitCommandToServer();
    this->disconnectFromHost();

}


void NntpSocket::answerTimeOutSlot() {

    //kDebug() << "Host answer time out, reconnecting..., groupId : " << this->parent->getServerGroup()->getRealServerGroupId();
    this->serverAnswerTimer->stop();

    // anticipate socket error notification -> reconnect immediately :
    this->sendQuitCommandToServer();
    this->abort();

    emit answerTimeOutSignal();

}


void NntpSocket::rateControlSlot() {

    if (this->bytesAvailable() > 0) {
        emit downloadSegmentFromServerSignal();
    }
}


void NntpSocket::peerVerifyErrorSlot() {

    // error occured during certificate verifying, set verify mode to QueryPeer in order to establish connection
    // but inform the user that certificate is not verified by tooltip in status bar :
    this->setPeerVerifyMode(QSslSocket::QueryPeer);

    this->certificateVerified = false;

}


void NntpSocket::socketEncryptedSlot() {

    QStringList issuerOrgranisation = QStringList() <<  "Unknown";

    // retrieve peer certificate :
    QSslCertificate sslCertificate = this->peerCertificate();

    // get issuer organization in order to display it as tooltip in status bar :
    if (!sslCertificate.isNull()) {
        issuerOrgranisation = sslCertificate.issuerInfo(QSslCertificate::Organization);
    }

    // retrieve errors occured during ssl handshake :
    QStringList sslErrors;

    foreach (const QSslError& currentSslError, this->sslErrors()) {
        sslErrors.append(currentSslError.errorString());
    }

    // SSL connection is active, send also encryption method used by host :
    //KF5 fix issuerOrgranisatio was QString in kde4, is a QStringList in kf5/qt5
    emit encryptionStatusPerServerSignal(true, this->sessionCipher().encryptionMethod(), this->certificateVerified, issuerOrgranisation.first(), sslErrors);

}



//============================================================================================================//
//                                            host commands                                                   //
//============================================================================================================//


void NntpSocket::sendBodyCommandToServer(const QString& part) {
    QString commandStr("BODY <" + part + ">\r\n");
    this->sendCommand(commandStr);
    this->serverAnswerTimer->start();
}

void NntpSocket::sendUserCommandToServer(const QString& login) {
    QString commandStr("AUTHINFO USER " + login + "\r\n");
    this->sendCommand(commandStr);
    this->serverAnswerTimer->start();
}

void NntpSocket::sendPasswordCommandToServer(const QString& password) {
    QString commandStr("AUTHINFO PASS " + password + "\r\n");
    this->sendCommand(commandStr);
    this->serverAnswerTimer->start();
}

void NntpSocket::sendQuitCommandToServer() {
    QString commandStr("QUIT\r\n");
    this->sendCommand(commandStr);
}

void NntpSocket::sendCommand(const QString& commandStr) {
    this->write(commandStr.toLatin1(), commandStr.size());
}

