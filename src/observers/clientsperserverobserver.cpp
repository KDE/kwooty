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


#include "clientsperserverobserver.h"

#include <KDebug>

#include "servergroup.h"
#include "sidebar.h"
#include "core.h"
#include "clientsobserver.h"
#include "statsinfobuilder.h"
#include "data/segmentinfodata.h"
#include "kwootysettings.h"



ClientsPerServerObserver::ClientsPerServerObserver(ServerGroup* parent) : ClientsObserverBase(parent) {

    this->parent = parent;

    this->resetVariables();
    this->setupConnections();

}


void ClientsPerServerObserver::setupConnections() {

    ClientsObserver* clientsObserver = this->parent->getCore()->getClientsObserver();
    // send connection status (connected, deconnected) to client observer for the current server :
    connect (this,
             SIGNAL(connectionStatusSignal(const int)),
             clientsObserver,
             SLOT(connectionStatusSlot(const int)));


    // send type of encryption used by host with ssl connection to client observer for the current server :
    connect (this,
             SIGNAL(encryptionStatusSignal(const bool, const QString, const bool, const QString, const QStringList)),
             clientsObserver,
             SLOT(encryptionStatusSlot(const bool, const QString, const bool, const QString, const QStringList)));

    // send eventual socket error to client observer for the current server :
    connect (this,
             SIGNAL(nntpErrorSignal(const int)),
             clientsObserver,
             SLOT(nntpErrorSlot(const int)));

    // send bytes downloaded to client observer for the current server :
    connect (this,
             SIGNAL(speedSignal(const int)),
             clientsObserver,
             SLOT(nntpClientSpeedSlot(const int)));

    // notify sidebar that some nntpClient info have been updated :
    connect (this,
             SIGNAL(serverStatisticsUpdateSignal(const int)),
             this->parent->getCore(),
             SLOT(serverStatisticsUpdateSlot(const int)));


    // calculate average download speed for all nntp client instances of this server group :
    connect(clientsObserver->getStatsInfoBuilder()->getDownloadSpeedTimer(),
            SIGNAL(timeout()),
            this,
            SLOT(updateDownloadSpeedSlot()));
}



void ClientsPerServerObserver::resetVariables() {

    ClientsObserverBase::resetVariables() ;

    this->downloadSpeed = 0;
    this->averageDownloadSpeed = 0;
    this->segmentInfoData = SegmentInfoData();
    this->bytesDownloadedForCurrentSession = 0;

}



void ClientsPerServerObserver::nntpClientSpeedPerServerSlot(const SegmentInfoData segmentInfoData) {

    int bytesDownloaded = segmentInfoData.getBytesDownloaded();

    this->addBytesDownloaded(bytesDownloaded);

    // store the number of bytes downloaded for the whole session in order to display it in sidebar :
    this->bytesDownloadedForCurrentSession += bytesDownloaded;

    // store info to compute remaining time and to display additional data in side bar :
    this->segmentInfoData = segmentInfoData;

    // forward current server download speed to global observer :
    emit speedSignal(bytesDownloaded);

}



void ClientsPerServerObserver::connectionStatusPerServerSlot(const int connectionStatus) {

    this->updateTotalConnections(connectionStatus);

    // forward current server download speed to global observer :
    emit connectionStatusSignal(connectionStatus);

    emit serverStatisticsUpdateSignal(this->parent->getRealServerGroupId());

}


void ClientsPerServerObserver::nntpErrorPerServerSlot(const int nttpErrorStatus) {

    this->setNntpErrorStatus(nttpErrorStatus);

    // forward current server download speed to global observer :
    emit nntpErrorSignal(nttpErrorStatus);


}

void ClientsPerServerObserver::encryptionStatusPerServerSlot(const bool sslActive, const QString encryptionMethod, const bool certificateVerified, const QString issuerOrgranisation, const QStringList sslErrors) {

    //kDebug() << "sslActive : " << sslActive << "encryptionMethod" << encryptionMethod;

    this->setSslHandshakeParameters(sslActive, encryptionMethod, certificateVerified, issuerOrgranisation, sslErrors);

    // forward current server download speed to global observer :
    emit encryptionStatusSignal (sslActive, encryptionMethod, certificateVerified, issuerOrgranisation, sslErrors);

    emit serverStatisticsUpdateSignal(this->parent->getRealServerGroupId());
}



void ClientsPerServerObserver::updateDownloadSpeedSlot() {

    // get current download speed :
    this->downloadSpeed = this->totalBytesDownloaded / StatsInfoBuilder::SPEED_AVERAGE_SECONDS;

    // compute average download speed :
    this->averageDownloadSpeed = (this->averageDownloadSpeed + this->downloadSpeed) / 2;

    // notify side bar that download speed has been updated :
    emit serverStatisticsUpdateSignal(this->parent->getRealServerGroupId());

    // reset number of bytes downloaded after text update :
    this->totalBytesDownloaded = 0;

}


bool ClientsPerServerObserver::isSslActive() const {
    return this->sslActive;
}


quint64 ClientsPerServerObserver::getDownloadSpeed() const {
    return this->downloadSpeed;
}


quint64 ClientsPerServerObserver::getAverageDownloadSpeed() const {
    return this->averageDownloadSpeed;
}


SegmentInfoData ClientsPerServerObserver::getSegmentInfoData() const {
    return this->segmentInfoData;
}


quint64 ClientsPerServerObserver::getBytesDownloadedForCurrentSession() const {
    return this->bytesDownloadedForCurrentSession;
}


