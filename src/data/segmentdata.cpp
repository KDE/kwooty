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

#include "segmentdata.h"

SegmentData::SegmentData()
{
    mParentUniqueIdentifier = QVariant();
    mElementInList = -1;
    mStatus = IdleStatus;
    mCrc32Match = CrcUnknown;
    mDataSize = 0;
    mSegmentInfoData.reset();
}

SegmentData::SegmentData(const QString &bytes, const QString &number, const QString &part, const UtilityNamespace::ItemStatus status)
{
    mBytes = bytes;
    mNumber = number;
    mPart = part;
    mStatus = status;
    mCrc32Match = CrcUnknown;
    mDataSize = 0;

    mServerGroupTarget = MasterServer;
    mArticlePresence = Unknown;
    mParentUniqueIdentifier = QVariant();
    mElementInList = -1;
    mSegmentInfoData.reset();
}

void SegmentData::setReadyForNewServer(const int &nextServerGroup)
{

    setProgress(PROGRESS_INIT);
    setStatus(IdleStatus);
    setArticlePresenceOnServer(Unknown);
    setServerGroupTarget(nextServerGroup);
}

void SegmentData::setDownloadFinished(const int &articlePresence)
{

    setProgress(PROGRESS_COMPLETE);
    setStatus(DownloadFinishStatus);
    setArticlePresenceOnServer(articlePresence);
}

bool SegmentData::isInitialized()
{
    return mParentUniqueIdentifier != QVariant();
}

void SegmentData::setBytes(const QString &bytes)
{
    mBytes = bytes;
}

QString SegmentData::getBytes() const
{
    return mBytes;
}

void SegmentData::setNumber(const QString &number)
{
    mNumber = number;
}

QString SegmentData::getNumber() const
{
    return mNumber;
}

void SegmentData::setPart(const QString &part)
{
    mPart = part;
}

QString SegmentData::getPart() const
{
    return mPart;
}

UtilityNamespace::ItemStatus SegmentData::getStatus() const
{
    return mStatus;
}

void SegmentData::setStatus(const UtilityNamespace::ItemStatus status)
{
    mStatus = status;
}

int SegmentData::getServerGroupTarget() const
{
    return mServerGroupTarget;
}

void SegmentData::setServerGroupTarget(const int serverGroupTarget)
{
    mServerGroupTarget = serverGroupTarget;
}

int SegmentData::getProgress() const
{
    return mProgress;
}

void SegmentData::setProgress(const int progress)
{
    mProgress = progress;
}

void SegmentData::setElementInList(const int elementInList)
{
    mElementInList = elementInList;
}

int SegmentData::getElementInList() const
{
    return mElementInList;
}

void SegmentData::setParentUniqueIdentifier(const QVariant &parentUniqueIdentifier)
{
    mParentUniqueIdentifier = parentUniqueIdentifier;
}

QVariant SegmentData::getParentUniqueIdentifier() const
{
    return mParentUniqueIdentifier;
}

int SegmentData::getArticlePresenceOnServer() const
{
    return mArticlePresence;
}

void SegmentData::setArticlePresenceOnServer(const int articlePresence)
{
    mArticlePresence = articlePresence;
}

SegmentInfoData SegmentData::getSegmentInfoData() const
{
    return mSegmentInfoData;
}

void SegmentData::setSegmentInfoData(const SegmentInfoData &segmentInfoData)
{
    mSegmentInfoData = segmentInfoData;
}

QIODevice *SegmentData::getIoDevice()
{
    return mIoDevice;
}

void SegmentData::setIoDevice(QIODevice *ioDevice)
{
    mIoDevice = ioDevice;
}

int SegmentData::getDataSize() const
{
    return mDataSize;
}

void SegmentData::setDataSize(const int &dataSize)
{
    mDataSize = dataSize;
}

UtilityNamespace::CrcNotify SegmentData::getCrc32Match() const
{
    return mCrc32Match;
}

void SegmentData::setCrc32Match(const UtilityNamespace::CrcNotify crc32Match)
{
    mCrc32Match = crc32Match;
}

QDataStream &operator<<(QDataStream &out, const SegmentData &segmentData)
{

    out << segmentData.getBytes()
        << segmentData.getNumber()
        << segmentData.getPart()
        << segmentData.getElementInList()
        << (qint16)segmentData.getStatus()
        << segmentData.getProgress()
        << segmentData.getArticlePresenceOnServer()
        << (qint16)segmentData.getCrc32Match();

    return out;
}

QDataStream &operator>>(QDataStream &in, SegmentData &segmentData)
{

    QString bytes;
    QString number;
    QString part;
    qint16 status;
    int elementInList;
    int progress;
    int articlePresenceOnServer;
    qint16 crc32Match;

    in >> bytes
       >> number
       >> part
       >> elementInList
       >> status
       >> progress
       >> articlePresenceOnServer
       >> crc32Match;

    segmentData.setBytes(bytes);
    segmentData.setNumber(number);
    segmentData.setPart(part);
    segmentData.setElementInList(elementInList);
    segmentData.setStatus((UtilityNamespace::ItemStatus)status);
    segmentData.setProgress(progress);
    segmentData.setArticlePresenceOnServer(articlePresenceOnServer);
    segmentData.setCrc32Match((UtilityNamespace::CrcNotify)crc32Match);
    segmentData.setServerGroupTarget(MasterServer);

    return in;
}

