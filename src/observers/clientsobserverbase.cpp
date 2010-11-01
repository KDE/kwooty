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


#include "clientsobserverbase.h"

ClientsObserverBase::ClientsObserverBase(QObject *parent) : QObject(parent) {

}


void ClientsObserverBase::resetVariables() {

    this->totalBytesDownloaded = 0;
    this->totalConnections = 0;
    this->sslActive = false;
    this->sslErrors = QStringList();
    this->nttpErrorStatus = NoError;

}


void ClientsObserverBase::addBytesDownloaded(const int& bytesDownloaded) {
    this->totalBytesDownloaded += bytesDownloaded;
}


void ClientsObserverBase::updateTotalConnections(const int& connectionStatus) {

    if (connectionStatus == Connected){
        this->totalConnections++;
    }

    if (connectionStatus == Disconnected){
        this->totalConnections--;
    }
}

void ClientsObserverBase::setNntpErrorStatus(const int& nttpErrorStatus) {
    this->nttpErrorStatus = nttpErrorStatus;
}


void ClientsObserverBase::setSslHandshakeParameters(const bool& sslActive, const QString& encryptionMethod, const bool& certificateVerified, const QString& issuerOrgranisation, const QStringList& sslErrors) {
    this->encryptionMethod = encryptionMethod;
    this->sslActive = sslActive;
    this->certificateVerified = certificateVerified;
    this->issuerOrgranisation = issuerOrgranisation;
    this->sslErrors = sslErrors;
}


bool ClientsObserverBase::isConnected() const {

    bool connected = false;

    if (this->totalConnections > 0) {
        connected = true;
    }

    return connected;
}


QString ClientsObserverBase::getEncryptionMethod() const {
    return this->encryptionMethod;
}

QString ClientsObserverBase::getIssuerOrgranisation() const {
    return this->issuerOrgranisation;
}

QStringList ClientsObserverBase::getSslErrors() const {
    return this->sslErrors;
}

int ClientsObserverBase::getTotalConnections() const {
    return this->totalConnections;
}


bool ClientsObserverBase::isCertificateVerified() const {
    return this->certificateVerified;
}

int ClientsObserverBase::getNttpErrorStatus() const {
    return this->nttpErrorStatus;
}

quint64 ClientsObserverBase::getTotalBytesDownloaded() const {
    return this->totalBytesDownloaded;
}

