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


#include "infocollectordispatcher.h"

#include <KDebug>

#include <QDateTime>

#include "centralwidget.h"
#include "mystatusbar.h"
#include "standarditemmodel.h"
#include "infobar.h"
#include "icontextwidget.h"
#include "settings.h"

#include "kdiskfreespaceinfo.h"


InfoCollectorDispatcher::InfoCollectorDispatcher(CentralWidget* parent) : QObject(parent) {

    this->parent = parent;

    this->downloadModel = parent->getDownloadModel();


    // set timer to compute average download speed every each SPEED_AVERAGE_SECONDS :
    this->downloadSpeedTimer = new QTimer(this);
    this->downloadSpeedTimer->start(SPEED_AVERAGE_SECONDS * 1000);

    this->setupConnections();

    this->resetVariables();

}



void InfoCollectorDispatcher::setupConnections() {

    // calculate average download speed for all nntp client instances :
    connect(this->downloadSpeedTimer, SIGNAL(timeout()), this, SLOT(updateDownloadSpeedSlot()));


    // send remaining size to status bar :
    connect (this,
             SIGNAL(updateSizeInfoSignal(const QString)),
             parent->getStatusBar(),
             SLOT(updateSizeInfoSlot(const QString)));

    // send download speed to status bar :
    connect (this,
             SIGNAL(updateDownloadSpeedInfoSignal(const QString)),
             parent->getStatusBar(),
             SLOT(updateDownloadSpeedInfoSlot(const QString)));

    // send remaining files to status bar :
    connect (this,
             SIGNAL(updateFileInfoSignal(const QString)),
             parent->getStatusBar(),
             SLOT(updateFileInfoSlot(const QString)));


    // send ETA to info bar :
    connect (this,
             SIGNAL(updateTimeInfoSignal(const QString)),
             parent->getInfoBar(),
             SLOT(updateTimeInfoSlot(const QString)));

    // send free space info to info bar :
    connect (this,
             SIGNAL(updateFreeSpaceSignal(const UtilityNamespace::FreeDiskSpace, const QString, const int)),
             parent->getInfoBar(),
             SLOT(updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace, const QString, const int)));


    // parent notify that settings have been changed :
    connect (parent,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));


}




void InfoCollectorDispatcher::resetVariables() {

    this->totalFiles = 0;
    this->totalSize = 0;
    this->totalBytesDownloaded = 0;
    this->meanDownloadSpeedKB = 0;
    this->meanSpeedActiveCounter = 0;
    this->timeoutCounter = 1;
    this->parentStateIndex = QModelIndex();

    // send reset values to status bar :
    this->fullFileSizeUpdate(0, 0);
    this->updateDownloadSpeedSlot();

    // send reset values to top widget :
    this->computeTimeInfo();
    this->retrieveFreeDiskSpace();
}




void InfoCollectorDispatcher::settingsChangedSlot() {

    // remaining time or estimated time of arrival could have been chosen, update text :
    this->computeTimeInfo();

}


void InfoCollectorDispatcher::nntpClientspeedSlot(const int bytesDownloaded) {

    this->totalBytesDownloaded += bytesDownloaded;

}


void InfoCollectorDispatcher::decrementSlot(const quint64 size, const int fileNumber = 1) {


    this->totalFiles -= fileNumber;
    this->totalSize -= size;

    // status bar updates :
    emit updateSizeInfoSignal(Utility::convertByteHumanReadable(totalSize));
    emit updateFileInfoSignal(QString::number(totalFiles));

}



void InfoCollectorDispatcher::updateDownloadSpeedSlot() {

    // calculate average download speed for remaining time calculation :
    float alpha = 0.2;
    this->meanDownloadSpeedKB = alpha * this->downloadSpeedKB + (1 - alpha) * this->meanDownloadSpeedKB;

    //kDebug() << this->meanDownloadSpeedKB << (1 - alpha) << (1 - alpha) * this->meanDownloadSpeedKB;

    // get current download speed :
    this->downloadSpeedKB = this->totalBytesDownloaded / NBR_BYTES_IN_KB / SPEED_AVERAGE_SECONDS;

    QString sizeUnit = i18n(" KiB/s");

    emit updateDownloadSpeedInfoSignal(QString::number(this->downloadSpeedKB) + sizeUnit);

    // reset number of bytes downloaded after text update :
    this->totalBytesDownloaded = 0;



    // calculate average download speed for remaining time calculation :
    if (this->meanSpeedActiveCounter < 10) {

        this->meanDownloadSpeedKB = this->downloadSpeedKB;
        this->meanSpeedActiveCounter++;
    }

    if (this->downloadSpeedKB  == 0) {
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

        // reset counter :
        this->timeoutCounter = 0;
    }

    this->timeoutCounter++;



}



void InfoCollectorDispatcher::fullFileSizeUpdate(const quint64 size, const quint64 files) {


    this->totalSize = size;
    this->totalFiles = files;

    // status bar updates :
    emit updateSizeInfoSignal(Utility::convertByteHumanReadable(totalSize));
    emit updateFileInfoSignal(QString::number(totalFiles));

}




void InfoCollectorDispatcher::retrieveQueuedFilesInfo(bool& parentDownloadingFound, bool& parentQueuedFound) {


    // get the root model :
    QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

    // get the first parent with download active :
    for (int i = 0; i < rootItem->rowCount(); i++) {

        QStandardItem* parentStateItem = rootItem->child(i, STATE_COLUMN);
        UtilityNamespace::ItemStatus currentStatus = this->parent->getDownloadModel()->getStatusFromStateItem(parentStateItem);


        if (!parentDownloadingFound && Utility::isDownloadOrPausing(currentStatus)) {

            // if the parent is different from the previous one :
            if (this->parentStateIndex != parentStateItem->index()) {

                // get the new parent currently being downloading :
                this->parentStateIndex = parentStateItem->index();
            }

            parentDownloadingFound = true;
            //break;

        }

        if (Utility::isInQueue(currentStatus)) {

            parentQueuedFound = true;

        }

    }


}



void InfoCollectorDispatcher::computeTimeInfo() {

    bool parentDownloadingFound = false;
    bool parentQueuedFound = false;

    // get index of the first parent being downloading and check if they are other queued parents :
    this->retrieveQueuedFilesInfo(parentDownloadingFound, parentQueuedFound);


    QString timeInfoStr;


    // compute remaining time only if a nzb is being downloading and if average speed not equals to 0 :
    if (parentDownloadingFound && this->meanDownloadSpeedKB != 0) {

        // retrieve accomplished download percentage :
        int downloadProgress = this->downloadModel->getProgressValueFromIndex(this->parentStateIndex);

        // retrieve nzb size :
        quint64 nzbSize = this->downloadModel->getSizeValueFromIndex(this->parentStateIndex);


        // compute CURRENT remaining download time (sec) :
        quint32 currentRemainingTimeSec = qRound(nzbSize * (PROGRESS_COMPLETE - downloadProgress) / (this->meanDownloadSpeedKB * PROGRESS_COMPLETE * NBR_BYTES_IN_KB));


        // calculate Estimated Time of Arrival :
        if (Settings::etaRadioButton()) {


            timeInfoStr.append(this->calculateArrivalTime(currentRemainingTimeSec));
        }

        // calculate Remaining Time :
        if (Settings::rtRadioButton()) {

            timeInfoStr.append(this->calculateRemainingTime(currentRemainingTimeSec));
        }

        // compute TOTAL remaining download time (sec) only if other pending parents have been found:
        if (parentQueuedFound) {

            quint32 totalRemainingTimeSec = qRound(this->totalSize / (this->meanDownloadSpeedKB * NBR_BYTES_IN_KB));

            QString delimitor = "  -  ";
            timeInfoStr.append(delimitor);

            // calculate Estimated Time of Arrival :
            if (Settings::etaRadioButton()) {

                timeInfoStr.append(this->calculateArrivalTime(totalRemainingTimeSec));

            }

            // calculate Remaining Time :
            if (Settings::rtRadioButton()) {

                timeInfoStr.append(this->calculateRemainingTime(totalRemainingTimeSec));
            }

        }


    }

    // no pending downloads or download speed = 0 :
    else {
        timeInfoStr.append(i18n("n/a"));
    }


    // send time info signal :
    emit updateTimeInfoSignal(timeInfoStr);

}


QString InfoCollectorDispatcher::calculateArrivalTime(const quint32& remainingSeconds) {


    QDateTime dateTimeETA = QDateTime::currentDateTime();
    dateTimeETA = dateTimeETA.addSecs(remainingSeconds);

    return dateTimeETA.toString("ddd hh:mm");

}


QString InfoCollectorDispatcher::calculateRemainingTime(const quint32& remainingSeconds) {


    QString remainingTimeStr;

    int remainingDays = remainingSeconds / SECONDS_IN_DAY;
    int remainingHours =  (remainingSeconds - (remainingDays * SECONDS_IN_DAY)) / SECONDS_IN_HOUR;
    int remainingMinutes =  (remainingSeconds - ( (remainingDays * SECONDS_IN_DAY) + remainingHours *SECONDS_IN_HOUR) ) / SECONDS_IN_MINUTE;

    QString dayStr = i18n("day");
    QString hourStr = i18n("hour");
    QString minuteStr = i18n("minute");

    if (remainingDays > 1) { dayStr = i18n("days"); }
    if (remainingHours > 1) { hourStr = i18n("hours"); }
    if (remainingMinutes > 1) { minuteStr = i18n("minutes"); }

    if (remainingDays > 0) {
        remainingTimeStr.append(QString::number(remainingDays) + " " + dayStr + " ");
    }

    if (remainingDays == 0 && remainingHours > 0) {
        remainingTimeStr.append(QString::number(remainingHours) + " " + hourStr + " ");
    }

    remainingTimeStr.append(QString::number(remainingMinutes) + " " + minuteStr);


    // when this is the last minute, display "< 1 minute" instead of 0 minute :
    if (remainingDays == 0 && remainingHours == 0 && remainingMinutes == 0) {

        remainingTimeStr = i18n("less than ") +"1 " + minuteStr;

    }




    return remainingTimeStr;

}




void InfoCollectorDispatcher::retrieveFreeDiskSpace() {


    // get download path :
    QString downloadDisk = Settings::completedFolder().path();


    if (KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).isValid()) {

        // get disk size :
        quint64 sizeVal = KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).size();

        // get disk used :
        quint64 usedVal = KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).used();


        UtilityNamespace::FreeDiskSpace diskSpaceStatus = SufficientDiskSpace;

        quint64 freeSpaceVal = KDiskFreeSpaceInfo::freeSpaceInfo(downloadDisk).available();
        
        if (this->totalSize >= freeSpaceVal) {
            diskSpaceStatus = InsufficientDiskSpace;
        }


        // get free space size :
        QString freeSpaceStr = Utility::convertByteHumanReadable(freeSpaceVal) + " " + i18n("free");

        // calculate percentage of used disk :
        int usedDiskPercentage = qMin( qRound(static_cast<qreal>(usedVal * 100 / sizeVal)), PROGRESS_COMPLETE );

        emit updateFreeSpaceSignal(diskSpaceStatus, freeSpaceStr, usedDiskPercentage);

    }

    else {

        emit updateFreeSpaceSignal(UnknownDiskSpace);
    }



}


