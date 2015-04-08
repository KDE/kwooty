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

#ifndef ITEMPARENTUPDATER_H
#define ITEMPARENTUPDATER_H

#include <QObject>
#include "itemabstractupdater.h"
#include "data/nzbfiledata.h"
#include "data/nzbcollectiondata.h"
#include "data/postdownloadinfodata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class ItemPostDownloadUpdater;
class ItemDownloadUpdater;
class ItemStatusData;
class StandardItemModel;
class ItemDownloadUpdater;
class ItemChildrenManager;

class ItemParentUpdater : public ItemAbstractUpdater {

    Q_OBJECT

public:
    ItemParentUpdater(Core*);
    ItemPostDownloadUpdater* getItemPostDownloadUpdater() const;
    ItemDownloadUpdater* getItemDownloadUpdater() const;
    ItemChildrenManager* getItemChildrenManager() const;
    StandardItemModel* getDownloadModel() const;
    void updateNzbItems(const QModelIndex&);
    void updateNzbItemsPostDecode(const PostDownloadInfoData&);
    void recalculateNzbSize(const QModelIndex&);
    void triggerPostProcessManually(const QStandardItem*);



private:

    // post process behavior :
    enum PostProcessBehavior {
        ForcePostProcess,
        AutomaticPostProcess
    };

    Core* parent;
    ItemPostDownloadUpdater* itemPostDownloadUpdater;
    ItemDownloadUpdater* itemDownloadUpdater;
    ItemChildrenManager* itemChildrenManager;
    bool isItemUpdated;

    ItemStatusData updateItemsDownload(ItemStatusData&, const int, const QModelIndex&, const quint64);
    ItemStatusData updateStatusItemDownload(ItemStatusData&, const int);
    ItemStatusData updateItemsDecode(ItemStatusData&, const int);
    ItemStatusData updateStatusItemDecode(ItemStatusData&, const int);
    ItemStatusData postProcessing(ItemStatusData&, const int, const QModelIndex&, PostProcessBehavior = AutomaticPostProcess);
    ItemStatusData updateDataStatus(ItemStatusData&);
    quint64 calculateDownloadProgress(const QModelIndex&, const ItemStatusData&, const int);
    void countGlobalItemStatus(const ItemStatusData&);
    void setupConnections();
    bool updatePar2ItemsIfCrcFailed(ItemStatusData&, const int rowNumber, const QModelIndex&);
    void updateItemsIfDirectExtractFailed(const QModelIndex, UtilityNamespace::ItemStatus);

signals:

    void repairDecompressSignal(NzbCollectionData);
    void downloadWaitingPar2Signal();

public slots:
    void recalculateNzbSizeSlot(const QModelIndex);


private slots:

};

#endif // ITEMPARENTUPDATER_H
