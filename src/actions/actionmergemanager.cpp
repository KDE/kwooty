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

#include <KMessageBox>
#include <KDebug>
#include <KActionCollection>
#include <KAction>

#include <QDir>

#include "core.h"
#include "mainwindow.h"
#include "actionsmanager.h"
#include "standarditemmodel.h"
#include "itemparentupdater.h"
#include "widgets/mytreeview.h"
#include "widgets/centralwidget.h"


ActionMergeManager::ActionMergeManager(ActionsManager* actionsManager) : QObject(actionsManager) {

    this->actionsManager = actionsManager;
    this->core = actionsManager->getCore();
    this->treeView = this->core->getTreeView();
    this->downloadModel = this->core->getDownloadModel();

    this->setupConnections();
}


void ActionMergeManager::setupConnections() {

    // recalculate full nzb size when merge has been done :
    connect (this,
             SIGNAL(recalculateNzbSizeSignal(const QModelIndex)),
             this->core->getItemParentUpdater(),
             SLOT(recalculateNzbSizeSlot(const QModelIndex)));

}


QList<QStandardItem*> ActionMergeManager::checkMergeCandidates(bool& mergeAvailable) {

    mergeAvailable = false;

    QStandardItem* selectedFileNameItem = 0;
    QList<QStandardItem*> fileNameItemList;

    // get selected rows :
    QList<QModelIndex> selectedIndexList = this->treeView->selectionModel()->selectedRows();

    // merge is allowed for only one selected row :
    if (selectedIndexList.size() == 1) {

        selectedFileNameItem = this->downloadModel->getFileNameItemFromIndex(selectedIndexList.at(0));

        // first, be sure that selected item is a parent one (nzb) :
        if (this->downloadModel->isNzbItem(selectedFileNameItem)) {

            // get the root item :
            QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

            // check state of each parent item :
            for (int i = 0; i < rootItem->rowCount(); i++) {

                // get corresponding nzb file name item :
                QStandardItem* fileNameItem = rootItem->child(i, FILE_NAME_COLUMN);

                // get current nzb item status :
                QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(fileNameItem->index());
                ItemStatus itemStatus = this->downloadModel->getStatusFromStateItem(stateItem);

                // merge is allowed for nzb item in download or post-process failed states :
                if ( Utility::isInDownloadProcess(itemStatus) ||
                     Utility::isPostDownloadFailed(itemStatus) ) {

                    // append items for which merge is possible :
                    if (selectedFileNameItem != fileNameItem) {

                        fileNameItemList.append(fileNameItem);

                    }

                }

            }

            if (!fileNameItemList.isEmpty()) {
                mergeAvailable = true;
            }

        }

    } // !selectedIndexList.isEmpty()

    return fileNameItemList;

}



void ActionMergeManager::processMerge(QStandardItem* selectedFileNameItem, QStandardItem* targetFileNameItem) {

    // get download folder from selected and target items :
    QString selectedFileSavePath = this->downloadModel->getNzbFileDataFromIndex(selectedFileNameItem->child(0)->index()).getFileSavePath();
    QString targetFileSavePath = this->downloadModel->getNzbFileDataFromIndex(targetFileNameItem->child(0)->index()).getFileSavePath();

    KUrl::List sourceFileList;

    // move all child item of selected parent item to target item :
    while (selectedFileNameItem->rowCount() > 0) {

        // retrieve first nzb child item :
        QStandardItem* childFileNameItem = selectedFileNameItem->child(0, FILE_NAME_COLUMN);

        // update file save path from selected item to file save path from target item :
        NzbFileData childNzbFileData = this->downloadModel->getNzbFileDataFromIndex(childFileNameItem->index());
        childNzbFileData.setFileSavePath(targetFileSavePath);
        this->downloadModel->updateNzbFileDataToItem(childFileNameItem, childNzbFileData);

        // then move the whole row :
        targetFileNameItem->appendRow(selectedFileNameItem->takeRow(0));

        // list downloaded files from selected item in order to move them in the target download folder :
        QFileInfo currentFileInfo(selectedFileSavePath + '/' + childNzbFileData.getDecodedFileName());

        if ( currentFileInfo.isFile() &&
             currentFileInfo.exists() ) {

            sourceFileList.append(currentFileInfo.filePath());

        }

    }

    // append eventual nzb files to files to be moved :
    foreach (QString nzbFile, QDir(selectedFileSavePath).entryList(QStringList() << "*.nzb", QDir::Files)) {
        sourceFileList.append(selectedFileSavePath + '/' + nzbFile);
    }

    // make sure that targetFolder already exists :
    Utility::createFolder(targetFileSavePath);

    // then move already decoded files into target download folder :
    KIO::CopyJob* moveJob = KIO::move(sourceFileList, KUrl(targetFileSavePath), KIO::Overwrite);

    // setup connections with job :
    connect(moveJob, SIGNAL(result(KJob*)), this, SLOT(handleResultSlot(KJob*)));

    moveJob->start();

    // update target nzb name :
    targetFileNameItem->setText(targetFileNameItem->text() + " + " + selectedFileNameItem->text());

    // remove the now empty source nzb item :
    this->downloadModel->invisibleRootItem()->removeRow(selectedFileNameItem->row());

    // update nzb size column :
    emit recalculateNzbSizeSignal(targetFileNameItem->index());

    // finaly, launch retry process :
    this->actionsManager->retryDownload(QList<QModelIndex>() << targetFileNameItem->index());

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void ActionMergeManager::mergeSubMenuAboutToShowSlot() {

    QAction* mergeNzbAction = this->core->getMainWindow()->getActionFromName("mergeNzb");

    // retrieve action submenu and clear it :
    QMenu* mergeSubMenu = mergeNzbAction->menu();
    mergeSubMenu->clear();

    bool mergeAvailable = false;

    // get list of candidates nzb that could be merged together :
    QList<QStandardItem*> fileNameItemList = this->checkMergeCandidates(mergeAvailable);

    // build new appropriate menu :
    if (mergeAvailable) {

        foreach (QStandardItem* fileNameItem, fileNameItemList) {

            QAction* currentAction = mergeSubMenu->addAction(fileNameItem->text());

            // set item uuid for identifying when user would select an action :
            currentAction->setData(this->downloadModel->getUuidStrFromIndex(fileNameItem->index()));

        }

    }

}


void ActionMergeManager::mergeNzbActionTriggeredSlot(QAction* subMenuAction) {

    bool mergeAvailable = false;

    // check that merge is still possible at this time :
    this->checkMergeCandidates(mergeAvailable);

    if (mergeAvailable) {

        // retrieve selected item and item corresponding to the action :
        QStandardItem* selectedFileNameItem = 0;
        QStandardItem* targetFileNameItem = 0;

        QList<QModelIndex> selectedIndexList = this->treeView->selectionModel()->selectedRows();

        if (!selectedIndexList.isEmpty()) {

            selectedFileNameItem = this->downloadModel->getFileNameItemFromIndex(selectedIndexList.at(0));

            // be sure that the item selected is a nzb :
            if ( !this->downloadModel->isNzbItem(selectedFileNameItem) ) {
                selectedFileNameItem = 0;
            }

        }


        // load stored uuid from action :
        QString targetItemUuid = subMenuAction->data().toString();

        // search the corresponding item :
        QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

        for (int i = 0; i < rootItem->rowCount(); i++) {

            QStandardItem* currentFileNameItem = rootItem->child(i, FILE_NAME_COLUMN);
            QString currentUuid = this->downloadModel->getUuidStrFromIndex(currentFileNameItem->index());

            // if uuid are matching, corresponding item has been found :
            if (currentUuid == targetItemUuid) {

                targetFileNameItem = currentFileNameItem;
                break;

            }

        }

        // if selected and target items have been found, merge is possible :
        if ( selectedFileNameItem != 0 &&
             targetFileNameItem != 0 &&
             selectedFileNameItem->rowCount() > 0 &&
             targetFileNameItem->rowCount() > 0 ) {

            // display merge confirmation dialog :
            int answer = this->core->getCentralWidget()->displayMergeItemsMessageBox(selectedFileNameItem->text(), targetFileNameItem->text());

            if (answer == KMessageBox::Yes) {
                // process to item merging :
                this->processMerge(selectedFileNameItem, targetFileNameItem);
            }

        }
        else {
            mergeAvailable = false;
        }


    }

    // merge is no more available, display a message to user :
    if (!mergeAvailable) {
        this->core->getCentralWidget()->displaySorryMessageBox(i18n("Merge can not be performed anymore"));
    }

}



void ActionMergeManager::handleResultSlot(KJob* moveJob) {

    // if job is succeed :
    if (moveJob->error() == 0) {

        // remove source folder (will be removed only if empty)
        KUrl::List sourceFileList = static_cast<KIO::CopyJob*>(moveJob)->srcUrls();

        if (!sourceFileList.isEmpty()) {

            // get source directory of the first moved file :
            QString sourceDirectory = sourceFileList.at(0).directory();

            QDir().rmdir(sourceDirectory);

        }

    }

}

