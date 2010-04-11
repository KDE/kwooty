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
#include <KDebug>


SegmentDecoderUUEnc::SegmentDecoderUUEnc() { }


SegmentDecoderUUEnc::~SegmentDecoderUUEnc() { }




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

    foreach(QByteArray uuLine, uuEncodedLines) {

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
    this->decodeProgression(qRound((elementInList * 100 / this->segmentDataList.size()) ), DecodeStatus);

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

        foreach (char currentChar, currentLine) {

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



QString SegmentDecoderUUEnc::searchPattern(QFile& segmentFile) {

    QString fileName;
    bool uuEncodedDataFound = false;


    while (!segmentFile.atEnd()) {

        QByteArray uuNamePattern = "begin";
        QByteArray yNamePattern = "=ybegin";
        QByteArray lineArray = segmentFile.readLine().trimmed();

        // if 'begin' pattern has been found, retrieve the file name :
        if (lineArray.contains(uuNamePattern) && !lineArray.contains(yNamePattern) ) {

            // pattern expected : "begin 644 fileName"
            QList<QByteArray> wordList =  lineArray.split(' ');

            // retrieve file name :
            if (wordList.size() == 3) {
                fileName = wordList.at(wordList.size() - 1);
            }

        }

        // then check that next lines are UU encoded data :
        if (!fileName.isEmpty()) {

            //kDebug() << "file name : " << fileName;
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


