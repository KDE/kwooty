/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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


#ifndef STANDARDITEMMODELQUERY_H
#define STANDARDITEMMODELQUERY_H

#include <QObject>

#include "kwooty_export.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class StandardItemModel;


class KWOOTY_EXPORT StandardItemModelQuery : public QObject {

    Q_OBJECT

public:

    explicit StandardItemModelQuery(Core*);

    QStandardItem* searchParentItemIdle();
    QStandardItem* searchParentItemDownloadOrPausing();
    QStandardItem* searchParentItemPause();
    QStandardItem* searchParentItemPostDownloadProcessing();
    ItemStatus isRetryDownloadAllowed(QStandardItem*, bool* = Q_NULLPTR);
    bool isManualRepairExtractAllowed(QStandardItem* fileNameItem) const;
    bool haveItemsSameParent(const QList<QModelIndex>&);
    bool isParentContainsPar2File(QStandardItem*) const;
    bool isParentFileNameExists(const QString&) const;
    bool isRootModelEmpty() const;
    bool areJobsFinished() const;
    QList<QModelIndex> retrieveStartPauseIndexList(const UtilityNamespace::ItemStatus) const;
    QList<QModelIndex> retrieveDecodeFinishParentIndexList() const;
    QStandardItem* retrieveParentFileNameItemFromUuid(const QString&);



private:

    // status to search from parent items :
    enum SearchItemStatus {
        SearchItemIdle,
        SearchItemDownloadOrPausing,
        SearchItemPause,
        SearchItemPostDownloadProcessing
    };

    Core* mCore;
    StandardItemModel* mDownloadModel;

    QStandardItem* searchParentItem(const SearchItemStatus&);

};

#endif // STANDARDITEMMODELQUERY_H
