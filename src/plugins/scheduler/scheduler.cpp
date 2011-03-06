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


#include "scheduler.h"

#include <KDebug>

#include <QTime>
#include <QDate>
#include <QStandardItem>

#include "mainwindow.h"
#include "centralwidget.h"
#include "fileoperations.h"
#include "servermanager.h"
#include "servergroup.h"
#include "observers/clientsperserverobserver.h"
#include "schedulerplugin.h"
#include "schedulerfilehandler.h"
#include "kwooty_schedulersettings.h"



Scheduler::Scheduler(SchedulerPlugin* parent) :  QObject(parent) {

    this->centralWidget = parent->getCore()->getCentralWidget();
    this->serverManager = parent->getCore()->getCentralWidget()->getServerManager();

    // get model :
    this->schedulerModel = SchedulerFileHandler().loadModelFromFile(this);

    // check average download speed every 5 seconds :
    this->schedulerTimer = new QTimer(this);
    this->schedulerTimer->start(5000);

    // init to no download limit :
    this->downloadLimitStatus = NoLimitDownload;

    // update scheduler behavior :
    this->settingsChanged();

    // setup signals/slots connections :
    this->setupConnections();

}


Scheduler::~Scheduler() {

}


void Scheduler::setupConnections() {

    // when time-out occurs, check if nzb files from list can be processed
    connect(this->schedulerTimer,
            SIGNAL(timeout()),
            this,
            SLOT(schedulerTimerSlot()));


    // be notified when server settings have changed :
    connect (this->serverManager,
             SIGNAL(serverManagerSettingsChangedSignal()),
             this,
             SLOT(serverManagerSettingsChangedSlot()));

}




//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void Scheduler::schedulerTimerSlot() {

    // permanent speed limit is enabled :
    if (SchedulerSettings::enablePermanentSpeedLimit()) {

        this->checkDownloadStatus(LimitDownload);
        this->applySpeedLimit();

    }
    // speed limit is scheduled :
    else {

        // retrieve current time :
        QTime currentTime = QTime::currentTime();

        // get row and column numbers from model according to current time :
        int column = (currentTime.hour() * 60 + currentTime.minute()) / 30;
        int row = QDate().currentDate().dayOfWeek();

        // get corresponding download limit status :
        QStandardItem* item = this->schedulerModel->item(row, column);
        DownloadLimitStatus downloadLimitStatus = static_cast<DownloadLimitStatus>(item->data(DownloadLimitRole).toInt());

        // start downloads if they were previously paused :
        this->checkDownloadStatus(downloadLimitStatus);

        if (downloadLimitStatus == LimitDownload) {
            this->applySpeedLimit();
        }

    }

}


void Scheduler::serverManagerSettingsChangedSlot() {

    // reset download status to full speed if server settings changed :
    this->downloadLimitStatus = NoLimitDownload;
    this->disableSpeedLimit();
}




void Scheduler::suspendDownloads() {
    this->centralWidget->pauseAllDownloadSlot();
}


void Scheduler::checkDownloadStatus(const DownloadLimitStatus& downloadLimitStatus) {

    // for each polling, check that pending files are set on pause if download status is disabled :
    if (downloadLimitStatus == DisabledDownload) {
        this->suspendDownloads();
    }

    // restart downloads if download status is no more disabled :
    if (downloadLimitStatus != this->downloadLimitStatus) {

        // if previous status has paused downloads, its time to restart them :
        if (this->downloadLimitStatus == DisabledDownload) {
            this->centralWidget->startAllDownloadSlot();
        }

        // then apply proper bandwidth management according to current status :
        if (downloadLimitStatus == NoLimitDownload) {
            this->serverManager->setBandwidthMode(BandwidthFull);
        }
        else if (downloadLimitStatus == LimitDownload) {
            this->serverManager->setBandwidthMode(BandwidthLimited);
        }

    }

    // store download status :
    this->downloadLimitStatus = downloadLimitStatus;

}


void Scheduler::applySpeedLimit() {

    int serverNumber = this->serverManager->getServerNumber();
    int serversCurrentlyDownloadingNumber = 0;
    int totalDownloadSpeed = 0;

    // retrieve download speed for all avaialable servers (master + backups) :
    for (int i = 0; i < serverNumber; i++) {

        int serverDownloadSpeed = this->serverManager->retrieveServerDownloadSpeed(i);

        if (serverDownloadSpeed > 0) {
            serversCurrentlyDownloadingNumber++;
        }

        totalDownloadSpeed += serverDownloadSpeed;
    }


    // for each server group, apply download speed limit :
    for (int i = 0; i < serverNumber; i++) {

        if (totalDownloadSpeed > SchedulerSettings::downloadLimitSpinBox()) {

            qint64 serverLimitSpeedInBytes = SchedulerSettings::downloadLimitSpinBox() * NBR_BYTES_IN_KB / serversCurrentlyDownloadingNumber;

            int serverDownloadSpeed = this->serverManager->retrieveServerDownloadSpeed(i);

            if (serverDownloadSpeed > 0) {

                this->serverManager->setLimitServerDownloadSpeed(i, serverLimitSpeedInBytes);

                //kDebug() << "serveur group : " << i << ", limited download speed : " << serverLimitSpeedInBytes / NBR_BYTES_IN_KB << " KiB/s";
            }
        }


    }


}



void Scheduler::disableSpeedLimit() {
    this->serverManager->setBandwidthMode(BandwidthFull);
}


void Scheduler::settingsChanged() {

    // reload settings from just saved config file :
    SchedulerSettings::self()->readConfig();

    this->schedulerModel = SchedulerFileHandler().loadModelFromFile(this);

    // restart previously paused downloads after settings changes :
    this->checkDownloadStatus(NoLimitDownload);

}


