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

#include "kwooty_debug.h"

#include "kwootysettings.h"
#include "segmentsdecoderthread.h"

SegmentDecoderYEnc::SegmentDecoderYEnc(SegmentsDecoderThread *parent) : SegmentDecoderBase(parent)
{

}

SegmentDecoderYEnc::~SegmentDecoderYEnc()
{
}

void SegmentDecoderYEnc::decodeProgression(PostDownloadInfoData &decodeInfoData)
{

    decodeInfoData.setCrc32Match(this->mCrc32Match);
    decodeInfoData.setArticleEncodingType(ArticleEncodingYEnc);

    this->mSegmentsDecoderThread->emitDecodeProgression(decodeInfoData);
}

bool SegmentDecoderYEnc::decodeEncodedData(const QString &temporaryFolder, SegmentData &segmentData, const QString &decodedFileName, bool &writeSuccess)
{

    this->mCrc32Match = false;

    // /!\ be sure that destination file has not already been moved in target folder.
    // this can happen only when a retry action has been performed :
    // file has already been post-processed and should be moved again in temporary folder to write decoded segment in it :
    QString temporaryFileStr = Utility::buildFullPath(temporaryFolder, segmentData.getSegmentInfoData().getTemporaryFileName());
    QString destinationFileStr = Utility::buildFullPath(segmentData.getSegmentInfoData().getDestinationFileSavePath(), decodedFileName);
    bool renamed = Utility::rename(destinationFileStr, temporaryFileStr);

    // check that temporary folder is already created, else create it :
    Utility::createFolder(temporaryFolder);

    QFile temporaryFile(Utility::buildFullPath(temporaryFolder, segmentData.getSegmentInfoData().getTemporaryFileName()));
    temporaryFile.open(QIODevice::ReadWrite);

    // retrieve data to decode :
    segmentData.getIoDevice()->open(QIODevice::ReadOnly);
    QByteArray segmentByteArray = segmentData.getIoDevice()->readAll();

    // get first line of the segment :
    int yDataBeginPos = 0;
    QByteArray yBeginArray = this->getLineByteArray("=ybegin", segmentByteArray, yDataBeginPos);

    qint64 fullSizeValue = this->getPatternValue(yBeginArray, "size=");

    if (renamed ||
            temporaryFile.size() < fullSizeValue) {

        writeSuccess = temporaryFile.resize(fullSizeValue + static_cast<qint64>(applicationFileOwner.size()));

        // go to the end of the file and put kwooty tag to be sure to delete only files belonging to kwooty
        // during file cleaning process :
        writeSuccess = temporaryFile.seek(fullSizeValue);
        temporaryFile.write(applicationFileOwner.toLatin1());
    }

    // if it is a multi part, get the next line :
    qint64 beginValue = 0;
    qint64 endValue = 0;
    bool singlePart = true;

    if (yBeginArray.contains("part=")) {

        singlePart = false;
        QByteArray yPartArray = this->getLineByteArray("=ypart", segmentByteArray, yDataBeginPos);

        // retrieve begin offset value :
        beginValue = this->getPatternValue(yPartArray, "begin=");

        // retrieve end offset value :
        endValue = this->getPatternValue(yPartArray, "end=");

    }

    // get the last line :
    int endPos = 0;
    QByteArray yEndArray = this->getLineByteArray("=yend", segmentByteArray, endPos);

    // if '=yend' pattern has been found, get the proper data end position :
    int yDataEndPos = segmentByteArray.indexOf("\n=yend") - 1;

    // retrieve segment size value :
    qint64 sizeValue = this->getPatternValue(yEndArray, "size=");

    // if segment is a single part or segment size is matching :
    if (singlePart ||
            (sizeValue == endValue - beginValue + 1)) {

        // get the yy encoded data :
        QByteArray captureArray = segmentByteArray.mid(yDataBeginPos , yDataEndPos - yDataBeginPos);

        if (!captureArray.isEmpty()) {

            // at this stage set crc32 value at false by default :
            this->mCrc32Match = false;

            // set the position to the right place in file to write data, return data is unchecked
            // for desperate mode : write data even if retrieved beginValue is incorrect (ie : corrupted segment) :
            temporaryFile.seek(beginValue - static_cast<qint64>(1));

            if (writeSuccess) {

                // retrieve crc32 value, be sure to retrieve only the first crc32 value as some
                // encoders could provide the following pattern : "=yend size=50 part=79 pcrc32=a4f04edb crc32=a5a1fb24" :
                qint64 crc32FromFile = this->getPatternValue(yEndArray, "crc32=", 16);

                // finally decode yenc data :
                QByteArray decodeArray = this->decodeYenc(captureArray, static_cast<quint32>(crc32FromFile));

                // write decoded array in the target file :
                if (temporaryFile.write(decodeArray) == -1) {
                    writeSuccess = false;
                }

            }

        }

    }

    else {

        qCDebug(KWOOTY_LOG) << "segment size not matching. beginValue:" << beginValue << ", endValue:" << endValue << ", sizeValue:" << sizeValue;

    }

    // if write fails, try to know more about the issue :
    if (!writeSuccess) {
        qCDebug(KWOOTY_LOG) << "write failed !" << temporaryFile.fileName() << temporaryFile.errorString();
    }

    // close devices :
    segmentData.getIoDevice()->close();
    temporaryFile.close();

    return this->mCrc32Match;
}

qint64 SegmentDecoderYEnc::getPatternValue(const QByteArray &yPartArray, const QString &pattern, const int &base)
{

    QRegExp regExp(".*p?" + pattern + "((\\w|\\d)*).*");
    qint64 patternValue = 0;

    // if crc32 pattern has been found :
    if (regExp.exactMatch(yPartArray)) {

        bool ok = true;
        patternValue = regExp.cap(1).toLongLong(&ok, base);
    }

    return patternValue;
}

QByteArray SegmentDecoderYEnc::getLineByteArray(const QString &lineBeginPattern, const QByteArray &segmentByteArray, int &yDataBeginPos)
{

    int beginLinePos = segmentByteArray.indexOf(lineBeginPattern);
    int endLinePos = segmentByteArray.indexOf("\n", beginLinePos) + 1;

    // indicate the position of the next line :
    yDataBeginPos = endLinePos;

    return segmentByteArray.mid(beginLinePos, endLinePos - beginLinePos);

}

QByteArray SegmentDecoderYEnc::decodeYenc(QByteArray &captureArray, const quint32 &crc32FromFile)
{

    // used for crc32 computation :
    quint32 crc32Computed = 0xffffffff;

    int dataSize = 0;

    // allocate maximum possible size array :
    char decodeArray[captureArray.size()];

    bool specialCharacter = false;

    foreach (const char &encodedCharacter, captureArray) {

        // decode char :
        if (encodedCharacter != '\r' && encodedCharacter != '\n') {

            bool decoded = false;
            char decodedCharacter;

            if (specialCharacter) {

                decodedCharacter = (encodedCharacter - 106) % 256;
                decoded = true;
                specialCharacter = false;
            } else {
                if (encodedCharacter == '=') {
                    specialCharacter = true;
                } else {

                    decodedCharacter = (encodedCharacter - 42) % 256;
                    decoded = true;
                }
            }

            if (decoded) {
                // append decoded character :
                decodeArray[dataSize] = decodedCharacter;
                dataSize++;

                // compute crc32 part for this char :
                crc32Computed = this->computeCrc32Part(crc32Computed, decodedCharacter);
            }

        }

    }

    // finish crc 32 computation of encoded segment data :
    crc32Computed ^= 0xffffffff;

    //check crc32 values :
    if (crc32FromFile == crc32Computed) {
        this->mCrc32Match = true;
    }

    // return only useful data :
    return QByteArray(decodeArray, dataSize);

}

quint32 SegmentDecoderYEnc::computeCrc32Part(const quint32 &hash, const unsigned char &data)
{
    return crc::crcArray[(hash ^ data) & 0xff] ^ (hash >> 8);
}

void SegmentDecoderYEnc::finishDecodingJob(const NzbFileData &nzbFileData)
{

    // init variables :
    this->mParentIdentifer = nzbFileData.getUniqueIdentifier();
    this->mCrc32Match = false;

    // decodeInfoData sent by defaut :
    PostDownloadInfoData decodeInfoData;
    decodeInfoData.initDecode(this->mParentIdentifer, PROGRESS_COMPLETE, DecodeErrorStatus);

    // move decoded file to destination folder :
    QString temporaryFileStr = Utility::buildFullPath(Settings::temporaryFolder().path(), nzbFileData.getTemporaryFileName());
    QString destinationFileStr = Utility::buildFullPath(nzbFileData.getFileSavePath(), nzbFileData.getDecodedFileName());

    // check if exists or create the destination folder :
    if (Utility::createFolder(nzbFileData.getFileSavePath())) {

        // check crc value for each segment :
        foreach (const SegmentData &segmentData, nzbFileData.getSegmentList()) {

            // if article is not present on server or computed crc is not matching,
            // the global crc for the file is considered as incorrect :
            if (segmentData.getCrc32Match() == CrcOk) {

                this->mCrc32Match = true;

            } else {

                this->mCrc32Match = false;
                break;

            }
        }

        // move file in destination folder :
        bool success = Utility::rename(temporaryFileStr, destinationFileStr);

        if (!success) {
            qCDebug(KWOOTY_LOG) << "can not move" << temporaryFileStr << "to" << destinationFileStr;
        }

        // remove kwooty tag application that should be present at the end of the temporary file :
        QFile destinationFile(destinationFileStr);
        destinationFile.open(QIODevice::ReadWrite);
        destinationFile.seek(destinationFile.size() - applicationFileOwner.size());

        // reduce size to remove tag :
        if (destinationFile.peek(applicationFileOwner.size()) == applicationFileOwner) {
            destinationFile.resize(destinationFile.size() - applicationFileOwner.size());
        }

        destinationFile.close();

        // decoding is over, also send name of decoded file :
        decodeInfoData.setDecodedFileName(nzbFileData.getDecodedFileName());
        decodeInfoData.setStatus(DecodeFinishStatus);

        this->decodeProgression(decodeInfoData);

    }

    // notify user of the issue :
    else {
        this->decodeProgression(decodeInfoData);
        this->mSegmentsDecoderThread->emitSaveFileError();
    }

    // clear variables :
    this->mParentIdentifer.clear();

}

QString SegmentDecoderYEnc::searchPattern(QIODevice *segmentFile)
{

    QString fileName;
    // look for file name :
    while (fileName.isEmpty() && !segmentFile->atEnd()) {

        QByteArray namePattern = "name=";
        QByteArray lineArray = segmentFile->readLine();

        // if 'name=' pattern has been found, retrieve the file name :
        if (lineArray.contains(namePattern)) {

            int nameValuePos = lineArray.indexOf(namePattern) + namePattern.size();
            fileName = lineArray.mid(nameValuePos, lineArray.size() - nameValuePos).trimmed();
            break;
        }
    }

    return fileName;

}
