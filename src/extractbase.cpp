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

#include "kwooty_debug.h"
#include <QFile>
#include "core.h"
#include "kwootysettings.h"
#include "widgets/centralwidget.h"
#include "repairdecompressthread.h"
#include "data/nzbfiledata.h"

ExtractBase::ExtractBase(RepairDecompressThread *parent): QObject(parent)
{

    mParent = parent;

    mExtractProcess = new KProcess(this);
    setupConnections();
    resetVariables();

}

ExtractBase::~ExtractBase()
{
    mExtractProcess->close();
}

bool ExtractBase::canHandleFormat(UtilityNamespace::ArchiveFormat archiveFormat)
{
    return (mArchiveFormat == archiveFormat);
}

void ExtractBase::preRepairProcessing(const NzbCollectionData &)
{
    // do nothing by default, may be implemented in derivated classes.
}

void ExtractBase::setupConnections()
{

    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    connect(mExtractProcess, SIGNAL(readyRead()), this, SLOT(extractReadyReadSlot()));
    connect(mExtractProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(extractFinishedSlot(int,QProcess::ExitStatus)));

    // begin verify - repair process for new pending files when signal has been received :
    connect(this,
            SIGNAL(extractProcessEndedSignal(NzbCollectionData)),
            mParent,
            SLOT(extractProcessEndedSlot(NzbCollectionData)));

    // display dialog box when password is required for archive extraction :
    connect(this,
            SIGNAL(extractPasswordRequiredSignal(QString)),
            mParent->getCore(),
            SLOT(extractPasswordRequiredSlot(QString)));

    // send password entered by the user :
    connect(mParent->getCore(),
            SIGNAL(passwordEnteredByUserSignal(bool,QString)),
            this,
            SLOT(passwordEnteredByUserSlot(bool,QString)));

}

void ExtractBase::launchProcess(const NzbCollectionData &nzbCollectionData, ExtractBase::ArchivePasswordStatus archivePasswordStatus, bool passwordEnteredByUSer, const QString &passwordStr)
{

    mNzbCollectionData = nzbCollectionData;
    mNzbFileDataList = nzbCollectionData.getNzbFileDataList();

    mArchivePasswordStatus = archivePasswordStatus;

    // search unrar program at each process launch in case settings have been changed at anytime :
    mExtractProgramPath = searchExtractProgram();

    // launch extract if unrar program found :
    if (mIsExtractProgramFound) {

        NzbFileData currentNzbFileData = getFirstArchiveFileFromList();

        // get archive saved path :
        QString fileSavePath = currentNzbFileData.getFileSavePath();

        // look for archive name (renamed name or decoded name) to set as argument :
        QString archiveName = getOriginalFileName(currentNzbFileData);

        // list of arguments for extract command line :
        QStringList args;

        // process priority has been checked :
        if (Settings::groupBoxExtractPriority()) {
            args.append(Utility::buildPriorityArgument(Settings::extractProcessValues(), Settings::extractNiceValue()));
        }

        args.append(mExtractProgramPath);
        args.append(createProcessArguments(archiveName, fileSavePath, passwordEnteredByUSer, passwordStr));

        //qCDebug(KWOOTY_LOG) << "ARGS :" << extractProgramPath <<args;

        mExtractProcess->setOutputChannelMode(KProcess::MergedChannels);
        mExtractProcess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered);
        mExtractProcess->setProgram(args);
        mExtractProcess->start();

    }
    // unrar or 7z program has not been found, notify user :
    else {
        sendExtractProgramNotFoundNotification();
    }

}

//==============================================================================================//
//                                       processing                                             //
//==============================================================================================//

QString ExtractBase::getOriginalFileName(const NzbFileData &currentNzbFileData) const
{

    // get the decoded file name (default behavior) :
    QString archiveName = currentNzbFileData.getDecodedFileName();

    // unless archives have been renamed, get original file name :
    if (!currentNzbFileData.getRenamedFileName().isEmpty()) {
        archiveName = currentNzbFileData.getRenamedFileName();
    }

    return archiveName;
}

NzbFileData ExtractBase::getFirstArchiveFileFromList(const QList<NzbFileData> &currentNzbFileDataList) const
{

    NzbFileData currentNzbFileData;
    foreach (const NzbFileData &nzbFileData, currentNzbFileDataList) {

        if (nzbFileData.isArchiveFile()) {
            //return the first archive file from list :
            currentNzbFileData = nzbFileData;
            break;
        }
    }

    return currentNzbFileData;
}

NzbFileData ExtractBase::getFirstArchiveFileFromList() const
{
    return getFirstArchiveFileFromList(mNzbFileDataList);
}

void ExtractBase::updateNzbFileDataInList(NzbFileData &currentNzbFileData, const UtilityNamespace::ItemStatus extractStep, const int index)
{

    currentNzbFileData.setExtractProgressionStep(extractStep);
    mNzbFileDataList.replace(index, currentNzbFileData);

}

void ExtractBase::resetVariables()
{

    mIsExtractProgramFound = false;
    mNzbCollectionData = NzbCollectionData();
    mNzbFileDataList.clear();
    mStdOutputLines.clear();
    mExtractProcess->close();
    mExtractProgressValue = PROGRESS_INIT;
}

void ExtractBase::removeArchiveFiles()
{

    foreach (const NzbFileData &nzbFileData, mNzbFileDataList) {

        if (nzbFileData.getExtractProgressionStep() == ExtractStatus) {

            // removed decoded file name :
            QString fullPathFileName = Utility::buildFullPath(nzbFileData.getFileSavePath(), nzbFileData.getDecodedFileName());
            Utility::removeData(fullPathFileName);

            // par2 may have rewrote repaired file names with ".1" ext, try to remove them :
            Utility::removeData(fullPathFileName + ".1");

            // if file has been renamed, par2 may have created the archive with the original name suppress it :
            removeRenamedArchiveFile(nzbFileData);
        }
    }
}

void ExtractBase::removeRenamedArchiveFile(const NzbFileData &nzbFileData)
{

    // if file has been renamed, par2 may have created the archive with the original name suppress it :
    if (!nzbFileData.getRenamedFileName().isEmpty()) {

        QString fullPathFileName = Utility::buildFullPath(nzbFileData.getFileSavePath(), nzbFileData.getRenamedFileName());
        Utility::removeData(fullPathFileName);

    }

}

//==============================================================================================//
//                                         SLOTS                                                //
//==============================================================================================//

void ExtractBase::extractReadyReadSlot()
{

    bool passwordCheckIsNextLine = false;

    // process each lines received :
    mStdOutputLines += QString::fromUtf8(mExtractProcess->readAllStandardOutput());

    QStringList lines = mStdOutputLines.split("\n");
    foreach (QString line, lines) {

        if (!line.isEmpty()) {

            //qCDebug(KWOOTY_LOG) << "line : " << line;

            if (mArchivePasswordStatus == ExtractBase::ArchiveCheckIfPassworded) {
                checkIfArchivePassworded(line, passwordCheckIsNextLine);
            }

            extractUpdate(line);
        }

    }

    // remove complete lines :
    if (mStdOutputLines.endsWith("\n")) {
        mStdOutputLines.clear();
    }
    // keep last line which is not complete :
    else {
        mStdOutputLines = lines.takeLast();
    }

}

void ExtractBase::extractFinishedSlot(const int exitCode, const QProcess::ExitStatus exitStatus)
{

    //qCDebug(KWOOTY_LOG) << "exitCode" << exitCode << " exitStatus " << exitStatus;

    // password checking has ended, files are *not* passworded, launch extract now :
    if (mArchivePasswordStatus == ExtractBase::ArchiveIsNotPassworded) {

        mExtractProcess->close();

        mNzbCollectionData.setNzbFileDataList(mNzbFileDataList);
        launchProcess(mNzbCollectionData, ExtractBase::ArchivePasswordCheckEnded);

    }
    // password checking has ended, files are passworded, display password box to user :
    else if (mArchivePasswordStatus == ExtractBase::ArchiveIsPassworded) {

        NzbFileData nzbFileData = getFirstArchiveFileFromList();
        emit extractPasswordRequiredSignal(nzbFileData.getDecodedFileName());

    }

    // file extraction has ended :
    else {

        // 1. exit without errors :
        if (exitStatus == QProcess::NormalExit && exitCode == QProcess::NormalExit) {

            // notify repairDecompressThread that extraction is over :
            mNzbCollectionData.setExtractTerminateStatus(ExtractSuccessStatus);

            emitFinishToArchivesWithoutErrors(ExtractSuccessStatus, PROGRESS_COMPLETE);

            // remove rar files if extract succeed :
            if (Settings::removeArchiveFiles()) {
                removeArchiveFiles();
            }

        }
        // 2. exit with errors :
        else {

            // notify repairDecompressThread that extraction has failed
            // send nzbCollectionData in order to cancel extracting of pending multi-set nzb (with same parent)
            // if par2 have not been downloaded yet
            // par2 will the be downloaded and extraction of multi-set nzb will then be made at that time :
            mNzbCollectionData.setExtractTerminateStatus(ExtractFailedStatus);

            emitFinishToArchivesWithoutErrors(ExtractFailedStatus, PROGRESS_COMPLETE);

        }

        // notify parent that extraction has finished :
        NzbFileData nzbFileData = getFirstArchiveFileFromList();

        emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, ExtractFinishedStatus, ParentItemTarget);

        emit extractProcessEndedSignal(mNzbCollectionData);

        resetVariables();

    }

}

void ExtractBase::passwordEnteredByUserSlot(bool passwordEntered, const QString &password)
{

    // this slot is shared between 7zextract and rarextract instances
    // do processing for proper instance : the one whih archivePasswordStatus set to ArchiveIsPassworded :
    if (mArchivePasswordStatus == ExtractBase::ArchiveIsPassworded) {

        // set password entered by user to the extract process :
        if (passwordEntered) {
            mNzbCollectionData.setNzbFileDataList(mNzbFileDataList);
            launchProcess(mNzbCollectionData, ExtractBase::ArchivePasswordCheckEnded, passwordEntered, password);
        }
        // password has not been entered, stop extract process :
        else {

            // notify children that extraction failed :
            emitStatusToAllArchives(PROGRESS_COMPLETE, ExtractFailedStatus, ChildItemTarget);

            // notify parent that extraction has finished :
            NzbFileData nzbFileData = getFirstArchiveFileFromList();

            emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, ExtractFinishedStatus, ParentItemTarget);

            resetVariables();

            // notify repairDecompressThread that extraction is over :
            emit extractProcessEndedSignal();

        }

    }

}

void ExtractBase::emitProcessUpdate(const QVariant &parentIdentifer, const int &progression, const UtilityNamespace::ItemStatus &status, const UtilityNamespace::ItemTarget &itemTarget)
{

    PostDownloadInfoData repairDecompressInfoData;
    repairDecompressInfoData.initRepairDecompress(parentIdentifer, progression, status, itemTarget);
    mParent->emitProcessUpdate(repairDecompressInfoData);

}

//==============================================================================================//
//                                     emit  SIGNALS                                            //
//==============================================================================================//

void ExtractBase::findItemAndNotifyUser(const QString &fileNameStr, const UtilityNamespace::ItemStatus status, UtilityNamespace::ItemTarget itemTarget)
{

    for (int i = 0; i < mNzbFileDataList.size(); ++i) {

        NzbFileData nzbFileData = mNzbFileDataList.at(i);

        // if nzbFileData has been identified :
        if (nzbFileData.match(fileNameStr)) {

            // used for 7z split files for eg
            // 7z split archive file is only the first field, set also the others one as archive files :
            nzbFileData.setArchiveFile(true);

            // update status for the corresponding nzbFileData :
            updateNzbFileDataInList(nzbFileData, status, i);

        }
    }

    // notify user of the current file being processed :
    emitProgressToArchivesWithCurrentStatus(status, itemTarget, mExtractProgressValue);
}

void ExtractBase::emitProgressToArchivesWithCurrentStatus(const UtilityNamespace::ItemStatus status, const UtilityNamespace::ItemTarget itemTarget,  const int percentage)
{

    foreach (const NzbFileData &nzbFileData, mNzbFileDataList) {

        if (nzbFileData.getExtractProgressionStep() == status) {

            // notify user of current file status and of its progression :
            emitProcessUpdate(nzbFileData.getUniqueIdentifier(), percentage, status, itemTarget);

        }

    }

}

void ExtractBase::emitFinishToArchivesWithoutErrors(const UtilityNamespace::ItemStatus status, const int percentage)
{

    foreach (const NzbFileData &nzbFileData, mNzbFileDataList) {

        UtilityNamespace::ItemStatus nzbFileDataStatus = nzbFileData.getExtractProgressionStep();

        // look for files without extracting errors :
        if (nzbFileDataStatus != ExtractBadCrcStatus) {

            if (nzbFileDataStatus == ExtractStatus) {

                emitProcessUpdate(nzbFileData.getUniqueIdentifier(), percentage, status, ChildItemTarget);

            }
        } else {

            // only used to send *progression %* value for files with extracting errors :
            emitProcessUpdate(nzbFileData.getUniqueIdentifier(), percentage, nzbFileData.getExtractProgressionStep(), ChildItemTarget);

        }

    }
}

void ExtractBase::emitStatusToAllArchives(const int &progress, const UtilityNamespace::ItemStatus status, const UtilityNamespace::ItemTarget target)
{

    foreach (const NzbFileData &nzbFileData, mNzbFileDataList) {

        if (nzbFileData.isArchiveFile()) {

            emitProcessUpdate(nzbFileData.getUniqueIdentifier(), progress, status, target);

        }

    }

}

