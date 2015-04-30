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

#include "itemdownloadupdater.h"

#include "kwooty_debug.h"

#include "core.h"
#include "itemparentupdater.h"
#include "itemabstractupdater.h"
#include "data/segmentdata.h"
#include "data/nzbfiledata.h"
#include "data/itemstatusdata.h"
#include "standarditemmodel.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

ItemDownloadUpdater::ItemDownloadUpdater(ItemParentUpdater *itemParentUpdater) : ItemAbstractUpdater(itemParentUpdater->getDownloadModel(), ItemAbstractUpdater::Child)
{

    this->itemParentUpdater = itemParentUpdater;

}

void ItemDownloadUpdater::updateItems(const QModelIndex &parentModelIndex, const NzbFileData &nzbFileData)
{

    // update children items status :
    this->updateNzbChildrenItems(nzbFileData, parentModelIndex);

    // update parent (nzb item) status :
    itemParentUpdater->updateNzbItems(parentModelIndex.parent());

}

void ItemDownloadUpdater::updateNzbChildrenItems(const NzbFileData &nzbFileData, const QModelIndex &parentModelIndex)
{

    // variable initialisation
    this->clear();
    quint32 totalProgress = 0;

    // get itemStatusData :
    ItemStatusData itemStatusData = this->downloadModel->getStatusDataFromIndex(parentModelIndex);

    // set a high value > to max number of backup servers by default :
    int nextServerIdMin = 100;

    // get list of segments for the current file :
    QList<SegmentData> segmentList = nzbFileData.getSegmentList();

    // get progression and status of all segments :
    foreach (const SegmentData &currentSegmentData, segmentList) {

        // calculate progression :
        totalProgress += currentSegmentData.getProgress();

        // count status of each segments :
        this->countGlobalItemStatus(currentSegmentData);

        nextServerIdMin = qMin(nextServerIdMin, currentSegmentData.getServerGroupTarget());

    }

    // when all pending segments have been scanned by one server, set next backup server ID
    // in order to speed up segment searching in "SegmentManager" by using filtering in getNextSegmentSlot();
    if (itemStatusData.getNextServerId() != nextServerIdMin) {
        itemStatusData.setNextServerId(nextServerIdMin);
    }

    // calculate progression % :
    this->progressNumber = totalProgress / segmentList.size();

    // set progression to item :
    this->downloadModel->updateProgressItem(parentModelIndex, this->progressNumber);

    // set status to item :
    itemStatusData = this->updateStatusNzbChildrenItem(itemStatusData, segmentList.size());
    itemStatusData = this->updateDataStatus(itemStatusData);

    // update status bar and treeview that all segments have been downloaded :
    itemStatusData = this->postDownloadProcessing(parentModelIndex, nzbFileData, itemStatusData);

    // update itemStatusData in state item :
    this->downloadModel->updateStatusDataFromIndex(parentModelIndex, itemStatusData);

}

ItemStatusData ItemDownloadUpdater::updateStatusNzbChildrenItem(ItemStatusData &itemStatusData, const int &rowNumber)
{

    // if all segments have been downloaded :
    if (rowNumber == this->downloadFinishItemNumber) {
        itemStatusData.setStatus(DownloadFinishStatus);
    }

    // if some segments are being downloaded :
    if (this->downloadItemNumber > 0) {

        if (this->pauseItemNumber == 0) {
            itemStatusData.setStatus(DownloadStatus);
        } else {
            itemStatusData.setStatus(PausingStatus);
        }
    }
    // if no segments are currently being downloaded :
    else {
        if (this->inQueueItemNumber > 0) {
            itemStatusData.setStatus(IdleStatus);
        }

        if (this->pauseItemNumber > 0) {
            itemStatusData.setStatus(PauseStatus);
        }
    }

    return itemStatusData;
}

ItemStatusData ItemDownloadUpdater::updateDataStatus(ItemStatusData &itemStatusData)
{

    // item has not been updated, check if pending data exist :
    if (Utility::isInQueue(itemStatusData.getStatus()) &&
            this->pendingSegmentsOnBackupNumber > 0) {

        itemStatusData.setDataStatus(DataPendingBackupServer);
    }
    // determine if current file has segments that are not present on server :
    else if (this->downloadFinishItemNumber > 0) {

        // no segment founds on server for the current file :
        if (this->articleFoundNumber == 0) {
            itemStatusData.setDataStatus(NoData);
        }
        // at least one segment has been found on server :
        else if (this->downloadFinishItemNumber > this->articleFoundNumber) {
            itemStatusData.setDataStatus(DataIncomplete);
        } else if (this->downloadFinishItemNumber == this->articleFoundNumber) {
            itemStatusData.setDataStatus(DataComplete);
        }

    }

    return itemStatusData;

}

ItemStatusData ItemDownloadUpdater::postDownloadProcessing(const QModelIndex &index, const NzbFileData &nzbFileData, ItemStatusData &itemStatusData)
{

    QList<SegmentData> segmentList = nzbFileData.getSegmentList();

    // if all children have been downloaded :
    if (segmentList.size() == this->downloadFinishItemNumber) {

        // retrieve current progression and status related items :
        QStandardItem *stateItem = this->downloadModel->getStateItemFromIndex(index);

        // item has been downloaded, notify status bar :
        if (!itemStatusData.isDownloadFinish()) {

            QStandardItem *sizeItem = this->downloadModel->getSizeItemFromIndex(stateItem->index());
            quint64 size = sizeItem->data(SizeRole).toULongLong();

            emit statusBarDecrementSignal(size, 1);

            // update statusBarRole in order to update downloaded item only once :
            itemStatusData.setDownloadFinish(true);
        }

        itemStatusData.setStatus(DownloadFinishStatus);

        // before decoding check that at least one segment have been downloaded :
        if (itemStatusData.getDataStatus() != NoData) {

            // decode downloaded segments :
            emit decodeSegmentsSignal(nzbFileData);

        }

    }

    return itemStatusData;

}

void ItemDownloadUpdater::countGlobalItemStatus(const SegmentData &segmentData)
{

    // count number of files present / not present :
    if (segmentData.getArticlePresenceOnServer() == Present) {
        this->articleFoundNumber++;
    }

    if (segmentData.getArticlePresenceOnServer() == NotPresent) {
        this->articleNotFoundNumber++;
    }

    if (segmentData.getServerGroupTarget() != MasterServer &&
            Utility::isInQueue(segmentData.getStatus())) {

        this->pendingSegmentsOnBackupNumber++;

    }

    // count items status :
    this->countItemStatus(segmentData.getStatus());

}

