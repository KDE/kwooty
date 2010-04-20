/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
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


#include "nzbcollectiondata.h"

#include <KDebug>


NzbCollectionData::NzbCollectionData() {

    this->par2FileDownloadStatus = DownloadFinishStatus;
    this->extractTerminateStatus = ExtractSuccessStatus;

}


QList<NzbFileData> NzbCollectionData::takeNzbFileDataList() {

    QList<NzbFileData> tempList = this->nzbFileDataList;
    this->nzbFileDataList.clear();
    return tempList;
}


QList<NzbFileData> NzbCollectionData::getNzbFileDataList() const{
    return this->nzbFileDataList;
}

void NzbCollectionData::setNzbFileDataList(const QList<NzbFileData>& nzbFileDataList){
    this->nzbFileDataList = nzbFileDataList;
}


void NzbCollectionData::setNzbParentId(const QString& nzbParentId){
    this->nzbParentId = nzbParentId;
}

QString NzbCollectionData::getNzbParentId() const{
    return this->nzbParentId;
}


void NzbCollectionData::setPar2FileDownloadStatus(const UtilityNamespace::ItemStatus par2FileDownloadStatus){
    this->par2FileDownloadStatus = par2FileDownloadStatus;
}

UtilityNamespace::ItemStatus NzbCollectionData::getPar2FileDownloadStatus() const {
    return this->par2FileDownloadStatus;
}


void NzbCollectionData::setPar2BaseName(const QString& par2BaseName) {
    this->par2BaseName = par2BaseName;
}

QString NzbCollectionData::getPar2BaseName() const{
    return this->par2BaseName;
}


void NzbCollectionData::setExtractTerminateStatus(const UtilityNamespace::ItemStatus extractTerminateStatus){
    this->extractTerminateStatus = extractTerminateStatus;
}

UtilityNamespace::ItemStatus NzbCollectionData::getExtractTerminateStatus() const {
    return this->extractTerminateStatus;
}


bool NzbCollectionData::operator==(const NzbCollectionData& nzbCollectionDataToCompare) {
    return (this->getNzbParentId() == nzbCollectionDataToCompare.getNzbParentId());
}
