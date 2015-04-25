/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
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


#include "extractzip.h"

#include "kwooty_debug.h"
#include <QFileInfo>

#include "repairdecompressthread.h"
#include "kwootysettings.h"
#include "data/nzbfiledata.h"


ExtractZip::ExtractZip(RepairDecompressThread* parent) : ExtractBase(parent) {

    this->archiveFormat = ZipOrSevenZipFormat;

    connect (this->extractProcess, SIGNAL(started()), this, SLOT(startedSlot()));

}


ExtractZip::~ExtractZip() { }



QStringList ExtractZip::createProcessArguments(const QString& archiveName, const QString& fileSavePath, const bool& passwordEnteredByUSer, const QString& passwordStr) {

    QStringList args;
    // first pass : check if archive is passworded :
    if (this->archivePasswordStatus == ExtractBase::ArchiveCheckIfPassworded) {
        args.append("l");
        args.append("-slt");
        args.append( Utility::buildFullPath(fileSavePath, archiveName) );
    }
    // second pass : extract archive with or without a password :
    else {

        this->archivePasswordStatus = ArchivePasswordCheckEnded;

        args.append("x");

        // overwrite the output file if option choosen in settings :
        if (Settings::overwriteExtractedFiles()) {
            args.append("-aoa");
        }
        // rename the output file if option choosen in settings :
        else {
            args.append("-aou");
        }

        args.append("-y");

        // if archive is passworded :
        if (passwordEnteredByUSer) {

            // set password entered by the user :
            if (!passwordStr.isEmpty()) {
                args.append("-p" + passwordStr);
            }
            // user only click ok without entering a password :
            else {
                args.append("-p");
            }

        }

        // set output directory :
        args.append("-o" + fileSavePath);

        args.append( Utility::buildFullPath(fileSavePath,archiveName) );


    }

    return args;

}



void ExtractZip::startedSlot() {

    // readyread slots for 7z command are only received when 7z command is about to finish
    // no status notifying is received during extract process, so set archive files as being extracted
    // just after 7z command has been launched :

    this->extractProgressValue = PROGRESS_UNKNOWN;

    for (int i = 0; i < this->nzbFileDataList.size(); ++i) {

        NzbFileData nzbFileData = this->nzbFileDataList.at(i);

        // get file extension :
        QFileInfo fileInfo(nzbFileData.getDecodedFileName());
        QString suffixName = fileInfo.suffix();

        // 7z and zip splitted files should have the following pattern : file.001, file.002, ...
        // file archive detection differs from rar ones as for 7z format,
        // only file.001 will embbed the magic number "7z", the other ones can not be identified.
        // so, if extension can be converted to int value, the file is a zip or 7z file
        // set this file as currently being extracted :
        bool intConversionSucceeded;
        suffixName.toInt(&intConversionSucceeded, 10);

        if (nzbFileData.isArchiveFile() || intConversionSucceeded) {

            this->findItemAndNotifyUser(nzbFileData.getDecodedFileName(), ExtractStatus, BothItemsTarget);
        }

    }

}





void ExtractZip::extractUpdate(const QString& line) {


    if (line.contains("Wrong password")) {

        this->archivePasswordStatus = ExtractBase::ArchiveIsPassworded;
        qCDebug(KWOOTY_LOG) << "password incorrect";
    }

    // search current processed archive :
    else if (line.contains("CRC Failed")) {

        this->extractProgressValue = PROGRESS_COMPLETE;

        // set each file currently as "Extracting" to "Bad Crc" status :
        foreach (const NzbFileData& nzbFileData, this->nzbFileDataList) {
            this->findItemAndNotifyUser(nzbFileData.getDecodedFileName(), ExtractBadCrcStatus, ChildItemTarget);
        }

    }


}



void ExtractZip::checkIfArchivePassworded(const QString& currentLine, bool& passwordCheckIsNextLine) {


    // if line contains "Encrypted", search if it contains "+" or "-" :
    if (currentLine.contains("Encrypted")) {

        if (currentLine.right(1) == "+") {
            this->archivePasswordStatus = ExtractBase::ArchiveIsPassworded;
        }
        else {
            // the archive is not passworded :
            this->archivePasswordStatus = ExtractBase::ArchiveIsNotPassworded;
        }
    }


    // not needed for 7z, leave it to false :
    passwordCheckIsNextLine = false;

}




void ExtractZip::sendExtractProgramNotFoundNotification() {

    // notify parent that program is missing :
    NzbFileData nzbFileData = this->getFirstArchiveFileFromList();

    this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, SevenZipProgramMissing, ParentItemTarget);

    // notify repairDecompressThread that extraction is over :
    emit extractProcessEndedSignal();

}


QString ExtractZip::searchExtractProgram() {
    return Utility::searchExternalPrograms(UtilityNamespace::sevenZipExtractProgram, this->isExtractProgramFound);
}
