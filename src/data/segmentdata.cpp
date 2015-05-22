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
    this->mParentUniqueIdentifier = QVariant();
    this->mElementInList = -1;
    this->mStatus = IdleStatus;
    this->mCrc32Match = CrcUnknown;
    this->mDataSize = 0;
    this->mSegmentInfoData.reset();
}

SegmentData::SegmentData(const QString &bytes, const QString &number, const QString &part, const UtilityNamespace::ItemStatus status)
{
    this->mBytes = bytes;
    this->mNumber = number;
    this->mPart = part;
    this->mStatus = status;
    this->mCrc32Match = CrcUnknown;
    this->mDataSize = 0;

    this->mServerGroupTarget = MasterServer;
    this->mArticlePresence = Unknown;
    this->mParentUniqueIdentifier = QVariant();
    this->mElementInList = -1;
    this->mSegmentInfoData.reset();
}

void SegmentData::setReadyForNewServer(const int &nextServerGroup)
{

    this->setProgress(PROGRESS_INIT);
    this->setStatus(IdleStatus);
    this->setArticlePresenceOnServer(Unknown);
    this->setServerGroupTarget(nextServerGroup);
}

void SegmentData::setDownloadFinished(const int &articlePresence)
{

    this->setProgress(PROGRESS_COMPLETE);
    this->setStatus(DownloadFinishStatus);
    this->setArticlePresenceOnServer(articlePresence);
}

bool SegmentData::isInitialized()
{
    return this->mParentUniqueIdentifier != QVariant();
}

void SegmentData::setBytes(const QString &bytes)
{
    this->mBytes = bytes;
}

QString SegmentData::getBytes() const
{
    return this->mBytes;
}

void SegmentData::setNumber(const QString &number)
{
    this->mNumber = number;
}

QString SegmentData::getNumber() const
{
    return this->mNumber;
}

void SegmentData::setPart(const QString &part)
{
    this->mPart = part;
}

QString SegmentData::getPart() const
{
    return this->mPart;
}

UtilityNamespace::ItemStatus SegmentData::getStatus() const
{
    return this->mStatus;
}

void SegmentData::setStatus(const UtilityNamespace::ItemStatus status)
{
    this->mStatus = status;
}

int SegmentData::getServerGroupTarget() const
{
    return this->mServerGroupTarget;
}

void SegmentData::setServerGroupTarget(const int serverGroupTarget)
{
    this->mServerGroupTarget = serverGroupTarget;
}

int SegmentData::getProgress() const
{
    return this->mProgress;
}

void SegmentData::setProgress(const int progress)
{
    this->mProgress = progress;
}

void SegmentData::setElementInList(const int elementInList)
{
    this->mElementInList = elementInList;
}

int SegmentData::getElementInList() const
{
    return this->mElementInList;
}

void SegmentData::setParentUniqueIdentifier(const QVariant &parentUniqueIdentifier)
{
    this->mParentUniqueIdentifier = parentUniqueIdentifier;
}

QVariant SegmentData::getParentUniqueIdentifier() const
{
    return this->mParentUniqueIdentifier;
}

int SegmentData::getArticlePresenceOnServer() const
{
    return this->mArticlePresence;
}

void SegmentData::setArticlePresenceOnServer(const int articlePresence)
{
    this->mArticlePresence = articlePresence;
}

SegmentInfoData SegmentData::getSegmentInfoData() const
{
    return this->mSegmentInfoData;
}

void SegmentData::setSegmentInfoData(const SegmentInfoData &segmentInfoData)
{
    this->mSegmentInfoData = segmentInfoData;
}

QIODevice *SegmentData::getIoDevice()
{
    return this->mIoDevice;
}

void SegmentData::setIoDevice(QIODevice *ioDevice)
{
    this->mIoDevice = ioDevice;
}

int SegmentData::getDataSize() const
{
    return this->mDataSize;
}

void SegmentData::setDataSize(const int &dataSize)
{
    this->mDataSize = dataSize;
}

UtilityNamespace::CrcNotify SegmentData::getCrc32Match() const
{
    return this->mCrc32Match;
}

void SegmentData::setCrc32Match(const UtilityNamespace::CrcNotify crc32Match)
{
    this->mCrc32Match = crc32Match;
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

