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


#include "statsinfobuilder.h"

#include <KDebug>
#include "kdiskfreespaceinfo.h"

#include <QDateTime>

#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "core.h"
#include "notificationmanager.h"
#include "servermanager.h"
#include "observers/clientsobserver.h"
#include "kwootysettings.h"


StatsInfoBuilder::StatsInfoBuilder(ClientsObserver* clientsObserver, Core* parent)  : QObject(parent) {

    this->parent = parent;
    this->clientsObserver = clientsObserver;

    this->downloadModel = parent->getDownloadModel();

    // set timer to compute average download speed each SPEED_AVERAGE_SECONDS :
    this->downloadSpeedTimer = new QTimer(this);
    this->downloadSpeedTimer->start(SPEED_AVERAGE_SECONDS * 1000);

    this->resetVariables();
    this->setupConnections();

}

void StatsInfoBuilder::sendFullUpdate() {

    this->updateDownloadSpeedSlot();
    this->computeTimeInfo();
    this->retrieveFreeDiskSpace();

}

QString StatsInfoBuilder::getTimeLabel() const {
    return this->timeLabel;
}

QString StatsInfoBuilder::getTotalTimeValue() const {
    return this->totalTimeValue;
}

QString StatsInfoBuilder::getCurrentTimeValue() const {
    return this->currentTimeValue;
}

QString StatsInfoBuilder::getNzbNameDownloading() const {
    return this->nzbNameDownloading;
}

QString StatsInfoBuilder::getDownloadSpeedReadableStr() const {
    return this->downloadSpeedReadableStr;
}

QTimer* StatsInfoBuilder::getDownloadSpeedTimer() const {
    return this->downloadSpeedTimer;
}


void StatsInfoBuilder::resetVariables() {

    this->meanDownloadSpeedTotal = 0;
    this->meanDownloadSpeedCurrent = 0;
    this->downloadSpeedTotal = 0;
    this->downloadSpeedCurrent = 0;
    this->meanSpeedActiveCounter = 0;
    this->timeoutCounter = 1;
    this->parentStateIndex = QModelIndex();
    this->nzbNameDownloading.clear();
    this->timeLabel.clear();
    this->totalTimeValue.clear();
    this->currentTimeValue.clear();
    this->downloadSpeedReadableStr.clear();
    this->previousDiskSpaceStatus = SufficientDiskSpace;

}


void StatsInfoBuilder::setupConnections() {

    // calculate average download speed for all nntp client instances :
    connect(this->downloadSpeedTimer, SIGNAL(timeout()), this, SLOT(updateDownloadSpeedSlot()));

    // parent notify that settings have been changed :
    connect (parent,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));

}


void StatsInfoBuilder::settingsChangedSlot() {

    // "remaining time" or "estimated time of arrival" could have been chosen, update text :
    this->computeTimeInfo();

    // disk free space should be displayed :
    if (Settings::displayCapacityBar()) {
        this->retrieveFreeDiskSpace();
    }
    // disk free space should be hidden :
    else {
        emit updateFreeSpaceSignal(UnknownDiskSpace);
    }

}



void StatsInfoBuilder::computeMeanSpeed(const quint64& downloadSpeed, quint64& meanDownloadSpeed) {
    float alpha = 0.2;
    meanDownloadSpeed = alpha * downloadSpeed + (1 - alpha) * meanDownloadSpeed;
}



void StatsInfoBuilder::updateDownloadSpeedSlot() {

    // 1. calculate average download speed for the current nzb item downloaded
    // (in case of multiserver usage, it can happens that several servers download the same nzb item simultaneously,
    // retrieve the download speed of these servers to get the overall download speed of the current nzb item) :
    ServerManager* serverManager = this->parent->getServerManager();

    if (serverManager) {

        // search first current item being downloading :
        QStandardItem* stateItem = this->parent->getModelQuery()->searchParentItem(DownloadStatus);

        // if item has been found :
        if (stateItem) {

            // calculate average download speed for remaining time calculation of the current nzb being downloading :
            this->computeMeanSpeed(this->downloadSpeedCurrent, this->meanDownloadSpeedCurrent);

            // calculate download speed of each server groups downloading simultaneoulsy the same nzb item
            // in order to get a more accurate time of arrival for the current downloaded item :
            this->downloadSpeedCurrent = serverManager->retrieveCumulatedDownloadSpeed(stateItem->row());
        }

    }



    // 2. calculate average download speed for remaining time calculation for total remaining nzb items :
    this->computeMeanSpeed(this->downloadSpeedTotal, this->meanDownloadSpeedTotal);

    // then, get current download speed :
    this->downloadSpeedTotal = this->clientsObserver->getTotalBytesDownloaded() / SPEED_AVERAGE_SECONDS;

    // send download speed to status bar :
    this->downloadSpeedReadableStr = Utility::convertDownloadSpeedHumanReadable(this->downloadSpeedTotal);
    emit updateDownloadSpeedInfoSignal(this->downloadSpeedReadableStr);

    // reset number of bytes downloaded after text update :
    this->clientsObserver->resetTotalBytesDownloaded();


    // at download begining mean speed equals current downloadSpeed in order to
    // not get too lag before reaching a proper mean speed value :
    if (this->meanSpeedActiveCounter < 10) {

        this->meanDownloadSpeedTotal = this->downloadSpeedTotal;
        this->meanDownloadSpeedCurrent  = this->downloadSpeedCurrent;

        this->meanSpeedActiveCounter++;
    }

    // when download speed is 0, calculate mean speed as above :
    if (this->downloadSpeedTotal == 0) {
        this->meanSpeedActiveCounter = 0;
    }


    // synchronize remaining time calculation and free disk space with this slot :

    // call retrieveCurrentRemainingSize() every 2 * SPEED_AVERAGE_SECONDS (4 secs) :
    if ( (this->timeoutCounter % 2) == 0 ) {

        this->computeTimeInfo();

    }

    // call retrieveFreeDiskSpace() every 5 * SPEED_AVERAGE_SECONDS (10 secs):
    if ( (this->timeoutCounter % 10) == 0 ) {

        this->retrieveFreeDiskSpace();

        // reset the counter :
        this->timeoutCounter = 0;
    }

    this->timeoutCounter++;



}



void StatsInfoBuilder::retrieveQueuedFilesInfo(bool& parentDownloadingFound, bool& parentQueuedFound) {

    // get the root model :
    QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

    // get the first parent with download active :
    for (int i = 0; i < rootItem->rowCount(); i++) {

        QStandardItem* parentStateItem = rootItem->child(i, STATE_COLUMN);
        UtilityNamespace::ItemStatus currentStatus = this->parent->getDownloadModel()->getStatusFromStateItem(parentStateItem);


        // check if parent status is either downloading or pausing :
        if (!parentDownloadingFound && Utility::isDownloadOrPausing(currentStatus)) {

            // if the parent is different from the previous one :
            if (this->parentStateIndex != parentStateItem->index()) {

                // update current "downloading" parent :
                this->parentStateIndex = parentStateItem->index();

            }

            // get name of the current download nzb :
            this->nzbNameDownloading = this->downloadModel->getFileNameItemFromIndex(this->parentStateIndex)->text();

            parentDownloadingFound = true;

        }

        // check if there are queued files in order to compute total remaining time :
        if (Utility::isInQueue(currentStatus)) {

            parentQueuedFound = true;

        }

    }


}



void StatsInfoBuilder::computeTimeInfo() {

    this->currentTimeValue = QString();
    this->totalTimeValue = QString();

    bool parentDownloadingFound = false;
    bool parentQueuedFound = false;

    // get index of the first parent being downloading and check if they are other queued parents :
    this->retrieveQueuedFilesInfo(parentDownloadingFound, parentQueuedFound);

    // compute remaining time only if a nzb is being downloading and if average speed not equals to 0 :
    if (parentDownloadingFound) {

        // compute remaining time for the current item being downloaded :
        if (this->meanDownloadSpeedCurrent != 0) {

            // retrieve accomplished download percentage :
            int downloadProgress = this->downloadModel->getProgressValueFromIndex(this->parentStateIndex);

            // retrieve nzb size :
            quint64 nzbSize = this->downloadModel->getSizeValueFromIndex(this->parentStateIndex);

            // compute *current* remaining download time (sec) :
            quint32 currentRemainingTimeSec = qRound(nzbSize * (PROGRESS_COMPLETE - downloadProgress) / (this->meanDownloadSpeedCurrent * PROGRESS_COMPLETE ));

            // calculate Estimated Time of Arrival :
            if (Settings::etaRadioButton()) {
                this->timeLabel = i18n("Time of arrival:");
                this->currentTimeValue = this->calculateArrivalTime(currentRemainingTimeSec);
            }

            // else calculate Remaining Time :
            if (Settings::rtRadioButton()) {
                this->timeLabel = i18n("Remaining time:");
                this->currentTimeValue = this->calculateRemainingTime(currentRemainingTimeSec);
            }

        }

        // compute remaining time for the total items to download :
        if (this->meanDownloadSpeedTotal != 0) {

            // compute *total* remaining download time (sec) only if other pending parents have been found :
            if (parentQueuedFound) {

                quint32 totalRemainingTimeSec = qRound(this->clientsObserver->getTotalSize() / this->meanDownloadSpeedTotal);

                // calculate Estimated Time of Arrival :
                if (Settings::etaRadioButton()) {
                    this->totalTimeValue = this->calculateArrivalTime(totalRemainingTimeSec);
                }

                // calculate Remaining Time :
                if (Settings::rtRadioButton()) {
                    this->totalTimeValue = this->calculateRemainingTime(totalRemainingTimeSec);
                }

            }
        }


    }

    // send time info signal :
    emit updateTimeInfoSignal(parentDownloadingFound);

}


QString StatsInfoBuilder::calculateArrivalTime(const quint32& remainingSeconds) {

    QDateTime dateTimeETA = QDateTime::currentDateTime();
    dateTimeETA = dateTimeETA.addSecs(remainingSeconds);

    return dateTimeETA.toString(Utility::getSystemTimeFormat("ddd hh:mm"));

}



QString StatsInfoBuilder::calculateRemainingTime(const quint32& remainingSeconds) {

    QString remainingTimeStr;

    // calculate remaining days, hours, minutes :
    int remainingDays = remainingSeconds / SECONDS_IN_DAY;
    int remainingHours = (remainingSeconds - (remainingDays * SECONDS_IN_DAY)) / SECONDS_IN_HOUR;
    int remainingMinutes = (remainingSeconds - ( (remainingDays * SECONDS_IN_DAY) + remainingHours * SECONDS_IN_HOUR) ) / SECONDS_IN_MINUTE;


    // display number of remaining days if any :
    if (remainingDays > 0) {
        remainingTimeStr.append(i18np("%1 day ", "%1 days ", remainingDays));
    }

    // display number of remaining hours if any :
    if (remainingHours > 0) {
        remainingTimeStr.append(i18np("%1 hour ", "%1 hours ", remainingHours));
    }

    // display number of remaining minutes :
    remainingTimeStr.append(i18np("%1 minute", "%1 minutes", remainingMinutes));

    // when this is the last minute, display "less than 1 minute" instead of 0 minute :
    if (remainingDays == 0 && remainingHours == 0 && remainingMinutes == 0) {
        remainingTimeStr = i18n("less than 1 minute");
    }


    return remainingTimeStr;

}




void StatsInfoBuilder::retrieveFreeDiskSpace() {

    if (Settings::displayCapacityBar()) {

        // get download path :
        QString downloadDisk = Settings::completedFolder().path();

        if (KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).isValid()) {

            // get disk size :
            quint64 sizeVal = KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).size();

            // get disk used :
            quint64 usedVal = KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).used();

            // consider disk space is sufficient by default :
            UtilityNamespace::FreeDiskSpace diskSpaceStatus = SufficientDiskSpace;

            quint64 freeSpaceVal = KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).available();

            if (this->clientsObserver->getTotalSize() >= freeSpaceVal) {

                diskSpaceStatus = InsufficientDiskSpace;

                // send notification event once if insufficent disk space occurs several times :
                if (this->previousDiskSpaceStatus != InsufficientDiskSpace) {
                    emit insufficientDiskSpaceSignal(i18n("Insufficient disk space"));
                }
            }

            // get free space size :
            QString freeSpaceStr = i18nc("free disk space available", "%1 free", Utility::convertByteHumanReadable(freeSpaceVal));

            // calculate percentage of used disk :
            int usedDiskPercentage = qMin( qRound(static_cast<qreal>(usedVal * 100 / sizeVal)), PROGRESS_COMPLETE );

            emit updateFreeSpaceSignal(diskSpaceStatus, freeSpaceStr, usedDiskPercentage);

            // store previous diskSpaceStatus :
            this->previousDiskSpaceStatus = diskSpaceStatus;

        }

        // disk space can not be retrieved :
        else {
            emit updateFreeSpaceSignal(UnknownDiskSpace);
        }


    }


}
