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

#include "kwooty_debug.h"

#include "core.h"
#include "servermanager.h"
#include "servergroup.h"
#include "statsinfobuilder.h"
#include "kwootysettings.h"

ClientsObserver::ClientsObserver(Core *parent) : ClientsObserverBase(parent)
{

    this->parent = parent;

    this->resetVariables();

    this->statsInfoBuilder = new StatsInfoBuilder(this, parent);

}

void ClientsObserver::resetVariables()
{

    ClientsObserverBase::resetVariables();

    this->totalFiles = 0;
    this->totalSize = 0;

}

void ClientsObserver::sendFullUpdate()
{

    // send reset values to status bar :
    emit updateFileSizeInfoSignal(this->totalFiles, this->totalSize);
    emit updateConnectionStatusSignal();

    this->statsInfoBuilder->sendFullUpdate();

}

void ClientsObserver::nntpClientSpeedSlot(const int bytesDownloaded)
{
    this->addBytesDownloaded(bytesDownloaded);
}

void ClientsObserver::decrementSlot(const quint64 size, const int fileNumber = 1)
{

    this->totalFiles -= fileNumber;
    this->totalSize -= size;

    // status bar updates :
    emit updateFileSizeInfoSignal(this->totalFiles, this->totalSize);

}

void ClientsObserver::fullFileSizeUpdate(const quint64 size, const quint64 files)
{

    this->totalSize = size;
    this->totalFiles = files;

    // status bar updates :
    emit updateFileSizeInfoSignal(this->totalFiles, this->totalSize);

}

void ClientsObserver::connectionStatusSlot(const int connectionStatus)
{

    this->updateTotalConnections(connectionStatus);

    emit updateConnectionStatusSignal();

}

void ClientsObserver::nntpErrorSlot(const int nttpErrorStatus)
{

    this->setNntpErrorStatus(nttpErrorStatus);

    emit updateConnectionStatusSignal();

}

void ClientsObserver::encryptionStatusSlot(const bool sslActive, const QString &encryptionMethod, const bool certificateVerified, const QString &issuerOrgranisation, const QStringList &sslErrors)
{

    //qCDebug(KWOOTY_LOG) << "sslActive : " << sslActive << "encryptionMethod" << encryptionMethod;

    this->setSslHandshakeParameters(sslActive, encryptionMethod, certificateVerified, issuerOrgranisation, sslErrors);
    emit updateConnectionStatusSignal();

}

bool ClientsObserver::isSingleServer(QString &hostName) const
{

    bool singleServer = true;

    ServerManager *serverManager = this->parent->getServerManager();

    if (serverManager->getServerNumber() > 1) {
        singleServer = false;
    }

    else {
        // set server host name if this is a sinle server configuration :
        hostName = serverManager->getServerGroupById(MasterServer)->getServerData().getHostName();
    }

    return singleServer;
}

bool ClientsObserver::isSslActive() const
{
    return this->parent->getServerManager()->areAllServersEncrypted();
}

void ClientsObserver::resetTotalBytesDownloaded()
{
    this->totalBytesDownloaded = 0;
}

quint64 ClientsObserver::getTotalSize() const
{
    return this->totalSize;
}

StatsInfoBuilder *ClientsObserver::getStatsInfoBuilder() const
{
    return this->statsInfoBuilder;
}

