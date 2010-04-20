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


#include "segmentdecoderbase.h"

#include <KDebug>

#include <QDir>

#include "settings.h"
#include "data/segmentdata.h"
#include "segmentsdecoderthread.h"


SegmentDecoderBase::SegmentDecoderBase() {


}


void SegmentDecoderBase::decodeProgression(const int progression, const UtilityNamespace::ItemStatus status, const QString& decodedFileName){
    emit updateDecodeSignal(this->parentIdentifer, progression, status, decodedFileName, this->crc32Match);
}



void SegmentDecoderBase::decodeSegments(NzbFileData currentNzbFileData, const QString& fileNameStr) {

    // init variables :
    this->parentIdentifer = currentNzbFileData.getUniqueIdentifier();
    this->segmentDataList = currentNzbFileData.getSegmentList();
    this->crc32Match = false;


    QString fileSavePath = currentNzbFileData.getFileSavePath();
    if (Utility::createFolder(fileSavePath)) {

        QFile targetFile(fileSavePath + '/' + fileNameStr);
        // open target file into write mode :
        if (targetFile.open(QIODevice::WriteOnly)) {

            // decode all segments and write them into the file :
            bool encodedDataFound = this->decodeSegmentFiles(targetFile);

            if (encodedDataFound) {
                // decoding is over, also send name of decoded file :
                this->decodeProgression(PROGRESS_COMPLETE, DecodeFinishStatus, fileNameStr);
            }
            // no data found in any segments, notify user about issue :
            else {
                this->decodeProgression(PROGRESS_COMPLETE, DecodeErrorStatus);
            }


            // close the file :
            targetFile.close();

        }
        // can not create the file :
        else {
            this->decodeProgression(PROGRESS_COMPLETE, DecodeErrorStatus);
            kDebug() << "can not create " << fileSavePath + '/' + fileNameStr;
        }

    }
    // notify user of the issue :
    else {
        emit saveFileErrorSignal(DuringDecode);
    }



    // clear variables :
    this->parentIdentifer.clear();
    this->segmentDataList.clear();

}




bool SegmentDecoderBase::decodeSegmentFiles(QFile& targetFile) {

    bool encodedDataFound = false;
    bool writeError = false;

    // decoding is starting :
    this->decodeProgression(PROGRESS_INIT, DecodeStatus);

    // count the number of segments with matchingCrc :
    int segmentCrc32MatchNumber = 0;

    // scan every files to decode :
    foreach (SegmentData currentSegment, this->segmentDataList) {

        // if segment has been downloaded :
        if (currentSegment.getArticlePresenceOnServer() == Present) {

            QString temporaryFolder = Settings::temporaryFolder().path() + '/';
            QString pathNameFile = temporaryFolder + currentSegment.getPart();
            QFile segmentFile(pathNameFile);

            segmentFile.open(QIODevice::ReadOnly);

            // read the whole file :
            QByteArray segmentByteArray = segmentFile.readAll();

            // decode specialisation (yenc or uuenc according to object instance) :
            this->decodeEncodedData(targetFile, currentSegment, segmentCrc32MatchNumber, segmentByteArray, encodedDataFound, writeError);

            // close the segment file :
            segmentFile.close();
            // then remove it :
            segmentFile.remove();

        }
        // if the segment was not present on the server :
        else {
            // file shall be repaired, set crc32Match to false :
            this->crc32Match = false;
        }

    }


    // if all segments have a correct crc32 :
    if (segmentDataList.size() != segmentCrc32MatchNumber) {
        this->crc32Match = false;
    }


    // end of decoding management :
    if (writeError) {

        encodedDataFound = false;
        this->crc32Match = false;

        emit saveFileErrorSignal(DuringDecode);
    }

    return encodedDataFound;

}



QString SegmentDecoderBase::scanSegmentFiles(const NzbFileData& currentNzbFileData) {

    this->parentIdentifer = currentNzbFileData.getUniqueIdentifier();
    this->segmentDataList = currentNzbFileData.getSegmentList();

    QString fileName;

    //notify item that list of files is being scanned :
    if (!this->segmentDataList.isEmpty()) {
        this->decodeProgression(PROGRESS_INIT, ScanStatus);
    }

    // scan every files to be decoded :
    foreach (SegmentData currentSegment, this->segmentDataList) {

        QString temporaryFolder = Settings::temporaryFolder().path() + '/';

        // if segment has been downloaded :
        if (currentSegment.getArticlePresenceOnServer() == Present) {

            QString pathNameFile = temporaryFolder + currentSegment.getPart();
            QFile segmentFile(pathNameFile);

            // open the current segment file :
            segmentFile.open(QIODevice::ReadOnly);


             // look for file name :
            fileName = this->searchPattern(segmentFile);

            // close the current segment file :
            segmentFile.close();

        }

        // file name has been found, stop scanning files :
        if (!fileName.isEmpty()) {
            break;
        }

    } // end of loop


    return fileName;

}


