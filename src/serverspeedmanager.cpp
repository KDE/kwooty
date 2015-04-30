/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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

#include "serverspeedmanager.h"

#include "kwooty_debug.h"

#include "servergroup.h"
#include "clientmanagerconn.h"
#include "observers/clientsperserverobserver.h"

ServerSpeedManager::ServerSpeedManager(ServerGroup *parent) : QObject(parent)
{

    this->parent = parent;
    // check average download speed every 15 seconds :
    this->downloadSpeedTimer = new QTimer(this);
    this->downloadSpeedTimer->setInterval(SPEED_MONITORING_TIME * 1000);

    this->setupConnections();
    this->resetVariables();

}

void ServerSpeedManager::setupConnections()
{
    // adjust download speed :
    connect(downloadSpeedTimer, SIGNAL(timeout()), this, SLOT(adjustDownloadSpeedSlot()));
}

void ServerSpeedManager::resetVariables()
{

    this->speedTooLowCounter = 0;
    this->clientSpeedPriority = NoPriorityClient;
    this->speedManagementStatus = NoChangeSpeed;
    this->downloadSpeedLimitInBytes = 0;

}

void ServerSpeedManager::setDownloadSpeedLimitInBytes(const qint64 &downloadSpeedLimitInBytes)
{
    this->downloadSpeedLimitInBytes = downloadSpeedLimitInBytes;
}

qint64 ServerSpeedManager::getDownloadSpeedLimitInBytes() const
{
    return this->downloadSpeedLimitInBytes;
}

void ServerSpeedManager::setBandwidthMode(const BandwidthClientMode &bandwidthClientMode)
{

    //qCDebug(KWOOTY_LOG) << "group : " << this->parent->getServerGroupId() << "BandwidthMode : " << bandwidthClientMode;

    if (bandwidthClientMode == BandwidthLimited) {
        this->downloadSpeedTimer->start();
    } else if (bandwidthClientMode == BandwidthFull) {
        this->downloadSpeedTimer->stop();
        this->resetVariables();
    }

    // notify all clients that bandwidth mode has changed :
    emit limitDownloadSpeedSignal(bandwidthClientMode);

}

int ServerSpeedManager::getEnabledClientNumber() const
{

    int clientNumber = 0;

    foreach (ClientManagerConn *clientManagerConn, this->parent->getClientManagerConnList()) {

        // if client is not disabled for speed limit :
        if (!clientManagerConn->isBandwidthNotNeeded()) {
            clientNumber++;
        }
    }

    return clientNumber;
}

bool ServerSpeedManager::disableClientForRateControl() const
{

    bool changeSuccessful = false;

    QList<ClientManagerConn *> clientManagerConnList = this->parent->getClientManagerConnList();

    // disable client for speed limit :
    if (this->speedManagementStatus == ReduceSpeed) {

        for (int i = clientManagerConnList.size() - 1; i >= 0; i--) {

            ClientManagerConn *clientManagerConn = clientManagerConnList.at(i);

            if (clientManagerConn->isBandwidthLimited() && this->getEnabledClientNumber() > 1) {

                clientManagerConn->setBandwidthMode(BandwidthNotNeeded);
                changeSuccessful = true;
                break;

            }
        }
    }
    // enable client for speed limit :
    else if (this->speedManagementStatus == IncreaseSpeed) {

        foreach (ClientManagerConn *clientManagerConn, clientManagerConnList) {

            // is current client has been disabled for speed limit purposes, enable it :
            if (clientManagerConn->isBandwidthNotNeeded()) {

                clientManagerConn->setBandwidthMode(BandwidthLimited);
                changeSuccessful = true;
                break;

            }
        }
    }

    return changeSuccessful;
}

void ServerSpeedManager::manageClientsNumber(const SpeedManagementStatus &speedManagementStatusOld)
{

    // get number of used clients :
    int enabledClientNumber = this->getEnabledClientNumber();

    // speed management status is different from previous one, do not currently change number of used clients :
    if (speedManagementStatusOld != this->speedManagementStatus) {
        this->clientSpeedPriority = NoPriorityClient;
    }

    // else define the number of clients to enable/disable :
    else {
        if (this->clientSpeedPriority == NoPriorityClient) {
            this->clientSpeedPriority = LowPriorityClient;
        }

        else if (this->clientSpeedPriority == LowPriorityClient) {
            this->clientSpeedPriority = HighPriorityClient;
        }

        int clientToChangeNumber = 0;

        // current download speed is too high, disable/enable half of enabled clients :
        if (this->clientSpeedPriority == HighPriorityClient) {

            // reduce the number of clients to half of used servers :
            if (this->speedManagementStatus == ReduceSpeed) {
                clientToChangeNumber = enabledClientNumber / 2;
            }

            // increase the number of clients to half of not used servers :
            else if (this->speedManagementStatus == IncreaseSpeed) {
                clientToChangeNumber = (this->parent->getClientManagerConnList().size() - enabledClientNumber) / 2;
            }
        }

        // a client was previously enabled/disabled , just disable/enable one client to properly adjust speed limit :
        else if (this->clientSpeedPriority == LowPriorityClient) {
            clientToChangeNumber = 1;
        }

        while (clientToChangeNumber > 0) {

            // disable/enable clients :
            this->disableClientForRateControl();

            clientToChangeNumber--;
            this->speedTooLowCounter = 0;

        }

    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ServerSpeedManager::adjustDownloadSpeedSlot()
{

    // compute current average download speed :
    quint64 meanDownloadSpeedInBytes = this->parent->getClientsPerServerObserver()->getAverageDownloadSpeed();

    // if server group is downloading :
    if (meanDownloadSpeedInBytes > 0) {

        // store current speedManagementStatus :
        SpeedManagementStatus speedManagementStatusOld = this->speedManagementStatus;

        // reduce the number of used clients if download speed if over 98 % of limit speed value :
        quint64 minSpeedLimit = this->downloadSpeedLimitInBytes - (this->downloadSpeedLimitInBytes * 2 / 100);

        if (meanDownloadSpeedInBytes > minSpeedLimit) {

            // update speedManagementStatus value :
            if (this->speedManagementStatus == IncreaseSpeed) {
                this->speedManagementStatus = NoChangeSpeed;
            }

            else if (this->speedManagementStatus == NoChangeSpeed) {
                this->speedManagementStatus = ReduceSpeed;

            }

            // reduce the number of clients :
            if (this->speedManagementStatus == ReduceSpeed) {
                this->manageClientsNumber(speedManagementStatusOld);
            }

        }
        // increase the number of used clients :
        else {

            this->speedTooLowCounter++;

            // if speed is too low 30 seconds in a row, reenable some clients :
            if (this->speedTooLowCounter > 1) {

                // update speedManagementStatus value :
                if (this->speedManagementStatus == ReduceSpeed) {
                    this->speedManagementStatus = NoChangeSpeed;
                }

                else if (this->speedManagementStatus == NoChangeSpeed) {
                    this->speedManagementStatus = IncreaseSpeed;

                }

                // increase the number of clients :
                if (this->speedManagementStatus == IncreaseSpeed) {
                    this->manageClientsNumber(speedManagementStatusOld);
                }

            }

        }

    }

}

