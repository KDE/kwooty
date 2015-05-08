/***************************************************************************
 *   Copyright (C) 2013 by Xavier Lefage                                   *
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

#include "actionrenamemanager.h"

#include <KInputDialog>
#include <KLocalizedString>
#include <KIO/CopyJob>

#include "standarditemmodelquery.h"

ActionRenameManager::ActionRenameManager(ActionsManager *actionsManager) : ActionFileManagerBase(actionsManager)
{

}

void ActionRenameManager::checkRenameCandidates(bool &renameAvailable)
{

    renameAvailable = false;
    QStandardItem *selectedFileNameItem = 0;

    // get selected rows :
    QList<QModelIndex> selectedIndexList = this->treeView->selectionModel()->selectedRows();

    if (selectedIndexList.size() == 1) {
        selectedFileNameItem = this->downloadModel->getFileNameItemFromIndex(selectedIndexList.at(0));
    }

    // rename is allowed for only one selected row :
    if (selectedFileNameItem &&
            this->actionFileStep == ActionFileIdle) {

        // first, be sure that selected item is a parent one (nzb) :
        if (this->downloadModel->isNzbItem(selectedFileNameItem) &&
                this->isRenameAllowed(selectedFileNameItem)) {

            renameAvailable = true;
        }

    }

}

bool ActionRenameManager::isRenameAllowed(QStandardItem *fileNameItem) const
{

    // get current nzb item status :
    ItemStatusData itemStatusData = this->downloadModel->getStatusDataFromIndex(fileNameItem->index());

    // rename is allowed for nzb item in download or post-process finish states :
    return (Utility::isInDownloadProcess(itemStatusData.getStatus()) ||
            itemStatusData.isPostProcessFinish());

}

bool ActionRenameManager::validateNewFolderName(QStandardItem *selectedFileNameItem) const
{

    bool validate = true;

    // get the root item :
    QStandardItem *rootItem = this->downloadModel->invisibleRootItem();

    // check state of each parent item :
    for (int i = 0; i < rootItem->rowCount(); ++i) {

        // get corresponding nzb file name item :
        QStandardItem *fileNameItem = rootItem->child(i, FILE_NAME_COLUMN);
        NzbFileData parentNzbFileData = downloadModel->getNzbFileDataFromIndex(fileNameItem->index());

        if (selectedFileNameItem != fileNameItem &&
                parentNzbFileData.getNzbName() == this->mInput) {

            validate = false;
            break;
        }

    }

    return validate;

}

void ActionRenameManager::launchProcess()
{

    QStandardItem *selectedFileNameItem = this->core->getModelQuery()->retrieveParentFileNameItemFromUuid(this->mSelectedItemUuid);

    if (selectedFileNameItem &&
            this->isRenameAllowed(selectedFileNameItem)) {

        // process to item renaming :
        this->processRename(selectedFileNameItem);
    }

    else {
        this->displayMessage(i18n("Rename can not be performed"));
    }

}

void ActionRenameManager::processRename(QStandardItem *selectedFileNameItem)
{

    this->actionFileStep = ActionFileProcessing;

    // retrieve the file save path stored by parent item :
    NzbFileData selectedNzbFileDataOld = this->downloadModel->getNzbFileDataFromIndex(this->downloadModel->getNzbItem(selectedFileNameItem)->index());

    // update new nzb name :
    NzbFileData selectedNzbFileData = selectedNzbFileDataOld;
    selectedNzbFileData.setNzbName(this->mInput);

    // update files with same parent waiting to be decoded with new nzb name folder :
    this->segmentBuffer->updateDecodeWaitingQueue(selectedNzbFileDataOld, selectedNzbFileData);

    for (int i = 0; i < selectedFileNameItem->rowCount(); ++i) {

        // retrieve first nzb child item :
        QStandardItem *childFileNameItem = selectedFileNameItem->child(i, FILE_NAME_COLUMN);

        // update file save path from selected item to file save path from target item :
        NzbFileData childNzbFileData = this->downloadModel->getNzbFileDataFromIndex(childFileNameItem->index());

        // then update its file save path :
        childNzbFileData.setNzbName(this->mInput);

        this->downloadModel->updateNzbFileDataToItem(childFileNameItem, childNzbFileData);

    }

    // also update parent nzb item :
    this->downloadModel->updateParentFileSavePathFromIndex(selectedFileNameItem->index(), selectedNzbFileData);

    // then rename nzb folder with new folder name :
    KIO::CopyJob *moveJob = KIO::move(KUrl(selectedNzbFileDataOld.getFileSavePath()), KUrl(selectedNzbFileData.getFileSavePath()));

    // make job non-interactive :
    moveJob->setUiDelegate(0);

    // setup connections with job :
    connect(moveJob, SIGNAL(result(KJob*)), this, SLOT(handleResultSlot(KJob*)));

    moveJob->start();

    // update displayed folder name :
    selectedFileNameItem->setText(this->mInput);

    // finaly, launch retry process :
    //this->actionsManager->retryDownload(QList<QModelIndex>() << selectedFileNameItem->index());

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ActionRenameManager::actionTriggeredSlot()
{

    // retrieve selected item and item corresponding to the action :
    QStandardItem *selectedFileNameItem = 0;
    QList<QModelIndex> selectedIndexList = this->treeView->selectionModel()->selectedRows();

    if (!selectedIndexList.isEmpty()) {

        selectedFileNameItem = this->downloadModel->getFileNameItemFromIndex(selectedIndexList.at(0));

        // be sure that the item selected is a nzb :
        if (!this->downloadModel->isNzbItem(selectedFileNameItem)) {
            selectedFileNameItem = 0;
        }

    }

    // if selected and target items have been found, merge is possible :
    if (this->actionFileStep == ActionFileIdle &&
            selectedFileNameItem &&
            selectedFileNameItem->rowCount() > 0) {

        this->mInput = KInputDialog::getText(i18n("Rename Folder"), xi18nc("@label:textbox",
                                            "Rename the folder %1 to:",
                                            selectedFileNameItem->text()),
                                            selectedFileNameItem->text());

        if (!this->mInput.isEmpty() &&
                this->downloadModel->getNzbFileDataFromIndex(selectedFileNameItem->index()).getNzbName() != this->mInput) {

            if (this->validateNewFolderName(selectedFileNameItem)) {

                this->mSelectedItemUuid = this->downloadModel->getUuidStrFromIndex(selectedFileNameItem->index());

                this->actionFileStep = ActionFileRequested;
                this->processFileSlot();

            } else {
                this->displayMessage(i18n("Name already exists"));

            }

        }

    }

    else {
        this->displayMessage(i18n("Rename can not be performed"));
    }

}

void ActionRenameManager::handleResultSlot(KJob *moveJob)
{

    // if job is succeed :
    if (moveJob->error() == 0) {
        qCDebug(KWOOTY_LOG) << "job renaming ok";
    }

    // job is finished :
    this->actionFileStep = ActionFileIdle;

}

