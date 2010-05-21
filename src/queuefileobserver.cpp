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

#include "centralwidget.h"
#include "mytreeview.h"
#include "standarditemmodel.h"


QueueFileObserver::QueueFileObserver(CentralWidget* parent) : QObject(parent) {

    this->downloadModel = parent->getDownloadModel();
    this->treeView = parent->getTreeView();

    this->setupConnections();

    // init variables :
    this->focusedProgressValue = PROGRESS_UNKNOWN;
    this->focusedItemStatus = IdleStatus;

}


void QueueFileObserver::setupConnections() {

    // parent progress item has been updated :
    connect(this->downloadModel,
            SIGNAL(parentProgressItemChangedSignal()),
            this,
            SLOT(parentItemChangedSlot()));

    // parent status item has been updated :
    connect(this->downloadModel,
            SIGNAL(parentStatusItemChangedSignal()),
            this,
            SLOT(parentItemChangedSlot()));


    // all rows have been removed :
    connect(this->treeView,
            SIGNAL(allRowRemovedSignal()),
            this,
            SLOT(parentItemChangedSlot()));

    // one or several rows have been removed :
    connect(this->treeView,
            SIGNAL(statusBarFileSizeUpdateSignal(StatusBarUpdateType)),
            this,
            SLOT(parentItemChangedSlot()));

}




QStandardItem* QueueFileObserver::searchParentItem(const UtilityNamespace::ItemStatus itemStatus) {

    QStandardItem* stateItem = NULL;

    // get the root model :
    QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

    // get the first parent with download active :
    for (int i = 0; i < rootItem->rowCount(); i++) {

        QStandardItem* parentStateItem = rootItem->child(i, STATE_COLUMN);
        UtilityNamespace::ItemStatus currentStatus = this->downloadModel->getStatusFromStateItem(parentStateItem);


        if (itemStatus == DownloadStatus) {
            // check if parent status is either downloading or pausing :
            if (Utility::isDownloadOrPausing(currentStatus)) {

                stateItem = parentStateItem;

                //kDebug() << "DOWNLOAD ITEM FOUND";
                break;
            }
        }


        if (itemStatus == PauseStatus) {
            // check if parent status is either downloading or pausing :
            if (Utility::isPaused(currentStatus)) {
                stateItem = parentStateItem;

                //kDebug() << "PAUSE ITEM FOUND";
                break;
            }
        }


    }


    return stateItem;


}



void QueueFileObserver::parentItemChangedSlot() {


    UtilityNamespace::ItemStatus currentItemStatus = this->focusedItemStatus;

    // search current item being downloading :
    QStandardItem* stateItem = this->searchParentItem(DownloadStatus);

    if (stateItem) {
        currentItemStatus = DownloadStatus;
    }
    // else search current item being paused :
    else {

        stateItem = this->searchParentItem(PauseStatus);

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

