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

#include "actionmergemanager.h"

#include <KActionCollection>
#include <KMessageBox>
#include <KIO/CopyJob>
#include <KLocalizedString>
#include <QDir>

#include "standarditemmodelquery.h"
#include "itemparentupdater.h"

ActionMergeManager::ActionMergeManager(ActionsManager *actionsManager) : ActionFileManagerBase(actionsManager)
{

    this->setupConnections();

}

void ActionMergeManager::setupConnections()
{

    // recalculate full nzb size when merge has been done :
    connect(this,
            SIGNAL(recalculateNzbSizeSignal(QModelIndex)),
            this->mCore->getItemParentUpdater(),
            SLOT(recalculateNzbSizeSlot(QModelIndex)));

}

QList<QStandardItem *> ActionMergeManager::checkMergeCandidates(bool &mergeAvailable)
{

    mergeAvailable = false;
    QStandardItem *selectedFileNameItem = 0;
    QList<QStandardItem *> fileNameItemList;

    // get selected rows :
    QList<QModelIndex> selectedIndexList = this->mTreeView->selectionModel()->selectedRows();

    if (selectedIndexList.size() == 1) {
        selectedFileNameItem = this->mDownloadModel->getFileNameItemFromIndex(selectedIndexList.at(0));
    }

    // merge is allowed for only one selected row :
    if (selectedFileNameItem &&
            this->mActionFileStep == ActionFileIdle) {

        // first, be sure that selected item is a parent one (nzb) :
        if (this->mDownloadModel->isNzbItem(selectedFileNameItem) &&
                this->isMergeAllowed(selectedFileNameItem)) {

            // get the root item :
            QStandardItem *rootItem = this->mDownloadModel->invisibleRootItem();

            // check state of each parent item :
            for (int i = 0; i < rootItem->rowCount(); ++i) {

                // get corresponding nzb file name item :
                QStandardItem *fileNameItem = rootItem->child(i, FILE_NAME_COLUMN);

                // merge is allowed for nzb item in download or post-process failed states :
                if (selectedFileNameItem != fileNameItem &&
                        this->isMergeAllowed(fileNameItem)) {

                    // append items for which merge is possible :
                    fileNameItemList.append(fileNameItem);
                }

            }

            if (!fileNameItemList.isEmpty()) {
                mergeAvailable = true;
            }

        }

    } // !selectedIndexList.isEmpty()

    return fileNameItemList;

}

bool ActionMergeManager::isMergeAllowed(QStandardItem *fileNameItem) const
{

    // get current nzb item status :
    QStandardItem *stateItem = this->mDownloadModel->getStateItemFromIndex(fileNameItem->index());
    ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();

    // merge is allowed for nzb item in download or post-process failed states :
    return (Utility::isInDownloadProcess(itemStatusData.getStatus()) ||
            (itemStatusData.isPostProcessFinish() &&
             !itemStatusData.areAllPostProcessingCorrect()));

}

void ActionMergeManager::launchProcess()
{

    QStandardItem *selectedFileNameItem = this->mCore->getModelQuery()->retrieveParentFileNameItemFromUuid(this->mSelectedItemUuid);
    QStandardItem *targetFileNameItem = this->mCore->getModelQuery()->retrieveParentFileNameItemFromUuid(this->mTargetItemUuid);

    if (selectedFileNameItem &&
            targetFileNameItem) {

        // process to item merging :
        this->processMerge(selectedFileNameItem, targetFileNameItem);
    }

    else {
        this->displayMessage(i18n("Merge can not be performed anymore"));
    }

}

void ActionMergeManager::processMerge(QStandardItem *selectedFileNameItem, QStandardItem *targetFileNameItem)
{

    this->mActionFileStep = ActionFileProcessing;

    // get download folder from selected and target items :
    NzbFileData selectedNzbFileData = this->mDownloadModel->getNzbFileDataFromIndex(selectedFileNameItem->child(0)->index());
    NzbFileData targetNzbFileData = this->mDownloadModel->getNzbFileDataFromIndex(targetFileNameItem->child(0)->index());

    QString selectedFileSavePath = selectedNzbFileData.getFileSavePath();
    QString targetFileSavePath = targetNzbFileData.getFileSavePath();

    // update files with same parent waiting to be decoded with the target path :
    this->mSegmentBuffer->updateDecodeWaitingQueue(selectedNzbFileData, targetNzbFileData);

    KUrl::List sourceFileList;

    // move all child item of selected parent item to target item :
    while (selectedFileNameItem->rowCount() > 0) {

        // retrieve first nzb child item :
        QStandardItem *childFileNameItem = selectedFileNameItem->child(0, FILE_NAME_COLUMN);

        // update file save path from selected item to file save path from target item :
        NzbFileData childNzbFileData = this->mDownloadModel->getNzbFileDataFromIndex(childFileNameItem->index());

        // then update its file save path :
        childNzbFileData.updateFileSavePath(targetNzbFileData);
        this->mDownloadModel->updateNzbFileDataToItem(childFileNameItem, childNzbFileData);

        // then move the whole row :
        targetFileNameItem->appendRow(selectedFileNameItem->takeRow(0));

        // list downloaded files from selected item in order to move them in the target download folder :
        QFileInfo currentFileInfo(Utility::buildFullPath(selectedFileSavePath, childNzbFileData.getDecodedFileName()));

        if (currentFileInfo.isFile() &&
                currentFileInfo.exists()) {

            sourceFileList.append(currentFileInfo.filePath());

        }

    }

    // append eventual nzb files to files to be moved :
    if (selectedFileSavePath != targetFileSavePath) {

        foreach (const QString &nzbFile, QDir(selectedFileSavePath).entryList(QStringList() << "*.nzb", QDir::Files)) {
            sourceFileList.append(Utility::buildFullPath(selectedFileSavePath, nzbFile));
        }
    }

    // make sure that targetFolder already exists :
    Utility::createFolder(targetFileSavePath);

    // then move already decoded files into target download folder :
    KIO::CopyJob *moveJob = KIO::move(sourceFileList, KUrl(targetFileSavePath), KIO::Overwrite);

    // make job non-interactive :
    moveJob->setUiDelegate(0);

    // setup connections with job :
    connect(moveJob, SIGNAL(result(KJob*)), this, SLOT(handleResultSlot(KJob*)));

    moveJob->start();

    // update target nzb name :
    targetFileNameItem->setText(targetFileNameItem->text() + " + " + selectedFileNameItem->text());

    // remove the now empty source nzb item :
    this->mDownloadModel->invisibleRootItem()->removeRow(selectedFileNameItem->row());

    // update nzb size column :
    emit recalculateNzbSizeSignal(targetFileNameItem->index());

    // finaly, launch retry process :
    this->mActionsManager->retryDownload(QList<QModelIndex>() << targetFileNameItem->index());

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ActionMergeManager::mergeSubMenuAboutToShowSlot()
{

    QAction *mergeNzbAction = this->mCore->getMainWindow()->getActionFromName("mergeNzb");

    // retrieve action submenu and clear it :
    QMenu *mergeSubMenu = mergeNzbAction->menu();
    mergeSubMenu->clear();

    bool mergeAvailable = false;

    // get list of candidates nzb that could be merged together :
    QList<QStandardItem *> fileNameItemList = this->checkMergeCandidates(mergeAvailable);

    // build new appropriate menu :
    if (mergeAvailable) {

        foreach (const QStandardItem *fileNameItem, fileNameItemList) {

            QAction *currentAction = mergeSubMenu->addAction(fileNameItem->text());

            // set item uuid for identifying when user would select an action :
            currentAction->setData(this->mDownloadModel->getUuidStrFromIndex(fileNameItem->index()));

        }

    }

}

void ActionMergeManager::actionTriggeredSlot() { }

void ActionMergeManager::actionTriggeredSlot(QAction *subMenuAction)
{

    bool mergeAvailable = false;

    // check that merge is still possible at this time :
    this->checkMergeCandidates(mergeAvailable);

    if (mergeAvailable) {

        // retrieve selected item and item corresponding to the action :
        QStandardItem *selectedFileNameItem = 0;
        QStandardItem *targetFileNameItem = 0;

        QList<QModelIndex> selectedIndexList = this->mTreeView->selectionModel()->selectedRows();

        if (!selectedIndexList.isEmpty()) {

            selectedFileNameItem = this->mDownloadModel->getFileNameItemFromIndex(selectedIndexList.at(0));

            // be sure that the item selected is a nzb :
            if (!this->mDownloadModel->isNzbItem(selectedFileNameItem)) {
                selectedFileNameItem = 0;
            }

        }

        // load stored uuid from action :
        QString targetUuid = subMenuAction->data().toString();
        targetFileNameItem = this->mCore->getModelQuery()->retrieveParentFileNameItemFromUuid(targetUuid);

        // if selected and target items have been found, merge is possible :
        if (this->mActionFileStep == ActionFileIdle &&
                selectedFileNameItem &&
                targetFileNameItem &&
                selectedFileNameItem->rowCount() > 0 &&
                targetFileNameItem->rowCount() > 0) {

            // display merge confirmation dialog :
            int answer = this->mCore->getCentralWidget()->displayMergeItemsMessageBox(selectedFileNameItem->text(), targetFileNameItem->text());

            if (answer == KMessageBox::Yes) {

                this->mSelectedItemUuid = this->mDownloadModel->getUuidStrFromIndex(selectedFileNameItem->index());
                this->mTargetItemUuid = this->mDownloadModel->getUuidStrFromIndex(targetFileNameItem->index());

                // process to item merging :
                this->mActionFileStep = ActionFileRequested;
                this->processFileSlot();

            }

        } else {
            mergeAvailable = false;
        }

    }

    // merge is no more available, display a message to user :
    if (!mergeAvailable) {
        this->displayMessage(i18n("Merge can not be performed anymore"));
    }

}

void ActionMergeManager::handleResultSlot(KJob *moveJob)
{

    // if job is succeed :
    if (moveJob->error() == 0) {

        // remove source folder (will be removed only if empty)
        KUrl::List sourceFileList = static_cast<KIO::CopyJob *>(moveJob)->srcUrls();

        if (!sourceFileList.isEmpty()) {

            // get source directory of the first moved file :
            QString sourceDirectory = sourceFileList.at(0).directory();

            QDir().rmdir(sourceDirectory);

        }

    }

    // job is finished :
    this->mActionFileStep = ActionFileIdle;

}

