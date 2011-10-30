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

#include "kwootysettings.h"
#include "data/segmentdata.h"
#include "segmentsdecoderthread.h"


SegmentDecoderBase::SegmentDecoderBase(SegmentsDecoderThread* parent) : QObject(parent) {

    this->segmentsDecoderThread = parent;

}



void SegmentDecoderBase::decodeSegments(NzbFileData currentNzbFileData, const QString& fileNameStr) {

    // init variables :
    this->parentIdentifer = currentNzbFileData.getUniqueIdentifier();
    this->segmentDataList = currentNzbFileData.getSegmentList();
    this->crc32Match = false;


    // decodeInfoData sent by defaut :
    PostDownloadInfoData decodeInfoData;
    decodeInfoData.initDecode(this->parentIdentifer, PROGRESS_COMPLETE, DecodeErrorStatus);


    QString fileSavePath = currentNzbFileData.getFileSavePath();
    if (Utility::createFolder(fileSavePath)) {

        QFile targetFile(fileSavePath + '/' + fileNameStr);
        // open target file into write mode :
        if (targetFile.open(QIODevice::WriteOnly)) {

            // decode all segments and write them into the file :
            bool encodedDataFound = this->decodeSegmentFiles(targetFile);

            if (encodedDataFound) {

                // decoding is over, also send name of decoded file :
                decodeInfoData.setStatus(DecodeFinishStatus);
                decodeInfoData.setDecodedFileName(fileNameStr);
                this->decodeProgression(decodeInfoData);

            }
            // no data found in any segments, notify user about issue :
            else {
                this->decodeProgression(decodeInfoData);
            }


            // close the file :
            targetFile.close();

        }
        // can not create the file :
        else {
            this->decodeProgression(decodeInfoData);
            kDebug() << "can not create " << fileSavePath + '/' + fileNameStr;

        }

    }
    // notify user of the issue :
    else {
        this->decodeProgression(decodeInfoData);
        this->segmentsDecoderThread->emitSaveFileError();
    }



    // clear variables :
    this->parentIdentifer.clear();
    this->segmentDataList.clear();

}




bool SegmentDecoderBase::decodeSegmentFiles(QFile& targetFile) {

    bool encodedDataFound = false;
    bool writeError = false;

    // decoding is starting :
    PostDownloadInfoData decodeInfoData;
    decodeInfoData.initDecode(this->parentIdentifer, PROGRESS_INIT, DecodeStatus);

    this->decodeProgression(decodeInfoData);

    // count the number of segments with matchingCrc :
    int segmentCrc32MatchNumber = 0;

    // scan every files to decode :
    foreach (SegmentData currentSegment, this->segmentDataList) {

        // if segment has been downloaded :
        if (currentSegment.getArticlePresenceOnServer() == Present) {

            QIODevice* segmentFile = currentSegment.getIoDevice();

            segmentFile->open(QIODevice::ReadOnly);

            // read the whole file :
            QByteArray segmentByteArray = segmentFile->readAll();

            // decode specialisation (yenc or uuenc according to object instance) :
            this->decodeEncodedData(targetFile, currentSegment, segmentCrc32MatchNumber, segmentByteArray, encodedDataFound, writeError);

            // close the segment file :
            segmentFile->close();

            // check if segmentFile inherits QFile :
            QFile* fileFromDisk = qobject_cast<QFile*>(segmentFile);

            if (fileFromDisk) {
                // then remove it :
                fileFromDisk->remove();
            }


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

        this->segmentsDecoderThread->emitSaveFileError();
    }

    return encodedDataFound;

}



QString SegmentDecoderBase::scanSegmentFiles(const NzbFileData& currentNzbFileData) {

    this->parentIdentifer = currentNzbFileData.getUniqueIdentifier();
    this->segmentDataList = currentNzbFileData.getSegmentList();

    QString fileName;

    //notify item that list of files is being scanned :
    if (!this->segmentDataList.isEmpty()) {

        PostDownloadInfoData decodeInfoData;
        decodeInfoData.initDecode(this->parentIdentifer, PROGRESS_INIT, ScanStatus);
        this->decodeProgression(decodeInfoData);

    }

    // scan every files to be decoded :
    foreach (SegmentData currentSegment, this->segmentDataList) {

        QString temporaryFolder = Settings::temporaryFolder().path() + '/';

        // if segment has been downloaded :
        if (currentSegment.getArticlePresenceOnServer() == Present) {

            // open the current segment file :
            QIODevice* ioDevice = currentSegment.getIoDevice();
            ioDevice->open(QIODevice::ReadOnly);


             // look for file name :
            fileName = this->searchPattern(ioDevice);

            // close the current segment file :
            ioDevice->close();

        }

        // file name has been found, stop scanning files :
        if (!fileName.isEmpty()) {
            break;
        }

    } // end of loop


    return fileName;

}


