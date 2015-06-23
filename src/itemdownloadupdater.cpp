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

    this->mItemParentUpdater = itemParentUpdater;

}

void ItemDownloadUpdater::updateItems(const QModelIndex &parentModelIndex, const NzbFileData &nzbFileData)
{

    // update children items status :
    this->updateNzbChildrenItems(nzbFileData, parentModelIndex);

    // update parent (nzb item) status :
    mItemParentUpdater->updateNzbItems(parentModelIndex.parent());

}

void ItemDownloadUpdater::updateNzbChildrenItems(const NzbFileData &nzbFileData, const QModelIndex &parentModelIndex)
{

    // variable initialisation
    this->clear();
    quint32 totalProgress = 0;

    // get itemStatusData :
    ItemStatusData itemStatusData = this->mDownloadModel->getStatusDataFromIndex(parentModelIndex);

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
    this->mProgressNumber = totalProgress / segmentList.size();

    // set progression to item :
    this->mDownloadModel->updateProgressItem(parentModelIndex, this->mProgressNumber);

    // set status to item :
    itemStatusData = this->updateStatusNzbChildrenItem(itemStatusData, segmentList.size());
    itemStatusData = this->updateDataStatus(itemStatusData);

    // update status bar and treeview that all segments have been downloaded :
    itemStatusData = this->postDownloadProcessing(parentModelIndex, nzbFileData, itemStatusData);

    // update itemStatusData in state item :
    this->mDownloadModel->updateStatusDataFromIndex(parentModelIndex, itemStatusData);

}

ItemStatusData ItemDownloadUpdater::updateStatusNzbChildrenItem(ItemStatusData &itemStatusData, const int &rowNumber)
{

    // if all segments have been downloaded :
    if (rowNumber == this->mDownloadFinishItemNumber) {
        itemStatusData.setStatus(DownloadFinishStatus);
    }

    // if some segments are being downloaded :
    if (this->mDownloadItemNumber > 0) {

        if (this->mPauseItemNumber == 0) {
            itemStatusData.setStatus(DownloadStatus);
        } else {
            itemStatusData.setStatus(PausingStatus);
        }
    }
    // if no segments are currently being downloaded :
    else {
        if (this->mInQueueItemNumber > 0) {
            itemStatusData.setStatus(IdleStatus);
        }

        if (this->mPauseItemNumber > 0) {
            itemStatusData.setStatus(PauseStatus);
        }
    }

    return itemStatusData;
}

ItemStatusData ItemDownloadUpdater::updateDataStatus(ItemStatusData &itemStatusData)
{

    // item has not been updated, check if pending data exist :
    if (Utility::isInQueue(itemStatusData.getStatus()) &&
            this->mPendingSegmentsOnBackupNumber > 0) {

        itemStatusData.setDataStatus(DataPendingBackupServer);
    }
    // determine if current file has segments that are not present on server :
    else if (this->mDownloadFinishItemNumber > 0) {

        // no segment founds on server for the current file :
        if (this->mArticleFoundNumber == 0) {
            itemStatusData.setDataStatus(NoData);
        }
        // at least one segment has been found on server :
        else if (this->mDownloadFinishItemNumber > this->mArticleFoundNumber) {
            itemStatusData.setDataStatus(DataIncomplete);
        } else if (this->mDownloadFinishItemNumber == this->mArticleFoundNumber) {
            itemStatusData.setDataStatus(DataComplete);
        }

    }

    return itemStatusData;

}

ItemStatusData ItemDownloadUpdater::postDownloadProcessing(const QModelIndex &index, const NzbFileData &nzbFileData, ItemStatusData &itemStatusData)
{

    QList<SegmentData> segmentList = nzbFileData.getSegmentList();

    // if all children have been downloaded :
    if (segmentList.size() == this->mDownloadFinishItemNumber) {

        // retrieve current progression and status related items :
        QStandardItem *stateItem = this->mDownloadModel->getStateItemFromIndex(index);

        // item has been downloaded, notify status bar :
        if (!itemStatusData.isDownloadFinish()) {

            QStandardItem *sizeItem = this->mDownloadModel->getSizeItemFromIndex(stateItem->index());
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
        this->mArticleFoundNumber++;
    }

    if (segmentData.getArticlePresenceOnServer() == NotPresent) {
        this->mArticleNotFoundNumber++;
    }

    if (segmentData.getServerGroupTarget() != MasterServer &&
            Utility::isInQueue(segmentData.getStatus())) {

        this->mPendingSegmentsOnBackupNumber++;

    }

    // count items status :
    this->countItemStatus(segmentData.getStatus());

}

