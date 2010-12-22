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
#include "servergroup.h"
#include "data/segmentdata.h"
#include "data/segmentinfodata.h"
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



bool SegmentManager::sendNextIdleSegment(QStandardItem* fileNameItem, ClientManagerConn* currentClientManagerConn, const SegmentInfoData& segmentInfoData){

    bool itemFound = false;

    // look for an item to download :
    NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
    QList<SegmentData> segmentList = nzbFileData.getSegmentList();

    // parse segment list to find the next idle segment :
    for (int segmentIndex = 0; segmentIndex < segmentList.size(); segmentIndex++) {

        SegmentData segmentData = segmentList.value(segmentIndex);

        int serverGroupId = currentClientManagerConn->getServerGroup()->getServerGroupId();

        // filter Idle segments searching according to corresponding serverGroup id :
        if ( !itemFound &&
             (segmentData.getStatus() == IdleStatus) &&
             (segmentData.getServerGroupTarget() == serverGroupId) ) {

            itemFound = true;

            // next idle status has been found, set it to download status :
            segmentData.setStatus(DownloadStatus);

            // reset elements in list (may fix elements out of sync rare issues ?) :
            segmentData.setElementInList(segmentIndex);

            // update parent item data :
            segmentList.replace(segmentIndex, segmentData);
            nzbFileData.setSegmentList(segmentList);
            this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);

            itemParentUpdater->getItemDownloadUpdater()->updateItems(fileNameItem->index(), nzbFileData);

            // set parent idenfier to segment before downloading it  :
            segmentData.setParentUniqueIdentifier(nzbFileData.getUniqueIdentifier());
            segmentData.setElementInList(segmentIndex);

            // set segment info data (including nzb file name and row position)
            // in order to notify sidebar of downloaded files per server :
            segmentData.setSegmentInfoData(segmentInfoData);

            // send the next part to the dedicated client :
            currentClientManagerConn->processNextSegment(segmentData);

            break;
        }

    } // end of for loop


    return itemFound;

}



void SegmentManager::setIdlePauseSegments(QStandardItem* fileNameItem, const UtilityNamespace::ItemStatus targetStatus){

    // check that the nzb status item is either Idle, Download or pausing / paused :
    QStandardItem* nzbStatusItem = this->downloadModel->getStateItemFromIndex(fileNameItem->index());
    UtilityNamespace::ItemStatus nzbStatus = this->downloadModel->getStatusFromStateItem(nzbStatusItem);

    // Set to pause / idle only items that are in download process (eg : not the ones decoded or verified...) :
    if (Utility::isInDownloadProcess(nzbStatus)) {

        // set segment with Idle back to Pause status and vice-versa :
        NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
        QList<SegmentData> segmentList = nzbFileData.getSegmentList();

        for (int segmentIndex = 0; segmentIndex < segmentList.size(); segmentIndex++) {

            SegmentData currentSegment = segmentList.value(segmentIndex);

            if (currentSegment.getStatus() == IdleStatus && targetStatus == PauseStatus) {
                currentSegment.setStatus(targetStatus);

            }
            if (currentSegment.getStatus() == PauseStatus && targetStatus == IdleStatus) {
                currentSegment.setStatus(targetStatus);
            }

            segmentList.replace(segmentIndex, currentSegment);

        }


        // update the nzbFileData of current fileNameItem and its corresponding items;
        nzbFileData.setSegmentList(segmentList);
        this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);


        itemParentUpdater->getItemDownloadUpdater()->updateItems(fileNameItem->index(), nzbFileData);
    }
}







QStandardItem* SegmentManager::searchItem(const QVariant& parentIdentifer, const UtilityNamespace::ItemStatus itemStatus){

    // init fileNameItem :
    QStandardItem* fileNameItem = 0;

    bool itemUpdated = false;
    QModelIndex parentModelIndex;
    QModelIndex nzbFileModelIndex;


    // search the unique identifier modelIndex that match with the segmentData's one for updating it :
    if ( (this->downloadModel->rowCount() > 0) &&
         (parentIdentifer != QVariant()) ) {

        int row = 0;

        while ((row < this->downloadModel->rowCount()) && !itemUpdated) {

            //retrieve root item (nzb item) :
            nzbFileModelIndex = this->downloadModel->item(row, FILE_NAME_COLUMN)->index();

            QStandardItem* nzbStatusItem = this->downloadModel->item(row, STATE_COLUMN);

            // before processing searches, check that the nzb status item is either Idle, Download or in being paused :
            UtilityNamespace::ItemStatus nzbStatus = this->downloadModel->getStatusFromStateItem(nzbStatusItem);


            // determine search in nzb parent item according to its status :
            bool isSearchInNzb = false;

            // search item according to parent nzb parent status :
            switch (itemStatus) {

            case DownloadStatus: {
                    if (Utility::isReadyToDownload(nzbStatus) ||
                        Utility::isPausing(nzbStatus)) {
                        isSearchInNzb = true;
                    }
                    break;
                }
            case DecodeStatus: {
                    if (Utility::isReadyToDownload(nzbStatus) ||
                        Utility::isDownloadFinish(nzbStatus) ||
                        Utility::isPausing(nzbStatus)  ||
                        Utility::isDecoding(nzbStatus)) {
                        isSearchInNzb = true;
                    }
                    break;
                }
            case RepairStatus: case ExtractStatus: {
                    isSearchInNzb = true;
                    break;
                }
            case PauseStatus: {
                    isSearchInNzb = Utility::isPaused(nzbStatus);
                    break;
                }
            default: {
                    break;
                }
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
                    UtilityNamespace::ItemStatus currentChildStatus = this->downloadModel->getStatusFromStateItem(stateItem);

                    // multi-server purpose :
                    // check if all segments have already been scanned by the same server,
                    // otherwise skip the current item and switch no next one for getting a next segment :
                    int currentServerId = currentClientManagerConn->getServerGroup()->getServerGroupId();

                    ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();

                    if (Utility::isReadyToDownload(currentChildStatus) &&
                        (currentServerId >= itemStatusData.getNextServerId()) ) {

                        SegmentInfoData segmentInfoData(nzbItem->text(), nzbItem->row());
                        itemFound = this->sendNextIdleSegment(fileNameItem, currentClientManagerConn, segmentInfoData);
                        
                    }
                }
            }
        }
        row++;
    } // end of loop

    //notify the client that no segment has been found to return to idle status :
    if (!itemFound) {
        currentClientManagerConn->noSegmentAvailable();
    };

}



void SegmentManager::updateDownloadSegmentSlot(SegmentData segmentData) {

    // search index
    QStandardItem* fileNameItem = this->searchItem(segmentData.getParentUniqueIdentifier(), DownloadStatus);

    // if corresponding item has been found :
    if (fileNameItem) {

        NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
        QList<SegmentData> segmentList = nzbFileData.getSegmentList();        

        SegmentData previousSegmentData = segmentList.value(segmentData.getElementInList());

        // update segment only if it has not already be downloaded :
        if ( (previousSegmentData.getStatus() <= PausingStatus) &&
             (segmentList.size() > segmentData.getElementInList()) ) {

            // segment has been processed, parent identifier can now be removed :
            segmentData.setParentUniqueIdentifier(QVariant());
            segmentData.setSegmentInfoData(SegmentInfoData());

            // update the segmentData list :
            segmentList.replace(segmentData.getElementInList(), segmentData);
            nzbFileData.setSegmentList(segmentList);

            // update the nzbFileData of current fileNameItem and its corresponding items :
            this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);

            itemParentUpdater->getItemDownloadUpdater()->updateItems(fileNameItem->index(), nzbFileData);
        }
        else {
            kDebug() << "ooops, something goes really wrong :" << previousSegmentData.getStatus() << segmentList.size() << segmentData.getElementInList();
        }

    }
    else {
        //kDebug() <<  "Item not found - status : " << segmentData.getStatus();
    }

}




void SegmentManager::updateDecodeSegmentSlot(QVariant parentIdentifer, int progression, UtilityNamespace::ItemStatus status, QString decodedFileName, bool crc32Match, UtilityNamespace::ArticleEncodingType articleEncodingType) {

    // search index in current downloading nzb :
    QStandardItem* fileNameItem = this->searchItem(parentIdentifer, DecodeStatus);

    // if item has not been found the nzb parent item could be in pause state, search it :
    if (!fileNameItem){
        fileNameItem = this->searchItem(parentIdentifer, PauseStatus);
    }

    // item has been found :
    if (fileNameItem){

        // add info about decoded file type (par2 file or rar file) ;
        itemParentUpdater->getItemPostDownloadUpdater()->addFileTypeInfo(fileNameItem, decodedFileName, crc32Match, articleEncodingType);

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

    // if corresponding item has been found :
    if (fileNameItem){
        // update items :
        itemParentUpdater->getItemPostDownloadUpdater()->updateItems(fileNameItem->index(), progression, status, itemTarget);

    }
    else {
        kDebug() <<  "Item not found - status : " << status;
    }

}






void SegmentManager::updatePendingSegmentsToTargetServer(const int& currentServerGroup, const int& nextServerGroup, const PendingSegments pendingSegments) {


    for (int i = 0; i < this->downloadModel->rowCount(); i++) {

        QStandardItem* nzbItem = this->downloadModel->getFileNameItemFromRowNumber(i);
        UtilityNamespace::ItemStatus currentStatus = this->downloadModel->getStatusDataFromIndex(nzbItem->index()).getStatus();

        // if nzb item is currently being downloaded :
        if (Utility::isInDownloadProcess(currentStatus)) {

            for (int j = 0; j < nzbItem->rowCount(); j++) {

                QStandardItem* childFileNameItem = nzbItem->child(j, FILE_NAME_COLUMN);
                UtilityNamespace::ItemStatus childStatus = this->downloadModel->getChildStatusFromNzbIndex(nzbItem->index(), j);

                // if the current file is pending or being downloaded :
                if (Utility::isInDownloadProcess(childStatus)) {

                    NzbFileData nzbFileData = childFileNameItem->data(NzbFileDataRole).value<NzbFileData>();
                    QList<SegmentData> segmentList = nzbFileData.getSegmentList();

                    bool listUpdated = false;

                    for (int segmentIndex = 0; segmentIndex < segmentList.size(); segmentIndex++) {

                        SegmentData currentSegment = segmentList.value(segmentIndex);

                        // update pending segments to a new backup server target :
                        if (pendingSegments == UpdateSegments) {

                            if (Utility::isInDownloadProcess(currentSegment.getStatus()) &&
                                currentSegment.getServerGroupTarget() == currentServerGroup) {

                                // if another backup server is available, set it ready to be downloaded with :
                                if (nextServerGroup != NoTargetServer) {
                                    currentSegment.setReadyForNewServer(nextServerGroup);
                                }

                                // there is no backup server, this segment cannot be downloaded :
                                else if (nextServerGroup == NoTargetServer) {
                                    currentSegment.setDownloadFinished(NotPresent);
                                }

                                // update segment list :
                                segmentList.replace(segmentIndex, currentSegment);
                                listUpdated = true;

                            }

                        }
                        // reset pending segments to master server target :
                        else if (pendingSegments == ResetSegments) {

                            if (Utility::isInDownloadProcess(currentSegment.getStatus())) {

                                // reset pending segments with master server as target :
                                currentSegment.setReadyForNewServer(MasterServer);

                                // update segment list :
                                segmentList.replace(segmentIndex, currentSegment);
                                listUpdated = true;
                            }

                        }

                    }


                    if (listUpdated) {
                        // update the nzbFileData of current fileNameItem and its corresponding items;
                        nzbFileData.setSegmentList(segmentList);
                        this->downloadModel->updateNzbFileDataToItem(childFileNameItem, nzbFileData);

                        itemParentUpdater->getItemDownloadUpdater()->updateItems(childFileNameItem->index(), nzbFileData);
                    }

                }

            }

        }

    }

}


