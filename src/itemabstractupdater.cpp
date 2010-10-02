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
#include "kwootysettings.h"
#include "utilityiconpainting.h"
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
    statusIconStrMap.insert(WaitForPar2IdleStatus,     "mail-mark-unread-new");
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
    this->pendingSegmentsOnBackupNumber = 0;

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
    case WaitForPar2IdleStatus: {
            this->decodeFinishItemNumber++;
            break;
        };

        // /!\ during download process the status could be ExtractSuccessStatus when following condition occurs :
        // A multi nzb-set has been fully downloaded and par2 files were not needed (WaitForPar2IdleStatus)
        // then extract process if directly done : 1st nzb-set is correctly extracted
        // but extracting of 2nd nzb-set failed (due to a bad crc for eg.)
        // par2 files are then downloaded for repairing of 2nd nzb-set.
        // => consider previously extracted files from 1st nzb-set as decodeFinish files :
    case ExtractSuccessStatus: {
            this->decodeFinishItemNumber++;
            break;
        };

    }

}



void ItemAbstractUpdater::setIconToFileNameItem(const QModelIndex& index, UtilityNamespace::ItemStatus status) {

    if (!this->downloadModel->isNzbItem(this->downloadModel->itemFromIndex(index))) {

        if (statusIconStrMap.contains(status)) {

            // get final status :
            QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(index);
            ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();

            if (status == DownloadFinishStatus) {

                if (itemStatusData.getDataStatus() == NoData) {
                    // in this case the status is set to DecodeErrorStatus only to display the proper icon :
                    status = DecodeErrorStatus;
                }

            }


            // set a special icon if segment is still pending and has not been found on master server :
            if (itemStatusData.getDataStatus() == DataPendingBackupServer)  {

                QString iconStr = "mail-mark-unread";
                if (this->statusIconStrMap.value(status) == iconStr) {

                    //QPixmap finalPixmap = UtilityIconPainting::getInstance()->blendOverLayTopRight(iconStr, "mail-reply-custom");

                    QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(index);
                    fileNameItem->setIcon(KIcon("mail-reply-list"));
                    return;
                }

            }


            // get fileName item and set corresponding icon :
            QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(index);
            fileNameItem->setIcon(KIcon(this->statusIconStrMap.value(status)));

        }
    }

}


