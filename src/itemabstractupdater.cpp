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
#include "data/itemstatusdata.h"
#include "data/nzbfiledata.h"



ItemAbstractUpdater::ItemAbstractUpdater(QObject* parent) : QObject (parent)
{
    // associate text to display according to item status :
    //    statusIconStrMap.insert(DownloadStatus,            "mail-receive");
    //    statusIconStrMap.insert(DownloadFinishStatus,      "dialog-ok-apply");
    //    statusIconStrMap.insert(IdleStatus,                "mail-mark-unread");
    //    statusIconStrMap.insert(PauseStatus,               "document-open-recent");
    //    statusIconStrMap.insert(PausingStatus,             "document-open-recent");
    //    statusIconStrMap.insert(ScanStatus,                "run-build");
    //    statusIconStrMap.insert(DecodeStatus,              "run-build");
    //    statusIconStrMap.insert(DecodeFinishStatus,        "mail-mark-read");
    //    statusIconStrMap.insert(DecodeErrorStatus,         "edit-delete");
    //    statusIconStrMap.insert(VerifyStatus,              "document-preview");
    //    statusIconStrMap.insert(VerifyFoundStatus,         "dialog-ok");
    //    statusIconStrMap.insert(VerifyMatchStatus,         "dialog-ok");
    //    statusIconStrMap.insert(VerifyMissingStatus,       "mail-mark-important");
    //    statusIconStrMap.insert(VerifyDamagedStatus,       "mail-mark-important");
    //    statusIconStrMap.insert(RepairStatus,              "document-preview");
    //    statusIconStrMap.insert(RepairNotPossibleStatus,   "edit-delete");
    //    statusIconStrMap.insert(RepairFailedStatus,        "edit-delete");
    //    statusIconStrMap.insert(ExtractStatus,             "archive-extract");
    //    statusIconStrMap.insert(ExtractBadCrcStatus,       "archive-remove");
    //    statusIconStrMap.insert(ExtractSuccessStatus,      "archive-remove");
    //    statusIconStrMap.insert(ExtractFailedStatus,       "archive-remove");

    statusIconStrMap.insert(DownloadStatus,            "mail-receive");
    statusIconStrMap.insert(DownloadFinishStatus,      "view-pim-news");
    statusIconStrMap.insert(IdleStatus,                "view-pim-news");
    statusIconStrMap.insert(PauseStatus,               "view-pim-news");
    statusIconStrMap.insert(PausingStatus,             "view-pim-news");
    statusIconStrMap.insert(ScanStatus,                "view-pim-news");
    statusIconStrMap.insert(DecodeStatus,              "view-pim-news");
    statusIconStrMap.insert(DecodeFinishStatus,        "news-subscribe");
    statusIconStrMap.insert(DecodeErrorStatus,         "news-unsubscribe");
    statusIconStrMap.insert(VerifyStatus,              "view-pim-news");
    statusIconStrMap.insert(VerifyFoundStatus,         "news-subscribe");
    statusIconStrMap.insert(VerifyMatchStatus,         "news-subscribe");
    statusIconStrMap.insert(VerifyMissingStatus,       "news-unsubscribe");
    statusIconStrMap.insert(VerifyDamagedStatus,       "news-unsubscribe");
    statusIconStrMap.insert(RepairStatus,              "view-pim-news");
    statusIconStrMap.insert(RepairNotPossibleStatus,   "news-unsubscribe");
    statusIconStrMap.insert(RepairFailedStatus,        "news-unsubscribe");
    statusIconStrMap.insert(ExtractStatus,             "news-subscribe");
    statusIconStrMap.insert(ExtractBadCrcStatus,       "news-unsubscribe");
    statusIconStrMap.insert(ExtractSuccessStatus,      "news-subscribe");
    statusIconStrMap.insert(ExtractFailedStatus,       "news-unsubscribe");

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

    QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(index);
    ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();

    if (statusIconStrMap.contains(status)) {

        if (status == DownloadFinishStatus) {

            if (itemStatusData.getDataStatus() == NoData) {
                status = DecodeErrorStatus;
            }

        }

        QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(index);
        fileNameItem->setIcon(KIcon(this->statusIconStrMap.value(status)));

    }

}

