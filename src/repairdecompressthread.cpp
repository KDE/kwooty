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

#include "repairdecompressthread.h"

#include <KDebug>
#include <KPasswordDialog>
#include <QFileInfo>
#include "centralwidget.h"
#include "repair.h"
#include "extractrar.h"
#include "extractzip.h"
#include "itemparentupdater.h"
#include "segmentmanager.h"
#include "settings.h"


RepairDecompressThread::RepairDecompressThread(){}


RepairDecompressThread::RepairDecompressThread(CentralWidget* inParent) : QThread(inParent) {

    this->parent = inParent;

    // start current thread :
    QTimer::singleShot(0, this, SLOT(start())) ;
}


RepairDecompressThread::~RepairDecompressThread() {

    quit();
    wait();

    delete repairDecompressTimer;
    delete extractRar;
    delete extractZip;
    delete repair;
}


void RepairDecompressThread::run() {

    this->waitForNextProcess = false;

    // create verify/repair instance :
    repair = new Repair();

    // create extract instances :
    extractRar = new ExtractRar(this);
    extractZip = new ExtractZip(this);

    // create incoming data monitoring timer :
    repairDecompressTimer = new QTimer();

    // setup connections :
    this->setupConnections();

    // start timer :
    repairDecompressTimer->start(100);

    this->exec();
}


void RepairDecompressThread::setupConnections() {

    // check if there are data to repair :
    connect (this->repairDecompressTimer ,
             SIGNAL(timeout()),
             this,
             SLOT(processJobSlot()),
             Qt::DirectConnection);


    // receive data to repair and decompress from repairDecompressSignal :
    qRegisterMetaType< QList<NzbFileData> >("QList<NzbFileData>");
    connect (parent->getItemParentUpdater(),
             SIGNAL(repairDecompressSignal(QList<NzbFileData>)),
             this,
             SLOT(repairDecompressSlot(QList<NzbFileData>)),
             Qt::QueuedConnection);


    // send repairing related updates to segmentmanager :
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<UtilityNamespace::ItemTarget>("UtilityNamespace::ItemTarget");
    connect (repair ,
             SIGNAL(updateRepairSignal(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget)),
             this,
             SIGNAL(updateRepairSignal(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget)));

    // begin file extraction when end repair signal has been received :
    connect (repair,
             SIGNAL(repairProcessEndedSignal(QList<NzbFileData>, UtilityNamespace::ItemStatus)),
             this,
             SLOT(repairProcessEndedSlot(QList<NzbFileData>, UtilityNamespace::ItemStatus)));


}



void RepairDecompressThread::extractPasswordRequiredSlot(QString currentArchiveFileName) {

    KPasswordDialog kPasswordDialog(this->parent);

    kPasswordDialog.setPrompt(i18n("The archive <b>%1</b> is password protected. <br>Please enter the password to extract the file.",  currentArchiveFileName));

    // if password has been entered :
    if(kPasswordDialog.exec()) {
        emit passwordEnteredByUserSignal(true, kPasswordDialog.password());
    }
    else {
        // else password has not been entered :
        emit passwordEnteredByUserSignal(false);
    }


}


void RepairDecompressThread::processJobSlot() {

    // pre-process job : group archive volumes together :
    this->processPendingFilesSlot();

    // repair and verify files :
    this->startRepairSlot();

    // extract files :
    this->startExtractSlot();

}



void RepairDecompressThread::startRepairSlot() {

    // if no data is currently being verified :
    if (!this->waitForNextProcess && !this->par2NzbFileDataListMap.isEmpty()) {

        this->waitForNextProcess = true;

        // get nzbfiledata to be verified from list :
        QMap<QString, QList<NzbFileData> >::iterator iterMap = this->par2NzbFileDataListMap.begin();

        // get par2 baseName from key :
        QString par2BaseName = iterMap.key();
        // get associated fileData list from value :
        QList<NzbFileData> currentNzbFileDataList = iterMap.value();

        this->par2NzbFileDataListMap.erase(iterMap);


        // verify data only if par has been found :
        if ( !par2BaseName.isEmpty() && (Settings::groupBoxAutoRepair()) ) {
            repair->launchProcess(currentNzbFileDataList, par2BaseName);
        }
        else {
            this->repairProcessEndedSlot(currentNzbFileDataList, RepairFinishedStatus);
        }

    }


}



void RepairDecompressThread::startExtractSlot() {

    // if no data is currently being verified :
    if (!this->filesToExtractList.isEmpty()) {

        mutex.lock();
        QList<NzbFileData> nzbFileDataListToExtract = filesToExtractList.takeFirst();
        mutex.unlock();

        // extract data :
        if (!nzbFileDataListToExtract.isEmpty() && (Settings::groupBoxAutoDecompress())) {

            // get archive format (rar, zip or 7z) :
            UtilityNamespace::ArchiveFormat archiveFormat = this->getArchiveFormatFromList(nzbFileDataListToExtract);

            // extract with unrar for rar files :
            if (archiveFormat == RarFormat) {
                extractRar->launchProcess(nzbFileDataListToExtract);
            }

            // extract with 7z for zip or 7-zip files :
            if ( (archiveFormat == ZipFormat) ||
                 (archiveFormat == SevenZipFormat) ) {
                extractZip->launchProcess(nzbFileDataListToExtract);
            }

            // if archive format is unknown, abort extracting process :
            if (archiveFormat == UnknownArchiveFormat) {
                this->extractProcessEndedSlot();
            }

        }
        else {
            this->extractProcessEndedSlot();
        }

    }



}


void RepairDecompressThread::repairProcessEndedSlot(QList<NzbFileData> nzbFileDataListToExtract, UtilityNamespace::ItemStatus repairStatus) {

    // if verify/repair ok, launch archive extraction :
    if (repairStatus == RepairFinishedStatus) {
        mutex.lock();
        this->filesToExtractList.append(nzbFileDataListToExtract);
        mutex.unlock();
    }

    // if verify/repair ko, do not launch archive extraction and verify next pending items :
    if (repairStatus == RepairFailedStatus) {
        this->waitForNextProcess = false;
    }

}




void RepairDecompressThread::extractProcessEndedSlot() {

    this->waitForNextProcess = false;

}




void RepairDecompressThread::repairDecompressSlot(QList<NzbFileData> nzbFileDataList) {

    // all files have been downloaded and decoded, set them in the pending list
    // in order to process them :
    mutex.lock();
    this->filesToProcessList.append(nzbFileDataList);
    mutex.unlock();

}



void RepairDecompressThread::processPendingFilesSlot() {

    if (!filesToProcessList.isEmpty()) {

        // get nzbFileDataList to verify and repair :
        mutex.lock();
        QList<NzbFileData> nzbFileDataList = this->filesToProcessList.takeFirst();
        mutex.unlock();


        // if the list contains rar files belonging to one group :
        if (!this->isListContainsdifferentGroups(nzbFileDataList)) {

            this->processRarFilesFromSameGroup(nzbFileDataList);

        }
        else {
            // get each baseName list :
            QStringList fileBaseNameList = this->listDifferentFileBaseName(nzbFileDataList);

            // group files together according to their names in order to verify them :
            this->processRarFilesFromDifferentGroups(fileBaseNameList, nzbFileDataList);
        }

    }

}






bool RepairDecompressThread::isListContainsdifferentGroups(const QList<NzbFileData>& nzbFileDataList) {

    bool containsDifferentGroups = true;
    QSet<QString> baseNamePar2Set;
    QSet<QString> baseNameRarSet;

    foreach (NzbFileData nzbFileData, nzbFileDataList) {

        if (nzbFileData.isPar2File()) {
            QString parBaseName = this->getBaseNameFromPar2(nzbFileData);
            baseNamePar2Set.insert(parBaseName);

        }
        if (nzbFileData.isArchiveFile()) {
            QString rarBaseName = this->getBaseNameFromRar(nzbFileData);
            baseNameRarSet.insert(rarBaseName);
        }

    }

    // The sets will contain all different base names founds for the nzb group
    // if sets are equal to one means that files belonging to nzb group are not mixed with other files.
    // There sets are separated because parSet could have been renamed and thought does not have the same base
    // name as rarSet :
    if ( (baseNamePar2Set.size() == 1) &&
         (baseNameRarSet.size() == 1) ) {

        containsDifferentGroups = false;

    }


    return containsDifferentGroups;
}



QStringList RepairDecompressThread::listDifferentFileBaseName(QList<NzbFileData>& nzbFileDataList) {

    // at this step nzbFileData list contains archives without par2 files,
    // group archive files together :
    QStringList fileBaseNameList;

    for (int i =0; i < nzbFileDataList.size(); i++) {

        NzbFileData nzbFileData = nzbFileDataList.at(i);

        QString fileBaseName;
        if (nzbFileData.isArchiveFile()) {
            fileBaseName = this->getBaseNameFromRar(nzbFileData);
        }

        else if (nzbFileData.isPar2File()) {
            fileBaseName = this->getBaseNameFromPar2(nzbFileData);
        }

        // update base name to nzbFileData :
        nzbFileData.setBaseName(fileBaseName);
        nzbFileDataList.replace(i, nzbFileData);

        // add rarBaseName to the list :
        if ( (!fileBaseName.isEmpty()) &&
             (!fileBaseNameList.contains(fileBaseName)) ) {

            fileBaseNameList.append(fileBaseName);

        }

    }


    return fileBaseNameList;
}



QString RepairDecompressThread::getBaseNameFromPar2(const NzbFileData& nzbFileData) {

    QString fileBaseName;

    QString par2FileWithoutExtension = nzbFileData.getDecodedFileName();
    // remove .par2 extension :
    par2FileWithoutExtension.chop(par2FileExt.size());

    // supress .volxxx-+xxx extension :
    if (par2FileWithoutExtension.contains("vol")) {

        QRegExp regExp("(.*)(\\.vol\\d+.\\d+)");
        regExp.setCaseSensitivity(Qt::CaseInsensitive);
        if (regExp.exactMatch(par2FileWithoutExtension)) {
            fileBaseName = regExp.cap(1);
        }
    }
    // par2 file does not contains .volxxx-+xxx extension :
    else{
        fileBaseName = par2FileWithoutExtension;
    }

    return fileBaseName;
}


QString RepairDecompressThread::getBaseNameFromRar(const NzbFileData& nzbFileData) {


    QString decodedFileName = nzbFileData.getDecodedFileName();

    // Archive name could be {baseName}.r** or {baseName}.part***.rar
    // determine baseName to group rar files (without associated par2) together :
    QFileInfo fileInfo(decodedFileName);

    // remove first extension .r** or .rar:
    QString fileBaseName = fileInfo.completeBaseName();

    // check if part*** is contained in baseName :
    fileInfo.setFile(fileBaseName);
    QString fileSuffix = fileInfo.suffix();

    if (fileSuffix.contains(QRegExp("part\\d+", Qt::CaseInsensitive))) {
        // remove .part*** extension :
        fileBaseName = fileInfo.completeBaseName();

    }

    return fileBaseName;
}




NzbFileData RepairDecompressThread::tryToGuessDecodedFileName(NzbFileData& targetNzbFileData, const QList<NzbFileData>& nzbFileDataList, const QString& fileBaseName) {

    QString builtFileName;

    foreach(NzbFileData currentNzbFileData, nzbFileDataList) {

        // search a decoded file name containing the rarbase name pattern :
        if (currentNzbFileData.isArchiveFile() && currentNzbFileData.getDecodedFileName().contains(fileBaseName)){

            int fileNamePos = targetNzbFileData.getFileName().indexOf(fileBaseName);
            if (fileNamePos > -1 ) {

                builtFileName = targetNzbFileData.getFileName().mid(fileNamePos, currentNzbFileData.getDecodedFileName().size());

                if (!builtFileName.isEmpty()) {

                    targetNzbFileData.setDecodedFileName(builtFileName);
                    targetNzbFileData.setBaseName(fileBaseName);

                    break;
                }

            }

        }

    }

    //kDebug() << "tryToGuessDecodedFileName from : " << targetNzbFileData.getFileName() << "BUILT NAME : " << builtFileName;
    return targetNzbFileData;

}





void RepairDecompressThread::processRarFilesFromDifferentGroups(const QStringList& fileBaseNameList, const QList<NzbFileData>& nzbFileDataList) {

    // group archives that own par2 files for verifying/repairing :
    if (!fileBaseNameList.isEmpty()) {

        foreach(QString fileBaseName, fileBaseNameList) {

            QList<NzbFileData> groupedFileList;
            QString par2BaseName;

            foreach(NzbFileData nzbFileData, nzbFileDataList) {

                // the file is missing if the decoded file name is empty :
                if (nzbFileData.getDecodedFileName().isEmpty()  && !nzbFileData.isPar2File()) {
                    // in that case, try to build the decoded file name :
                    nzbFileData = this->tryToGuessDecodedFileName(nzbFileData, nzbFileDataList, fileBaseName);

                }

                // trying to get the most generic discriminant according to rar file base names and
                // par2 file base names that could not exactly match :
                if ( (!nzbFileData.isPar2File() && nzbFileData.getDecodedFileName().contains(fileBaseName) ) ||
                     (nzbFileData.isPar2File() && (nzbFileData.getBaseName() == fileBaseName)) ) {
                    //if (nzbFileData.getDecodedFileName().contains(fileBaseName)){

                    // group nzbFileData with the same baseName :
                    groupedFileList.append(nzbFileData);

                    // if par2Files holding base name have been found, set par2BaseName :
                    if (nzbFileData.isPar2File()) {
                        par2BaseName = fileBaseName + "*";
                    }
                }

            }


            //kDebug() << "par2BaseName :" << par2BaseName << "groupedFileList.size() : " << groupedFileList.size();

            // if par2BaseName is not empty, verify/repair will be proceeded,
            // if par2BaseName is empty, extraction will be done directly :
            if (!groupedFileList.isEmpty()) {
                this->par2NzbFileDataListMap.insertMulti(par2BaseName, groupedFileList);
            }
        }

    }

}


void RepairDecompressThread::processRarFilesFromSameGroup(const QList<NzbFileData>& nzbFileDataList) {

    // get file base name for rar file :
    QString rarBaseName;
    foreach (NzbFileData nzbFileData, nzbFileDataList) {

        if (nzbFileData.isArchiveFile()) {
            rarBaseName = this->getBaseNameFromRar(nzbFileData);
            break;
        }
    }


    // set decodedFileName for missing files :
    QList<NzbFileData> groupedFileList;
    foreach (NzbFileData nzbFileData, nzbFileDataList) {

        // the file is missing if the decoded file name is empty :
        if (nzbFileData.getDecodedFileName().isEmpty()  && !nzbFileData.isPar2File()) {

            // in that case, try to build the decoded file name :
            nzbFileData = this->tryToGuessDecodedFileName(nzbFileData, nzbFileDataList, rarBaseName);

        }

        // decoded file name has been built or already exists, add it to list:
        if (!nzbFileData.getDecodedFileName().isEmpty()) {
            groupedFileList.append(nzbFileData);
        }
    }

    // in this case, par2Base name is not initialized in order to get only "*.*" as par2 argument during verifiying process :
    if (!groupedFileList.isEmpty()) {
        QString par2BaseName = "*.*";
        this->par2NzbFileDataListMap.insertMulti(par2BaseName, groupedFileList);
    }

}


UtilityNamespace::ArchiveFormat RepairDecompressThread::getArchiveFormatFromList(const QList<NzbFileData>& nzbFileDataListToExtract) {

    UtilityNamespace::ArchiveFormat archiveFormat = UnknownArchiveFormat;

    foreach (NzbFileData nzbFileData, nzbFileDataListToExtract) {

        // files are grouped with unique archive type, get corresponding archive format :
        if ( nzbFileData.isArchiveFile() &&
             (nzbFileData.getArchiveFormat() != UtilityNamespace::UnknownArchiveFormat) ) {

            archiveFormat = nzbFileData.getArchiveFormat();
            break;
        }

    }


    return archiveFormat;

}

