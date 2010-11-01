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
#include "standarditemmodel.h"
#include "itemdownloadupdater.h"
#include "itempostdownloadupdater.h"
#include "itemchildrenmanager.h"
#include "observers/clientsobserver.h"
#include "data/itemstatusdata.h"
#include "kwootysettings.h"


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

#if (QT_VERSION >= 0x040600) && (QT_VERSION <= 0x040602)
    CentralWidget* ItemParentUpdater::getCentraWidget() const{
    return this->parent;
}
#endif



void ItemParentUpdater::setupConnections() {

    // update buttons enable / disable after item status has been updated :
    connect (itemDownloadUpdater,
             SIGNAL(statusItemUpdatedSignal()),
             parent->getTreeView(),
             SLOT(selectedItemSlot()));

    connect (this,
             SIGNAL(statusItemUpdatedSignal()),
             parent->getTreeView(),
             SLOT(selectedItemSlot()));

    connect (itemDownloadUpdater,
             SIGNAL(statusBarDecrementSignal(const quint64, const int)),
             parent->getClientsObserver(),
             SLOT(decrementSlot(const quint64, const int)));

    // recalculate full nzb size when children may have been removed :
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

    // if crc fail, emit a signal in order to download par2 files :
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
    this->downloadModel->updateStateItem(stateItem, status);

    // update progression :
    this->downloadModel->updateProgressItem(nzbIndex, progression);

    // if extract failed, download par2 files if there were considered as not required :
    this->updateItemsIfDirectExtractFailed(nzbIndex, stateItem, status);

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

    quint64 nzbSize = this->downloadModel->getSizeValueFromIndex(nzbIndex);
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


        // by default, consider that par2 files have been downloaded :
        UtilityNamespace::ItemStatus par2FileStatus = DownloadFinishStatus;

        // prepare nzbFileDataList :
        QList<NzbFileData> nzbFileDataList;

        for (int i = 0; i < rowNumber; i++) {

            QModelIndex childIndex = nzbIndex.child(i, FILE_NAME_COLUMN);
            NzbFileData currentNzbFileData = this->downloadModel->getNzbFileDataFromIndex(childIndex);


            // get itemStatusData :
            ItemStatusData childItemStatusData = this->downloadModel->getStatusDataFromIndex(childIndex);

            // if par2 files have been required after extracting of a 1st nzb-set
            // just append files of nzb-sets that have not been previously extracted :
            if (childItemStatusData.getStatus() != ExtractSuccessStatus) {

                // remove segmentData list has they are no more used :
                currentNzbFileData.setSegmentList(QList<SegmentData>());

                nzbFileDataList.append(currentNzbFileData);

            }


            // if current item is a par2 file, check if it status is Idle or WaitForPar2IdleStatus :
            if ( currentNzbFileData.isPar2File() &&
                 (childItemStatusData.getStatus() == WaitForPar2IdleStatus) )  {

                par2FileStatus = WaitForPar2IdleStatus;

            }

        }


        // build data for repairing - extracting process :
        NzbCollectionData nzbCollectionData;
        nzbCollectionData.setNzbFileDataList(nzbFileDataList);
        nzbCollectionData.setPar2FileDownloadStatus(par2FileStatus);
        nzbCollectionData.setNzbParentId(this->downloadModel->getUuidStrFromIndex(nzbIndex));

        //kDebug() << "UUID : " << nzbFileName->data(IdentifierRole).toString();

        // send nzbCollectionData to repairDecompressThread class :
        emit repairDecompressSignal(nzbCollectionData);

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
        if (nzbItemStatusData.getCrc32Match() == CrcOk) {

            for (int i = 0; i < rowNumber; i++) {

                ItemStatusData itemStatusData = nzbIndex.child(i, STATE_COLUMN).data(StatusRole).value<ItemStatusData>();

                // if a children has an incorrect crc, set the crc of the parent as incorrect :
                if ( (itemStatusData.getCrc32Match() == CrcKo) ||
                     (itemStatusData.getDataStatus() == NoData) ){

                    nzbItemStatusData.setCrc32Match(CrcKo);
                    break;
                }
            }
        }

        // if parent crc has been set to incorrect par2 files are required, set them to IdleStatus :
        if (nzbItemStatusData.getCrc32Match() == CrcKo) {

            itemChildrenManager->changePar2FilesStatusSlot(nzbIndex, IdleStatus);

            nzbItemStatusData.setCrc32Match(CrcKoNotified);
            par2FilesUpdated = true;

        }
    }

    return par2FilesUpdated;
}


void ItemParentUpdater::updateItemsIfDirectExtractFailed(const QModelIndex nzbIndex, QStandardItem* stateItem, UtilityNamespace::ItemStatus status) {

    if ( (status == ExtractFinishedStatus) &&
         Settings::smartPar2Download() ) {

        bool par2NotDownloaded = this->itemChildrenManager->resetItemStatusIfExtractFail(nzbIndex);

        if (par2NotDownloaded) {

            // set nzbItem to IdleStatus in order to enable par2 file downloads :
            this->downloadModel->updateStateItem(stateItem, IdleStatus);

            // set decodeFinish to false in order to allow post download process another time
            // and store statusData :
            ItemStatusData nzbItemStatusData = this->downloadModel->getStatusDataFromIndex(nzbIndex);
            nzbItemStatusData.setDecodeFinish(false);
            this->downloadModel->updateStatusDataFromIndex(nzbIndex, nzbItemStatusData);

            emit downloadWaitingPar2Signal();

        }
    }
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void ItemParentUpdater::recalculateNzbSizeSlot(const QModelIndex index) {

    this->recalculateNzbSize(index);
    this->updateNzbItems(index);

}

