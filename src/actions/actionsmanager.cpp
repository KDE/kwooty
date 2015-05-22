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
#include "actions/actionrenamemanager.h"
#include "actions/actionfiledeletemanager.h"
#include "kwootysettings.h"

ActionsManager::ActionsManager(Core *core) : QObject(core)
{

    this->mCore = core;
    this->mTreeView = this->mCore->getTreeView();
    this->mDownloadModel = this->mCore->getDownloadModel();
    this->mModelQuery = this->mCore->getModelQuery();

    // create action button manager :
    this->mActionButtonsManager = new ActionButtonsManager(this);

    // create action merge manager :
    this->mActionMergeManager = new ActionMergeManager(this);

    // create action merge manager :
    this->mActionRenameManager = new ActionRenameManager(this);

    // create action file delete manager :
    this->mActionFileDeleteManager = new ActionFileDeleteManager(this);

    this->setupConnections();

}

ActionButtonsManager *ActionsManager::getActionButtonsManager() const
{
    return this->mActionButtonsManager;
}

ActionMergeManager *ActionsManager::getActionMergeManager() const
{
    return this->mActionMergeManager;
}

ActionRenameManager *ActionsManager::getActionRenameManager() const
{
    return this->mActionRenameManager;
}

ActionFileDeleteManager *ActionsManager::getActionFileDeleteManager() const
{
    return this->mActionFileDeleteManager;
}

Core *ActionsManager::getCore() const
{
    return this->mCore;
}

void ActionsManager::setupConnections()
{

    // enable or disable buttons according to selected items :
    connect(this->mDownloadModel,
            SIGNAL(childStatusItemChangedSignal(QStandardItem*,ItemStatusData)),
            this->mActionButtonsManager,
            SLOT(selectedItemSlot()));

    connect(this->mDownloadModel,
            SIGNAL(parentStatusItemChangedSignal(QStandardItem*,ItemStatusData)),
            this->mActionButtonsManager,
            SLOT(selectedItemSlot()));

    // update status bar info :
    connect(this,
            SIGNAL(statusBarFileSizeUpdateSignal(StatusBarUpdateType)),
            this->mCore,
            SLOT(statusBarFileSizeUpdateSlot(StatusBarUpdateType)));

    // one or several rows have been removed :
    connect(this,
            SIGNAL(statusBarFileSizeUpdateSignal(StatusBarUpdateType)),
            this->mCore->getQueueFileObserver(),
            SLOT(parentItemChangedSlot()));

    // all rows have been removed :
    connect(this,
            SIGNAL(allRowRemovedSignal()),
            this->mCore->getQueueFileObserver(),
            SLOT(parentItemChangedSlot()));

    // recalculate full nzb size when children may have been removed :
    connect(this,
            SIGNAL(recalculateNzbSizeSignal(QModelIndex)),
            this->mCore->getItemParentUpdater(),
            SLOT(recalculateNzbSizeSlot(QModelIndex)));

    // disable shutdown scheduler if user removed all rows :
    connect(this,
            SIGNAL(allRowRemovedSignal()),
            this->mCore->getShutdownManager(),
            SLOT(shutdownCancelledSlot()));

    // enable smart par2 download for incoming nzb files :
    connect(this,
            SIGNAL(changePar2FilesStatusSignal(QModelIndex,UtilityNamespace::ItemStatus)),
            this->mCore->getItemParentUpdater()->getItemChildrenManager(),
            SLOT(changePar2FilesStatusSlot(QModelIndex,UtilityNamespace::ItemStatus)));

}

void ActionsManager::changePar2FilesStatus(const QModelIndex index, UtilityNamespace::ItemStatus itemStatus)
{

    emit changePar2FilesStatusSignal(index, itemStatus);

}

void ActionsManager::moveRow(ActionsManager::MoveRowType moveRowType)
{

    // get selected indexes :
    QList<QModelIndex> indexesList = this->mTreeView->selectionModel()->selectedRows();

    // get parent item :
    QStandardItem *parentItem = 0;
    if (!indexesList.isEmpty()) {
        parentItem = this->mDownloadModel->getParentItem(indexesList.at(0));
    }

    // sort indexes by decremental order
    qSort(indexesList.begin(), indexesList.end(), qGreater<QModelIndex>());

    // remove selected indexes from model
    QMap< int, QList<QStandardItem *> > itemRowsMap;

    foreach (const QModelIndex &index, indexesList) {

        int rowNumber = index.row();
        QList<QStandardItem *> rowItems = parentItem->takeRow(rowNumber);

        itemRowsMap.insert(rowNumber, rowItems);
    }

    QList<int> rowNumberList = itemRowsMap.keys();

    // sort indexes by incremental order
    qSort(rowNumberList);
    QList<int> updatedRowNumberList;

    // then replace removed indexes to the proper position :
    foreach (const int &currentRow, rowNumberList) {

        QList<QStandardItem *> itemRows = itemRowsMap.value(currentRow);

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
        if ((updatedRowNumber < 0) ||
                (updatedRowNumber > parentItem->rowCount()) ||
                updatedRowNumberList.contains(updatedRowNumber)) {

            if ((moveRowType == MoveRowsUp) ||
                    (moveRowType == MoveRowsDown))  {

                updatedRowNumber = currentRow;
            }

            if ((moveRowType == MoveRowsTop) ||
                    (moveRowType == MoveRowsBottom)) {

                updatedRowNumber = updatedRowNumberList.at(updatedRowNumberList.size() - 1) + 1;
            }

        }

        // insert row to the model :
        parentItem->insertRow(updatedRowNumber, itemRows);

        // keep row number of inserted items :
        updatedRowNumberList.append(updatedRowNumber);

        this->mTreeView->selectionModel()->select(QItemSelection(itemRows.at(0)->index(), itemRows.at(itemRows.size() - 1)->index()),
                QItemSelectionModel::Select);
    }

    // ensure that moved rows are still visible :
    if (!rowNumberList.isEmpty()) {

        QModelIndex visibleIndex;

        if ((moveRowType == MoveRowsUp) ||
                (moveRowType == MoveRowsTop)) {

            visibleIndex = itemRowsMap.value(rowNumberList.takeFirst()).at(0)->index();
        }

        if ((moveRowType == MoveRowsDown) ||
                (moveRowType == MoveRowsBottom)) {

            visibleIndex = itemRowsMap.value(rowNumberList.takeLast()).at(0)->index();
        }

        this->mTreeView->scrollTo(visibleIndex);

    }

}

void ActionsManager::setStartPauseDownload(const UtilityNamespace::ItemStatus targetStatus, const QModelIndex &index)
{

    QList<QModelIndex> targetIndexesList;
    targetIndexesList.append(index);
    this->setStartPauseDownload(targetStatus, targetIndexesList);

}

void ActionsManager::setStartPauseDownload(const UtilityNamespace::ItemStatus targetStatus, const QList<QModelIndex> &indexesList)
{

    // notify listeners that start/pause download action is about to be triggered :
    emit startPauseAboutToBeTriggeredSignal(targetStatus, indexesList);

    foreach (const QModelIndex &currentModelIndex, indexesList) {

        // get file name item related to selected index :
        QStandardItem *fileNameItem = this->mDownloadModel->getFileNameItemFromIndex(currentModelIndex);

        // if the item is a nzbItem, retrieve their children :
        if (this->mDownloadModel->isNzbItem(fileNameItem)) {

            for (int i = 0; i < fileNameItem->rowCount(); ++i) {

                QStandardItem *nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                this->mCore->getSegmentManager()->setIdlePauseSegments(nzbChildrenItem, targetStatus);
            }
        }

        else {
            // update selected nzb children segments :
            this->mCore->getSegmentManager()->setIdlePauseSegments(fileNameItem, targetStatus);
        }

    }

    // reset default buttons :
    this->mActionButtonsManager->selectedItemSlot();

    // notify nntp clients that data is ready to be downloaded :
    if (Utility::isReadyToDownload(targetStatus)) {

        this->mCore->emitDataHasArrived();

    }

    // notify listeners that start/pause download action has been triggered :
    emit startPauseTriggeredSignal(targetStatus);

}

void ActionsManager::setStartPauseDownloadAllItems(const UtilityNamespace::ItemStatus targetStatus)
{

    this->setStartPauseDownload(targetStatus, this->mModelQuery->retrieveStartPauseIndexList(targetStatus));

}

void ActionsManager::retryDownload(const QModelIndexList &indexList)
{

    ItemParentUpdater *itemParentUpdater = this->mCore->getItemParentUpdater();

    foreach (const QModelIndex &currentModelIndex, indexList) {

        bool changeItemStatus = false;

        // by default consider that item does not need to be downloaded again :
        ItemStatus itemStatusResetTarget = ExtractFinishedStatus;

        // get file name item related to selected index :
        QStandardItem *fileNameItem = this->mDownloadModel->getFileNameItemFromIndex(currentModelIndex);

        // if current item is a nzbItem, retrieve their children :
        if (this->mDownloadModel->isNzbItem(fileNameItem)) {

            // check that at least on child will have its status reset to IdleStatus for requesting a new download.
            // It can happens (especially if user manually removes several files) that
            // all children are reverted back to DecodeFinishStatus with no child reset in queue.
            // Check that this case does not happen :
            bool childStatusConsistencyCorrect = false;
            for (int i = 0; i < fileNameItem->rowCount(); ++i) {

                QStandardItem *nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                if (this->mModelQuery->isRetryDownloadAllowed(nzbChildrenItem) == IdleStatus) {

                    childStatusConsistencyCorrect = true;
                    break;

                }

            }
            // if at leat one child item is reset to IdleStatus, then allow download retry :
            if (childStatusConsistencyCorrect) {

                for (int i = 0; i < fileNameItem->rowCount(); ++i) {

                    QStandardItem *nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                    itemStatusResetTarget = this->mModelQuery->isRetryDownloadAllowed(nzbChildrenItem);

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
            itemStatusResetTarget = this->mModelQuery->isRetryDownloadAllowed(fileNameItem);

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

            ItemStatusData itemStatusData = this->mDownloadModel->getStatusDataFromIndex(fileNameItem->index());
            itemStatusData.downloadRetry(IdleStatus);

            this->mDownloadModel->updateStatusDataFromIndex(fileNameItem->index(), itemStatusData);

        }

    }

    // update the status bar :
    emit statusBarFileSizeUpdateSignal(Incremental);

    // reset nntp clients connection :
    this->mCore->getServerManager()->resetAllServerConnection();

}

void ActionsManager::removeRow(const QList<QModelIndex> &indexesList)
{

    bool par2FilesStatuschanged = false;
    QList<int> rowList;

    //stores rows in a list
    for (int i = 0; i < indexesList.size(); ++i) {
        rowList.append(indexesList.at(i).row());
    }

    qSort(rowList.begin(), rowList.end(), qGreater<int>());

    for (int i = 0; i < indexesList.size(); ++i) {

        QModelIndex currentModelIndex = indexesList.at(i);
        if (currentModelIndex.isValid()) {

            // if the parent has been selected (a nzb item):
            if (currentModelIndex.parent() == QModelIndex()) {
                this->mDownloadModel->removeRow(rowList.at(i));
            }
            // else files of the parent (nzb item) has been selected :
            else {
                QStandardItem *nzbItem = this->mDownloadModel->itemFromIndex(currentModelIndex.parent());
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
                    this->mDownloadModel->invisibleRootItem()->removeRow(nzbItem->row());

                }

            }
        }
    }

    // then send signal to nntp clients to download Par2 files :
    if (par2FilesStatuschanged) {
        this->mCore->downloadWaitingPar2Slot();
    }

    // disable shutdown if all rows have been removed by the user :
    if (this->mDownloadModel->invisibleRootItem()->rowCount() == 0) {
        // disable shutdown scheduler :
        emit allRowRemovedSignal();
    }

    // update the status bar :
    emit statusBarFileSizeUpdateSignal(Incremental);

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ActionsManager::moveToTopSlot()
{
    this->moveRow(MoveRowsTop);
}

void ActionsManager::moveToBottomSlot()
{
    this->moveRow(MoveRowsBottom);
}

void ActionsManager::moveUpSlot()
{
    this->moveRow(MoveRowsUp);
}

void ActionsManager::moveDownSlot()
{
    this->moveRow(MoveRowsDown);
}

void ActionsManager::clearSlot()
{

    // clear rows by default :
    int answer = KMessageBox::Yes;

    // display confirm Dialog if checked in settings :
    if (Settings::confirmClear()) {

        if (this->mDownloadModel->rowCount() != 0) {
            answer = this->mCore->getCentralWidget()->displayRemoveAllFilesMessageBox();
        }
    }

    // if selected rows has not been canceled :
    if (answer == KMessageBox::Yes) {

        this->mDownloadModel->clear();
        // add the labels to the header :
        this->mTreeView->setHeaderLabels();

        //reset default buttons :
        this->mActionButtonsManager->selectedItemSlot();

        // reset the status bar :
        emit statusBarFileSizeUpdateSignal(Reset);

        // disable shutdown scheduler :
        emit allRowRemovedSignal();

    }

}

void ActionsManager::removeRowSlot()
{

    // remove selected rows by default :
    int answer = KMessageBox::Yes;

    // display confirm Dialog if checked in settings :
    if (Settings::confirmRemove()) {

        if (this->mDownloadModel->rowCount() != 0) {
            answer = this->mCore->getCentralWidget()->displayRemoveSelectedFilesMessageBox();
        }

    }

    // if selected rows has not been canceled :
    if (answer == KMessageBox::Yes) {

        this->removeRow(this->mTreeView->selectionModel()->selectedRows());

    }
}

void ActionsManager::openFolderSlot()
{

    // get selected indexes :
    QList<QModelIndex> indexesList = this->mTreeView->selectionModel()->selectedRows();
    qSort(indexesList);

    // open download folder by default :
    QString fileSavePath = Settings::completedFolder().path();

    // if a row has been selected, open folder below download folder :
    if (!indexesList.isEmpty()) {

        QModelIndex index = indexesList.at(0);

        // retrieve the file save path stored by parent item :
        QString nzbFileSavePath = this->mDownloadModel->getParentFileSavePathFromIndex(index);

        // check that file save path is really present :
        if (QDir(nzbFileSavePath).exists()) {
            fileSavePath = nzbFileSavePath;
        }

    }

    // do not manage delete as KRun uses auto deletion by default :
    new KRun(QUrl::fromLocalFile(fileSavePath), this->mTreeView);

}

void ActionsManager::pauseDownloadSlot()
{

    QList<QModelIndex> indexesList = this->mTreeView->selectionModel()->selectedRows();
    this->setStartPauseDownload(UtilityNamespace::PauseStatus, indexesList);

}

void ActionsManager::pauseAllDownloadSlot()
{
    this->setStartPauseDownloadAllItems(UtilityNamespace::PauseStatus);
}

void ActionsManager::startDownloadSlot()
{

    QList<QModelIndex> indexesList = this->mTreeView->selectionModel()->selectedRows();
    this->setStartPauseDownload(IdleStatus, indexesList);

}

void ActionsManager::startAllDownloadSlot()
{

    // ensure that any previous save file error message box is closed before starting pending downloads again :
    if (!this->mCore->getCentralWidget()->isDialogExisting()) {

        this->setStartPauseDownloadAllItems(UtilityNamespace::IdleStatus);

    }
}

void ActionsManager::retryDownloadSlot()
{
    this->retryDownload(this->mTreeView->selectionModel()->selectedRows());
}

void ActionsManager::manualExtractSlot()
{

    QList<QModelIndex> indexesList = this->mTreeView->selectionModel()->selectedRows();

    // take the first selected item :
    if (!indexesList.isEmpty()) {

        QStandardItem *nzbFileItem = this->mDownloadModel->getNzbItem(indexesList.at(0));

        if (this->mModelQuery->isManualRepairExtractAllowed(nzbFileItem)) {

            this->mCore->getItemParentUpdater()->triggerPostProcessManually(nzbFileItem);

        }

    }

}
