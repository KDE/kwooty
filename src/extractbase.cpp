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

#include "extractbase.h"

#include <KDebug>
#include <QFile>
#include "core.h"
#include "kwootysettings.h"
#include "widgets/centralwidget.h"
#include "repairdecompressthread.h"
#include "data/nzbfiledata.h"



ExtractBase::ExtractBase(RepairDecompressThread* parent): QObject (parent) {

    this->parent = parent;

    this->extractProcess = new KProcess(this);
    this->setupConnections();
    this->resetVariables();

}


ExtractBase::~ExtractBase() {
    this->extractProcess->close();
}


bool ExtractBase::canHandleFormat(UtilityNamespace::ArchiveFormat archiveFormat) {
    return (this->archiveFormat == archiveFormat);
}

void ExtractBase::preRepairProcessing(const NzbCollectionData&) {
    // do nothing by default, may be implemented in derivated classes.
}



void ExtractBase::setupConnections() {

    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    connect (this->extractProcess, SIGNAL(readyRead()), this, SLOT(extractReadyReadSlot()));
    connect (this->extractProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(extractFinishedSlot(int, QProcess::ExitStatus)));


    // begin verify - repair process for new pending files when signal has been received :
    connect (this,
             SIGNAL(extractProcessEndedSignal(NzbCollectionData)),
             this->parent,
             SLOT(extractProcessEndedSlot(NzbCollectionData)));

    // display dialog box when password is required for archive extraction :
    connect (this,
             SIGNAL(extractPasswordRequiredSignal(QString)),
             parent->getCore(),
             SLOT(extractPasswordRequiredSlot(QString)));

    // send password entered by the user :
    connect (parent->getCore(),
             SIGNAL(passwordEnteredByUserSignal(bool, QString)),
             this,
             SLOT(passwordEnteredByUserSlot(bool, QString)));

}



void ExtractBase::launchProcess(const NzbCollectionData& nzbCollectionData, ExtractBase::ArchivePasswordStatus archivePasswordStatus, bool passwordEnteredByUSer, QString passwordStr){

    this->nzbCollectionData = nzbCollectionData;
    this->nzbFileDataList = nzbCollectionData.getNzbFileDataList();

    this->archivePasswordStatus = archivePasswordStatus;

    // search unrar program at each process launch in case settings have been changed at anytime :
    this->extractProgramPath = this->searchExtractProgram();

    // launch extract if unrar program found :
    if (this->isExtractProgramFound) {


        NzbFileData currentNzbFileData = this->getFirstArchiveFileFromList();

        // get archive saved path :
        QString fileSavePath = currentNzbFileData.getFileSavePath();

        // look for archive name (renamed name or decoded name) to set as argument :
        QString archiveName = this->getOriginalFileName(currentNzbFileData);

        // list of arguments for extract command line :
        QStringList args;

        // process priority has been checked :
        if (Settings::groupBoxExtractPriority()) {
            args.append(Utility::buildPriorityArgument(Settings::extractProcessValues(), Settings::extractNiceValue()));
        }

        args.append(this->extractProgramPath);
        args.append(this->createProcessArguments(archiveName, fileSavePath, passwordEnteredByUSer, passwordStr));

        //kDebug() << "ARGS :" << this->extractProgramPath <<args;

        this->extractProcess->setOutputChannelMode(KProcess::MergedChannels);
        this->extractProcess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered);
        this->extractProcess->setProgram(args);
        this->extractProcess->start();


    }
    // unrar or 7z program has not been found, notify user :
    else {
        this->sendExtractProgramNotFoundNotification();
    }

}



//==============================================================================================//
//                                       processing                                             //
//==============================================================================================//

QString ExtractBase::getOriginalFileName(const NzbFileData& currentNzbFileData) const {

    // get the decoded file name (default behavior) :
    QString archiveName = currentNzbFileData.getDecodedFileName();

    // unless archives have been renamed, get original file name :
    if (!currentNzbFileData.getRenamedFileName().isEmpty()) {
        archiveName = currentNzbFileData.getRenamedFileName();
    }

    return archiveName;
}



NzbFileData ExtractBase::getFirstArchiveFileFromList(const QList<NzbFileData>& currentNzbFileDataList) const {

    NzbFileData currentNzbFileData;
    foreach (const NzbFileData& nzbFileData, currentNzbFileDataList) {

        if (nzbFileData.isArchiveFile()) {
            //return the first archive file from list :
            currentNzbFileData = nzbFileData;
            break;
        }
    }

    return currentNzbFileData;
}


NzbFileData ExtractBase::getFirstArchiveFileFromList() const {
   return this->getFirstArchiveFileFromList(this->nzbFileDataList);
}



void ExtractBase::updateNzbFileDataInList(NzbFileData& currentNzbFileData, const UtilityNamespace::ItemStatus extractStep, const int index) {

    currentNzbFileData.setExtractProgressionStep(extractStep);
    this->nzbFileDataList.replace(index, currentNzbFileData);

}


void ExtractBase::resetVariables(){

    this->isExtractProgramFound = false;
    this->nzbCollectionData = NzbCollectionData();
    this->nzbFileDataList.clear();
    this->stdOutputLines.clear();
    this->extractProcess->close();
    this->extractProgressValue = PROGRESS_INIT;
}


void ExtractBase::removeArchiveFiles(){

    foreach (const NzbFileData& nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.getExtractProgressionStep() == ExtractStatus) {

            // removed decoded file name :
            QString fullPathFileName = Utility::buildFullPath(nzbFileData.getFileSavePath(), nzbFileData.getDecodedFileName());
            Utility::removeData(fullPathFileName);

            // par2 may have rewrote repaired file names with ".1" ext, try to remove them :
            Utility::removeData(fullPathFileName + ".1");

            // if file has been renamed, par2 may have created the archive with the original name suppress it :
            this->removeRenamedArchiveFile(nzbFileData);
        }
    }
}

void ExtractBase::removeRenamedArchiveFile(const NzbFileData& nzbFileData) {

    // if file has been renamed, par2 may have created the archive with the original name suppress it :
    if (!nzbFileData.getRenamedFileName().isEmpty()) {

        QString fullPathFileName = Utility::buildFullPath(nzbFileData.getFileSavePath(), nzbFileData.getRenamedFileName());
        Utility::removeData(fullPathFileName);

    }

}


//==============================================================================================//
//                                         SLOTS                                                //
//==============================================================================================//

void ExtractBase::extractReadyReadSlot(){

    bool passwordCheckIsNextLine = false;

    // process each lines received :
    this->stdOutputLines += QString::fromUtf8(extractProcess->readAllStandardOutput());

    QStringList lines = this->stdOutputLines.split("\n");
    foreach (QString line, lines) {

        if (!line.isEmpty()) {

            //kDebug() << "line : " << line;

            if (this->archivePasswordStatus == ExtractBase::ArchiveCheckIfPassworded) {
                this->checkIfArchivePassworded(line, passwordCheckIsNextLine);
            }

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



void ExtractBase::extractFinishedSlot(const int exitCode, const QProcess::ExitStatus exitStatus) {

    //kDebug() << "exitCode" << exitCode << " exitStatus " << exitStatus;

    // password checking has ended, files are *not* passworded, launch extract now :
    if (archivePasswordStatus == ExtractBase::ArchiveIsNotPassworded) {

        this->extractProcess->close();

        this->nzbCollectionData.setNzbFileDataList(this->nzbFileDataList);
        this->launchProcess(this->nzbCollectionData, ExtractBase::ArchivePasswordCheckEnded);

    }
    // password checking has ended, files are passworded, display password box to user :
    else if (this->archivePasswordStatus == ExtractBase::ArchiveIsPassworded) {

        NzbFileData nzbFileData = this->getFirstArchiveFileFromList();
        emit extractPasswordRequiredSignal(nzbFileData.getDecodedFileName());

    }

    // file extraction has ended :
    else {

        // 1. exit without errors :
        if (exitStatus == QProcess::NormalExit && exitCode == QProcess::NormalExit){

            // notify repairDecompressThread that extraction is over :
            this->nzbCollectionData.setExtractTerminateStatus(ExtractSuccessStatus);

            this->emitFinishToArchivesWithoutErrors(ExtractSuccessStatus, PROGRESS_COMPLETE);

            // remove rar files if extract succeed :
            if (Settings::removeArchiveFiles()) {
                this->removeArchiveFiles();
            }

        }
        // 2. exit with errors :
        else {

            // notify repairDecompressThread that extraction has failed
            // send nzbCollectionData in order to cancel extracting of pending multi-set nzb (with same parent)
            // if par2 have not been downloaded yet
            // par2 will the be downloaded and extraction of multi-set nzb will then be made at that time :
            this->nzbCollectionData.setExtractTerminateStatus(ExtractFailedStatus);

            this->emitFinishToArchivesWithoutErrors(ExtractFailedStatus, PROGRESS_COMPLETE);

        }


        // notify parent that extraction has finished :
        NzbFileData nzbFileData = this->getFirstArchiveFileFromList();

        this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, ExtractFinishedStatus, ParentItemTarget);

        emit extractProcessEndedSignal(this->nzbCollectionData);

        this->resetVariables();

    }

}



void ExtractBase::passwordEnteredByUserSlot(bool passwordEntered, QString password) {

    // this slot is shared between 7zextract and rarextract instances
    // do processing for proper instance : the one whih archivePasswordStatus set to ArchiveIsPassworded :
    if (this->archivePasswordStatus == ExtractBase::ArchiveIsPassworded) {

        // set password entered by user to the extract process :
        if (passwordEntered) {
            this->nzbCollectionData.setNzbFileDataList(this->nzbFileDataList);
            this->launchProcess(this->nzbCollectionData, ExtractBase::ArchivePasswordCheckEnded, passwordEntered, password);
        }
        // password has not been entered, stop extract process :
        else {

            // notify children that extraction failed :
            this->emitStatusToAllArchives(PROGRESS_COMPLETE, ExtractFailedStatus, ChildItemTarget);

            // notify parent that extraction has finished :
            NzbFileData nzbFileData = this->getFirstArchiveFileFromList();

            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, ExtractFinishedStatus, ParentItemTarget);

            this->resetVariables();

            // notify repairDecompressThread that extraction is over :
            emit extractProcessEndedSignal();



        }

    }

}



void ExtractBase::emitProcessUpdate(const QVariant& parentIdentifer, const int& progression, const UtilityNamespace::ItemStatus& status, const UtilityNamespace::ItemTarget& itemTarget) {

    PostDownloadInfoData repairDecompressInfoData;
    repairDecompressInfoData.initRepairDecompress(parentIdentifer, progression, status, itemTarget);
    this->parent->emitProcessUpdate(repairDecompressInfoData);

}




//==============================================================================================//
//                                     emit  SIGNALS                                            //
//==============================================================================================//


void ExtractBase::findItemAndNotifyUser(const QString& fileNameStr, const UtilityNamespace::ItemStatus status, UtilityNamespace::ItemTarget itemTarget) {

    for (int i = 0; i < this->nzbFileDataList.size(); i++) {

        NzbFileData nzbFileData = this->nzbFileDataList.at(i);

        // if nzbFileData has been identified :
        if (nzbFileData.match(fileNameStr)) {

            // used for 7z split files for eg
            // 7z split archive file is only the first field, set also the others one as archive files :
            nzbFileData.setArchiveFile(true);

            // update status for the corresponding nzbFileData :
            this->updateNzbFileDataInList(nzbFileData, status, i);

        }
    }

    // notify user of the current file being processed :
    this->emitProgressToArchivesWithCurrentStatus(status, itemTarget, this->extractProgressValue);
}



void ExtractBase::emitProgressToArchivesWithCurrentStatus(const UtilityNamespace::ItemStatus status, const UtilityNamespace::ItemTarget itemTarget,  const int percentage) {

    foreach (const NzbFileData& nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.getExtractProgressionStep() == status) {

            // notify user of current file status and of its progression :
            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), percentage, status, itemTarget);

        }

    }

}



void ExtractBase::emitFinishToArchivesWithoutErrors(const UtilityNamespace::ItemStatus status, const int percentage) {

    foreach (const NzbFileData& nzbFileData, this->nzbFileDataList) {

        UtilityNamespace::ItemStatus nzbFileDataStatus = nzbFileData.getExtractProgressionStep();

        // look for files without extracting errors :
        if (nzbFileDataStatus != ExtractBadCrcStatus) {

            if (nzbFileDataStatus == ExtractStatus){

                this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), percentage, status, ChildItemTarget);

            }
        }
        else {

            // only used to send *progression %* value for files with extracting errors :
            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), percentage, nzbFileData.getExtractProgressionStep(), ChildItemTarget);

        }

    }
}


void ExtractBase::emitStatusToAllArchives(const int& progress, const UtilityNamespace::ItemStatus status, const UtilityNamespace::ItemTarget target) {

    foreach (const NzbFileData& nzbFileData, this->nzbFileDataList) {

        if (nzbFileData.isArchiveFile()) {

            this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), progress, status, target);

        }

    }

}


