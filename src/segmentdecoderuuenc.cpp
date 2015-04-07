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


#include "segmentdecoderuuenc.h"

#include "kwooty_debug.h"

#include "kwootysettings.h"
#include "segmentsdecoderthread.h"



SegmentDecoderUUEnc::SegmentDecoderUUEnc(SegmentsDecoderThread* parent) : SegmentDecoderBase(parent) {
}

SegmentDecoderUUEnc::~SegmentDecoderUUEnc() {
}


void SegmentDecoderUUEnc::decodeProgression(PostDownloadInfoData& decodeInfoData) {

    decodeInfoData.setCrc32Match(this->crc32Match);
    decodeInfoData.setArticleEncodingType(ArticleEncodingUUEnc);

    this->segmentsDecoderThread->emitDecodeProgression(decodeInfoData);

}



void SegmentDecoderUUEnc::decodeSegments(NzbFileData currentNzbFileData, const QString& fileNameStr) {

    // init variables :
    this->parentIdentifer = currentNzbFileData.getUniqueIdentifier();
    this->segmentDataList = currentNzbFileData.getSegmentList();
    this->crc32Match = false;


    // decodeInfoData sent by defaut :
    PostDownloadInfoData decodeInfoData;
    decodeInfoData.initDecode(this->parentIdentifer, PROGRESS_COMPLETE, DecodeErrorStatus);


    QString fileSavePath = currentNzbFileData.getFileSavePath();
    if (Utility::createFolder(fileSavePath)) {

        QFile targetFile(Utility::buildFullPath(fileSavePath, fileNameStr));
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
            qCDebug(KWOOTY_LOG) << "can not create " << Utility::buildFullPath(fileSavePath, fileNameStr);

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




bool SegmentDecoderUUEnc::decodeSegmentFiles(QFile& targetFile) {

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

            QString pathNameFile = Utility::buildFullPath(Settings::temporaryFolder().path(), currentSegment.getPart());
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

        this->segmentsDecoderThread->emitSaveFileError();
    }

    return encodedDataFound;

}





void SegmentDecoderUUEnc::decodeEncodedData(QFile& targetFile, SegmentData& currentSegment, int& segmentCrc32MatchNumber, const QByteArray& segmentByteArray, bool& encodedDataFound, bool& writeError) {


    // crc32 can not be checked for uu encoded files, set it to false :
    this->crc32Match = false;

    // decode encoded data and check crc :
    writeError = this->decodeUUenc(segmentByteArray, targetFile, currentSegment.getElementInList());

    // encoded data in at least one file has been found :
    encodedDataFound = true;

    // no used for uuenc, set it to 0 :
    segmentCrc32MatchNumber = 0;

}



bool SegmentDecoderUUEnc::decodeUUenc(const QByteArray& captureArray, QFile& targetFile, const int& elementInList) {

    bool writeError = false;

    QByteArray decodeArray;

    QList<QByteArray> uuEncodedLines = captureArray.split('\n');

    foreach (QByteArray uuLine, uuEncodedLines) {

        uuLine = uuLine.trimmed();

        if (this->isUUEncodedLine(uuLine)) {

            int counter = 0;

            // first char corresponds to line size :
            for (int i = 1; i < uuLine.size() - 1; i++) {

                // skip current loop (get groups of 4 bytes):
                if ( counter == 3) {
                    counter = 0;
                }
                // process decoding :
                else {
                    int shiftLeft = (2 * counter) + 2;
                    int shiftRight = 6 - shiftLeft;

                    decodeArray.append( ((uuLine.at(i) - 32) & 077) << shiftLeft |
                                        ((uuLine.at(i + 1) - 32) & 077 )>> shiftRight );


                    counter++;
                }

            }

        }

    }


    // write decoded array in the target file :
    if (targetFile.write(decodeArray) == -1) {
        writeError = true;
    }

    // send decoding progression :
    PostDownloadInfoData decodeInfoData;
    decodeInfoData.initDecode(this->parentIdentifer, qRound(((double)(elementInList * 100) / (double)(this->segmentDataList.size()))), DecodeStatus);
    this->decodeProgression(decodeInfoData);

    return writeError;

}




bool SegmentDecoderUUEnc::isUUEncodedLine(QByteArray& currentLine) {

    bool uuEncodedLine = false;

    // If line is size 61, first character should be "M" :
    if ( (currentLine.size() == UUMaxEncodedLineSize) && (currentLine.at(0) == 'M') ) {
        uuEncodedLine = true;
    }
    // If size is lower, check first character :
    else if ( (currentLine.size() > 0) && (currentLine.size() < UUMaxEncodedLineSize) ) {

        int dataSize = (int)(currentLine.at(0) - 32);
        int dataSizeComputed = (currentLine.size() - 1) * 3 / 4;

        if (dataSize == dataSizeComputed) {
            uuEncodedLine = true;
        }

        // if dataSizeComputed is bigger, maybe padding characters have been added :
        else if ( (dataSizeComputed > dataSize) &&
                  (dataSize > 0) && (dataSize < UUMaxDecodedLineSize) ) {

            int paddedCharSize = dataSizeComputed - dataSize;

            // check if exceed characters are padded ones :
            if (currentLine.right(paddedCharSize) == QByteArray(paddedCharSize, '`')) {

                // remove padded characters before decoding process :
                currentLine.chop(paddedCharSize);
                uuEncodedLine = true;
            }
        }

    }

    // if line match previous criteria, check that all characters belong to uuencode character table :
    if (uuEncodedLine) {

        foreach (const char& currentChar, currentLine) {

            // if characters are out of scope, this is not a uuEncoded line :
            if ( (currentChar < ' ') ||
                 ((currentChar > '_') && (currentChar != '`')) ) {

                uuEncodedLine = false;
                break;

            }


        }
    }

    return uuEncodedLine;

}


QString SegmentDecoderUUEnc::searchPattern(QIODevice* segmentFile) {

    QString fileName;
    bool uuEncodedDataFound = false;


    while (!segmentFile->atEnd()) {

        QByteArray uuNamePattern = "begin";
        QByteArray yNamePattern = "=ybegin";
        QByteArray lineArray = segmentFile->readLine().trimmed();

        // if 'begin' pattern has been found, retrieve the file name :
        if (lineArray.contains(uuNamePattern) && !lineArray.contains(yNamePattern) ) {

            // pattern expected : "begin 644 fileName"
            QList<QByteArray> wordList =  lineArray.split(' ');

            // retrieve file name,
            // file name could have spaces in it, retrieve the whole file name (thanks to Nicholas Redgrave) :
            if (wordList.size() > 2) {

                QString modeWord = wordList.at(1);

                fileName = lineArray.mid(lineArray.indexOf(modeWord) + modeWord.size());

                // remove whitespace from start and end of just retrieved file name :
                fileName = fileName.trimmed();

            }

        }

        // then check that next lines are UU encoded data :
        if (!fileName.isEmpty()) {

            //qCDebug(KWOOTY_LOG) << "file name : " << fileName;
            if (this->isUUEncodedLine(lineArray)) {

                uuEncodedDataFound = true;
                break;
            }

        }

    }

    // reset fileName as no uu Encoded data found :
    if (!uuEncodedDataFound) {
        fileName = QString();
    }

    return fileName;

}


