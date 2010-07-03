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

#include "utility.h"

#include <KDebug>
#include <KStandardDirs>
#include <KLocale>
#include <kio/global.h>

#include <QFile>
#include <QDir>
#include "kwootysettings.h"


using namespace UtilityNamespace;

Utility::Utility()
{
}



QString Utility::convertByteHumanReadable(const quint64 sizeInByte) {

    // compute the file size :
    QString sizeUnit(i18n(" GiB"));
    double fileSize = sizeInByte / NBR_BYTES_IN_GB;

    if (fileSize < ONE_UNIT){
        sizeUnit = i18n(" MiB");
        fileSize = sizeInByte / NBR_BYTES_IN_MB;
        
        if (fileSize < ONE_UNIT){
            fileSize = sizeInByte / NBR_BYTES_IN_KB;
            sizeUnit = i18n(" KiB");
        }
    }
    return QString::number(fileSize, 'f', 2) + sizeUnit;

}


QString Utility::convertDownloadSpeedHumanReadable(const quint64 downloadSpeedInByte) {

    QString downloadSpeedStr;
    double fileSize = downloadSpeedInByte / NBR_BYTES_IN_MB;

    if (fileSize > ONE_UNIT){

        downloadSpeedStr = ki18n("%1 MiB/s").subs(fileSize, 0, 'f', 2).toString();
    }
    else {
        downloadSpeedStr = i18n("<numid>%1</numid> KiB/s", static_cast<int>(downloadSpeedInByte / NBR_BYTES_IN_KB));
    }

    return downloadSpeedStr;
}




bool Utility::isInDownloadProcess(const UtilityNamespace::ItemStatus statusItem){

    bool status = false;

    if ( isReadyToDownload(statusItem) ||
         isPaused(statusItem)          ||
         isPausing(statusItem) ) {

        status = true;
    }
    return status;
}


bool Utility::isReadyToDownload(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( (statusItem == IdleStatus) || (statusItem == DownloadStatus)){
        status = true;
    }

    return status;
}

bool Utility::isInQueue(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( (statusItem == IdleStatus) || (statusItem == PauseStatus)){
        status = true;
    }

    return status;
}

bool Utility::isDownloadOrPausing(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( statusItem == DownloadStatus ||
         isPausing(statusItem) ) {

        status = true;
    }
    return status;
}



bool Utility::isPaused(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( statusItem == PauseStatus){
        status = true;
    }

    return status;
}

bool Utility::isPausing(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( statusItem == PausingStatus){
        status = true;
    }

    return status;
}


bool Utility::isDecoding(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( (statusItem == DecodeStatus) || (statusItem == ScanStatus)){
        status = true;
    }

    return status;
}


bool Utility::isWaitingForDecode(const UtilityNamespace::ItemStatus statusItem, const UtilityNamespace::Data data){
    bool status = false;

    if ( (statusItem == DownloadFinishStatus) &&
         (data != NoData) ) {

        status = true;
    }

    return status;
}



bool Utility::isDownloadFinish(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( statusItem == DownloadFinishStatus){
        status = true;
    }

    return status;
}

bool Utility::isPostDownloadProcessing(const UtilityNamespace::ItemStatus statusItem){
    bool status = false;

    if ( (statusItem == VerifyStatus)  ||
         (statusItem == RepairStatus)  ||
         (statusItem == ExtractStatus) ) {

        status = true;
    }

    return status;
}


bool Utility::isJobFinish(const UtilityNamespace::ItemStatus statusItem){

    bool status = false;

    if ( (statusItem == DownloadFinishStatus)    ||
         (statusItem == DecodeFinishStatus)      ||
         (statusItem == VerifyFinishedStatus)    ||
         (statusItem == RepairFinishedStatus)    ||
         (statusItem == ExtractFinishedStatus) ){

        status = true;
    }

    return status;
}


bool Utility::saveData(const QString& fileSavePath, const QString& fileName, const QByteArray& segmentData){

    bool isSaveSucceeded = false;

    // check that save folder exists, otherwise create it :
    isSaveSucceeded = createFolder(fileSavePath);

    if (isSaveSucceeded) {

        // open file in write mode :
        QFile file(fileSavePath + "/" + fileName);
        isSaveSucceeded = file.open(QIODevice::WriteOnly);

        if (isSaveSucceeded) {
            // write data :
            qint64 bytesWritten = file.write(segmentData);
            if (bytesWritten == -1) {
                isSaveSucceeded = false;
            }
            file.close();
        }
    }

    return isSaveSucceeded;
}



bool Utility::createFolder(const QString& fileSavePath){

    bool folderExists = true;

    // if the folder does not exists, create it :
    QDir directory(fileSavePath);
    if (!directory.exists()) {
        folderExists = directory.mkpath(fileSavePath);
    }

    return folderExists;
}


bool Utility::isFolderExists(const QString& fileSavePath){

    bool folderExists = true;
    // check if the folder exists :
    QDir directory(fileSavePath);
    if (!directory.exists()) {
        folderExists = false;
    }

    return folderExists;
}



bool Utility::removeData(const QString& fileName){

    QFile file(fileName);
    bool isFileRemoved = false;
    if (file.exists()) {
        isFileRemoved = file.remove();
    }

    return isFileRemoved;
}



QString Utility::searchExternalPrograms(const QString& programToSearch, bool& programFound){

    QString programPathName;
    QStringList searchPathList = Settings::searchPathList();

    QStringList programsWithDifferentNames = programToSearch.split(";");

    foreach (QString currentProgramName, programsWithDifferentNames) {

        foreach (QString searchPath, searchPathList) {

            // search program :
            if (searchPath.endsWith("/")) {
                searchPath.chop(1);
            }

            QFile ProgramFile(searchPath + "/" + currentProgramName);

            if (ProgramFile.exists()) {

                programPathName = searchPath + "/" + currentProgramName;

                programFound = true;

            }

        }

        if (programFound) {
            break;
        }

    }


    return programPathName;
}


QString Utility::getSystemTimeFormat(const QString& dateFormat) {

    QString properDateFormat = dateFormat;

    // set time format to am/pm format if used by system :
    if (KGlobal::locale()->use12Clock()) {

        properDateFormat.append(" ap");
    }

    return properDateFormat;
}



QString Utility::buildToolTipRow(const QString& label, const QString& value) {

    QString toolRipRow = "<tr><td>" + label + "</td><td>" + value + "</td></tr>";
    return toolRipRow;
}



QStringList Utility::buildPriorityArgument(const int& processPriority, const int& niceValue) {


    QStringList niceProcessArgs;

    // look for 'nice' location :
    QString nicePath = KStandardDirs::findExe("nice");
    niceProcessArgs.append(nicePath);
    niceProcessArgs.append("-n");

    // set priority value :
    switch (processPriority) {

    case UtilityNamespace::LowPriority: {
            niceProcessArgs.append("10");
            break;
        }
    case UtilityNamespace::LowestPriority: {
            niceProcessArgs.append("19");
            break;
        }
    case UtilityNamespace::CustomPriority: {
            niceProcessArgs.append(QString::number(niceValue));
            break;
        }
    default: {
            break;
        }
    }


    // if program has not been found, clear argument list :
    if (nicePath.isEmpty()) {
        niceProcessArgs.clear();
    }

    return niceProcessArgs;

}

