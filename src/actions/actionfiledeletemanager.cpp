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

#include "actionfiledeletemanager.h"

#include <kurl.h>
#include <KIO/JobUiDelegate>
#include <KIO/DeleteJob>

#include <QDir>

#include "standarditemmodelquery.h"

ActionFileDeleteManager::ActionFileDeleteManager(ActionsManager *actionsManager) : ActionFileManagerBase(actionsManager)
{

}

bool ActionFileDeleteManager::isDeleteAllowed(QStandardItem *selectedFileNameItem) const
{

    ItemStatusData itemStatusData = this->mDownloadModel->getStatusDataFromIndex(selectedFileNameItem->index());

    // check that selected row is a nzb item :
    return (this->mDownloadModel->isNzbItem(selectedFileNameItem) &&
            !Utility::isPostDownloadProcessing(itemStatusData.getStatus()));

}

QString ActionFileDeleteManager::retrieveFileSavePath(QStandardItem *selectedFileNameItem) const
{
    return this->mDownloadModel->getNzbFileDataFromIndex(selectedFileNameItem->index()).getFileSavePath();

}

void ActionFileDeleteManager::launchProcess()
{
    this->removeRowDeleteFile();

}

void ActionFileDeleteManager::resetState()
{

    this->mActionFileStep = ActionFileIdle;
    this->mSelectedItemUuidList.clear();

}

void ActionFileDeleteManager::removeRowDeleteFile()
{

    this->mActionFileStep = ActionFileProcessing;

    KUrl::List selectedUrlsCheck;

    QList<QModelIndex> indexesListCheck;

    foreach (QString uuid, this->mSelectedItemUuidList) {

        QStandardItem *selectedFileNameItem = this->mCore->getModelQuery()->retrieveParentFileNameItemFromUuid(uuid);
        QString fileSavePath = this->retrieveFileSavePath(selectedFileNameItem);

        // delete is allowed if row is not is post processing state :
        if (this->isDeleteAllowed(selectedFileNameItem)) {

            indexesListCheck.append(selectedFileNameItem->index());

            // ensure that directory to be removed exists :
            if (!fileSavePath.isEmpty() &&
                    QDir(fileSavePath).exists()) {

                // remove data pending to be decoded as folder need to be removed :
                NzbFileData selectedNzbFileData = this->mDownloadModel->getNzbFileDataFromIndex(this->mDownloadModel->getNzbItem(selectedFileNameItem)->index());
                this->mSegmentBuffer->removeDataFromDecodeWaitingQueue(selectedNzbFileData);

                selectedUrlsCheck.append(KUrl(fileSavePath));

            }
        }

    }

    // remove row from treeview :
    this->mActionsManager->removeRow(indexesListCheck);

    // remove data :
    KIO::Job *job = KIO::del(selectedUrlsCheck);

    if (job->ui()) {
        //PORT KF5 job->ui()->setWindow(this->treeView);
    }

    connect(job, SIGNAL(result(KJob*)), this, SLOT(handleResultSlot(KJob*)));

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ActionFileDeleteManager::actionTriggeredSlot()
{

    bool deleteRequested = false;

    KUrl::List selectedUrls;
    QList<QModelIndex> selectedIndexList = this->mTreeView->selectionModel()->selectedRows();

    if (this->mActionFileStep == ActionFileIdle  &&
            this->mCore->getModelQuery()->haveItemsSameParent(selectedIndexList)) {

        // store rows in a list :
        for (int i = 0; i < selectedIndexList.size(); ++i) {

            QStandardItem *selectedFileNameItem = this->mDownloadModel->getFileNameItemFromIndex(selectedIndexList.at(i));

            // delete is allowed if row is not is post processing state :
            if (this->isDeleteAllowed(selectedFileNameItem)) {

                QString uuid = this->mCore->getDownloadModel()->getUuidStrFromIndex(selectedFileNameItem->index());
                this->mSelectedItemUuidList.append(uuid);

                QString fileSavePath = this->retrieveFileSavePath(selectedFileNameItem);

                if (!fileSavePath.isEmpty()) {
                    selectedUrls.append(KUrl(fileSavePath));
                }

            }

        }

        // if url list is not empty :
        if (!selectedUrls.isEmpty()) {

            // ask confirmation to user :
            KIO::JobUiDelegate uiDelegate;
            uiDelegate.setWindow(this->mTreeView);

            deleteRequested = uiDelegate.askDeleteConfirmation(selectedUrls,
                              KIO::JobUiDelegate::Delete,
                              KIO::JobUiDelegate::ForceConfirmation);
        }

    }

    // if confirmed :
    if (deleteRequested) {

        this->mActionFileStep = ActionFileRequested;
        this->processFileSlot();

    } else {
        this->resetState();
    }

}

void ActionFileDeleteManager::handleResultSlot(KJob *job)
{

    if (job->error() == 0) {
        qCDebug(KWOOTY_LOG) << "file delete job finished";
    }

    // job is finished :
    this->resetState();

}
