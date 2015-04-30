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

#ifndef ITEMDOWNLOADUPDATER_H
#define ITEMDOWNLOADUPDATER_H

#include <QStandardItem>
#include <QModelIndex>
#include "data/nzbfiledata.h"
#include "itemabstractupdater.h"

class ItemStatusData;
class SegmentData;

class ItemDownloadUpdater : public ItemAbstractUpdater
{

    Q_OBJECT

public:
    ItemDownloadUpdater(ItemParentUpdater *);
    void updateItems(const QModelIndex &, const NzbFileData &);

private:
    ItemStatusData updateStatusNzbChildrenItem(ItemStatusData &, const int &);
    ItemStatusData updateDataStatus(ItemStatusData &);
    ItemStatusData postDownloadProcessing(const QModelIndex &, const NzbFileData &, ItemStatusData &);
    void updateNzbChildrenItems(const NzbFileData &, const QModelIndex &);
    void countGlobalItemStatus(const SegmentData &);

Q_SIGNALS:
    void statusBarDecrementSignal(quint64, int);
    void decodeSegmentsSignal(NzbFileData);
};

#endif // ITEMDOWNLOADUPDATER_H
