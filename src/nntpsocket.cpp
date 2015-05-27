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

#include "kwooty_debug.h"

#include <QSslCipher>
#include "clientmanagerconn.h"
#include "data/serverdata.h"

NntpSocket::NntpSocket(ClientManagerConn *parent) : QSslSocket(parent)
{

    this->mParent = parent;

    this->setPeerVerifyMode(QSslSocket::QueryPeer);

    // set a timer to reconnect to host after 20 seconds if disconnection occurs :
    this->mTryToReconnectTimer = new QTimer(this);
    this->mTryToReconnectTimer->setInterval(20000);

    // set a timer to disconnect from host after idle activity :
    this->mIdleTimeOutTimer = new QTimer(this);
    this->mIdleTimeOutTimer->setInterval(parent->getServerData().getDisconnectTimeout() * UtilityNamespace::MINUTES_TO_MILLISECONDS);

    // set a timer to check that stream communication is not stuck,
    // disconnect from host after 20 seconds with no answer from host :
    this->mServerAnswerTimer = new QTimer(this);
    this->mServerAnswerTimer->setInterval(20000);
    this->mServerAnswerTimer->setSingleShot(true);

    this->mRateControlTimer = new QTimer(this);
    this->mRateControlTimer->setInterval(50);

    this->mMissingBytes = 0;
    this->mCertificateVerified = false;

    this->setupConnections();

    // notify status bar that SSL is disabled by default :
    emit encryptionStatusPerServerSignal(false);

}

NntpSocket::~NntpSocket()
{

    this->stopAllTimers();
}

void NntpSocket::stopAllTimers()
{

    // stop all timers :
    this->mIdleTimeOutTimer->stop();
    this->mTryToReconnectTimer->stop();
    this->mServerAnswerTimer->stop();

}

void NntpSocket::abort()
{

    this->stopAllTimers();
    QSslSocket::abort();
}

void NntpSocket::quitAndReconnectInMs(const int &reconnectSeconds)
{

    this->sendQuitCommandToServer();
    this->abort();
    QTimer::singleShot(reconnectSeconds, this, SLOT(answerTimeOutSlot()));
}

void NntpSocket::setupConnections()
{

    // timer connections :
    connect(this->mTryToReconnectTimer, SIGNAL(timeout()), this, SLOT(tryToReconnectSlot()));
    connect(this->mIdleTimeOutTimer, SIGNAL(timeout()), this, SLOT(idleTimeOutSlot()));
    connect(this->mServerAnswerTimer, SIGNAL(timeout()), this, SLOT(answerTimeOutSlot()));
    connect(this->mRateControlTimer, SIGNAL(timeout()), this, SLOT(rateControlSlot()));

    connect(this, SIGNAL(encrypted()), this, SLOT(socketEncryptedSlot()));
    connect(this, SIGNAL(peerVerifyError(QSslError)), this, SLOT(peerVerifyErrorSlot()));

}

bool NntpSocket::isSocketUnconnected() const
{
    return (this->state() == QAbstractSocket::UnconnectedState);
}

bool NntpSocket::isSocketConnected() const
{
    return (this->state() == QAbstractSocket::ConnectedState);
}

void NntpSocket::dataReadArrived()
{

    // answer received before time-out : OK
    if (this->mServerAnswerTimer->isActive()) {
        this->mServerAnswerTimer->stop();
    }

}

void NntpSocket::dataReadPending()
{

    this->mServerAnswerTimer->start();

}

void NntpSocket::dataReadComplete()
{

    // if segment data has been fully transferred and connection is encrypted :
    if (this->isEncrypted()) {

        // reset buffer settings :
        this->manageBuffer(NntpSocket::SegmentDownloadFinished);
    }

    // stop serverAnswerTimer :
    this->dataReadArrived();

}

int NntpSocket::readAnswer()
{

    int answer = this->readLine().left(3).toInt();

    // stop reconnect timer if authentication has been rejected :
    if (answer == NntpClient::AuthenticationRequired   ||
            answer == NntpClient::PasswordRequested        ||
            answer == NntpClient::AuthenticationDenied     ||
            answer == NntpClient::AuthenticationRejected) {

        this->mServerAnswerTimer->stop();
    }

    return answer;
}

void NntpSocket::connected()
{

    this->mTryToReconnectTimer->stop();
    this->mIdleTimeOutTimer->start();

}

void NntpSocket::tryToReconnect()
{

    if (this->mIdleTimeOutTimer->isActive()) {
        this->mIdleTimeOutTimer->stop();
    } else if (!this->mTryToReconnectTimer->isActive()) {
        this->mTryToReconnectTimer->start();
    }

}

void NntpSocket::retryDownloadDelayed()
{

    this->mServerAnswerTimer->stop();
    this->mIdleTimeOutTimer->stop();
}

void NntpSocket::connectToHost()
{

    if (this->isSocketUnconnected()) {

        this->mIdleTimeOutTimer->stop();
        this->mIdleTimeOutTimer->setInterval(this->mParent->getServerData().getDisconnectTimeout() * UtilityNamespace::MINUTES_TO_MILLISECONDS);

        // try to reconnect if connection fails :
        this->mTryToReconnectTimer->start();

        ServerData serverData = this->mParent->getServerData();

        if (serverData.isEnableSSL()) {

            // by default, consider that certificate has been verified.
            // It could be set to false if peerVerifyErrorSlot() is raised :
            this->mCertificateVerified = true;
            QSslSocket::connectToHostEncrypted(serverData.getHostName(), serverData.getPort(), ReadWrite);

        } else {

            QSslSocket::connectToHost(serverData.getHostName(), serverData.getPort(), ReadWrite);
            // SSL is disabled :
            emit encryptionStatusPerServerSignal(false);

        }

    }
}

void NntpSocket::notifyClientStatus(NntpClient::NntpClientStatus nntpClientStatus, NntpClient::TimerJob timerJob)
{

    // start/stop timers according to where this method has been called :
    if (timerJob == NntpClient::StartStopTimers) {

        // start disconnect timeout if idle :
        if (nntpClientStatus == NntpClient::ClientIdle) {

            if (!mIdleTimeOutTimer->isActive()) {
                mIdleTimeOutTimer->start();

            }
        }

        else {

            // client is connected and working, stop timers :
            if (mIdleTimeOutTimer->isActive()) {
                mIdleTimeOutTimer->stop();
            }

            if (mTryToReconnectTimer->isActive()) {
                mTryToReconnectTimer->stop();
            }
        }

    }

}

QByteArray NntpSocket::readAll()
{

    this->dataReadArrived();

    return QSslSocket::readAll();

}

QByteArray NntpSocket::readChunck(const qint64 &speedLimitInBytes, const int &enabledClientNumber)
{

    this->dataReadArrived();

    this->manageBuffer(NntpSocket::SegmentDownloading);

    // compute maximum byte to fetch :
    qint64 maxReadbytes = (speedLimitInBytes * this->mRateControlTimer->interval()) / (enabledClientNumber * 1000) + this->mMissingBytes;
    QByteArray chunckData = this->read(maxReadbytes);

    this->mMissingBytes = qMax(maxReadbytes - chunckData.size(), static_cast<qint64>(0));

    return chunckData;
}

void NntpSocket::checkRateControlTimer()
{

    // bandwidth control has been disabled :
    if (this->mParent->isBandwidthFull() &&
            this->mRateControlTimer->isActive()) {

        this->setReadBufferSize(0);
        this->mRateControlTimer->stop();

    }
    // bandwidth control has been enabled :
    else if (this->mParent->isBandwidthLimited() &&
             !this->mRateControlTimer->isActive()) {

        this->mMissingBytes = 0;
        this->mRateControlTimer->start();
    }

}

void NntpSocket::manageBuffer(const SegmentDownload &segmentDownload)
{

    int readBufferSize = 0;

    if (this->isEncrypted()) {
        readBufferSize = 1024;
    }

    if (segmentDownload == SegmentDownloadFinished) {
        readBufferSize = 0;
        this->mMissingBytes = 0;
    }

    if (this->readBufferSize() != readBufferSize) {
        this->setReadBufferSize(readBufferSize);
    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void NntpSocket::tryToReconnectSlot()
{

    // try to connect, be sure to be unconnected before :
    if (this->isSocketUnconnected()) {
        emit tryToReconnectSignal();
    }

}

void NntpSocket::idleTimeOutSlot()
{

    this->sendQuitCommandToServer();
    this->disconnectFromHost();

}

void NntpSocket::answerTimeOutSlot()
{

    //qCDebug(KWOOTY_LOG) << "Host answer time out, reconnecting..., groupId : " << this->parent->getServerGroup()->getRealServerGroupId();
    this->mServerAnswerTimer->stop();

    // anticipate socket error notification -> reconnect immediately :
    this->sendQuitCommandToServer();
    this->abort();

    emit answerTimeOutSignal();

}

void NntpSocket::rateControlSlot()
{

    if (this->bytesAvailable() > 0) {
        emit downloadSegmentFromServerSignal();
    }
}

void NntpSocket::peerVerifyErrorSlot()
{

    // error occurred during certificate verifying, set verify mode to QueryPeer in order to establish connection
    // but inform the user that certificate is not verified by tooltip in status bar :
    this->setPeerVerifyMode(QSslSocket::QueryPeer);

    this->mCertificateVerified = false;

}

void NntpSocket::socketEncryptedSlot()
{

    QStringList issuerOrgranisation = QStringList() <<  "Unknown";

    // retrieve peer certificate :
    QSslCertificate sslCertificate = this->peerCertificate();

    // get issuer organization in order to display it as tooltip in status bar :
    if (!sslCertificate.isNull()) {
        issuerOrgranisation = sslCertificate.issuerInfo(QSslCertificate::Organization);
    }

    // retrieve errors occurred during ssl handshake :
    QStringList sslErrors;

    foreach (const QSslError &currentSslError, this->sslErrors()) {
        sslErrors.append(currentSslError.errorString());
    }

    // SSL connection is active, send also encryption method used by host :
    //KF5 fix issuerOrgranisatio was QString in kde4, is a QStringList in kf5/qt5
    emit encryptionStatusPerServerSignal(true, this->sessionCipher().encryptionMethod(), this->mCertificateVerified, issuerOrgranisation.first(), sslErrors);

}

//============================================================================================================//
//                                            host commands                                                   //
//============================================================================================================//

void NntpSocket::sendBodyCommandToServer(const QString &part)
{
    QString commandStr("BODY <" + part + ">\r\n");
    this->sendCommand(commandStr);
    this->mServerAnswerTimer->start();
}

void NntpSocket::sendUserCommandToServer(const QString &login)
{
    QString commandStr("AUTHINFO USER " + login + "\r\n");
    this->sendCommand(commandStr);
    this->mServerAnswerTimer->start();
}

void NntpSocket::sendPasswordCommandToServer(const QString &password)
{
    QString commandStr("AUTHINFO PASS " + password + "\r\n");
    this->sendCommand(commandStr);
    this->mServerAnswerTimer->start();
}

void NntpSocket::sendQuitCommandToServer()
{
    QString commandStr("QUIT\r\n");
    this->sendCommand(commandStr);
}

void NntpSocket::sendCommand(const QString &commandStr)
{
    this->write(commandStr.toLatin1(), commandStr.size());
}

