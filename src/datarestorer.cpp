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
#include <KDebug>

#include "centralwidget.h"
#include "data/segmentdata.h"
#include "data/nzbfiledata.h"
#include "data/itemstatusdata.h"
#include "data/globalfiledata.h"
#include "standarditemmodel.h"
#include "kwootysettings.h"

DataRestorer::DataRestorer(CentralWidget* parent) : QObject (parent) {

    this->parent = parent;
    this->downloadModel = parent->getDownloadModel();

    // enable data restorer :
    this->setActive(true);

    // set a timer to save pending data to be downloaded every 5 minutes :
    dataSaverTimer = new QTimer(this);
    dataSaverTimer->start(5 * UtilityNamespace::MINUTES_TO_MILLISECONDS);


    magicNumber = 0xC82F1D37;

    // should be upgraded everytime a change is done in data stream :
    applicationVersion1 = 004;

    // map kwooty serialization version and its corresponding dataStream version
    versionStreamMap.insert(applicationVersion1, QDataStream::Qt_4_4);

    this->setupConnections();

    // wait 1 second, time to get several nntp instances created, before restoring pending data :
    if (Settings::restoreDownloads()) {
        QTimer::singleShot(500, this, SLOT(readDataFromDiskSlot()));
    }



}


void DataRestorer::setupConnections() {

    connect(dataSaverTimer, SIGNAL(timeout()), this, SLOT(saveQueueDataSilentlySlot()));

}



int DataRestorer::saveQueueData(const bool saveSilently) {

    int answer = KMessageBox::Yes;

    // if option is enabled in settings and is datarestored has not been disabled by shutdown feature :
    if (Settings::restoreDownloads() && this->active)  {

        if (this->isDataToSaveExist()) {

            // ask question if previous pending downloads have to be restored :
            answer = this->displaySaveMessageBox(saveSilently);

            // pendings downloads have to be saved :
            if (answer == KMessageBox::Yes) {

                this->writeDataToDisk();

            }

        }
        // there are no more pending downloads :
        else {
            // get temporary data restore file name :
            QString pendingDataStr = Settings::temporaryFolder().path() + '/' + UtilityNamespace::remainingDownloadsFile;
            // no more pendings jobs, remove eventual previously saved file :
            Utility::removeData(pendingDataStr);
        }

    }

    return answer;


}




void DataRestorer::writeDataToDisk() {

    // global list of items to save :
    QList< QList<GlobalFileData> > nzbFileList;

    // get temporary path :
    QString pendingDataStr = Settings::temporaryFolder().path() + '/' + UtilityNamespace::remainingDownloadsFile;
    QFile file(pendingDataStr);

    // open the file :
    if (!file.open(QIODevice::WriteOnly)) {
        kDebug() << "Cannot open file for writing";
        return;
    }

    // create the dataStream :
    QDataStream dataStreamOut(&file);

    // Write a header with a "magic number" and a version
    dataStreamOut << (quint32)magicNumber;
    dataStreamOut << (quint32)applicationVersion1;
    dataStreamOut.setVersion(versionStreamMap.value(applicationVersion1));


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

    if (!nzbFileList.isEmpty()){
        // Write the data (nzb file and its belonging files and their download status) :
        dataStreamOut << nzbFileList;
    }


    file.close();

}




bool DataRestorer::isHeaderOk(QDataStream& dataStreamIn) const {

    bool headerOk = true;

    quint32 magic;
    quint32 appVersion;

    // get saved magic and number;
    dataStreamIn >> magic >> appVersion;

    if (magic != magicNumber) {
        kDebug() << "file does not belong to this application";
        headerOk = false;
    }


    if (appVersion != applicationVersion1) {
        kDebug() << "temporary file can not be processed (version changed)";
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

    // the current item is being downloaded, set it with default values
    // (in order to set it to Idle during restoring) :
    currentStatusData.init();


    // set corresponding segments being downloaded to Idle, keep the status of the previous downloaded ones :
    QList<SegmentData> segmentDataList = currentNzbFileData.getSegmentList();

    for (int i = 0; i < segmentDataList.size(); i ++) {

        SegmentData currentSegmentData = segmentDataList.at(i);

        // segment is currently being downloaded, set it to IdleStatus in order
        // to download it properly during next session
        // and set segment download progress to 0% :
        if (currentSegmentData.getStatus() != DownloadFinishStatus) {

            currentSegmentData.setStatus(IdleStatus);
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
            if (Utility::isDecoding(itemStatus) ||
                Utility::isWaitingForDecode(itemStatus, currentStatusData.getDataStatus())) {
                this->resetDataForDecodingFile(currentNzbFileData, currentStatusData, currentDownloadProgress);
            }

            // - case 2: if file is currently being downloaded,
            // only reset segments from DownloadStatus to IdleStatus in order to download them again :
            if (Utility::isInDownloadProcess(itemStatus)) {
                this->resetDataForDownloadingFile(currentNzbFileData, currentStatusData);
            }

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

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void DataRestorer::readDataFromDiskSlot() {


    // get temporary path :
    QString pendingDataStr = Settings::temporaryFolder().path() + '/' + UtilityNamespace::remainingDownloadsFile;
    QFile file(pendingDataStr);

    //open file in order to restore prending downloads from previous session :
    if (file.open(QIODevice::ReadOnly)) {

        QDataStream dataStreamIn(&file);

        // check that header retrieved from file is matching :
        if (this->isHeaderOk(dataStreamIn)) {

            // ask question if previous pending downloads have to be restored :
            int answer = this->displayRestoreMessageBox();

            // if data have to be restored :
            if (answer == KMessageBox::Yes) {

                // retrieve saved data :
                QList< QList<GlobalFileData> > nzbFileList;
                dataStreamIn >> nzbFileList;

                // reset some data belonging to pending items and populate treeview :
                this->preprocessAndHandleData(nzbFileList);
            }
            // user did not load download pending files, remove previous segments :
            else {
                this->requestSuppressOldOrphanedSegments();
            }

            // remove processed file :
            file.close();
            file.remove();

        }
        // saved data file can not be processed, remove previous segments
        else {
            this->requestSuppressOldOrphanedSegments();
        }

    }
    // if file can not be opened, remove useless downloaded segments :
    else {
        this->requestSuppressOldOrphanedSegments();
    }

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



void DataRestorer::saveQueueDataSilentlySlot() {

    if (Settings::restoreDownloads()) {

        // if pending downloads remain :
        if (this->isDataToSaveExist()) {

            this->writeDataToDisk();
        }

        else {
            // get temporary data restore file name :
            QString pendingDataStr = Settings::temporaryFolder().path() + '/' + UtilityNamespace::remainingDownloadsFile;
            // no more pendings jobs, remove eventual previously saved file :
            Utility::removeData(pendingDataStr);
        }

    }

}


int DataRestorer::displayRestoreMessageBox() const {

    int answer = KMessageBox::Yes;

    // ask question if confirmRestoreSilently is checked:
    if (Settings::restoreDownloadsMethods() == DataRestorer::WithConfirmation) {

        answer = KMessageBox::messageBox(parent,
                                         KMessageBox::QuestionYesNo,
                                         i18n("Reload pending downloads from previous session ?"));
    }

    return answer;

}


int DataRestorer::displaySaveMessageBox(const bool saveSilently) const {

    int answer = KMessageBox::Yes;

    if (!saveSilently) {

        // ask question if confirmSaveSilently is checked:
        if (Settings::saveDownloadsMethods() == DataRestorer::WithConfirmation) {

            answer = KMessageBox::messageBox(parent,
                                             KMessageBox::QuestionYesNoCancel,
                                             i18n("Save pending downloads from current session ?"));
        }

    }

    return answer;
}


