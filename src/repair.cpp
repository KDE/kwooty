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

#include "repair.h"

#include <KDebug>
#include <KStandardDirs>
#include <QFile>
#include "kwootysettings.h"
#include "repairdecompressthread.h"
#include "data/nzbfiledata.h"
#include "data/postdownloadinfodata.h"

Repair::Repair(RepairDecompressThread* parent) : QObject(parent) {

    this->parent = parent;

    this->repairProcess = new KProcess(this);
    this->setupConnections();
    this->resetVariables();

    // init file status - enum status map :
    statusEnumMap.insert("found", VerifyFoundStatus);
    statusEnumMap.insert("damaged", VerifyDamagedStatus);
    statusEnumMap.insert("missing", VerifyMissingStatus);
    statusEnumMap.insert("is a match for", VerifyMatchStatus);

}

Repair::~Repair() {
    this->repairProcess->close();
}


void Repair::setupConnections() {

    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    connect (this->repairProcess, SIGNAL(readyRead()), this, SLOT(repairReadyReadSlot()));
    connect (this->repairProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(repairFinishedSlot(int, QProcess::ExitStatus)));

}



void Repair::launchProcess(const NzbCollectionData& nzbCollectionData){

    // clear global variables :
    this->resetVariables();
    this->nzbCollectionData = nzbCollectionData;
    this->nzbFileDataList = nzbCollectionData.getNzbFileDataList();


    // search par2 program at each process launch in case settings have been changed at anytime :
    this->par2ProgramPath = Utility::searchExternalPrograms(repairProgram, isPar2ProgramFound);


    // launch repair if par2 program found :
    if (isPar2ProgramFound) {

        // repair process has begun :
        this->isProcessingStatus = true;

        // sort par2 files :
        QString fileSavePath = this->sortPar2FilesBySize();

        // launch repair process only if fileSavePath folder exists :
        if (Utility::isFolderExists(fileSavePath)) {

            // only launch repair if at least one .par2 file has been found :
            if (!this->par2FilesOrderedList.isEmpty()) {

                // list of arguments for par2repair command line :
                QStringList args;

                // process priority has been checked :
                if (Settings::groupBoxVerifyPriority()) {
                    args.append(Utility::buildPriorityArgument(Settings::verifyProcessValues(), Settings::verifyNiceValue()));
                }

                args.append(this->par2ProgramPath);
                args.append("r");
                args.append(this->par2FilesOrderedList.at(0));
                args.append(fileSavePath + nzbCollectionData.getPar2BaseName());

                //kDebug() << "ARGS :" << args;
                this->repairProcess->setTextModeEnabled(true);
                this->repairProcess->setOutputChannelMode(KProcess::MergedChannels);
                this->repairProcess->setNextOpenMode(QIODevice::ReadOnly | QIODevice::Unbuffered);
                this->repairProcess->setProgram(args);
                this->repairProcess->start();
                //this->repairProcess->closeWriteChannel();

            }
            // try to decompress files directly :
            else {
                //kDebug() << "No Par2 files found!";
                this->nzbCollectionData.setVerifyRepairTerminateStatus(RepairFinishedStatus);
                emit repairProcessEndedSignal(this->nzbCollectionData);

                // clear global variables :
                this->resetVariables();

                kDebug() << "try to decompress files directly...";
            }
        }
        // folder not exists :
        else {

            // if file save path has not been found, it is useless to launch extract process :
            this->nzbCollectionData.setVerifyRepairTerminateStatus(RepairFailedStatus);
            emit repairProcessEndedSignal(this->nzbCollectionData);

            // clear global variables :
            this->resetVariables();
        }

    }
    // par2 program has not been found, notify user :
    else {
        this->sendPar2ProgramNotFoundNotification();

        // try to decompress files directly :
        this->nzbCollectionData.setVerifyRepairTerminateStatus(RepairFinishedStatus);
        emit repairProcessEndedSignal(this->nzbCollectionData);

        // clear global variables :
        this->resetVariables();
    }
}


bool Repair::isProcessing(){
    return this->isProcessingStatus;
}


void Repair::emitProcessUpdate(const QVariant& parentIdentifer, const int& progression, const UtilityNamespace::ItemStatus& status, const UtilityNamespace::ItemTarget& itemTarget) {

    PostDownloadInfoData repairDecompressInfoData;
    repairDecompressInfoData.initRepairDecompress(parentIdentifer, progression, status, itemTarget);
    this->parent->emitProcessUpdate(repairDecompressInfoData);

}


//==============================================================================================//
//                                       processing                                             //
//==============================================================================================//
QString Repair::sortPar2FilesBySize(){

    QString fileSavePath;
    QMap<quint64, QString> sizePar2fileMap;

    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        QString currentFileName = nzbFileData.getDecodedFileName();

        // search par2 files that only match with par2BaseName pattern :
        if (nzbFileData.isPar2File()) {

            // check that file exists :
            fileSavePath = nzbFileData.getFileSavePath();
            QString currentFileNamePath = fileSavePath + currentFileName;
            QFile currentPar2File(currentFileNamePath);

            // add file size and it's path to map :
            if (currentPar2File.exists()) {
                sizePar2fileMap.insert(currentPar2File.size(), currentFileNamePath);
            }
        }
    } // end of loop

    // order par2 files by size criteria :
    this->par2FilesOrderedList = sizePar2fileMap.values();

    return fileSavePath;
}


void Repair::verifyUpdate(const QString& repairProcessOutput) {

    // search current processed file and its status :
    QRegExp regExp(".*\"(.*)\"\\s-\\s(missing|found|damaged|is a match for)(\\s\\d+)?(\\sduplicate)?.*(\"(.*)\")?\\.");

    // if file status has been found :
    if (regExp.exactMatch(repairProcessOutput)) {

        QString fileNameStr = regExp.cap(1);
        QString statusStr = regExp.cap(2);
        QString damagedBlocks = regExp.cap(3);
        QString duplicateName = regExp.cap(4);
        QString originalFileNameStr = regExp.cap(6);

        // set status to damaged if damaged block pattern has been found :
        if (!damagedBlocks.isEmpty() && statusStr == "found") {
            statusStr = "damaged";
        }

        // if "duplicate" pattern has been found it means that current item has already been verified, don't send notification :
        if (duplicateName.isEmpty()) {
            this->sendVerifyNotification(fileNameStr, originalFileNameStr, statusEnumMap.value(statusStr));
        }
    }

}


void Repair::repairUpdate(const QString& repairProcessOutput) {

    QRegExp regExp("Repairing:\\s*(\\d+).\\d+%");

    if (regExp.indexIn(repairProcessOutput) > -1) {

        // retrieve progression percentage value :
        QString repairProgressStr = regExp.cap(1);
        int repairProgressValue = repairProgressStr.toInt();

        // emit signal only if progress value has changed :
        if (this->repairProgressValueOld != repairProgressValue) {

            foreach (NzbFileData currentNzbFileData, this->nzbFileDataList) {

                // notify missing and damaged children of their repairing status
                // and notify nzb parent item of overall children progression repairing :
                if ((currentNzbFileData.getVerifyProgressionStep() == VerifyDamagedStatus) ||
                    (currentNzbFileData.getVerifyProgressionStep() == VerifyMissingStatus) ||
                    currentNzbFileData.isPar2File() ) {

                    this->emitProcessUpdate(currentNzbFileData.getUniqueIdentifier(), repairProgressValue, RepairStatus, this->getItemTarget(currentNzbFileData));

                }

            }


            this->repairProgressValueOld = repairProgressValue;
        }
    }

}



void Repair::resetVariables() {

    this->isPar2ProgramFound = false;
    this->isProcessingStatus = false;
    this->repairStatus = Repair::IdleRepair;
    this->nzbCollectionData = NzbCollectionData();
    this->nzbFileDataList.clear();
    this->par2FilesOrderedList.clear();
    this->repairProgressValueOld = -1;
    this->stdOutputLines.clear();
    this->repairProcess->close();

}



void Repair::updateNzbFileDataInList(NzbFileData& currentNzbFileData, const UtilityNamespace::ItemStatus verifyStep, const int index) {

    currentNzbFileData.setVerifyProgressionStep(verifyStep);
    this->nzbFileDataList.replace(index, currentNzbFileData);

}



void Repair::removePar2Files(){

    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.isPar2File()){

            Utility::removeData(nzbFileData.getFileSavePath() + nzbFileData.getDecodedFileName());
        }
    }
}



UtilityNamespace::ItemTarget Repair::getItemTarget(const NzbFileData& nzbFileData){

    UtilityNamespace::ItemTarget itemTarget;

    if (nzbFileData.isPar2File()) {
        itemTarget = ParentItemTarget;
    }
    else {
        itemTarget = ChildItemTarget;
    }

    return itemTarget;
}



//==============================================================================================//
//                                         SLOTS                                                //
//==============================================================================================//

void Repair::repairReadyReadSlot(){

    // read all data :
    this->stdOutputLines += repairProcess->readAll().replace("\r", "\n");
    QStringList lines = this->stdOutputLines.split("\n");

    // process each lines received :
    foreach (QString line, lines) {

        if (!line.isEmpty()) {

            //kDebug() << "line : " << line;

            switch(repairStatus) {

            case  Repair::IdleRepair: {

                    if (line.contains("Loading")) {
                        //kDebug() << "verifying";
                        // notify nzb parent and children files that verification has begun :
                        this->sendVerifyingFilesNotification();

                        repairStatus = Repair::Verifying;

                    }
                    break;
                }

            case Repair::Verifying: {

                    if (line.contains("Repair is possible")) {
                        //kDebug() << "Repair is possible";
                        this->sendMissingFilesNotification();
                        repairStatus = Repair::Repairing;
                    }
                    else if (line.contains("Repair is not possible")) {
                        //kDebug() << "Repair is not possible";
                        this->sendMissingFilesNotification();
                        repairStatus = Repair::RepairingNotPossible;
                    }
                    else if (line.contains("Repair complete")) {
                        //kDebug() << "Repair complete";
                        repairStatus = Repair::RepairComplete;
                    }

                    else {
                        // monitor verification process :
                        this->verifyUpdate(line);
                    }
                    break;

                }

            case Repair::Repairing: {

                    if (line.contains("Verifying repaired files")) {
                        //kDebug() << "Verifying repaired files";
                        repairStatus = Repair::Verifying;
                    }

                    else {
                        // monitor repairing process :
                        this->repairUpdate(line);
                    }
                    break;
                }

            default: {
                    break;
                }

            }
        }
    }

    // remove complete lines :
    if (this->stdOutputLines.endsWith("\n")) {
        this->stdOutputLines.clear();
    }
    else {
        // keep last line which is not complete :
        this->stdOutputLines = lines.takeLast();
    }


}


void Repair::repairFinishedSlot(const int exitCode, const QProcess::ExitStatus exitStatus){

    //kDebug() << "exitCode" << exitCode << " exitStatus " << exitStatus;

    // notify nzb parent item that verification has ended :
    for (int i = 0; i < this->nzbFileDataList.size(); i++) {

        NzbFileData nzbFileData = this->nzbFileDataList.at(i);

        if (nzbFileData.isPar2File()) {

            this->updateNzbFileDataInList(nzbFileData, VerifyFinishedStatus, i);

            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, VerifyFinishedStatus, ParentItemTarget);

        }
    }

    // 1. exit without errors :
    if (exitStatus == QProcess::NormalExit && exitCode == QProcess::NormalExit) {

        if (repairStatus == Repair::RepairComplete) {

            foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

                if ( (nzbFileData.getVerifyProgressionStep() == RepairStatus) ||
                     (nzbFileData.getVerifyProgressionStep() == VerifyStatus) ) {

                    this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, RepairFinishedStatus, ChildItemTarget);

                }
            }

        }

        // remove par2 files if repair succeed :
        if (Settings::removeParFiles()) {
            this->removePar2Files();
        }

        // notify that verify/repair process has ended in order to launch extract process :
        this->nzbCollectionData.setNzbFileDataList(this->nzbFileDataList);
        this->nzbCollectionData.setVerifyRepairTerminateStatus(RepairFinishedStatus);

        emit repairProcessEndedSignal(this->nzbCollectionData);

    }

    // 2. exit with errors :
    else {

        // notify damaged / missing files that repair is not possible :
        UtilityNamespace::ItemStatus errorStatus;

        if (repairStatus == Repair::RepairingNotPossible) {
            errorStatus = RepairNotPossibleStatus;
        }
        else {
            errorStatus = RepairFailedStatus;
        }


        foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

            if ( (nzbFileData.getVerifyProgressionStep() == VerifyMissingStatus) ||
                 (nzbFileData.getVerifyProgressionStep() == VerifyDamagedStatus) ||
                 (nzbFileData.getVerifyProgressionStep() == VerifyStatus) )  {

                this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, errorStatus, ChildItemTarget);

            }
        }

        // notify that verify/repair process has ended in order to verify remaining pending files :
        this->nzbCollectionData.setNzbFileDataList(this->nzbFileDataList);
        this->nzbCollectionData.setVerifyRepairTerminateStatus(RepairFailedStatus);

        emit repairProcessEndedSignal(this->nzbCollectionData);

    }

    this->resetVariables();

}


//==============================================================================================//
//                                     emit  SIGNALS                                            //
//==============================================================================================//

void Repair::sendPar2ProgramNotFoundNotification() {

    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {
        // notify parent items that program has not been found (nzbFileData.isPar2File() allows to notify parent item) :
        if (nzbFileData.isPar2File() ) {

            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, Par2ProgramMissing, ParentItemTarget);

        }
    }

    this->nzbCollectionData.setNzbFileDataList(this->nzbFileDataList);
    this->nzbCollectionData.setVerifyRepairTerminateStatus(RepairFailedStatus);

    emit repairProcessEndedSignal(this->nzbCollectionData);

}




void Repair::sendVerifyingFilesNotification() {

    // for each file name found in .par2 file, set status as VerifyStatus :
    QFile par2File(this->par2FilesOrderedList.at(0));
    par2File.open(QIODevice::ReadOnly);
    QByteArray par2ByteArray = par2File.readAll();

    for (int i = 0; i < this->nzbFileDataList.size(); i++) {
        NzbFileData nzbFileData = this->nzbFileDataList.at(i);

        // notify that items are verifying (nzbFileData.isPar2File() allows to notify parent item) :
        if ( par2ByteArray.contains(nzbFileData.getDecodedFileName().toAscii())  ||
             nzbFileData.isPar2File() ) {
            // update list :
            this->updateNzbFileDataInList(nzbFileData, VerifyStatus, i);

            // notifty user :
            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), 0, VerifyStatus, this->getItemTarget(nzbFileData));

        }

    }
    par2File.close();

}




void Repair::sendMissingFilesNotification() {

    // get only item identifiers with "missing" status :
    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.getVerifyProgressionStep() == VerifyMissingStatus) {

            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, VerifyMissingStatus, ChildItemTarget);

        }
    }

}



void Repair::sendVerifyNotification(const QString& fileNameStr, const QString& originalFileNameStr, const UtilityNamespace::ItemStatus fileStatus) {

    // search corresponding file into the list :
    for (int i = 0; i < this->nzbFileDataList.size(); i++) {

        NzbFileData nzbFileData = this->nzbFileDataList.at(i);

        if (nzbFileData.match(fileNameStr, originalFileNameStr)) {

            // It can occurs that archive files have been renamed,
            // repairing process will save repaired files with their original file name.
            // The name of theses original files should be stored for extracting process.
            // (note : both fileName and originalFileName are set as arguments because
            // par2 command mix fileName and originalFile name together according to their repair status
            // and are not clearly identifiable) :
            nzbFileData.setRenamedFileName(fileNameStr, originalFileNameStr);

            // update status for the corresponding nzbFileData :
            this->updateNzbFileDataInList(nzbFileData, fileStatus, i);

            // missing files will only be notified to user after verify process has ended :
            if (fileStatus != VerifyMissingStatus) {

                this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, fileStatus, ChildItemTarget);

            }

            // break removed :
            // => it can occur the same file name is present several times in the .nzb file, so
            // notify *all occurences of the file name* of its current verify status and not only the first one :
            //break;
        }

    } // end of loop

}
