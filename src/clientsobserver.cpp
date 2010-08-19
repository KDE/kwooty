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


#include "clientsobserver.h"

#include <KDebug>

#include "centralwidget.h"
#include "statsinfobuilder.h"   
#include "kwootysettings.h"



ClientsObserver::ClientsObserver(CentralWidget* parent) : QObject(parent) {

    this->parent = parent;

    this->resetVariables();

    this->statsInfoBuilder = new StatsInfoBuilder(this, parent);

}



void ClientsObserver::resetVariables() {

    this->totalFiles = 0;
    this->totalSize = 0;
    this->totalBytesDownloaded = 0;

    this->totalConnections = 0;
    this->sslActive = false;
    this->sslErrors = QStringList();
    this->nttpErrorStatus = NoError;

}



void ClientsObserver::sendFullUpdate() {

    // send reset values to status bar :
    emit updateFileSizeInfoSignal(this->totalFiles, this->totalSize);
    emit updateConnectionStatusSignal();

    this->statsInfoBuilder->sendFullUpdate();


}


void ClientsObserver::nntpClientSpeedSlot(const int bytesDownloaded) {

    this->totalBytesDownloaded += bytesDownloaded;

}


void ClientsObserver::decrementSlot(const quint64 size, const int fileNumber = 1) {

    this->totalFiles -= fileNumber;
    this->totalSize -= size;

    // status bar updates :
    emit updateFileSizeInfoSignal(this->totalFiles, this->totalSize);

}


void ClientsObserver::fullFileSizeUpdate(const quint64 size, const quint64 files) {

    this->totalSize = size;
    this->totalFiles = files;

    // status bar updates :
    emit updateFileSizeInfoSignal(this->totalFiles, this->totalSize);

}



void ClientsObserver::connectionStatusSlot(const int connectionStatus) {

    if (connectionStatus == Connected){
        totalConnections++;
    }

    if (connectionStatus == Disconnected){
        totalConnections--;
    }

    emit updateConnectionStatusSignal();

}


void ClientsObserver::nntpErrorSlot(const int nttpErrorStatus) {

    this->nttpErrorStatus = nttpErrorStatus;
    emit updateConnectionStatusSignal();

}

void ClientsObserver::encryptionStatusSlot(const bool sslActive, const QString encryptionMethod, const bool certificateVerified, const QString issuerOrgranisation, const QStringList sslErrors) {

    //kDebug() << "sslActive : " << sslActive << "encryptionMethod" << encryptionMethod;

    this->encryptionMethod = encryptionMethod;
    this->sslActive = sslActive;
    this->certificateVerified = certificateVerified;
    this->issuerOrgranisation = issuerOrgranisation;
    this->sslErrors = sslErrors;

    emit updateConnectionStatusSignal();

}


QString ClientsObserver::getEncryptionMethod() const {
    return this->encryptionMethod;
}

QString ClientsObserver::getIssuerOrgranisation() const {
    return this->issuerOrgranisation;
}

QStringList ClientsObserver::getSslErrors() const {
  return this->sslErrors;
}

int ClientsObserver::getTotalConnections() const {
    return this->totalConnections;
}

bool ClientsObserver::isSslActive() const {
    return this->sslActive;
}

bool ClientsObserver::isCertificateVerified() const {
    return this->certificateVerified;
}

int ClientsObserver::getNttpErrorStatus() const {
    return this->nttpErrorStatus;
}

quint64 ClientsObserver::getTotalBytesDownloaded() const {
    return this->totalBytesDownloaded;
}

void ClientsObserver::resetTotalBytesDownloaded() {
    this->totalBytesDownloaded = 0;
}

quint64 ClientsObserver::getTotalSize() const {
    return this->totalSize;
}

StatsInfoBuilder* ClientsObserver::getStatsInfoBuilder() const {
    return this->statsInfoBuilder;
}


