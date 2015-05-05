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

NzbCollectionData::NzbCollectionData()
{

    nPar2FileDownloadStatus = DownloadFinishStatus;
    mExtractTerminateStatus = ExtractSuccessStatus;
    mVerifyRepairTerminateStatus = RepairFinishedStatus;
    mAllPostProcessingCorrect = true;
    mExtractProcessAllowed = false;
    mRepairProcessAllowed = false;

}

QList<NzbFileData> NzbCollectionData::takeNzbFileDataList()
{

    QList<NzbFileData> tempList = mNzbFileDataList;
    mNzbFileDataList.clear();
    return tempList;
}

QVariant NzbCollectionData::getFirstChildUniqueIdentifier() const
{

    QVariant uniqueIdentifier;

    if (!mNzbFileDataList.isEmpty()) {
        uniqueIdentifier = mNzbFileDataList.at(0).getUniqueIdentifier();
    }
    return uniqueIdentifier;
}

QList<NzbFileData> NzbCollectionData::getNzbFileDataList() const
{
    return mNzbFileDataList;
}
void NzbCollectionData::setNzbFileDataList(const QList<NzbFileData> &nzbFileDataList)
{
    mNzbFileDataList = nzbFileDataList;
}

void NzbCollectionData::setNzbParentId(const QString &nzbParentId)
{
    mNzbParentId = nzbParentId;
}
QString NzbCollectionData::getNzbParentId() const
{
    return mNzbParentId;
}

void NzbCollectionData::setPar2FileDownloadStatus(const UtilityNamespace::ItemStatus par2FileDownloadStatus)
{
    nPar2FileDownloadStatus = par2FileDownloadStatus;
}
UtilityNamespace::ItemStatus NzbCollectionData::getPar2FileDownloadStatus() const
{
    return nPar2FileDownloadStatus;
}

void NzbCollectionData::setPar2BaseName(const QString &par2BaseName)
{
    mPar2BaseName = par2BaseName;
}
QString NzbCollectionData::getPar2BaseName() const
{
    return mPar2BaseName;
}

void NzbCollectionData::setExtractTerminateStatus(const UtilityNamespace::ItemStatus extractTerminateStatus)
{
    mExtractTerminateStatus = extractTerminateStatus;

    if (extractTerminateStatus == ExtractFailedStatus) {
        mAllPostProcessingCorrect = false;
    }

}
UtilityNamespace::ItemStatus NzbCollectionData::getExtractTerminateStatus() const
{
    return mExtractTerminateStatus;
}

void NzbCollectionData::setVerifyRepairTerminateStatus(const UtilityNamespace::ItemStatus verifyRepairTerminateStatus)
{
    mVerifyRepairTerminateStatus = verifyRepairTerminateStatus;

    if (verifyRepairTerminateStatus == RepairFailedStatus) {
        mAllPostProcessingCorrect = false;
    }

}
UtilityNamespace::ItemStatus NzbCollectionData::getVerifyRepairTerminateStatus() const
{
    return mVerifyRepairTerminateStatus;
}

void NzbCollectionData::setAllPostProcessingCorrect(const bool &allPostProcessingCorrect)
{
    mAllPostProcessingCorrect = allPostProcessingCorrect;
}
bool NzbCollectionData::isAllPostProcessingCorrect() const
{
    return mAllPostProcessingCorrect;
}

bool NzbCollectionData::isExtractProcessAllowed() const
{
    return mExtractProcessAllowed;
}

void NzbCollectionData::setExtractProcessAllowed(const bool &extractProcessAllowed)
{
    mExtractProcessAllowed = extractProcessAllowed;
}

bool NzbCollectionData::isRepairProcessAllowed() const
{
    return mRepairProcessAllowed;
}

void NzbCollectionData::setRepairProcessAllowed(const bool &repairProcessAllowed)
{
    mRepairProcessAllowed = repairProcessAllowed;
}

bool NzbCollectionData::operator==(const NzbCollectionData &nzbCollectionDataToCompare)
{
    return (getNzbParentId() == nzbCollectionDataToCompare.getNzbParentId());
}
