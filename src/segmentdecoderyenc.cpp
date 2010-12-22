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

#include "segmentdecoderyenc.h"
#include <KDebug>


SegmentDecoderYEnc::SegmentDecoderYEnc(SegmentsDecoderThread* parent) : SegmentDecoderBase(parent){
}

SegmentDecoderYEnc::~SegmentDecoderYEnc() {
}


void SegmentDecoderYEnc::decodeProgression(const int progression, UtilityNamespace::ItemStatus status, const QString& decodedFileName){

    emit updateDecodeSignal(this->parentIdentifer, progression, status, decodedFileName, this->crc32Match);

}


void SegmentDecoderYEnc::decodeEncodedData(QFile& targetFile, SegmentData& currentSegment, int& segmentCrc32MatchNumber, const QByteArray& segmentByteArray, bool& encodedDataFound, bool& writeError) {


    int yDataBeginPos = 0;
    // get first line of the segment :
    QByteArray yBeginArray = this->getLineByteArray("=ybegin", segmentByteArray, yDataBeginPos);

    // if it is a multi part, get the next line :
    if (yBeginArray.contains("part=")) {
        QByteArray yPartArray = this->getLineByteArray("=ypart", segmentByteArray, yDataBeginPos);
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

    // retrieve crc32 value :
    bool ok;
    QString crcPattern = "crc32=";
    int crcValuePos = yEndArray.indexOf(crcPattern) + crcPattern.size();
    quint32 crc32FromFile = yEndArray.mid(crcValuePos, yEndArray.size() - crcValuePos).trimmed().toLongLong(&ok, 16);


    // get the yy encoded data :
    QByteArray captureArray = segmentByteArray.mid(yDataBeginPos , yDataEndPos - yDataBeginPos);

    if (!captureArray.isEmpty()) {

        // at this stage set crc32 value at true by default :
        this->crc32Match = true;

        // decode encoded data and check crc :
        writeError = this->decodeYenc(captureArray, targetFile, currentSegment.getElementInList(), crc32FromFile, segmentCrc32MatchNumber);

        // encoded data in at least one file has been found :
        encodedDataFound = true;
    }

}





QByteArray SegmentDecoderYEnc::getLineByteArray(const QString& lineBeginPattern, const QByteArray& segmentByteArray, int& yDataBeginPos){

    int beginLinePos = segmentByteArray.indexOf(lineBeginPattern);
    int endLinePos = segmentByteArray.indexOf("\n", beginLinePos) + 1;

    // indicate the position of the next line :
    yDataBeginPos = endLinePos;

    return segmentByteArray.mid(beginLinePos, endLinePos - beginLinePos);

}





bool SegmentDecoderYEnc::decodeYenc(QByteArray& captureArray, QFile& targetFile, const int& elementInList, const quint32& crc32FromFile, int& segmentCrc32MatchNumber){

    bool writeError = false;

    // used for crc32 computation :
    quint32 crc32Computed = 0xffffffff;

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
                crc32Computed = this->computeCrc32Part(crc32Computed, decodedCharacter);
            }

        }

    }

    // write decoded array in the target file :
    if (targetFile.write(decodeArray) == -1) {
        writeError = true;
    }


    // finish crc 32 computation of encoded segment data :
    crc32Computed ^= 0xffffffff;
    //kDebug() << "crcComputed" << crcComputed;

    //check crc32 values :
    if (crc32FromFile == crc32Computed) {
        segmentCrc32MatchNumber++;
    }




    // send decoding progression :
    this->decodeProgression(qRound((elementInList * 100 / this->segmentDataList.size()) ), DecodeStatus);

    return writeError;

}




quint32 SegmentDecoderYEnc::computeCrc32Part(quint32& hash, unsigned char data) {
    return crc::crcArray[(hash^data) & 0xff]^(hash >> 8);
}





QString SegmentDecoderYEnc::searchPattern(QFile& segmentFile) {

    QString fileName;
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


    return fileName;

}
