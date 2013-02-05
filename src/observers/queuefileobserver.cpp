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


#include "queuefileobserver.h"

#include <KDebug>

#include "core.h"
#include "widgets/mytreeview.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "kwootysettings.h"


QueueFileObserver::QueueFileObserver(Core* parent) : QObject(parent) {

    this->downloadModel = parent->getDownloadModel();
    this->modelQuery = parent->getModelQuery();

    this->jobNotifyTimer = new QTimer(this);

    this->setupConnections();

    // init variables :
    this->focusedProgressValue = PROGRESS_UNKNOWN;
    this->focusedItemStatus = IdleStatus;
    this->pluginJobRunning = false;


}


void QueueFileObserver::setupConnections() {

    // parent progress item has been updated :
    connect(this->downloadModel,
            SIGNAL(parentProgressItemChangedSignal()),
            this,
            SLOT(parentItemChangedSlot()));

    // parent status item has been updated :
    connect(this->downloadModel,
            SIGNAL(parentStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
            this,
            SLOT(parentItemChangedSlot()));

    // parent status item has been updated :
    connect(this->downloadModel,
            SIGNAL(parentStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
            this,
            SLOT(jobFinishStatusSlot(QStandardItem*)));


    // ensure that jobs are finished after a time-out :
    connect(this->jobNotifyTimer, SIGNAL(timeout()), this, SLOT(checkJobFinishSlot()));

}


void QueueFileObserver::checkProgressItemValue(QStandardItem* stateItem) {

    if (stateItem) {

        // retrieve current progress value of current row :
        int currentProgressValue = this->downloadModel->getProgressValueFromIndex(this->downloadModel->indexFromItem(stateItem));

        // if progress has been updated send update signal :
        if (this->focusedProgressValue != currentProgressValue) {

            this->focusedProgressValue = currentProgressValue;
            emit progressUpdateSignal(this->focusedProgressValue);

        }

    }

}


int QueueFileObserver::getFocusedProgressValue() const {
    return this->focusedProgressValue;
}


UtilityNamespace::ItemStatus QueueFileObserver::getFocusedItemStatus() const {
    return this->focusedItemStatus;
}



void QueueFileObserver::addToList(const JobNotifyData& jobNotifyData) {

    // keep a list with a max size of 10 :
    if (this->jobNotifyDataList.size() > MAX_LIST_SIZE) {
        this->jobNotifyDataList.takeFirst();
    }

    // append the nzb file to the list :
    this->jobNotifyDataList.append(jobNotifyData);

}



JobNotifyData QueueFileObserver::retrieveJobNotifyData(QStandardItem* stateItem, UtilityNamespace::ItemStatus status) {


    QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(stateItem->index());

    // store parent identifier :
    JobNotifyData jobNotifyData;
    jobNotifyData.setParentUniqueIdentifier(fileNameItem->data(IdentifierRole).toString());
    jobNotifyData.setNzbFileName(fileNameItem->text());
    jobNotifyData.setStatus(status);
    jobNotifyData.setDateTime(QDateTime::currentDateTime());

    return jobNotifyData;
}


bool QueueFileObserver::isPluginJobRunning() {
    return this->pluginJobRunning;
}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void QueueFileObserver::parentItemChangedSlot() {


    UtilityNamespace::ItemStatus currentItemStatus = this->focusedItemStatus;

    // search current item being downloading :
    QStandardItem* stateItem = this->modelQuery->searchParentItem(DownloadStatus);

    if (stateItem) {
        currentItemStatus = DownloadStatus;
    }
    // else search current item being paused :
    else {

        stateItem = this->modelQuery->searchParentItem(PauseStatus);

        if (stateItem) {
            currentItemStatus = PauseStatus;
        }
        // else overall queue is considered as idle :
        else {

            // reset item status :
            this->focusedItemStatus = IdleStatus;

            // reset icon by setting a negative value :
            this->focusedProgressValue = PROGRESS_UNKNOWN;
        }
    }


    // item has not been found, there is no more download activities in queue, emit reset progress signal :
    if (!stateItem) {
        emit progressUpdateSignal(this->focusedProgressValue);
    }
    // item found, proceed to updates :
    else {

        // update status :
        if (this->focusedItemStatus != currentItemStatus) {

            this->focusedItemStatus = currentItemStatus;

            // if current status has changed, prior item could be another item,
            // update its current progress value :
            this->checkProgressItemValue(stateItem);

            // update status
            emit statusUpdateSignal(this->focusedItemStatus);

            // status has changed, get its current progress value :
            this->checkProgressItemValue(stateItem);

        }
        // status remains the same, progress value may need to be updated :
        else {
            this->checkProgressItemValue(stateItem);
        }

    }

}



void QueueFileObserver::jobFinishStatusSlot(QStandardItem* stateItem) {


    UtilityNamespace::ItemStatus status = this->downloadModel->getStatusFromStateItem(stateItem);
    JobNotifyData currentJobNotifyData = this->retrieveJobNotifyData(stateItem, status);

    // pending finish job has already been found in the list :
    if (this->jobNotifyDataList.contains(currentJobNotifyData)) {


        int currentIndex = this->jobNotifyDataList.indexOf(currentJobNotifyData);

        if (Utility::isJobFinish(status)) {

            // update data object in the list with new status value :
            this->jobNotifyDataList.replace(currentIndex, currentJobNotifyData);

        }
        // job is currently no more finished :
        else {
            // status is not as job finished anymore (ie, switched from downloadFinish to Verifying...) :
            this->jobNotifyDataList.removeAt(currentIndex);
        }

    }
    // the current item is not in the list :
    else {

        // add it to the list if the status corresponds to a job finished :
        if (Utility::isJobFinish(status)) {

            JobNotifyData jobNotifyData = this->retrieveJobNotifyData(stateItem, status);
            this->addToList(jobNotifyData);

            this->jobNotifyTimer->start(1000);
        }

    }

}



void QueueFileObserver::checkJobFinishSlot() {

    QList<JobNotifyData> pendingJobList;

    foreach (const JobNotifyData& jobNotifyData, this->jobNotifyDataList) {

        // if status of item did not changed after few seconds and that there is no verifing or extracting processed
        // (eg : current item could be pending for verifying while a previous nzb is currently being verified),
        // then send notification :
        if ( (jobNotifyData.getDateTime().secsTo(QDateTime::currentDateTime()) > 2) &&
             !this->modelQuery->searchParentItem(UtilityNamespace::VerifyStatus) ) {

            // notifications will handle this signal :
            emit jobFinishSignal(jobNotifyData.getStatus(), jobNotifyData.getNzbFileName());

        }

        else {
            pendingJobList.append(jobNotifyData);
        }


    }

    this->jobNotifyDataList = pendingJobList;

    if (this->jobNotifyDataList.isEmpty()) {
        this->jobNotifyTimer->stop();
    }

}


void QueueFileObserver::pluginJobRunningSlot(bool pluginJobRunning) {

    this->pluginJobRunning = pluginJobRunning;

}
