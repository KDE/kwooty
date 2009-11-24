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

#include "segmentmanager.h"

#include <KDebug>
#include <QModelIndexList>
#include <QModelIndex>

#include "centralwidget.h"
#include "itemdownloadupdater.h"
#include "itempostdownloadupdater.h"
#include "clientmanagerconn.h"
#include "itemparentupdater.h"
#include "data/segmentdata.h"
#include "data/nzbfiledata.h"
#include "standarditemmodel.h"



SegmentManager::SegmentManager(CentralWidget* parent) : QObject (parent)
{
    this->downloadModel = parent->getDownloadModel();
    this->itemParentUpdater = parent->getItemParentUpdater();
}


SegmentManager::SegmentManager()
{
}



bool SegmentManager::sendNextIdleSegment(QStandardItem* fileNameItem, ClientManagerConn* currentClientManagerConn){

    bool itemFound = false;

    // look for an item to download :
    NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
    QList<SegmentData> segmentList = nzbFileData.getSegmentList();

    // parse segment list to find the next idle segment :
    for (int i = 0; i < segmentList.size(); i++) {

        SegmentData segmentData = segmentList.at(i);

        if ( (segmentData.getStatus() == IdleStatus) && !itemFound) {

            itemFound = true;

            // next idle status has been found, set it to download status :
            segmentData.setStatus(DownloadStatus);

            // update parent item data :
            segmentList.replace(i, segmentData);
            nzbFileData.setSegmentList(segmentList);
            this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);

            itemParentUpdater->getItemDownloadUpdater()->updateItems(fileNameItem->index(), nzbFileData);

            // set parent idenfier to segment before downloading it  :
            segmentData.setParentUniqueIdentifier(nzbFileData.getUniqueIdentifier());

            // send the next part to the dedicated client :
            currentClientManagerConn->processNextSegment(segmentData);

            break;
        }


    } // end of for loop


    return itemFound;

}



void SegmentManager::setIdlePauseSegments(QStandardItem* fileNameItem, const int targetStatus){

    // check that the nzb status item is either Idle, Download or pausing / paused :
    QStandardItem* nzbStatusItem = this->downloadModel->getStateItemFromIndex(fileNameItem->index());
    UtilityNamespace::ItemStatus nzbStatus = this->downloadModel->getStatusFromStateItem(nzbStatusItem);

    // Set to pause / idle only items that are in download process (eg : not the ones decoded or verified...) :
    if (Utility::isInDownloadProcess(nzbStatus)) {

        // set segment with Idle back to Pause status and vice-versa :
        NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
        QList<SegmentData> segmentList = nzbFileData.getSegmentList();

        for (int i = 0; i < segmentList.size(); i++){
            SegmentData currentSegment = segmentList.at(i);

            if (currentSegment.getStatus() == IdleStatus && targetStatus == PauseStatus) {
                currentSegment.setStatus(targetStatus);

            }
            if (currentSegment.getStatus() == PauseStatus && targetStatus == IdleStatus) {
                currentSegment.setStatus(targetStatus);
            }

            segmentList.replace(currentSegment.getElementInList(), currentSegment);
        }


        // update the nzbFileData of current fileNameItem and its corresponding items;
        nzbFileData.setSegmentList(segmentList);
        this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);


        itemParentUpdater->getItemDownloadUpdater()->updateItems(fileNameItem->index(), nzbFileData);
    }
}







QStandardItem* SegmentManager::searchItem(const QVariant& parentIdentifer, const UtilityNamespace::ItemStatus itemStatus){

    bool itemUpdated = false;

    QModelIndex parentModelIndex;
    QStandardItem* fileNameItem = NULL;
    QModelIndex nzbFileModelIndex;


    // search the unique identifier modelIndex that match with the segmentData's one for updating it :
    if (this->downloadModel->rowCount() > 0) {

        int row = 0;

        while ((row < this->downloadModel->rowCount()) && !itemUpdated) {

            //retrieve root item (nzb item) :
            nzbFileModelIndex = this->downloadModel->item(row , FILE_NAME_COLUMN)->index();

            QStandardItem* nzbStatusItem = this->downloadModel->item(row , STATE_COLUMN);

            // before processing searches, check that the nzb status item is either Idle, Download or in being paused :
            UtilityNamespace::ItemStatus nzbStatus = this->downloadModel->getStatusFromStateItem(nzbStatusItem);


            // determine search in nzb parent item according to its status :
            bool isSearchInNzb = false;

            // search item according to parent nzb parent status :
            if (itemStatus == DownloadStatus) {
                if (Utility::isReadyToDownload(nzbStatus) ||
                    Utility::isPausing(nzbStatus)) {
                    isSearchInNzb = true;
                }
            }
            else if (itemStatus == DecodeStatus) {
                if (Utility::isReadyToDownload(nzbStatus) ||
                    Utility::isDownloadFinish(nzbStatus) ||
                    Utility::isPausing(nzbStatus)  ||
                    Utility::isDecoding(nzbStatus)) {
                    isSearchInNzb = true;
                }
            }
            else if ((itemStatus == RepairStatus) || (itemStatus == ExtractStatus)) {
                isSearchInNzb = true;
            }
            else if (itemStatus == PauseStatus) {
                isSearchInNzb = Utility::isPaused(nzbStatus);
            }



            // if search in Nzb item :
            if (isSearchInNzb) {

                // get its first file among the whole list :
                parentModelIndex = nzbFileModelIndex.child(0, FILE_NAME_COLUMN);

                // search parent item that match among the list :
                QModelIndexList parentIndexList = this->downloadModel->match(parentModelIndex,
                                                                             IdentifierRole,
                                                                             parentIdentifer,
                                                                             1,
                                                                             Qt::MatchExactly);

                // parent should be found (otherwise the row has been removed by the user) :
                if (!parentIndexList.isEmpty()){
                    // only one element is returned :
                    parentModelIndex = parentIndexList.at(0);
                    fileNameItem = this->downloadModel->itemFromIndex(parentModelIndex);

                    itemUpdated = true;
                    break;
                }


            }

            row++;
        }


    }


    return fileNameItem;

}





void SegmentManager::addFileTypeInfo(QStandardItem* fileNameItem, const QString& decodedFileName) {

    NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
    // set the name of the decoded file :
    if (!decodedFileName.isEmpty()) {
        nzbFileData.setDecodedFileName(decodedFileName);

        // add info about type of file (par2 or rar file) :
        if (!nzbFileData.isPar2File() &&
            !nzbFileData.isRarFile()) {

            QFile decodedFile(nzbFileData.getFileSavePath() + nzbFileData.getDecodedFileName());
            if (decodedFile.exists()) {

                decodedFile.open(QIODevice::ReadOnly);

                // rar headers could be corrupted because repair process has not been proceeded yet at this stage
                // but assume that at least one rar file will have a correct header to launch decompress process later :
                if (decodedFile.peek(rarFilePattern.size()) == rarFilePattern) {
                    nzbFileData.setRarFile(true);
                }
                // check if it is a par2 file :
                else if ( (decodedFile.peek(par2FilePattern.size()) == par2FilePattern) ||
                          decodedFileName.endsWith(par2FileExt, Qt::CaseInsensitive)) {

                    nzbFileData.setPar2File(true);
                }

                decodedFile.close();
            }
        }

    }

    // update the nzbFileData of current fileNameItem and its corresponding items :
    this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void SegmentManager::getNextSegmentSlot(ClientManagerConn* currentClientManagerConn){

    bool itemFound = false;
    int row = 0;

    while ( (row < this->downloadModel->rowCount()) && !itemFound ) {

        QStandardItem* nzbItem = this->downloadModel->item(row, FILE_NAME_COLUMN);
        QStandardItem* nzbStatusItem = this->downloadModel->item(row, STATE_COLUMN);
        UtilityNamespace::ItemStatus currentStatus = this->downloadModel->getStatusFromStateItem(nzbStatusItem);

        // if nzb(parent) item is in Idle or Download state look for idle segment, otherwise switch to next one :
        if (Utility::isReadyToDownload(currentStatus) && !itemFound) {

            for (int i = 0; i < nzbItem->rowCount(); i++) {

                if (!itemFound) {

                    QStandardItem* fileNameItem = nzbItem->child(i, FILE_NAME_COLUMN);
                    QStandardItem* stateItem = nzbItem->child(i, STATE_COLUMN);

                    // check children status and send idle segment :
                    UtilityNamespace::ItemStatus currentStatus = this->downloadModel->getStatusFromStateItem(stateItem);
                    if (Utility::isReadyToDownload(currentStatus)) {

                        itemFound = this->sendNextIdleSegment(fileNameItem, currentClientManagerConn);
                        
                    }
                }
            }
        }
        row++;
    } // end of loop

    //notify the client that no segment has been found to return to idle status :
    if (!itemFound) {
        currentClientManagerConn->noSegmentAvailable();
    }

}




void SegmentManager::updateDownloadSegmentSlot(SegmentData segmentData){

    // search index
    QStandardItem* fileNameItem = this->searchItem(segmentData.getParentUniqueIdentifier(), DownloadStatus);

    if (fileNameItem != NULL){

        NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
        QList<SegmentData> segmentList = nzbFileData.getSegmentList();

        // segment has been processed, parent identifier can now be removed :
        segmentData.setParentUniqueIdentifier(QString());

        // update the segmentData list :
        segmentList.replace(segmentData.getElementInList(), segmentData);
        nzbFileData.setSegmentList(segmentList);

        // update the nzbFileData of current fileNameItem and its corresponding items :
        this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);

        itemParentUpdater->getItemDownloadUpdater()->updateItems(fileNameItem->index(), nzbFileData);

    } else {
        //kDebug() <<  "ITEM NOT FOUND - status : " << segmentData.getStatus();
    }

}




void SegmentManager::updateDecodeSegmentSlot(QVariant parentIdentifer, int progression, UtilityNamespace::ItemStatus status, QString decodedFileName) {

    // search index in current downloading nzb :
    QStandardItem* fileNameItem = this->searchItem(parentIdentifer, DecodeStatus);

    // if item has not been found the nzb parent item could be in pause state, search it :
    if (fileNameItem == NULL){
        fileNameItem = this->searchItem(parentIdentifer, PauseStatus);
    }

    // item has been found :
    if (fileNameItem != NULL){

        // add info about decoded file type (par2 file or rar file) ;
        this->addFileTypeInfo(fileNameItem, decodedFileName);

        // update items :
        itemParentUpdater->getItemPostDownloadUpdater()->updateItems(fileNameItem->index(), progression, status);

    }
    else {
        kDebug() <<  "Item not found - status : " << status;
    }

}




void SegmentManager::updateRepairExtractSegmentSlot(QVariant parentIdentifer, int progression, UtilityNamespace::ItemStatus status, UtilityNamespace::ItemTarget itemTarget) {

    // search item according to its ID :
    QStandardItem* fileNameItem = this->searchItem(parentIdentifer, RepairStatus);

    if (fileNameItem != NULL){
        // update items :
        itemParentUpdater->getItemPostDownloadUpdater()->updateItems(fileNameItem->index(), progression, status, itemTarget);

    }
    else {
        kDebug() <<  "Item not found - status : " << status;
    }

}

