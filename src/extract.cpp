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

#include "extract.h"

#include <KDebug>
#include <QFile>
#include "settings.h"
#include "data/nzbfiledata.h"


Extract::Extract() {

    this->extractProcess = new KProcess(this);
    this->setupConnections();
    this->resetVariables();

}


Extract::~Extract() {
    this->extractProcess->close();
}


void Extract::setupConnections() {

    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    connect (this->extractProcess, SIGNAL(readyRead()), this, SLOT(extractReadyReadSlot()));
    connect (this->extractProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(extractFinishedSlot(int, QProcess::ExitStatus)));

}



void Extract::launchProcess(const QList<NzbFileData>& nzbFileDataList, Extract::ArchivePasswordStatus archivePasswordStatus, bool passwordEnteredByUSer, QString passwordStr){

    this->nzbFileDataList = nzbFileDataList;
    this->archivePasswordStatus = archivePasswordStatus;

    // search unrar program at each process launch in case settings have been changed at anytime :
    unrarProgramPath = Utility::searchExternalPrograms(extractProgram, isUnrarProgramFound);

    // launch extract if unrar program found :
    if (isUnrarProgramFound) {

        // look for archive name (renamed name or decoded name) to set as argument :
        QString archiveName;

        NzbFileData currentNzbFileData = this->getFirstRarFileFromList();

        // get archive saved path :
        this->fileSavePath = currentNzbFileData.getFileSavePath();

        // case archives have been renamed, get original file name :
        if (!currentNzbFileData.getRenamedFileName().isEmpty()) {
            archiveName = currentNzbFileData.getRenamedFileName();
        }
        // otherwise get the decoded file name (default behavior) :
        else {
            archiveName = currentNzbFileData.getDecodedFileName();
        }

        QStringList args;
        // first pass : check if archive is passworded :
        if (this->archivePasswordStatus == Extract::ArchiveCheckIfPassworded) {
            args.append("l");
            args.append("-p-");
            args.append(this->fileSavePath + archiveName);
        }
        // second pass : extract archive with or without a password :
        else {
            args.append("x");

            // overwrite the output file if option choosen in settings :
            if (Settings::overwriteExtractedFiles()) {
                args.append("-o+");
            }
            // rename the output file if option choosen in settings :
            else {
                args.append("-or");
            }

            args.append("-c-");
            args.append("-y");

            // if archive is passworded :
            if (passwordEnteredByUSer) {
                // set password entered by the user :
                if (!passwordStr.isEmpty()) {
                    args.append("-p" + passwordStr);
                }
                // user only click ok without entering a password :
                else {
                    args.append("-p-");
                }
            }

            args.append(this->fileSavePath + archiveName);
            args.append(this->fileSavePath);

        }

        //kDebug() << "ARGS :" << args;

        this->extractProcess->setTextModeEnabled(true);
        this->extractProcess->setOutputChannelMode(KProcess::MergedChannels);
        this->extractProcess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered);
        this->extractProcess->setProgram(unrarProgramPath, args);
        this->extractProcess->start();

        // if path contains "*" add "\\" in order to avoid issues with QRegExp pattern search :
        this->fileSavePath.replace("*", "\\*");

    }
    // unrar program has not been found, notify user :
    else {
        this->sendUnrarProgramNotFoundNotification();
    }

}



//==============================================================================================//
//                                       processing                                             //
//==============================================================================================//

void Extract::extractUpdate(const QString& line) {

    // get extraction progress :
    if (line.contains("%")) {

        QRegExp regExp(".*\\s*(\\d+)%");

        // if percentage has been found :
        if (regExp.exactMatch(line)) {

            QString extractProgressStr = regExp.cap(1);
            this->extractProgressValue = extractProgressStr.toInt();
            // emit to files with ExtractStatus as status, the extract progression value :
            this->emitProgressToArchivesWithCurrentStatus(ExtractStatus, BothItemsTarget, this->extractProgressValue);
        }

    }

    else if (line.contains("password incorrect")) {

        this->archivePasswordStatus = Extract::ArchiveIsPassworded;
        kDebug() << "password incorrect";
    }


    // get files with crc errors :
    else if (line.contains("CRC failed")) {

        QRegExp regExp(".*" + this->fileSavePath + "(.*)?");

        // if file with bad crc found :
        if (regExp.exactMatch(line)) {

            QString fileNameStr = regExp.cap(1);
            // search corresponding file into the list :
            this->findItemAndNotifyUser(fileNameStr, ExtractBadCrcStatus, ChildItemTarget);
        }
    }

    // search current processed archive :
    else {

        QRegExp regExp("Extracting from " + this->fileSavePath + "(.*)?");

        // if current extracted file has been found :
        if (regExp.exactMatch(line)) {

            QString fileNameStr = regExp.cap(1);
            // search corresponding file into the list :
            this->findItemAndNotifyUser(fileNameStr, ExtractStatus, BothItemsTarget);
        }
    }


}


void Extract::checkIfArchivePassworded(const QString& currentLine, bool& passwordCheckIsNextLine) {

    if (passwordCheckIsNextLine) {

        // "*" means the archive is passworded, look for it :
        if (currentLine.left(1) == "*") {
            this->archivePasswordStatus = Extract::ArchiveIsPassworded;
        }
        else {
            // the archive is not passworded :
            this->archivePasswordStatus = Extract::ArchiveIsNotPassworded;
        }

    }

    // search this pattern in order to know that next line indicate if archive is passworded or not :
    if (currentLine.contains("------------------")){
        passwordCheckIsNextLine = true;
    }

}



NzbFileData Extract::getFirstRarFileFromList() const {

    NzbFileData currentNzbFileData;
    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        if ( nzbFileData.isRarFile() &&
             QFile::exists(nzbFileData.getFileSavePath() + nzbFileData.getDecodedFileName()) ) {
            //return the first achive file from list :
            currentNzbFileData = nzbFileData;
            break;
        }
    }

    return currentNzbFileData;
}



void Extract::updateNzbFileDataInList(NzbFileData& currentNzbFileData, const UtilityNamespace::ItemStatus extractStep, const int index) {

    currentNzbFileData.setExtractProgressionStep(extractStep);
    this->nzbFileDataList.replace(index, currentNzbFileData);

}


void Extract::resetVariables(){
    this->isUnrarProgramFound = false;
    this->fileSavePath.clear();
    this->fileNameToExtract.clear();
    this->nzbFileDataList.clear();
    this->stdOutputLines.clear();
    this->extractProcess->close();
    this->extractProgressValue = PROGRESS_INIT;
}


void Extract::removeRarFiles(){

    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.getExtractProgressionStep() == ExtractStatus) {

            // removed decoded file name :
            QString fullPathFileName = nzbFileData.getFileSavePath() + nzbFileData.getDecodedFileName();
            Utility::removeData(fullPathFileName);

            // par2 may have rewrote repaired file names with ".1" ext, try to remove them :
            Utility::removeData(fullPathFileName + ".1");

            // if file has been renamed, par2 may have created the archive with the original name suppress it :
            if (!nzbFileData.getRenamedFileName().isEmpty()) {

                fullPathFileName = nzbFileData.getFileSavePath() + nzbFileData.getRenamedFileName();
                Utility::removeData(fullPathFileName);

            }
        }

    }
}




//==============================================================================================//
//                                         SLOTS                                                //
//==============================================================================================//

void Extract::extractReadyReadSlot(){

    bool passwordCheckIsNextLine = false;

    // process each lines received :
    this->stdOutputLines += extractProcess->readAllStandardOutput();

    QStringList lines = this->stdOutputLines.split("\n");
    foreach (QString line, lines) {

        if (!line.isEmpty()) {

            // kDebug() << "line : " << line;

            if (this->archivePasswordStatus == Extract::ArchiveCheckIfPassworded) {
                this->checkIfArchivePassworded(line, passwordCheckIsNextLine);
            }
            //this->manageProcessDialog(line);
            this->extractUpdate(line);
        }

    }

    // remove complete lines :
    if (this->stdOutputLines.endsWith("\n")) {
        this->stdOutputLines.clear();
    }
    // keep last line which is not complete :
    else {
        this->stdOutputLines = lines.takeLast();
    }

}



void Extract::extractFinishedSlot(const int exitCode, const QProcess::ExitStatus exitStatus){

    //kDebug() << "exitCode" << exitCode << " exitStatus " << exitStatus;

    // password checking has ended, files are *not* passworded, launch extract now :
    if (archivePasswordStatus == Extract::ArchiveIsNotPassworded) {

        this->extractProcess->close();
        this->launchProcess(this->nzbFileDataList, Extract::ArchivePasswordCheckEnded);

    }
    // password checking has ended, files are passworded, display password box to user :
    else if (this->archivePasswordStatus == Extract::ArchiveIsPassworded) {

        NzbFileData nzbFileData = this->getFirstRarFileFromList();
        emit extractPasswordRequiredSignal(nzbFileData.getDecodedFileName());

    }

    // file extraction has ended :
    else {

        // notify repairDecompressThread that extraction is over :
        emit extractProcessEndedSignal();

        // 1. exit without errors :
        if (exitStatus == QProcess::NormalExit && exitCode == QProcess::NormalExit ){

            this->emitFinishToArchivesWithoutErrors(ExtractSuccessStatus, PROGRESS_COMPLETE);

            // remove rar files if extract succeed :
            if (Settings::removeArchiveFiles()) {
                this->removeRarFiles();
            }

        }
        // 2. exit with errors :
        else {
            this->emitFinishToArchivesWithoutErrors(ExtractFailedStatus, PROGRESS_COMPLETE);
        }


        // notify parent that extraction has finished :
        NzbFileData nzbFileData = this->getFirstRarFileFromList();
        emit updateExtractSignal(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, ExtractFinishedStatus, ParentItemTarget);

        this->resetVariables();

    }

}



void Extract::passwordEnteredByUserSlot(bool passwordEntered, QString password) {

    // set password entered by user to the extract process :
    if (passwordEntered) {
        this->launchProcess(this->nzbFileDataList, Extract::ArchivePasswordCheckEnded, passwordEntered, password);
    }
    // password has not been entered, stop extract process :
    else {

        // notify childrent that extraction failed :
        this->emitStatusToAllArchives(ExtractFailedStatus);

        // notify parent that extraction has finished :
        NzbFileData nzbFileData = this->getFirstRarFileFromList();
        emit updateExtractSignal(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, ExtractFinishedStatus, ParentItemTarget);

        this->resetVariables();

        // notify repairDecompressThread that extraction is over :
        emit extractProcessEndedSignal();
    }

}



//==============================================================================================//
//                                     emit  SIGNALS                                            //
//==============================================================================================//

void Extract::sendUnrarProgramNotFoundNotification(){

    // notify parent that program is missing :
    NzbFileData nzbFileData = this->getFirstRarFileFromList();
    emit updateExtractSignal(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, UnrarProgramMissing, ParentItemTarget);

    // notify repairDecompressThread that extraction is over :
    emit extractProcessEndedSignal();

}


void Extract::findItemAndNotifyUser(const QString& fileNameStr, const UtilityNamespace::ItemStatus status, UtilityNamespace::ItemTarget itemTarget) {

    for (int i = 0; i < this->nzbFileDataList.size(); i++) {

        NzbFileData nzbFileData = this->nzbFileDataList.at(i);

        // if nzbFileData has been identified :
        if (nzbFileData.match(fileNameStr)) {
            // update status for the corresponding nzbFileData :
            this->updateNzbFileDataInList(nzbFileData, status, i);
        }
    }

    // notify user of the current file being proessed :
    this->emitProgressToArchivesWithCurrentStatus(status, itemTarget, this->extractProgressValue);
}



void Extract::emitProgressToArchivesWithCurrentStatus(const UtilityNamespace::ItemStatus status, const UtilityNamespace::ItemTarget itemTarget,  const int percentage) {

    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.getExtractProgressionStep() == status) {
            // notify user of current file status and of its progression :
            emit updateExtractSignal(nzbFileData.getUniqueIdentifier(), percentage, status, itemTarget);
        }

    }

}



void Extract::emitFinishToArchivesWithoutErrors(const UtilityNamespace::ItemStatus status, const int percentage) {

    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        UtilityNamespace::ItemStatus nzbFileDataStatus = nzbFileData.getExtractProgressionStep();

        // look for files without extracting errors :
        if (nzbFileDataStatus != ExtractBadCrcStatus) {

            if (nzbFileDataStatus == ExtractStatus){
                emit updateExtractSignal(nzbFileData.getUniqueIdentifier(), percentage, status, ChildItemTarget);
            }
        }
        else {
            // only used to send *progression %* value for files with extracting errors :
            emit updateExtractSignal(nzbFileData.getUniqueIdentifier(), percentage, nzbFileData.getExtractProgressionStep(), ChildItemTarget);
        }

    }
}


void Extract::emitStatusToAllArchives(const UtilityNamespace::ItemStatus status) {

    foreach (NzbFileData nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.isRarFile()) {
            emit updateExtractSignal(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, status, ChildItemTarget);
        }

    }

}


