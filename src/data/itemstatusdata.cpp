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


ItemStatusData::ItemStatusData()
{
    this->init();
}


ItemStatusData::~ItemStatusData()
{
}

void ItemStatusData::init(){

    this->status = IdleStatus;
    this->downloadFinish = false;
    this->decodeFinish = false;
    this->crc32Match = CrcOk;
    this->data = DataComplete;

}


UtilityNamespace::Data ItemStatusData::getDataStatus() const{
    return this->data;
}

void ItemStatusData::setDataStatus(const UtilityNamespace::Data data){
    this->data = data;
}


bool ItemStatusData::isDownloadFinish() const{
    return this->downloadFinish;
}

void ItemStatusData::setDownloadFinish(const bool downloadFinish){
    this->downloadFinish = downloadFinish;
}

bool ItemStatusData::isDecodeFinish() const{
    return this->decodeFinish;
}

void ItemStatusData::setDecodeFinish(const bool decodeFinish){
    this->decodeFinish = decodeFinish;
}

UtilityNamespace::CrcNotify ItemStatusData::getCrc32Match() const {
    return this->crc32Match;
}

void ItemStatusData::setCrc32Match(const UtilityNamespace::CrcNotify crc32Match) {
    this->crc32Match = crc32Match;
}


void ItemStatusData::setStatus(const UtilityNamespace::ItemStatus status){
    this->status = status;
}

UtilityNamespace::ItemStatus ItemStatusData::getStatus() const{
    return this->status;
}


bool ItemStatusData::operator!=(const ItemStatusData& itemStatusDataToCompare) {

    bool different = false;

    if ( (this->status            != itemStatusDataToCompare.getStatus())        ||
         (this->data              != itemStatusDataToCompare.getDataStatus())    ||
         (this->downloadFinish    != itemStatusDataToCompare.isDownloadFinish()) ||
         (this->decodeFinish      != itemStatusDataToCompare.isDecodeFinish())   ||
         (this->crc32Match        != itemStatusDataToCompare.getCrc32Match()) ) {

            different = true;
    }

    return different;
}


QDataStream& operator<<(QDataStream& out, const ItemStatusData& itemStatusData) {

    out << (qint16)itemStatusData.getStatus()
        << (qint16)itemStatusData.getDataStatus()
        << itemStatusData.isDownloadFinish()
        << itemStatusData.isDecodeFinish()
        << (qint16)itemStatusData.getCrc32Match();

    return out;
}



QDataStream& operator>>(QDataStream& in, ItemStatusData& itemStatusData)
{
    qint16 status;
    qint16 data;
    bool downloadFinish;
    bool decodeFinish;
    qint16 crc32Match;

    in >> status
       >> data
       >> downloadFinish
       >> decodeFinish
       >> crc32Match;

    itemStatusData.setStatus((UtilityNamespace::ItemStatus)status);
    itemStatusData.setDataStatus((UtilityNamespace::Data)data);
    itemStatusData.setDownloadFinish(downloadFinish);
    itemStatusData.setDecodeFinish(decodeFinish);
    itemStatusData.setCrc32Match((UtilityNamespace::CrcNotify)crc32Match);

    return in;
}
