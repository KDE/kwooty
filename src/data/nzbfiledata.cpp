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

#include "nzbfiledata.h"

NzbFileData::NzbFileData()
{

    mPar2File = false;
    mArchiveFile = false;
    mArchiveFormat = UnknownArchiveFormat;

}

NzbFileData::NzbFileData(const QString &fileName, const QStringList &groupList, const QList<SegmentData> &segmentList)
{

    setFileName(fileName);
    setGroupList(groupList);
    setSegmentList(segmentList);
    mPar2File = false;
    mArchiveFile = false;
    mArchiveFormat = UnknownArchiveFormat;
}

NzbFileData::~NzbFileData() { }

QString NzbFileData::getFileName() const
{
    return mFileName;
}

void NzbFileData::setFileName(const QString &fileName)
{
    mFileName = fileName;
}

QString NzbFileData::getReducedFileName() const
{
    return mReducedFileName;
}

void NzbFileData::setReducedFileName(const QString &reducedFileName)
{
    mReducedFileName = reducedFileName;
}

QString NzbFileData::getTemporaryFileName() const
{
    return mTemporaryFileName;
}

void NzbFileData::setTemporaryFileName(const QString &temporaryFileName)
{

    // remove '-' separator from uuid string and retrieve the first 12 characters
    // that will be used to create the temporary file name :
    mTemporaryFileName = temporaryFileName;
    mTemporaryFileName = mTemporaryFileName.remove(QRegExp("[-{].")).left(12);

}

QString NzbFileData::getDecodedFileName() const
{
    return mDecodedFileName;
}

void NzbFileData::setDecodedFileName(const QString &decodedFileName)
{

    mDecodedFileName = decodedFileName;

    // add decoded fileName to list of candidate file names
    // (this fileName shall always be the first element of the list) :
    if (!mPossibleFileNameList.contains(decodedFileName)) {
        mPossibleFileNameList.append(decodedFileName);
    }

}

bool NzbFileData::match(const QString &fileNameStr, const QString &originalFileNameStr)
{

    return (mPossibleFileNameList.contains(fileNameStr) ||
            mPossibleFileNameList.contains(originalFileNameStr));

}

void NzbFileData::setRenamedFileName(const QString &fileNameStr, const QString &originalFileNameStr)
{

    if (!mPossibleFileNameList.contains(fileNameStr) && !fileNameStr.isEmpty()) {
        mPossibleFileNameList.append(fileNameStr);
    }

    if (!mPossibleFileNameList.contains(originalFileNameStr) && !originalFileNameStr.isEmpty()) {
        mPossibleFileNameList.append(originalFileNameStr);
    }

    if (mPossibleFileNameList.size() > 1) {
        mRenamedFileName = mPossibleFileNameList.at(1);
    }

}

QString NzbFileData::getRenamedFileName() const
{
    return mRenamedFileName;
}

QString NzbFileData::getNzbName() const
{
    return mNzbName;
}

void NzbFileData::setNzbName(const QString &nzbName)
{
    mNzbName = nzbName;
}

bool NzbFileData::isPar2File() const
{
    return mPar2File;
}

void NzbFileData::setPar2File(const bool par2File)
{
    mPar2File = par2File;
}

bool NzbFileData::isArchiveFile() const
{
    return mArchiveFile;
}

void NzbFileData::setArchiveFile(const bool archiveFile)
{
    mArchiveFile = archiveFile;
}

UtilityNamespace::ArchiveFormat NzbFileData::getArchiveFormat() const
{
    return mArchiveFormat;
}

void NzbFileData::setArchiveFormat(const UtilityNamespace::ArchiveFormat archiveFormat)
{

    // archive is a known format handled for extracting :
    if (archiveFormat != UnknownArchiveFormat) {
        setArchiveFile(true);
    }

    mArchiveFormat = archiveFormat;
}

void NzbFileData::setVerifyProgressionStep(const UtilityNamespace::ItemStatus verifyProgressionStep)
{
    mVerifyProgressionStep = verifyProgressionStep;
}

UtilityNamespace::ItemStatus NzbFileData::getVerifyProgressionStep() const
{
    return mVerifyProgressionStep;
}

void NzbFileData::setExtractProgressionStep(const UtilityNamespace::ItemStatus extractProgressionStep)
{
    mExtractProgressionStep = extractProgressionStep;
}

UtilityNamespace::ItemStatus NzbFileData::getExtractProgressionStep() const
{
    return mExtractProgressionStep;
}

quint64 NzbFileData::getSize() const
{
    return mSize;
}

void NzbFileData::setSize(const quint64 size)
{
    mSize = size;
}

QString NzbFileData::getFileSavePath() const
{

    QString fileSavePath;

    if (!mDownloadFolderPath.isEmpty() && !mNzbName.isEmpty()) {
        fileSavePath = Utility::buildFullPath(mDownloadFolderPath, mNzbName);
    }
    return fileSavePath;

}

void NzbFileData::updateFileSavePath(const NzbFileData &targetNzbFileData)
{

    mDownloadFolderPath = targetNzbFileData.getDownloadFolderPath();
    mNzbName = targetNzbFileData.getNzbName();
}

QString NzbFileData::getDownloadFolderPath() const
{
    return mDownloadFolderPath;
}

void NzbFileData::setDownloadFolderPath(const QString &downloadFolderPath)
{
    mDownloadFolderPath = downloadFolderPath;
}

void NzbFileData::setBaseName(const QString &baseName)
{
    mBaseName = baseName;
}

QString NzbFileData::getBaseName() const
{
    return mBaseName;
}

QStringList NzbFileData::getGroupList() const
{
    return mGroupList;
}

void NzbFileData::setGroupList(const QStringList &groupList)
{
    mGroupList = groupList;
}

QList<SegmentData> NzbFileData::getSegmentList() const
{
    return mSegmentList;
}

void NzbFileData::setSegmentList(const QList<SegmentData> &segmentList)
{
    mSegmentList = segmentList;
}

void NzbFileData::setUniqueIdentifier(const QVariant &uniqueIdentifier)
{
    mUniqueIdentifier = uniqueIdentifier;
}

QVariant NzbFileData::getUniqueIdentifier() const
{
    return mUniqueIdentifier;
}

bool NzbFileData::operator==(const NzbFileData &nzbFileDataToCompare)
{
    return (getUniqueIdentifier() == nzbFileDataToCompare.getUniqueIdentifier());
}

bool NzbFileData::operator<(const NzbFileData &nzbFileDataToCompare) const
{
    // this data object will be sorted according to file name criteria :
    return (getDecodedFileName() < nzbFileDataToCompare.getDecodedFileName());
}

QDataStream &operator<<(QDataStream &out, const NzbFileData &nzbFileData)
{

    out << nzbFileData.getFileName()
        << nzbFileData.getReducedFileName()
        << nzbFileData.getDecodedFileName()
        << nzbFileData.getTemporaryFileName()
        << nzbFileData.getBaseName()
        << nzbFileData.getNzbName()
        << nzbFileData.getDownloadFolderPath()
        << nzbFileData.getGroupList()
        << nzbFileData.getSegmentList()
        << nzbFileData.getUniqueIdentifier()
        << nzbFileData.getSize()
        << nzbFileData.isPar2File()
        << nzbFileData.isArchiveFile()
        << (qint16)nzbFileData.getArchiveFormat();

    return out;
}

QDataStream &operator>>(QDataStream &in, NzbFileData &nzbFileData)
{

    QString fileName;
    QString reducedFileName;
    QString decodedFileName;
    QString temporaryFileName;
    QString baseName;
    QString nzbName;
    QString downloadFolderPath;
    QStringList groupList;
    QList<SegmentData> segmentList;
    QVariant uniqueIdentifier;
    quint64 size;
    bool par2File;
    bool archiveFile;
    qint16 archiveFormat;

    in >> fileName
       >> reducedFileName
       >> decodedFileName
       >> temporaryFileName
       >> baseName
       >> nzbName
       >> downloadFolderPath
       >> groupList
       >> segmentList
       >> uniqueIdentifier
       >> size
       >> par2File
       >> archiveFile
       >> archiveFormat;

    nzbFileData.setFileName(fileName);
    nzbFileData.setReducedFileName(reducedFileName);
    nzbFileData.setTemporaryFileName(temporaryFileName);
    nzbFileData.setBaseName(baseName);
    nzbFileData.setNzbName(nzbName);
    nzbFileData.setDownloadFolderPath(downloadFolderPath);
    nzbFileData.setGroupList(groupList);
    nzbFileData.setSegmentList(segmentList);
    nzbFileData.setUniqueIdentifier(uniqueIdentifier);
    nzbFileData.setSize(size);
    nzbFileData.setPar2File(par2File);
    nzbFileData.setArchiveFile(archiveFile);
    nzbFileData.setArchiveFormat((UtilityNamespace::ArchiveFormat)archiveFormat);

    // do not set decoded file name if it is empty as it will also
    // be apended to possibleFileNameList and will cause issues during extract process :
    if (!decodedFileName.isEmpty()) {
        nzbFileData.setDecodedFileName(decodedFileName);
    }

    return in;
}

