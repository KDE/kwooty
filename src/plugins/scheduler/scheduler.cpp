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
#include <KCMultiDialog>
#include <KActionCollection>

#include <QTime>
#include <QDate>
#include <QStandardItem>
#include <QAction>

#include "mainwindow.h"
#include "core.h"
#include "fileoperations.h"
#include "servermanager.h"
#include "servergroup.h"
#include "actionsmanager.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "observers/clientsperserverobserver.h"
#include "widgets/mytreeview.h"
#include "schedulerplugin.h"
#include "schedulerfilehandler.h"
#include "kwooty_schedulersettings.h"



Scheduler::Scheduler(SchedulerPlugin* parent) :  QObject(parent) {

    this->core = parent->getMainWindow()->getCore();
    this->serverManager = this->core->getServerManager();
    this->statusBar = parent->getMainWindow()->getStatusBar();

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

    // when time-out occurs, check current download speed :
    connect(this->schedulerTimer,
            SIGNAL(timeout()),
            this,
            SLOT(schedulerTimerSlot()));


    // be notified when server settings have changed :
    connect (this->serverManager,
             SIGNAL(serverManagerSettingsChangedSignal()),
             this,
             SLOT(serverManagerSettingsChangedSlot()));


    // be notified when nzb data has arrived :
    connect (this->core,
             SIGNAL(dataAboutToArriveSignal()),
             this,
             SLOT(dataAboutToArriveSlot()));


    // display settings when statur bar widget has been double clicked :
    connect (this->statusBar,
             SIGNAL(statusBarWidgetDblClickSignal(MyStatusBar::WidgetIdentity)),
             this,
             SLOT(statusBarWidgetDblClickSlot(MyStatusBar::WidgetIdentity)));


    // allow user to bypass scheduler if start/pause actions have been manually trigered :
    connect(this->core->getActionsManager(),
            SIGNAL(startPauseAboutToBeTriggeredSignal(UtilityNamespace::ItemStatus)),
            this,
            SLOT(startPauseAboutToBeTriggeredSlot(UtilityNamespace::ItemStatus)));

    connect(this->core->getActionsManager(),
            SIGNAL(startPauseTriggeredSignal(UtilityNamespace::ItemStatus)),
            this,
            SLOT(dataAboutToArriveSlot()));

}




void Scheduler::suspendDownloads() {

    kDebug() << "suspendDownloads";
    this->scheduleStartPauseDownload(PauseStatus);

}


void Scheduler::resumeDownloads() {

    kDebug() << "resumeDownloads";
    this->scheduleStartPauseDownload(IdleStatus);

}


void Scheduler::scheduleStartPauseDownload(UtilityNamespace::ItemStatus itemStatus) {

    kDebug() << this->core->getModelQuery()->retrieveDecodeFinishParentIndexList();

    // 1.first remove all indexes from list whose download is over :
    foreach (QModelIndex decodeFinishParentIndex, this->core->getModelQuery()->retrieveDecodeFinishParentIndexList()) {

        QString parentUuid = this->core->getDownloadModel()->getUuidStrFromIndex(decodeFinishParentIndex);

        if (this->manuallyUuidStartPauseMap.contains(parentUuid)) {

            kDebug() << "ITEM REMOVED => Download finish";
            this->manuallyUuidStartPauseMap.remove(parentUuid);

        }
    }

    // 2. retrieve all uuid items currently Idle to set on Pause or vice versa :
    QList<QModelIndex> targetIndexesList;
    foreach (QModelIndex index, this->core->getModelQuery()->retrieveStartPauseIndexList(itemStatus)) {

        // if bypass is enabled and items have not been manually set on start/pause :
        if (!this->retrieveProperListFromMap(itemStatus).contains(this->core->getDownloadModel()->getUuidStrFromIndex(index)) ) {

            targetIndexesList.append(index);

            kDebug() << this->core->getDownloadModel()->getUuidStrFromIndex(index) << itemStatus;
        }

    }

    // if list of items to start/pause is not empty, trigger action :
    if (!targetIndexesList.isEmpty()) {
        this->core->getActionsManager()->setStartPauseDownload(itemStatus, targetIndexesList);
    }

}


QList<QString> Scheduler::retrieveProperListFromMap(const UtilityNamespace::ItemStatus& targetItemStatus) const {

    QList<QString> bypassedItemList = this->manuallyUuidStartPauseMap.keys(Scheduler::BypassItemsPauseOrStart);

    // if download has to triggered, search for manually paused items that shall not be started :
    if (targetItemStatus == IdleStatus) {
        bypassedItemList.append(this->manuallyUuidStartPauseMap.keys(Scheduler::BypassItemsPause));
    }
    // if pause has to triggered, search for manually started items that shall not be paused :
    else if (targetItemStatus == PauseStatus) {
        bypassedItemList.append(this->manuallyUuidStartPauseMap.keys(Scheduler::BypassItemsStart));
    }

    return bypassedItemList;

}



void Scheduler::initUuidStartPauseMap() {

    if (SchedulerSettings::schedulerBypassMethods() == Scheduler::BypassItemsPause) {

        foreach (QString uuid, this->manuallyUuidStartPauseMap.keys(Scheduler::BypassItemsStart)) {
            this->manuallyUuidStartPauseMap.remove(uuid);
        }

        foreach (QString uuid, this->manuallyUuidStartPauseMap.keys(Scheduler::BypassItemsPauseOrStart)) {
            this->manuallyUuidStartPauseMap.insert(uuid, BypassItemsPause);

        }

    }
    else if (SchedulerSettings::schedulerBypassMethods() == Scheduler::BypassItemsStart) {

        foreach (QString uuid, this->manuallyUuidStartPauseMap.keys(Scheduler::BypassItemsPause)) {
            this->manuallyUuidStartPauseMap.remove(uuid);
        }

        foreach (QString uuid, this->manuallyUuidStartPauseMap.keys(Scheduler::BypassItemsPauseOrStart)) {
            this->manuallyUuidStartPauseMap.insert(uuid, BypassItemsStart);

        }

    }

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
            this->resumeDownloads();
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

    // retrieve total download speed for all avaialable servers (master + backups) :
    for (int i = 0; i < serverNumber; i++) {

        int serverDownloadSpeed = static_cast<int>(this->serverManager->retrieveServerDownloadSpeed(i));

        if (serverDownloadSpeed > 0) {
            serversCurrentlyDownloadingNumber++;
        }

        totalDownloadSpeed += serverDownloadSpeed;
    }


    // for each server group, apply download speed limit :
    for (int i = 0; i < serverNumber; i++) {

        if (totalDownloadSpeed > SchedulerSettings::downloadLimitSpinBox()) {

            qint64 serverLimitSpeedInBytes = SchedulerSettings::downloadLimitSpinBox() * NBR_BYTES_IN_KB / serversCurrentlyDownloadingNumber;

            qint64 serverDownloadSpeed = this->serverManager->retrieveServerDownloadSpeed(i);

            if (serverDownloadSpeed > 0) {

                this->serverManager->setLimitServerDownloadSpeed(i, serverLimitSpeedInBytes);

                //kDebug() << "serveur group : " << i << ", limited download speed : " << serverLimitSpeedInBytes / NBR_BYTES_IN_KB << " KiB/s";
            }
        }


    }


}

DownloadLimitStatus Scheduler::getCurrentDownloadLimitStatus() {

    // retrieve current time :
    QTime currentTime = QTime::currentTime();

    // get row and column numbers from model according to current time :
    int column = (currentTime.hour() * 60 + currentTime.minute()) / 30;
    int row = QDate().currentDate().dayOfWeek();

    // get corresponding download limit status :
    QStandardItem* item = this->schedulerModel->item(row, column);
    return static_cast<DownloadLimitStatus>(item->data(DownloadLimitRole).toInt());

}


void Scheduler::disableSpeedLimit() {
    this->serverManager->setBandwidthMode(BandwidthFull);
}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void Scheduler::statusBarWidgetDblClickSlot(MyStatusBar::WidgetIdentity WidgetIdentity) {

    // if double click has been done on download speed widget :
    if (WidgetIdentity == MyStatusBar::SpeedWidgetIdentity) {

        // display scheduler settings dialog page :
        KCMultiDialog schedulerConfigDialog;

        schedulerConfigDialog.setFaceType( KCMultiDialog::Plain );
        schedulerConfigDialog.setWindowTitle(i18n("Bandwidth manager"));
        schedulerConfigDialog.addModule("kwooty_schedulersettings");
        schedulerConfigDialog.resize (600, 400);
        schedulerConfigDialog.exec();

        // once the page has been closed, apply new settings :
        this->settingsChanged();
    }

}


void Scheduler::schedulerTimerSlot() {

    DownloadLimitStatus downloadLimitStatus = LimitDownload;

    // if speed limit is scheduled :
    if (SchedulerSettings::enableScheduler()) {

        downloadLimitStatus = this->getCurrentDownloadLimitStatus();
    }

    // if downloadLimitSpinBox is set to 0, it corresponds to no limit download :
    if ( SchedulerSettings::downloadLimitSpinBox() == NO_SPEED_LIMIT &&
         downloadLimitStatus == LimitDownload ) {

        downloadLimitStatus = NoLimitDownload;

    }

    // start downloads if they were previously paused :
    this->checkDownloadStatus(downloadLimitStatus);

    if (downloadLimitStatus == LimitDownload) {
        this->applySpeedLimit();
    }

}


void Scheduler::serverManagerSettingsChangedSlot() {

    // reset download status to full speed if server settings changed :
    this->downloadLimitStatus = NoLimitDownload;
    this->disableSpeedLimit();
}



void Scheduler::dataAboutToArriveSlot() {

    if (SchedulerSettings::enableScheduler()) {

        // new nzb has been appended and download is currently disabled, set new items to pause status right now :
        if (this->getCurrentDownloadLimitStatus() == DisabledDownload) {
            this->suspendDownloads();

        }
        // else set new items to start status right now :
        else {
            this->resumeDownloads();
        }
    }

}


void Scheduler::startPauseAboutToBeTriggeredSlot(UtilityNamespace::ItemStatus targetItemStatus) {

    if ( SchedulerSettings::enableScheduler() &&
         SchedulerSettings::schedulerBypass() ) {

        // by default consider that scheduler does not bypass any items :
        BypassSchedulerMethod bypassSchedulerMethod = BypassNoItems;

        if (SchedulerSettings::schedulerBypassMethods() == Scheduler::BypassItemsPauseOrStart) {

            bypassSchedulerMethod = BypassItemsPauseOrStart;
        }

        // TODO : how to manage pause trigger when disk is full ???
        // if items have been manually set on pause :
        else if ( targetItemStatus == PauseStatus &&
                  SchedulerSettings::schedulerBypassMethods() == Scheduler::BypassItemsPause ) {

            bypassSchedulerMethod = BypassItemsPause;
        }

        // if items have been manually set on start :
        else if ( targetItemStatus == IdleStatus &&
                  SchedulerSettings::schedulerBypassMethods() == Scheduler::BypassItemsStart ) {

            bypassSchedulerMethod = BypassItemsStart;
        }

        if (bypassSchedulerMethod != BypassNoItems) {

            // retrieve items selected by user :
            foreach (QModelIndex selectedIndex, this->core->getTreeView()->selectionModel()->selectedRows()) {

                // retrieve their corresponding uuid and store them :
                QString indexUuidStr = this->core->getDownloadModel()->getUuidStrFromIndex(selectedIndex);

                kDebug() << "insert :" << indexUuidStr << bypassSchedulerMethod;
                this->manuallyUuidStartPauseMap.insert(indexUuidStr, bypassSchedulerMethod);

            }
        }
    }

}




void Scheduler::settingsChanged() {

    // reload settings from just saved config file :
    SchedulerSettings::self()->readConfig();

    this->schedulerModel = SchedulerFileHandler().loadModelFromFile(this);


    if (SchedulerSettings::enableScheduler()) {

        if (SchedulerSettings::schedulerBypass()) {

            this->initUuidStartPauseMap();

        }
        else {
            kDebug() << "clear";
            this->manuallyUuidStartPauseMap.clear();

            dataAboutToArriveSlot();
            //if (this->getCurrentDownloadLimitStatus() == NoLimitDownload) {
              //  this->downloadLimitStatus = DisabledDownload;
            //}

        }

    }

    // restart previously paused downloads after settings changes :
    this->checkDownloadStatus(NoLimitDownload);

}


