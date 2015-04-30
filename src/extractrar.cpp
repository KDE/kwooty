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

#include "extractrar.h"

#include "kwooty_debug.h"

#include "repairdecompressthread.h"
#include "kwootysettings.h"
#include "data/nzbfiledata.h"

ExtractRar::ExtractRar(RepairDecompressThread *parent) : ExtractBase(parent)
{

    this->archiveFormat = RarFormat;
}

ExtractRar::~ExtractRar() { }

QStringList ExtractRar::createProcessArguments(const QString &archiveName, const QString &fileSavePath, const bool &passwordEnteredByUSer, const QString &passwordStr)
{

    QStringList args;
    // first pass : check if archive is passworded :
    if (this->archivePasswordStatus == ExtractBase::ArchiveCheckIfPassworded) {
        args.append("l");
        args.append("-p-");
        args.append(Utility::buildFullPath(fileSavePath, archiveName));
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

        args.append(Utility::buildFullPath(fileSavePath, archiveName));
        args.append(fileSavePath);

    }

    qCDebug(KWOOTY_LOG) << args;

    return args;

}

void ExtractRar::extractUpdate(const QString &line)
{

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

        this->archivePasswordStatus = ExtractBase::ArchiveIsPassworded;
        qCDebug(KWOOTY_LOG) << "password incorrect";
    }

    // get files with crc errors :
    else if (line.contains("CRC failed")) {

        QRegExp regExp(".*(/.*/)+(.*)?");

        // if file with bad crc found :
        if (regExp.exactMatch(line)) {

            QString fileNameStr = regExp.cap(2);
            // search corresponding file into the list :
            this->findItemAndNotifyUser(fileNameStr, ExtractBadCrcStatus, ChildItemTarget);
        }
    }

    // search current processed archive :
    else if (line.contains("Extracting from")) {

        QRegExp regExp(".*(/.*/)+(.*)?");

        // if current extracted file has been found :
        if (regExp.exactMatch(line)) {

            QString fileNameStr = regExp.cap(2);
            // search corresponding file into the list :
            this->findItemAndNotifyUser(fileNameStr, ExtractStatus, BothItemsTarget);
        }
    }

}

void ExtractRar::checkIfArchivePassworded(const QString &currentLine, bool &passwordCheckIsNextLine)
{

    if (passwordCheckIsNextLine) {

        // "*" means the archive is passworded, look for it :
        if (currentLine.left(1) == "*") {
            this->archivePasswordStatus = ExtractBase::ArchiveIsPassworded;
        } else {
            // the archive is not passworded :
            this->archivePasswordStatus = ExtractBase::ArchiveIsNotPassworded;
        }

    }

    // search this pattern in order to know that next line indicate if archive is passworded or not :
    QString unrar4PasswordPattern = "------------------";
    QString unrar5PasswordPattern = "----------- ---------";

    if (currentLine.contains(unrar4PasswordPattern) ||
            currentLine.contains(unrar5PasswordPattern)) {

        passwordCheckIsNextLine = true;

    }

}

void ExtractRar::sendExtractProgramNotFoundNotification()
{

    // notify parent that program is missing :
    NzbFileData nzbFileData = this->getFirstArchiveFileFromList();

    this->emitProcessUpdate(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, UnrarProgramMissing, ParentItemTarget);

    // notify repairDecompressThread that extraction is over :
    emit extractProcessEndedSignal();

}

QString ExtractRar::searchExtractProgram()
{
    return Utility::searchExternalPrograms(UtilityNamespace::rarExtractProgram, this->isExtractProgramFound);

}
