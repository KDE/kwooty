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

#ifndef NZBCOLLECTIONDATA_H
#define NZBCOLLECTIONDATA_H

#include "nzbfiledata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class NzbCollectionData
{

public:
    NzbCollectionData();

    QList<NzbFileData> takeNzbFileDataList();
    QList<NzbFileData> getNzbFileDataList() const;
    QVariant getFirstChildUniqueIdentifier() const;
    void setNzbFileDataList(const QList<NzbFileData> &);
    void setNzbParentId(const QString &);
    QString getNzbParentId() const;
    void setPar2FileDownloadStatus(const UtilityNamespace::ItemStatus);
    UtilityNamespace::ItemStatus getPar2FileDownloadStatus() const;
    void setPar2BaseName(const QString &);
    QString getPar2BaseName() const;
    void setExtractTerminateStatus(const UtilityNamespace::ItemStatus);
    UtilityNamespace::ItemStatus getExtractTerminateStatus() const;
    void setVerifyRepairTerminateStatus(const UtilityNamespace::ItemStatus);
    UtilityNamespace::ItemStatus getVerifyRepairTerminateStatus() const;
    void setAllPostProcessingCorrect(const bool &);
    bool isAllPostProcessingCorrect() const;
    bool isExtractProcessAllowed() const;
    void setExtractProcessAllowed(const bool &);
    bool isRepairProcessAllowed() const;
    void setRepairProcessAllowed(const bool &);

    bool operator==(const NzbCollectionData &);

private:
    QList<NzbFileData> nzbFileDataList;
    QString nzbParentId;
    UtilityNamespace::ItemStatus par2FileDownloadStatus;
    UtilityNamespace::ItemStatus extractTerminateStatus;
    UtilityNamespace::ItemStatus verifyRepairTerminateStatus;
    QString par2BaseName;
    bool allPostProcessingCorrect;
    bool extractProcessAllowed;
    bool repairProcessAllowed;

};

#endif // NZBCOLLECTIONDATA_H
