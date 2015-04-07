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

#include "datarestorer.h"

#include <KMessageBox>
#include "kwooty_debug.h"

#include <QUuid>

#include "core.h"
#include "mainwindow.h"
#include "itemparentupdater.h"
#include "standarditemmodel.h"
#include "widgets/centralwidget.h"
#include "kwootysettings.h"
#include "data/segmentdata.h"
#include "data/nzbfiledata.h"
#include "data/itemstatusdata.h"
#include "data/globalfiledata.h"


DataRestorer::DataRestorer(Core* parent) : QObject (parent) {

    this->parent = parent;
    this->downloadModel = parent->getDownloadModel();

    // enable data restorer :
    this->setActive(true);

    // set a timer to save pending data to be downloaded every 5 minutes :
    dataSaverTimer = new QTimer(this);
    dataSaverTimer->start(5 * UtilityNamespace::MINUTES_TO_MILLISECONDS);


    magicNumber = 0xC82F1D37;

    // should be upgraded everytime a change is done in data stream :
    applicationVersion1 = 9;

    // map kwooty serialization version and its corresponding dataStream version
    versionStreamMap.insert(applicationVersion1, QDataStream::Qt_4_4);

    this->setupConnections();

}


void DataRestorer::setupConnections() {

    // initialisation is finished, restore pending data from previous session :
    connect(this->parent->getMainWindow(),
            SIGNAL(startupCompleteSignal()),
            this,
            SLOT(readDataFromDiskSlot()));

    // save data queue every 5 minutes :
    connect(dataSaverTimer,
            SIGNAL(timeout()),
            this,
            SLOT(saveQueueDataSilentlySlot()));

    // download is finish for one nzb file, save eventual remaining downloads now :
    connect(this->downloadModel,
            SIGNAL(parentStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
            this,
            SLOT(parentStatusItemChangedSlot(QStandardItem*, ItemStatusData)));


}



int DataRestorer::saveQueueData(const SaveFileBehavior& saveFileBehavior) {

    int answer = KMessageBox::Yes;

    // if option is enabled in settings and is datarestored has not been disabled by shutdown feature :
    if (Settings::restoreDownloads() && this->active)  {

        if (this->isDataToSaveExist()) {

            // ask question if previous pending downloads have to be restored :
            answer = this->parent->getCentralWidget()->displaySaveMessageBox(saveFileBehavior);

            // pendings downloads have to be saved :
            if (answer == KMessageBox::Yes) {

                this->writeDataToDisk();

            }
            // else do not save pending downloads *and* remove previous pending downloads file :
            else {
                this->removePendingDataFile();
            }

        }
        // there are no more pending downloads :
        else {
            this->removePendingDataFile();
        }

    }

    // option is unchecked in settings, be sure to remove the file :
    if (!Settings::restoreDownloads()) {

        this->removePendingDataFile();
    }

    return answer;


}


void DataRestorer::removePendingDataFile() {

    // no more pendings jobs, remove eventual previously saved file :
    Utility::removeData(this->getPendingFileStr());

}



void DataRestorer::writeDataToDisk() {

    // global list of items to save :
    QList< QList<GlobalFileData> > nzbFileList;


    // get nzb items whose download is not complete :
    for (int i = 0; i < downloadModel->rowCount(); i++) {

        QList<GlobalFileData> globalFileDataList;

        // retrieve nzb parent item :
        QStandardItem* parentFileNameItem = this->downloadModel->item(i, FILE_NAME_COLUMN);

        // get status of the parent item (the nzb file) :
        ItemStatusData parentStatusData = this->downloadModel->getStatusDataFromIndex(parentFileNameItem->index());
        UtilityNamespace::ItemStatus itemParentStatus = parentStatusData.getStatus();

        // save only files with download process not complete :
        if (Utility::isInDownloadProcess(itemParentStatus)) {

            // retrieve NzbFileData  from nzb parent item in order to save them :
            for (int j = 0; j < parentFileNameItem->rowCount(); j++) {

                QModelIndex fileNameIndex = parentFileNameItem->child(j, FILE_NAME_COLUMN)->index();

                // get nzbFileData, progress value and status data related to a file in order to save them :
                NzbFileData currentNzbFileData = this->downloadModel->getNzbFileDataFromIndex(fileNameIndex);
                ItemStatusData currentStatusData = this->downloadModel->getStatusDataFromIndex(fileNameIndex);
                int currentDownloadProgress = this->downloadModel->getProgressValueFromIndex(fileNameIndex);

                //append to list :
                globalFileDataList.append(GlobalFileData(currentNzbFileData, currentStatusData, currentDownloadProgress));


            }

            //append to files list belonging to the nzb :
            if (!globalFileDataList.isEmpty()) {
                nzbFileList.append(globalFileDataList);
            }

        }

    }

    if (!nzbFileList.isEmpty()) {

        QFile file(this->getPendingFileStr());

        // open the file :
        if (!file.open(QIODevice::WriteOnly)) {
            qCDebug(KWOOTY_LOG) << "Cannot open file for writing";
            return;
        }

        // create the dataStream :
        QDataStream dataStreamOut(&file);

        // write a header with a "magic number" and a version
        dataStreamOut << (quint32)magicNumber;
        dataStreamOut << (quint32)applicationVersion1;
        dataStreamOut.setVersion(versionStreamMap.value(applicationVersion1));

        // compute data checksum and write it :
        QByteArray byteArray;
        QDataStream dataStreamByteArray(&byteArray, QIODevice::ReadWrite);
        dataStreamByteArray << nzbFileList;

        dataStreamOut << qChecksum(byteArray.data(), byteArray.size());

        // write the data (nzb file and its belonging files and their download status) :
        file.write(byteArray);
        file.close();


    }




}


bool DataRestorer::isHeaderOk(QDataStream& dataStreamIn) const {

    bool headerOk = true;

    quint32 magic;
    quint32 appVersion;

    // get saved magic and number;
    dataStreamIn >> magic >> appVersion;

    if (magic != magicNumber) {
        qCDebug(KWOOTY_LOG) << "file does not belong to this application";
        headerOk = false;
    }


    if (appVersion != applicationVersion1) {
        qCDebug(KWOOTY_LOG) << "temporary file can not be processed (version changed)";
        headerOk = false;
    }
    else {
        dataStreamIn.setVersion(versionStreamMap.value(applicationVersion1));
    }


    return headerOk;
}




void DataRestorer::resetDataForDecodingFile(NzbFileData& currentNzbFileData, ItemStatusData& currentStatusData, int& currentDownloadProgress) {

    // set currentStatusData to default values :
    currentStatusData.init();

    // set file download progress to 0%
    currentDownloadProgress = 0;

    // set status of all segments to IdleStatus in order to download them again,
    // and set segment download progress to 0% :
    QList<SegmentData> segmentDataList = currentNzbFileData.getSegmentList();
    for (int i = 0; i < segmentDataList.size(); i ++) {

        SegmentData currentSegmentData = segmentDataList.at(i);

        currentSegmentData.setStatus(IdleStatus);
        currentSegmentData.setProgress(PROGRESS_INIT);

        segmentDataList.replace(i, currentSegmentData);

    }
    // update segment list :
    currentNzbFileData.setSegmentList(segmentDataList);


}


void DataRestorer::resetDataForDownloadingFile(NzbFileData& currentNzbFileData, ItemStatusData& currentStatusData) {

    // retrieve parent status :
    UtilityNamespace::ItemStatus parentItemStatus = currentStatusData.getStatus();

    // the current item is being downloaded, set it with default values
    // (in order to set it to Idle during restoring) :
    currentStatusData.init();

    // check if parent status was previously set on pause :
    if (Utility::isPaused(parentItemStatus)) {
        currentStatusData.setStatus(PauseStatus);
    }


    // set corresponding segments being downloaded to Idle, keep the status of the previous downloaded ones :
    QList<SegmentData> segmentDataList = currentNzbFileData.getSegmentList();

    for (int i = 0; i < segmentDataList.size(); i ++) {

        SegmentData currentSegmentData = segmentDataList.at(i);

        // segment is currently being downloaded, set it to IdleStatus in order
        // to download it properly during next session
        // and set segment download progress to 0% :
        if (currentSegmentData.getStatus() != DownloadFinishStatus) {

            currentSegmentData.setStatus(IdleStatus);

            // if parent status was previously set on pause, set also current segment on pause :
            if (Utility::isPaused(parentItemStatus)) {
                currentSegmentData.setStatus(PauseStatus);
            }

            currentSegmentData.setProgress(PROGRESS_INIT);

            segmentDataList.replace(i, currentSegmentData);

        }

    }

    // update segment list :
    currentNzbFileData.setSegmentList(segmentDataList);
}




void DataRestorer::preprocessAndHandleData(const QList< QList<GlobalFileData> >& nzbFileList) {

    // get nzb items whose download is not complete :
    for (int i = 0; i < nzbFileList.size(); i++) {

        QList<GlobalFileData> globalFileDataList = nzbFileList.at(i);

        // retrieve NzbFileData  from nzb parent item in order to save them :
        for (int j = 0; j < globalFileDataList.size(); j++) {

            GlobalFileData currentGlobalFileData = globalFileDataList.at(j);

            // get nzbFileData :
            NzbFileData currentNzbFileData = currentGlobalFileData.getNzbFileData();

            // get progress value :
            int currentDownloadProgress = currentGlobalFileData.getProgressValue();

            // get status data :
            ItemStatusData currentStatusData = currentGlobalFileData.getItemStatusData();
            UtilityNamespace::ItemStatus itemStatus = currentStatusData.getStatus();


            // - case 1: if file is currently being decoded, file shall be downloaded again next time :
            if ( Utility::isDecoding(itemStatus) ||
                 Utility::isWaitingForDecode(itemStatus, currentStatusData.getDataStatus()) ) {
                this->resetDataForDecodingFile(currentNzbFileData, currentStatusData, currentDownloadProgress);
            }

            // - case 2: if file is currently being downloaded,
            // only reset segments from DownloadStatus to IdleStatus in order to download them again :
            if (Utility::isInDownloadProcess(itemStatus)) {
                this->resetDataForDownloadingFile(currentNzbFileData, currentStatusData);
            }

            // set a new uuid during restoring (fix elements out of sync rare issues due to duplicate uuid ??) :
            currentNzbFileData.setUniqueIdentifier(QVariant(QUuid::createUuid().toString()));

            // update status :
            currentGlobalFileData.setNzbFileData(currentNzbFileData);
            currentGlobalFileData.setItemStatusData(currentStatusData);
            currentGlobalFileData.setProgressValue(currentDownloadProgress);


            //update global data to the list :
            globalFileDataList.replace(j, currentGlobalFileData);

        }

        if (!globalFileDataList.isEmpty()) {

            // restore saved data in model and populate treeview :
            parent->restoreDataFromPreviousSession(globalFileDataList);

        }

    }

}



bool DataRestorer::isDataToSaveExist() const {

    bool dataToSaveExist = false;

    for (int i = 0; i < downloadModel->rowCount(); i++) {

        // retrieve nzb parent item :
        QStandardItem* parentFileNameItem = this->downloadModel->item(i, FILE_NAME_COLUMN);

        // get status of the parent item (the nzb file) :
        ItemStatusData parentStatusData = this->downloadModel->getStatusDataFromIndex(parentFileNameItem->index());
        UtilityNamespace::ItemStatus itemParentStatus = parentStatusData.getStatus();

        // check if some data may be saved :
        if (Utility::isInDownloadProcess(itemParentStatus)) {
            dataToSaveExist = true;

        }

    }

    return dataToSaveExist;
}


void DataRestorer::setActive(const bool active) {
    this->active = active;
}


QString DataRestorer::getPendingFileStr() const {
    return Utility::buildFullPath(Settings::temporaryFolder().path(), UtilityNamespace::remainingDownloadsFile);
}



void DataRestorer::requestSuppressOldOrphanedSegments() {

    // kwooty could just have been launched from another app (with "open with...") at this stage
    // so check that nothing new is being downloading before removing old segments :
    QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

    // just check that model is empty :
    if (rootItem->rowCount() == 0) {
        emit suppressOldOrphanedSegmentsSignal();
    }

}






//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void DataRestorer::readDataFromDiskSlot() {

    bool removeOldFiles = true;

    if (Settings::restoreDownloads()) {

        // get temporary path :
        QFile file(this->getPendingFileStr());

        //open file in order to restore prending downloads from previous session :
        if (file.open(QIODevice::ReadOnly)) {

            QDataStream dataStreamIn(&file);

            // check that header retrieved from file is matching :
            if (this->isHeaderOk(dataStreamIn)) {

                // ask question if previous pending downloads have to be restored :
                int answer = this->parent->getCentralWidget()->displayRestoreMessageBox();

                // if data have to be restored :
                if (answer == KMessageBox::Yes) {

                    // retrieve saved checksum :
                    quint16 checksumFromFile = 0;
                    dataStreamIn >> checksumFromFile;

                    // retrieve saved data :
                    QList< QList<GlobalFileData> > nzbFileList;
                    dataStreamIn >> nzbFileList;

                    // verify checksum :
                    QByteArray byteArray;
                    QDataStream dataStreamByteArray(&byteArray, QIODevice::ReadWrite);
                    dataStreamByteArray << nzbFileList;
                    quint16 checksumCompute = qChecksum(byteArray.data(), byteArray.size());

                    if (checksumFromFile == checksumCompute) {

                        removeOldFiles = false;
                        // reset some data belonging to pending items and populate treeview :
                        this->preprocessAndHandleData(nzbFileList);

                    }
                    else {
                        qCDebug(KWOOTY_LOG) << "data can not be restored, checksum ko !!!" << checksumFromFile << checksumCompute;
                    }

                }

            }

        }

        // close the file :
        file.close();

    }

    if (removeOldFiles) {
        this->requestSuppressOldOrphanedSegments();
    }


}


void DataRestorer::parentStatusItemChangedSlot(QStandardItem*, ItemStatusData parentItemStatusData) {

    // file download has just finished, be sure that it will not be reloaded
    // at the next kwooty session :
    if (Utility::isDecodeFinish(parentItemStatusData.getStatus())) {

        // remove previous .dat file :
        QFile::remove(this->getPendingFileStr());

        // check if a new one shall be created if any other pending downloads :
        this->writeDataToDisk();

    }

}



void DataRestorer::saveQueueDataSilentlySlot() {

    if (Settings::restoreDownloads()) {

        // if pending downloads remain :
        if (this->isDataToSaveExist()) {

            this->writeDataToDisk();
        }

        else {

            this->removePendingDataFile();

        }

    }

}

