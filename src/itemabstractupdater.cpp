/***************************************************************************
 *   Copyright (C) 2009 by Xavier Lefage                                   *
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

#include "itemabstractupdater.h"

#include <KDebug>
#include <KIcon>
#include "itemparentupdater.h"
#include "standarditemmodel.h"
#include "settings.h"
#include "data/itemstatusdata.h"
#include "data/nzbfiledata.h"



ItemAbstractUpdater::ItemAbstractUpdater(QObject* parent) : QObject (parent)
{
    // build map in order to display status icon near to file name item :

    statusIconStrMap.insert(DownloadStatus,            "mail-receive");
    statusIconStrMap.insert(DownloadFinishStatus,      "mail-mark-unread");
    statusIconStrMap.insert(IdleStatus,                "mail-mark-unread");
    statusIconStrMap.insert(PauseStatus,               "mail-mark-unread");
    statusIconStrMap.insert(PausingStatus,             "mail-mark-unread");
    statusIconStrMap.insert(ScanStatus,                "mail-mark-unread");
    statusIconStrMap.insert(DecodeStatus,              "mail-mark-unread");
    statusIconStrMap.insert(DecodeFinishStatus,        "mail-mark-read");
    statusIconStrMap.insert(DecodeErrorStatus,         "edit-delete");
    statusIconStrMap.insert(VerifyStatus,              "mail-mark-read");
    statusIconStrMap.insert(VerifyFoundStatus,         "dialog-ok-apply");
    statusIconStrMap.insert(VerifyMatchStatus,         "dialog-ok-apply");
    statusIconStrMap.insert(VerifyMissingStatus,       "edit-delete");
    statusIconStrMap.insert(VerifyDamagedStatus,       "edit-delete");
    statusIconStrMap.insert(RepairStatus,              "mail-mark-read");
    statusIconStrMap.insert(RepairNotPossibleStatus,   "edit-delete");
    statusIconStrMap.insert(RepairFailedStatus,        "edit-delete");
    statusIconStrMap.insert(ExtractStatus,             "dialog-ok-apply");
    statusIconStrMap.insert(ExtractBadCrcStatus,       "edit-delete");
    statusIconStrMap.insert(ExtractSuccessStatus,      "dialog-ok-apply");
    statusIconStrMap.insert(ExtractFailedStatus,       "edit-delete");


}


ItemAbstractUpdater::ItemAbstractUpdater()
{
}


void ItemAbstractUpdater::clear() {

    this->downloadItemNumber = 0;
    this->pauseItemNumber = 0;
    this->progressNumber = 0;
    this->downloadFinishItemNumber = 0;
    this->inQueueItemNumber = 0;
    this->pausingItemNumber = 0;
    this->decodeFinishItemNumber = 0;
    this->decodeErrorItemNumber = 0;
    this->articleNotFoundNumber = 0;
    this->articleFoundNumber = 0;
    this->verifyItemNumber = 0;
    this->verifyFinishItemNumber = 0;
    this->repairItemNumber = 0;

}



void ItemAbstractUpdater::countItemStatus(const int status) {


    switch (status) {

    case DownloadStatus: {
            this->downloadItemNumber++;
            break;
        };
    case DownloadFinishStatus: {
            this->downloadFinishItemNumber++;
            break;
        };
    case IdleStatus: {
            this->inQueueItemNumber++;
            break;
        };
    case PauseStatus: {
            this->pauseItemNumber++;
            break;
        };
    case PausingStatus: {
            this->pausingItemNumber++;
            break;
        };
    case DecodeFinishStatus: {
            this->decodeFinishItemNumber++;
            break;
        };
    case DecodeErrorStatus: {
            this->decodeErrorItemNumber++;
            break;
        };
    case DecodeStatus: {
            this->decodeItemNumber++;
            break;
        };
    case ScanStatus: {
            this->scanItemNumber++;
            break;
        };

    }

}


void ItemAbstractUpdater::setIconToFileNameItem(const QModelIndex& index, UtilityNamespace::ItemStatus status) {

    if (!this->downloadModel->isNzbItem(this->downloadModel->itemFromIndex(index))) {

        // if icons have to be displayed :
        if (Settings::displayIcons()) {

            // get final status :
            QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(index);
            ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();

            if (statusIconStrMap.contains(status)) {

                if (status == DownloadFinishStatus) {

                    if (itemStatusData.getDataStatus() == NoData) {
                        // in this case the status is set to DecodeErrorStatus only to display the proper icon :
                        status = DecodeErrorStatus;
                    }

                }

                // get fileName item and set corresponding icon :
                QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(index);
                fileNameItem->setIcon(KIcon(this->statusIconStrMap.value(status)));

            }
        }
        // if icon does not have to be displayed :
        else {
            QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(index);
            if (!fileNameItem->icon().isNull()) {
                // remove icon :
                fileNameItem->setIcon(QIcon());

            }

        }

    }

}

