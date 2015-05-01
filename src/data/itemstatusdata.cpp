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

#include "itemstatusdata.h"

ItemStatusData::ItemStatusData()
{

    this->init();
    this->mDownloadRetryCounter = 0;
    this->mArticleEncodingType = UtilityNamespace::ArticleEncodingUnknown;
}

ItemStatusData::~ItemStatusData()
{
}

void ItemStatusData::init()
{

    this->mStatus = IdleStatus;
    this->mDownloadFinish = false;
    this->mDecodeFinish = false;
    this->mPostProcessFinish = false;
    this->mAllPostProcessingCorrect = false;
    this->mCrc32Match = CrcOk;
    this->mData = DataComplete;
    this->mNextServerId = UtilityNamespace::MasterServer;

}

void ItemStatusData::downloadRetry(const ItemStatus &itemStatusResetTarget)
{

    this->init();

    // item has to be reset to IdleStatus, meaning that download must be performed another time :
    if (itemStatusResetTarget == IdleStatus) {
        this->mDownloadRetryCounter++;
    }
    // else the current item has been correctly downloaded, set it status to DecodeFinishStatus
    // in order to reenable post processing (repair/extract) :
    else if (itemStatusResetTarget == DecodeFinishStatus) {

        this->mStatus = DecodeFinishStatus;
        this->mDownloadFinish = true;
        this->mDecodeFinish = true;

    }

}

UtilityNamespace::Data ItemStatusData::getDataStatus() const
{
    return this->mData;
}

void ItemStatusData::setDataStatus(const UtilityNamespace::Data data)
{
    this->mData = data;
}

bool ItemStatusData::isDownloadFinish() const
{
    return this->mDownloadFinish;
}

void ItemStatusData::setDownloadFinish(const bool downloadFinish)
{
    this->mDownloadFinish = downloadFinish;
}

bool ItemStatusData::isDecodeFinish() const
{
    return this->mDecodeFinish;
}

void ItemStatusData::setDecodeFinish(const bool decodeFinish)
{
    this->mDecodeFinish = decodeFinish;
}

bool ItemStatusData::isPostProcessFinish() const
{
    return this->mPostProcessFinish;
}

void ItemStatusData::setPostProcessFinish(const bool postProcessFinish)
{
    this->mPostProcessFinish = postProcessFinish;
}

bool ItemStatusData::areAllPostProcessingCorrect() const
{
    return this->mAllPostProcessingCorrect;
}

void ItemStatusData::setAllPostProcessingCorrect(const bool &allPostProcessingCorrect)
{
    this->mAllPostProcessingCorrect = allPostProcessingCorrect;
}

UtilityNamespace::CrcNotify ItemStatusData::getCrc32Match() const
{
    return this->mCrc32Match;
}

void ItemStatusData::setCrc32Match(const UtilityNamespace::CrcNotify crc32Match)
{
    this->mCrc32Match = crc32Match;
}

UtilityNamespace::ArticleEncodingType ItemStatusData::getArticleEncodingType() const
{
    return this->mArticleEncodingType;
}

void ItemStatusData::setArticleEncodingType(const UtilityNamespace::ArticleEncodingType articleEncodingType)
{
    this->mArticleEncodingType = articleEncodingType;
}

void ItemStatusData::setStatus(const UtilityNamespace::ItemStatus status)
{
    this->mStatus = status;
}

UtilityNamespace::ItemStatus ItemStatusData::getStatus() const
{
    return this->mStatus;
}

int ItemStatusData::getNextServerId() const
{
    return this->mNextServerId;
}
void ItemStatusData::setNextServerId(const int &nextServerId)
{
    this->mNextServerId = nextServerId;
}

int ItemStatusData::getDownloadRetryCounter() const
{
    return this->mDownloadRetryCounter;
}

bool ItemStatusData::operator!=(const ItemStatusData &itemStatusDataToCompare)
{

    bool different = false;

    if ((this->mStatus               != itemStatusDataToCompare.getStatus())                 ||
            (this->mData                 != itemStatusDataToCompare.getDataStatus())             ||
            (this->mDownloadFinish       != itemStatusDataToCompare.isDownloadFinish())          ||
            (this->mDecodeFinish         != itemStatusDataToCompare.isDecodeFinish())            ||
            (this->mPostProcessFinish    != itemStatusDataToCompare.isPostProcessFinish())       ||
            (this->mCrc32Match           != itemStatusDataToCompare.getCrc32Match())             ||
            (this->mArticleEncodingType  != itemStatusDataToCompare.getArticleEncodingType())    ||
            (this->mNextServerId         != itemStatusDataToCompare.getNextServerId())           ||
            (this->mDownloadRetryCounter != itemStatusDataToCompare.getDownloadRetryCounter())) {

        different = true;
    }

    return different;
}

QDataStream &operator<<(QDataStream &out, const ItemStatusData &itemStatusData)
{

    out << (qint16)itemStatusData.getStatus()
        << (qint16)itemStatusData.getDataStatus()
        << itemStatusData.isDownloadFinish()
        << itemStatusData.isDecodeFinish()
        << itemStatusData.isPostProcessFinish()
        << (qint16)itemStatusData.getCrc32Match()
        << (qint16)itemStatusData.getArticleEncodingType();

    return out;
}

QDataStream &operator>>(QDataStream &in, ItemStatusData &itemStatusData)
{

    qint16 status;
    qint16 data;
    bool downloadFinish;
    bool decodeFinish;
    bool postProcessFinish;
    qint16 crc32Match;
    qint16 articleEncodingType;

    in >> status
       >> data
       >> downloadFinish
       >> decodeFinish
       >> postProcessFinish
       >> crc32Match
       >> articleEncodingType;

    itemStatusData.setStatus((UtilityNamespace::ItemStatus)status);
    itemStatusData.setDataStatus((UtilityNamespace::Data)data);
    itemStatusData.setDownloadFinish(downloadFinish);
    itemStatusData.setDecodeFinish(decodeFinish);
    itemStatusData.setPostProcessFinish(postProcessFinish);
    itemStatusData.setCrc32Match((UtilityNamespace::CrcNotify)crc32Match);
    itemStatusData.setArticleEncodingType((UtilityNamespace::ArticleEncodingType)articleEncodingType);
    itemStatusData.setNextServerId(MasterServer);

    return in;
}
