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

#include <KDebug>


ItemStatusData::ItemStatusData() {

    this->init();
    this->downloadRetryCounter = 0;
}


ItemStatusData::~ItemStatusData() {
}

void ItemStatusData::init() {

    this->status = IdleStatus;
    this->downloadFinish = false;
    this->decodeFinish = false;
    this->postProcessFinish = false;
    this->allPostProcessingCorrect = false;
    this->crc32Match = CrcOk;
    this->data = DataComplete;
    this->nextServerId = UtilityNamespace::MasterServer;

}


void ItemStatusData::downloadRetry(const ItemStatus& itemStatusResetTarget) {

    this->init();

    // item has to be reset to IdleStatus, meaning that download must be performed another time :
    if (itemStatusResetTarget == IdleStatus) {
        this->downloadRetryCounter++;
    }
    // else the current item has been correctly downloaded, set it status to DecodeFinishStatus
    // in order to reenable post processing (repair/extract) :
    else if (itemStatusResetTarget == DecodeFinishStatus) {

        this->status = DecodeFinishStatus;
        this->downloadFinish = true;
        this->decodeFinish = true;

    }

}


UtilityNamespace::Data ItemStatusData::getDataStatus() const {
    return this->data;
}

void ItemStatusData::setDataStatus(const UtilityNamespace::Data data) {
    this->data = data;
}


bool ItemStatusData::isDownloadFinish() const {
    return this->downloadFinish;
}

void ItemStatusData::setDownloadFinish(const bool downloadFinish) {
    this->downloadFinish = downloadFinish;
}

bool ItemStatusData::isDecodeFinish() const {
    return this->decodeFinish;
}

void ItemStatusData::setDecodeFinish(const bool decodeFinish) {
    this->decodeFinish = decodeFinish;
}

bool ItemStatusData::isPostProcessFinish() const {
    return this->postProcessFinish;
}

void ItemStatusData::setPostProcessFinish(const bool postProcessFinish) {
    this->postProcessFinish = postProcessFinish;
}

bool ItemStatusData::areAllPostProcessingCorrect() const {
    return this->allPostProcessingCorrect;
}

void ItemStatusData::setAllPostProcessingCorrect(const bool& allPostProcessingCorrect) {
    this->allPostProcessingCorrect = allPostProcessingCorrect;
}


UtilityNamespace::CrcNotify ItemStatusData::getCrc32Match() const {
    return this->crc32Match;
}

void ItemStatusData::setCrc32Match(const UtilityNamespace::CrcNotify crc32Match) {
    this->crc32Match = crc32Match;
}

UtilityNamespace::ArticleEncodingType ItemStatusData::getArticleEncodingType() const {
    return this->articleEncodingType;
}

void ItemStatusData::setArticleEncodingType(const UtilityNamespace::ArticleEncodingType articleEncodingType) {
    this->articleEncodingType = articleEncodingType;
}


void ItemStatusData::setStatus(const UtilityNamespace::ItemStatus status) {
    this->status = status;
}

UtilityNamespace::ItemStatus ItemStatusData::getStatus() const {
    return this->status;
}

int ItemStatusData::getNextServerId() const {
    return this->nextServerId;
}
void ItemStatusData::setNextServerId(const int& nextServerId) {
    this->nextServerId = nextServerId;
}

int ItemStatusData::getDownloadRetryCounter() const {
    return this->downloadRetryCounter;
}


bool ItemStatusData::operator!=(const ItemStatusData& itemStatusDataToCompare) {

    bool different = false;

    if ( (this->status               != itemStatusDataToCompare.getStatus())                 ||
         (this->data                 != itemStatusDataToCompare.getDataStatus())             ||
         (this->downloadFinish       != itemStatusDataToCompare.isDownloadFinish())          ||
         (this->decodeFinish         != itemStatusDataToCompare.isDecodeFinish())            ||
         (this->postProcessFinish    != itemStatusDataToCompare.isPostProcessFinish())       ||
         (this->crc32Match           != itemStatusDataToCompare.getCrc32Match())             ||
         (this->articleEncodingType  != itemStatusDataToCompare.getArticleEncodingType())    ||
         (this->nextServerId         != itemStatusDataToCompare.getNextServerId())           ||
         (this->downloadRetryCounter != itemStatusDataToCompare.getDownloadRetryCounter()) ) {

        different = true;
    }

    return different;
}


QDataStream& operator<<(QDataStream& out, const ItemStatusData& itemStatusData) {

    out << (qint16)itemStatusData.getStatus()
        << (qint16)itemStatusData.getDataStatus()
        << itemStatusData.isDownloadFinish()
        << itemStatusData.isDecodeFinish()
        << itemStatusData.isPostProcessFinish()
        << (qint16)itemStatusData.getCrc32Match()
        << (qint16)itemStatusData.getArticleEncodingType();

    return out;
}



QDataStream& operator>>(QDataStream& in, ItemStatusData& itemStatusData) {

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
