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

#include "itemparentupdater.h"

#include <KDebug>

#include "centralwidget.h"
#include "mystatusbar.h"
#include "standarditemmodel.h"
#include "itemdownloadupdater.h"
#include "itempostdownloadupdater.h"
#include "itemchildrenmanager.h"
#include "settings.h"
#include "data/itemstatusdata.h"



ItemParentUpdater::ItemParentUpdater(CentralWidget* parent) : ItemAbstractUpdater (parent)
{
    this->parent = parent;
    this->downloadModel = parent->getDownloadModel();
    
    // instanciate item updater classes :
    this->itemPostDownloadUpdater = new ItemPostDownloadUpdater(this);
    this->itemDownloadUpdater = new ItemDownloadUpdater(this);
    this->itemChildrenManager = new ItemChildrenManager(parent, this);

    // setup connections :
    this->setupConnections();

}

ItemPostDownloadUpdater* ItemParentUpdater::getItemPostDownloadUpdater() const{
    return this->itemPostDownloadUpdater;
}
ItemDownloadUpdater* ItemParentUpdater::getItemDownloadUpdater() const{
    return this->itemDownloadUpdater;
}
StandardItemModel* ItemParentUpdater::getDownloadModel() const{
    return this->downloadModel;
}



void ItemParentUpdater::setupConnections() {

    // update buttons enable / disable after item status has been updated :
    connect (itemDownloadUpdater,
             SIGNAL(statusItemUpdatedSignal()),
             parent,
             SLOT(selectedItemSlot()));

    connect (this,
             SIGNAL(statusItemUpdatedSignal()),
             parent,
             SLOT(selectedItemSlot()));

    connect (itemDownloadUpdater,
             SIGNAL(statusBarDecrementSignal(const quint64, const int)),
             parent->getStatusBar(),
             SLOT(decrementSlot(const quint64, const int)));

    // recalcultate full nzb size when children may have been removed :
    connect (parent,
             SIGNAL(recalculateNzbSizeSignal(const QModelIndex)),
             this,
             SLOT(recalculateNzbSizeSlot(const QModelIndex)));

    // download par2 files if crc check failed during archive download :
    connect (this,
             SIGNAL(downloadWaitingPar2Signal()),
             parent,
             SLOT(downloadWaitingPar2Slot()));


}




void ItemParentUpdater::updateNzbItems(const QModelIndex& nzbIndex){

    // variable initialisation
    this->clear();
    quint64 totalProgress  = 0;
    this->isItemUpdated = false;


    // get itemStatusData :
    ItemStatusData nzbItemStatusData = this->downloadModel->getStatusDataFromIndex(nzbIndex);

    // get current item status :
    int previousStatus = nzbItemStatusData.getStatus();

    // get number of rows :
    int rowNumber = this->downloadModel->itemFromIndex(nzbIndex)->rowCount();

    // smart par2 download, set them to Idle only if a file crc has failed :
    bool par2FilesUpdated = this->updatePar2ItemsIfCrcFailed(nzbItemStatusData, rowNumber, nzbIndex);

    for (int i = 0; i < rowNumber; i++) {

        ItemStatusData itemStatusData = nzbIndex.child(i, STATE_COLUMN).data(StatusRole).value<ItemStatusData>();

        //  count children item status :
        this->countGlobalItemStatus(itemStatusData);

        // calculate global download progression :
        totalProgress += this->calculateDownloadProgress(nzbIndex, itemStatusData, i);

    }


    // 1. try to set status item as "DECODE"
    nzbItemStatusData = this->updateItemsDecode(nzbItemStatusData, rowNumber);

    // 2. else try to set status item as "DOWNLOAD"
    if (!this->isItemUpdated) {
        nzbItemStatusData = this->updateItemsDownload(nzbItemStatusData, rowNumber, nzbIndex, totalProgress);
    }

    // if decoding is finished for all elements, proceed to repairing :
    nzbItemStatusData = this->postProcessing(nzbItemStatusData, rowNumber, nzbIndex);

    // store statusData :
    this->downloadModel->updateStatusDataFromIndex(nzbIndex, nzbItemStatusData);


    // if item status has been updated :
    if (previousStatus != nzbItemStatusData.getStatus()) {
        // send signal to central widget to update enabled/disabled buttons :
        emit statusItemUpdatedSignal();
    }

    if (par2FilesUpdated) {
        emit downloadWaitingPar2Signal();
    }


}




//==============================================================================================//
//                            Verify/Repair - Extract related updates                           //
//==============================================================================================//

void ItemParentUpdater::updateNzbItemsPostDecode(const QModelIndex& nzbIndex, const int progression, UtilityNamespace::ItemStatus status){

    // if child are being verified / repaired or extracted :
    QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(nzbIndex);
    this->downloadModel->updateSateItem(stateItem, status);

    // update progression :
    this->downloadModel->updateProgressItem(nzbIndex, progression);
}



//==============================================================================================//
//                                   Decoding related updates                                   //
//==============================================================================================//


ItemStatusData ItemParentUpdater::updateItemsDecode(ItemStatusData& nzbItemStatusData, const int rowNumber) {

    // update status item :
    return this->updateStatusItemDecode(nzbItemStatusData, rowNumber);
}


ItemStatusData ItemParentUpdater::updateStatusItemDecode(ItemStatusData& nzbItemStatusData, const int rowNumber) {

    // if all children have been decoded :
    if (this->downloadItemNumber == 0) {

        if ( (this->decodeErrorItemNumber > 0) &&
             (rowNumber == this->decodeErrorItemNumber + this->articleNotFoundNumber) ) {

            nzbItemStatusData.setStatus(DecodeErrorStatus);
            this->isItemUpdated = true;
        }
        else if ( (this->decodeFinishItemNumber > 0) &&
                  (rowNumber == (this->decodeErrorItemNumber +
                                 this->articleNotFoundNumber +
                                 this->decodeFinishItemNumber)) ) {

            nzbItemStatusData.setStatus(DecodeFinishStatus);
            this->isItemUpdated = true;

        }
    }
    return nzbItemStatusData;

}



//==============================================================================================//
//                                   Downloading related updates                                //
//==============================================================================================//

ItemStatusData ItemParentUpdater::updateItemsDownload(ItemStatusData& nzbItemStatusData, const int rowNumber, const QModelIndex& nzbIndex, const quint64 totalProgress) {

    // calculate progression % :
    quint64 nzbSize = this->downloadModel->getSizeItemFromIndex(nzbIndex)->data(SizeRole).toULongLong();
    nzbSize = qMax(nzbSize, (quint64)1); // avoid division by zero (should never happen)
    this->progressNumber = qMin( qRound((qreal)(totalProgress / nzbSize)), PROGRESS_COMPLETE );

    // set progress to item :
    this->downloadModel->updateProgressItem(nzbIndex, this->progressNumber);

    // update status item :
    nzbItemStatusData = this->updateStatusItemDownload(nzbItemStatusData, rowNumber);

    // update data Status in nzbItemStatusData :
    nzbItemStatusData = this->updateDataStatus(nzbItemStatusData);

    return nzbItemStatusData;
}



ItemStatusData ItemParentUpdater::updateStatusItemDownload(ItemStatusData& nzbItemStatusData, const int rowNumber) {


    // if all children have been downloaded :
    if (rowNumber == this->downloadFinishItemNumber) {
        nzbItemStatusData.setStatus(DownloadFinishStatus);
    }

    // if some children are being downloaded :
    else if (this->downloadItemNumber > 0) {
        nzbItemStatusData.setStatus(DownloadStatus);
    }

    // if no children are currently being downloaded :
    else if (this->downloadItemNumber == 0 ) {

        // if some children are Idle :
        if (this->inQueueItemNumber > 0) {
            nzbItemStatusData.setStatus(IdleStatus);
        }
        // else if children are in pause :
        else {
            if (this->pausingItemNumber > 0) {
                nzbItemStatusData.setStatus(PausingStatus);
            }
            else if (this->pauseItemNumber > 0) {
                nzbItemStatusData.setStatus(PauseStatus);
            }
            else if (this->decodeItemNumber > 0) {
                nzbItemStatusData.setStatus(DecodeStatus);
            }
            else if (this->scanItemNumber > 0) {
                nzbItemStatusData.setStatus(ScanStatus);
            }
        }

    }

    return nzbItemStatusData;
}



ItemStatusData ItemParentUpdater::updateDataStatus(ItemStatusData& nzbItemStatusData) {

    // determine if current file has segments that are not present on server :
    if (nzbItemStatusData.getStatus() == DownloadStatus) {

        // no segment founds on server for the current file :
        if (this->articleNotFoundNumber > 0) {

            if (this->articleFoundNumber == 0) {
                nzbItemStatusData.setDataStatus(NoData);
            }
            else{
                nzbItemStatusData.setDataStatus(DataIncomplete);
            }
        }
        // at least one segment has been found on server :
        else {
            nzbItemStatusData.setDataStatus(DataComplete);
        }

    }

    return nzbItemStatusData;
}



quint64 ItemParentUpdater::calculateDownloadProgress(const QModelIndex& nzbIndex, const ItemStatusData& itemStatusData, const int i) {

    // calculate progression :
    quint64 totalProgress = 0;
    int fileProgress = nzbIndex.child(i, PROGRESS_COLUMN).data(ProgressRole).toInt();
    quint64 fileSize = nzbIndex.child(i, SIZE_COLUMN).data(SizeRole).toULongLong();


    // file has been already downloaded :
    if (itemStatusData.isDownloadFinish()){
        totalProgress += PROGRESS_COMPLETE * fileSize;
    }
    // else the file is currently being downloaded :
    else {
        totalProgress += fileProgress * fileSize;
    }

    return totalProgress;

}



ItemStatusData ItemParentUpdater::postProcessing(ItemStatusData& nzbItemStatusData, const int rowNumber, const QModelIndex& nzbIndex){

    // nzb files have been decoded, it's time to repair them :
    if ( (nzbItemStatusData.getStatus() == DecodeFinishStatus) &&
         !nzbItemStatusData.isDecodeFinish() ) {

        // set decode finish to true in order to emit repairing signal only once :
        nzbItemStatusData.setDecodeFinish(true);

        // prepare nzbFileDataList :
        QList<NzbFileData> nzbFileDataList;

        for (int i = 0; i < rowNumber; i++) {
            NzbFileData currentNzbFileData = nzbIndex.child(i, FILE_NAME_COLUMN).data(NzbFileDataRole).value<NzbFileData>();

            // remove segmentData list has they are no more used :
            currentNzbFileData.setSegmentList(QList<SegmentData>());
            nzbFileDataList.append(currentNzbFileData);
        }


        // send nzbFileDataList to repairDecompressThread class :
        emit repairDecompressSignal(nzbFileDataList);

    }


    return nzbItemStatusData;
}




void ItemParentUpdater::recalculateNzbSize(const QModelIndex& nzbIndex){

    // variable initialisation
    quint64 size = 0;

    // retrieve size related item :
    QStandardItem* sizeItem = this->downloadModel->getSizeItemFromIndex(nzbIndex);

    // get size of all nzb children :
    int rowNumber = this->downloadModel->itemFromIndex(nzbIndex)->rowCount();

    for (int i = 0; i < rowNumber; i++) {

        // get itemStatusData :
        ItemStatusData childItemStatusData = this->downloadModel->getStatusDataFromIndex(nzbIndex.child(i, SIZE_COLUMN));

        // do not count size of par2 files with WaitForPar2IdleStatus as status :
        if (childItemStatusData.getStatus() != WaitForPar2IdleStatus) {
            // recalculate size :
            size += nzbIndex.child(i, SIZE_COLUMN).data(SizeRole).toULongLong();
        }

    }

    // set size value to item :
    sizeItem->setData(QVariant(size), SizeRole);

}


void ItemParentUpdater::countGlobalItemStatus(const ItemStatusData& itemStatusData) {

    // count number of files present / not present :
    if ( itemStatusData.getDataStatus() == NoData){
        this->articleNotFoundNumber++;
    }

    if (itemStatusData.getDataStatus() != NoData){
        this->articleFoundNumber++;
    }

    // count items status :
    this->countItemStatus(itemStatusData.getStatus());

}


bool ItemParentUpdater::updatePar2ItemsIfCrcFailed(ItemStatusData& nzbItemStatusData, const int rowNumber, const QModelIndex& nzbIndex) {

    bool par2FilesUpdated = false;

    if (Settings::smartPar2Download()) {

        // search for children with a bad crc :
        if (nzbItemStatusData.getCrc32Match() == crcOk) {

            for (int i = 0; i < rowNumber; i++) {

                ItemStatusData itemStatusData = nzbIndex.child(i, STATE_COLUMN).data(StatusRole).value<ItemStatusData>();

                // if a children has an incorrect crc, set the crc of the parent as incorrect :
                if ( (itemStatusData.getCrc32Match() == crcKo) ||
                     (itemStatusData.getDataStatus() == NoData) ){

                    nzbItemStatusData.setCrc32Match(crcKo);
                    break;
                }
            }
        }

        // if parent crc has been set to incorrect par2 files are required, set them to IdleStatus :
        if (nzbItemStatusData.getCrc32Match() == crcKo) {

            itemChildrenManager->changePar2FilesStatusSlot(nzbIndex, IdleStatus);

            nzbItemStatusData.setCrc32Match(crcKoNotified);
            par2FilesUpdated = true;

        }
    }

    return par2FilesUpdated;
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void ItemParentUpdater::recalculateNzbSizeSlot(const QModelIndex index) {

    this->recalculateNzbSize(index);
    this->updateNzbItems(index);

}

