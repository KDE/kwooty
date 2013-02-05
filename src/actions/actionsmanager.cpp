/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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


#include "actionsmanager.h"

#include <KMessageBox>
#include <KRun>

#include <QDir>

#include "core.h"
#include "actionbuttonsmanager.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "itemparentupdater.h"
#include "itemchildrenmanager.h"
#include "segmentmanager.h"
#include "servermanager.h"
#include "shutdown/shutdownmanager.h"
#include "widgets/centralwidget.h"
#include "widgets/mytreeview.h"
#include "observers/queuefileobserver.h"
#include "actions/actionmergemanager.h"
#include "kwootysettings.h"


ActionsManager::ActionsManager(Core* core) : QObject (core) {

    this->core = core;
    this->treeView = this->core->getTreeView();
    this->downloadModel = this->core->getDownloadModel();
    this->modelQuery = this->core->getModelQuery();

    // create action button manager :
    this->actionButtonsManager = new ActionButtonsManager(this);

    // create action merge manager :
    this->actionMergeManager = new ActionMergeManager(this);

    this->setupConnections();

}


ActionButtonsManager* ActionsManager::getActionButtonsManager() const {
    return this->actionButtonsManager;
}

ActionMergeManager* ActionsManager::getActionMergeManager() const {
    return this->actionMergeManager;
}

Core* ActionsManager::getCore() const {
    return this->core;
}


void ActionsManager::setupConnections() {


    // enable or disable buttons according to selected items :
    connect (this->downloadModel,
             SIGNAL(childStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
             this->actionButtonsManager,
             SLOT(selectedItemSlot()));

    connect (this->downloadModel,
             SIGNAL(parentStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
             this->actionButtonsManager,
             SLOT(selectedItemSlot()));

    // update status bar info :
    connect( this,
             SIGNAL(statusBarFileSizeUpdateSignal(StatusBarUpdateType)),
             this->core,
             SLOT(statusBarFileSizeUpdateSlot(StatusBarUpdateType)) );

    // one or several rows have been removed :
    connect(this,
            SIGNAL(statusBarFileSizeUpdateSignal(StatusBarUpdateType)),
            this->core->getQueueFileObserver(),
            SLOT(parentItemChangedSlot()));

    // all rows have been removed :
    connect(this,
            SIGNAL(allRowRemovedSignal()),
            this->core->getQueueFileObserver(),
            SLOT(parentItemChangedSlot()));

    // recalculate full nzb size when children may have been removed :
    connect (this,
             SIGNAL(recalculateNzbSizeSignal(const QModelIndex)),
             this->core->getItemParentUpdater(),
             SLOT(recalculateNzbSizeSlot(const QModelIndex)));


    // disable shutdown scheduler if user removed all rows :
    connect (this,
             SIGNAL(allRowRemovedSignal()),
             this->core->getShutdownManager(),
             SLOT(shutdownCancelledSlot()));


    // enable smart par2 download for incoming nzb files :
    connect (this,
             SIGNAL(changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus)),
             this->core->getItemParentUpdater()->getItemChildrenManager(),
             SLOT(changePar2FilesStatusSlot(const QModelIndex, UtilityNamespace::ItemStatus)));

}

void ActionsManager::changePar2FilesStatus(const QModelIndex index, UtilityNamespace::ItemStatus itemStatus) {

    emit changePar2FilesStatusSignal(index, itemStatus);

}


void ActionsManager::moveRow(ActionsManager::MoveRowType moveRowType) {

    // get selected indexes :
    QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();

    // get parent item :
    QStandardItem* parentItem = 0;
    if (!indexesList.isEmpty()) {
        parentItem = this->downloadModel->getParentItem(indexesList.at(0));
    }

    // sort indexes by decremental order
    qSort(indexesList.begin(), indexesList.end(), qGreater<QModelIndex>());


    // remove selected indexes from model
    QMap< int, QList<QStandardItem*> > itemRowsMap;

    foreach (const QModelIndex& index, indexesList) {

        int rowNumber = index.row();
        QList<QStandardItem*> rowItems = parentItem->takeRow(rowNumber);

        itemRowsMap.insert(rowNumber, rowItems);
    }


    QList<int> rowNumberList = itemRowsMap.keys();

    // sort indexes by incremental order
    qSort(rowNumberList);
    QList<int> updatedRowNumberList;

    // then replace removed indexes to the proper position :
    foreach (const int& currentRow, rowNumberList) {

        QList<QStandardItem*> itemRows = itemRowsMap.value(currentRow);

        int updatedRowNumber;

        if (moveRowType == MoveRowsUp) {
            updatedRowNumber = currentRow - 1;
        }

        if (moveRowType == MoveRowsDown) {
            updatedRowNumber = currentRow + 1;
        }

        if (moveRowType == MoveRowsTop) {
            updatedRowNumber = 0;
        }

        if (moveRowType == MoveRowsBottom) {
            updatedRowNumber = parentItem->rowCount();
        }

        // control out of range and multiple row selection :
        if ( (updatedRowNumber < 0) ||
             (updatedRowNumber > parentItem->rowCount()) ||
             updatedRowNumberList.contains(updatedRowNumber) ) {


            if ( (moveRowType == MoveRowsUp) ||
                 (moveRowType == MoveRowsDown) )  {

                updatedRowNumber = currentRow;
            }

            if ( (moveRowType == MoveRowsTop) ||
                 (moveRowType == MoveRowsBottom) ) {

                updatedRowNumber = updatedRowNumberList.at(updatedRowNumberList.size() - 1) + 1;
            }


        }


        // insert row to the model :
        parentItem->insertRow(updatedRowNumber, itemRows);

        // keep row number of inserted items :
        updatedRowNumberList.append(updatedRowNumber);

        this->treeView->selectionModel()->select(QItemSelection(itemRows.at(0)->index(), itemRows.at(itemRows.size() - 1)->index()),
                                                 QItemSelectionModel::Select);
    }



    // ensure that moved rows are still visible :
    if (!rowNumberList.isEmpty()) {

        QModelIndex visibleIndex;

        if ( (moveRowType == MoveRowsUp) ||
             (moveRowType == MoveRowsTop) ) {

            visibleIndex = itemRowsMap.value(rowNumberList.takeFirst()).at(0)->index();
        }

        if ( (moveRowType == MoveRowsDown) ||
             (moveRowType == MoveRowsBottom) ){

            visibleIndex = itemRowsMap.value(rowNumberList.takeLast()).at(0)->index();
        }


        this->treeView->scrollTo(visibleIndex);

    }

}


void ActionsManager::setStartPauseDownload(const UtilityNamespace::ItemStatus targetStatus, const QModelIndex& index){

    QList<QModelIndex> targetIndexesList;
    targetIndexesList.append(index);
    this->setStartPauseDownload(targetStatus, targetIndexesList);

}



void ActionsManager::setStartPauseDownload(const UtilityNamespace::ItemStatus targetStatus, const QList<QModelIndex>& indexesList){

    // notify listeners that start/pause download action is about to be triggered :
    emit startPauseAboutToBeTriggeredSignal(targetStatus, indexesList);

    foreach (const QModelIndex& currentModelIndex, indexesList) {

        // get file name item related to selected index :
        QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(currentModelIndex);

        // if the item is a nzbItem, retrieve their children :
        if (this->downloadModel->isNzbItem(fileNameItem)){

            for (int i = 0; i < fileNameItem->rowCount(); i++){

                QStandardItem* nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                this->core->getSegmentManager()->setIdlePauseSegments(nzbChildrenItem, targetStatus);
            }
        }

        else {
            // update selected nzb children segments :
            this->core->getSegmentManager()->setIdlePauseSegments(fileNameItem, targetStatus);
        }

    }

    // reset default buttons :
    this->actionButtonsManager->selectedItemSlot();

    // notify nntp clients that data is ready to be downloaded :
    if (Utility::isReadyToDownload(targetStatus)) {

        this->core->emitDataHasArrived();

    }

    // notify listeners that start/pause download action has been triggered :
    emit startPauseTriggeredSignal(targetStatus);

}



void ActionsManager::setStartPauseDownloadAllItems(const UtilityNamespace::ItemStatus targetStatus) {

    this->setStartPauseDownload(targetStatus, this->modelQuery->retrieveStartPauseIndexList(targetStatus));

}


void ActionsManager::retryDownload(const QModelIndexList& indexList) {

    ItemParentUpdater* itemParentUpdater = this->core->getItemParentUpdater();

    foreach (const QModelIndex& currentModelIndex, indexList) {

        bool changeItemStatus = false;

        // by default consider that item does not need to be downloaded again :
        ItemStatus itemStatusResetTarget = ExtractFinishedStatus;

        // get file name item related to selected index :
        QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(currentModelIndex);

        // if current item is a nzbItem, retrieve their children :
        if (this->downloadModel->isNzbItem(fileNameItem)) {

            // check that at least on child will have its status reset to IdleStatus for requesting a new download.
            // It can happens (especially if user manually removes several files) that
            // all children are reverted back to DecodeFinishStatus with no child reset in queue.
            // Check that this case does not happen :
            bool childStatusConsistencyCorrect = false;
            for (int i = 0; i < fileNameItem->rowCount(); i++) {

                QStandardItem* nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                if (this->modelQuery->isRetryDownloadAllowed(nzbChildrenItem) == IdleStatus) {

                    childStatusConsistencyCorrect = true;
                    break;

                }

            }
            // if at leat one child item is reset to IdleStatus, then allow download retry :
            if (childStatusConsistencyCorrect) {

                for (int i = 0; i < fileNameItem->rowCount(); i++) {

                    QStandardItem* nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                    itemStatusResetTarget = this->modelQuery->isRetryDownloadAllowed(nzbChildrenItem);

                    // if itemStatusResetTarget is different from ExtractFinishedStatus, retry download is allowed :
                    if (itemStatusResetTarget != ExtractFinishedStatus) {

                        itemParentUpdater->getItemChildrenManager()->resetItemStatusToTarget(nzbChildrenItem, itemStatusResetTarget);
                        changeItemStatus = true;

                    }

                }

            }
        }
        // else current item is a child :
        else {
            // update selected nzb children segments :
            itemStatusResetTarget = this->modelQuery->isRetryDownloadAllowed(fileNameItem);

            // reset current child item :
            if (itemStatusResetTarget != ExtractFinishedStatus) {

                itemParentUpdater->getItemChildrenManager()->resetItemStatusToTarget(fileNameItem, itemStatusResetTarget);

                fileNameItem = fileNameItem->parent();
                itemParentUpdater->getItemChildrenManager()->resetFinishedChildrenItemToDecodeFinish(fileNameItem);

                changeItemStatus = true;
            }

        }

        // finish to update *parent* status :
        if (changeItemStatus) {

            ItemStatusData itemStatusData = this->downloadModel->getStatusDataFromIndex(fileNameItem->index());
            itemStatusData.downloadRetry(IdleStatus);

            this->downloadModel->updateStatusDataFromIndex(fileNameItem->index(), itemStatusData);

        }

    }

    // update the status bar :
    emit statusBarFileSizeUpdateSignal(Incremental);

    // reset nntp clients connection :
    this->core->getServerManager()->resetAllServerConnection();

}




//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ActionsManager::moveToTopSlot() {
    this->moveRow(MoveRowsTop);
}

void ActionsManager::moveToBottomSlot() {
    this->moveRow(MoveRowsBottom);
}

void ActionsManager::moveUpSlot() {
    this->moveRow(MoveRowsUp);
}

void ActionsManager::moveDownSlot() {
    this->moveRow(MoveRowsDown);
}



void ActionsManager::clearSlot() {

    // clear rows by default :
    int answer = KMessageBox::Yes;

    // display confirm Dialog if checked in settings :
    if (Settings::confirmClear()) {

        if (this->downloadModel->rowCount() != 0) {
            answer = this->core->getCentralWidget()->displayRemoveAllFilesMessageBox();
        }
    }

    // if selected rows has not been canceled :
    if (answer == KMessageBox::Yes) {

        this->downloadModel->clear();
        // add the labels to the header :
        this->treeView->setHeaderLabels();

        //reset default buttons :
        this->actionButtonsManager->selectedItemSlot();

        // reset the status bar :
        emit statusBarFileSizeUpdateSignal(Reset);

        // disable shutdown scheduler :
        emit allRowRemovedSignal();

    }


}


void ActionsManager::removeRowSlot() {

    // remove selected rows by default :
    int answer = KMessageBox::Yes;

    // display confirm Dialog if checked in settings :
    if (Settings::confirmRemove()) {

        if (this->downloadModel->rowCount() != 0) {
            answer = this->core->getCentralWidget()->displayRemoveSelectedFilesMessageBox();
        }

    }

    // if selected rows has not been canceled :
    if (answer == KMessageBox::Yes) {

        bool par2FilesStatuschanged = false;
        QList<int> rowList;
        QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();

        //stores rows in a list
        for (int i = 0; i < indexesList.size(); i++) {
            rowList.append(indexesList.at(i).row());
        }

        qSort(rowList.begin(), rowList.end(), qGreater<int>());


        for (int i = 0; i < indexesList.size(); i++) {

            QModelIndex currentModelIndex = indexesList.at(i);
            if (currentModelIndex.isValid()) {

                // if the parent has been selected (a nzb item):
                if (currentModelIndex.parent() == QModelIndex()) {
                    this->downloadModel->removeRow(rowList.at(i));
                }
                // else files of the parent (nzb item) has been selected :
                else {
                    QStandardItem* nzbItem = this->downloadModel->itemFromIndex(currentModelIndex.parent());
                    nzbItem->removeRow(rowList.at(i));

                    if (nzbItem->rowCount() > 0) {

                        // set nzb parent row up to date :
                        emit recalculateNzbSizeSignal(nzbItem->index());

                        // item has been removed extract could fail, download Par2 files :
                        this->changePar2FilesStatus(nzbItem->index(), IdleStatus);

                        par2FilesStatuschanged = true;

                    }
                    // if the nzb item has no more child, remove it :
                    else {
                        this->downloadModel->invisibleRootItem()->removeRow(nzbItem->row());

                    }

                }
            }
        }


        // then send signal to nntp clients to download Par2 files :
        if (par2FilesStatuschanged) {
            this->core->downloadWaitingPar2Slot();
        }

    }


    // disable shutdown if all rows have been removed by the user :
    if (this->downloadModel->invisibleRootItem()->rowCount() == 0) {
        // disable shutdown scheduler :
        emit allRowRemovedSignal();
    }

    // update the status bar :
    emit statusBarFileSizeUpdateSignal(Incremental);


}


void ActionsManager::openFolderSlot() {

    // get selected indexes :
    QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();
    qSort(indexesList);

    // open download folder by default :
    QString fileSavePath = Settings::completedFolder().path();

    // if a row has been selected, open folder below download folder :
    if (!indexesList.isEmpty()) {

        QModelIndex index = indexesList.at(0);

        // retrieve the file save path stored by parent item :
        QString nzbFileSavePath = this->downloadModel->getParentFileSavePathFromIndex(index);

        // check that file save path is really present :
        if (QDir(nzbFileSavePath).exists()) {
            fileSavePath = this->downloadModel->getParentFileSavePathFromIndex(index);
        }


    }

    // do not manage delete as KRun uses auto deletion by default :
    new KRun(KUrl(fileSavePath), this->treeView);

}


void ActionsManager::pauseDownloadSlot() {

    QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();
    this->setStartPauseDownload(UtilityNamespace::PauseStatus, indexesList);

}


void ActionsManager::pauseAllDownloadSlot() {
    this->setStartPauseDownloadAllItems(UtilityNamespace::PauseStatus);
}



void ActionsManager::startDownloadSlot() {

    QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();
    this->setStartPauseDownload(IdleStatus, indexesList);

}


void ActionsManager::startAllDownloadSlot() {

    // ensure that any previous save file error message box is closed before starting pending downloads again :
    if (!this->core->getCentralWidget()->isDialogExisting()) {

        this->setStartPauseDownloadAllItems(UtilityNamespace::IdleStatus);

    }
}



void ActionsManager::retryDownloadSlot() {
    this->retryDownload(this->treeView->selectionModel()->selectedRows());
}


void ActionsManager::manualExtractSlot() {

    QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();

    // take the first selected item :
    if (!indexesList.isEmpty()) {

        QStandardItem* nzbFileItem = this->downloadModel->getNzbItem(indexesList.at(0));

        if (this->modelQuery->isManualRepairExtractAllowed(nzbFileItem)) {

            this->core->getItemParentUpdater()->triggerPostProcessManually(nzbFileItem);

        }

    }

}
