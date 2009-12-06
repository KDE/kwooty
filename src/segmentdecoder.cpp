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

#include "segmentdecoder.h"

#include <KDebug>
#include <QDir>
#include "settings.h"
#include "data/segmentdata.h"
#include "segmentsdecoderthread.h"


SegmentDecoder::SegmentDecoder() {
    isDecodingStatus = false;
}

SegmentDecoder::~SegmentDecoder() { }




void SegmentDecoder::decodeProgression(const int progression, const UtilityNamespace::ItemStatus status, const QString& decodedFileName){
    emit updateDecodeSignal(this->parentIdentifer, progression, status, decodedFileName);
}



void SegmentDecoder::decodeSegments(NzbFileData currentNzbFileData){

    // init variables :
    this->isDecodingStatus = true;
    this->parentIdentifer = currentNzbFileData.getUniqueIdentifier();
    this->segmentDataList = currentNzbFileData.getSegmentList();

    // scan segment files in order to get the file name of the corresponding file :
    QString fileNameStr = this->scanSegmentFiles();

    if (!fileNameStr.isEmpty()) {

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

    }
    else {
        // file name has not been found is all segments, send a decodeErrorStatus :
        this->decodeProgression(PROGRESS_COMPLETE, DecodeErrorStatus);
    }


    // clear variables :
    this->parentIdentifer.clear();
    this->segmentDataList.clear();
    this->isDecodingStatus = false;

}





bool SegmentDecoder::decodeSegmentFiles(QFile& targetFile){

    bool encodedDataFound = false;
    bool writeError = false;

    // decoding is starting :
    this->decodeProgression(PROGRESS_INIT, DecodeStatus);

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

            int yDataBeginPos = 0;
            // get first line of the segment :
            QByteArray yBeginArray = this->getLineByteArray("=ybegin", segmentByteArray, yDataBeginPos);
            //kDebug() << beginArray;

            // if it is a multi part, get the next line :
            if (yBeginArray.contains("part=")) {
                QByteArray yPartArray = this->getLineByteArray("=ypart", segmentByteArray, yDataBeginPos);
                //kDebug() << yPartArray;
            }

            // get the last line :
            int endPos = 0;
            QByteArray yEndArray = this->getLineByteArray("=yend", segmentByteArray, endPos);

            // by default take the whole byteArray to decode in case '=yend' line not present
            // (could occur if data has been corrupted on server) :
            int yDataEndPos = segmentByteArray.length();

            // if '=yend' pattern has been found, get the proper data end position :
            if (!yEndArray.isEmpty()) {
                yDataEndPos = segmentByteArray.indexOf("\n=yend") - 1;
            }

            bool ok;
            QString crcPattern = "crc32=";
            int crcValuePos = yEndArray.indexOf(crcPattern) + crcPattern.size();
            quint32 crcStr = yEndArray.mid(crcValuePos, yEndArray.size() - crcValuePos).trimmed().toLongLong(&ok, 16);
            //kDebug() << "crcStr" << crcStr;


            // get the yy encoded data :
            QByteArray captureArray = segmentByteArray.mid(yDataBeginPos , yDataEndPos - yDataBeginPos);

            if (!captureArray.isEmpty()) {
                // decode encoded data and check crc :
                writeError = this->decodeYenc(captureArray, targetFile, currentSegment.getElementInList());

                // encoded data in at least one file has been found :
                encodedDataFound = true;
            }

            // close the segment file :
            segmentFile.close();
            // then remove it :
            segmentFile.remove();


        }


    }

    // end of decoding management :
    if (writeError) {

        this->decodeProgression(PROGRESS_COMPLETE, DecodeErrorStatus);

        emit saveFileErrorSignal(DuringDecode);
    }

    return encodedDataFound;

}


QByteArray SegmentDecoder::getLineByteArray(const QString& lineBeginPattern, const QByteArray& segmentByteArray, int& yDataBeginPos){

    int beginLinePos = segmentByteArray.indexOf(lineBeginPattern);
    int endLinePos = segmentByteArray.indexOf("\n", beginLinePos) + 1;

    // indicate the position of the next line :
    yDataBeginPos = endLinePos;

    return segmentByteArray.mid(beginLinePos, endLinePos - beginLinePos);

}





bool SegmentDecoder::decodeYenc(QByteArray& captureArray, QFile& targetFile, const int& elementInList){

    bool writeError = false;

    // used for crc32 computation :
    quint32 hash = 0xffffffff;

    QByteArray decodeArray;
    bool specialCharacter = false;


    foreach (char encodedCharacter, captureArray) {      

        // decode char :
        if (encodedCharacter != '\r' && encodedCharacter != '\n') {

            bool decoded = false;
            char decodedCharacter;

            if (specialCharacter) {

                decodedCharacter = (encodedCharacter - 106) % 256;
                decoded = true;
                specialCharacter = false;
            }
            else {
                if (encodedCharacter == '=') {
                    specialCharacter = true;
                }
                else {

                    decodedCharacter = (encodedCharacter - 42) % 256;
                    decoded = true;
                }
            }

            if (decoded) {
                //  append decoded character :
                decodeArray.append(decodedCharacter);

                // compute crc32 part for this char :
                hash = this->computeCrc32Part(hash, decodedCharacter);
            }

        }

    }

    // write decoded array in the target file :
    if (targetFile.write(decodeArray) == -1) {
        writeError = true;
    }


    // finish crc 32 computation of encoded segment data :
    hash ^= 0xffffffff;
    //kDebug() << "hash" << hash;

    // send decoding progression :
    this->decodeProgression(qRound((elementInList * 100 / this->segmentDataList.size()) ), DecodeStatus);

    return writeError;

}




quint32 SegmentDecoder::computeCrc32Part(quint32& hash, unsigned char data) {
    return crc::crcArray[(hash^data) & 0xff]^(hash >> 8);
}



bool SegmentDecoder::isDecoding(){
    return isDecodingStatus;
}




QString SegmentDecoder::scanSegmentFiles() {

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
            while (fileName.isEmpty() && !segmentFile.atEnd()) {

                QByteArray namePattern = "name=";
                QByteArray lineArray = segmentFile.readLine();

                // if 'name=' pattern has been found, retrieve the file name :
                if (lineArray.contains(namePattern)) {

                    int nameValuePos = lineArray.indexOf(namePattern) + namePattern.size();
                    fileName = lineArray.mid(nameValuePos, lineArray.size() - nameValuePos).trimmed();
                    break;
                }
            }

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

