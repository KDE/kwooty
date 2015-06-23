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

#ifndef ITEMABSTRACTUPDATER_H
#define ITEMABSTRACTUPDATER_H

#include <QObject>
#include <QStandardItem>
#include <QModelIndex>
#include <QHash>

#include "data/itemstatusdata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class ItemParentUpdater;
class StandardItemModel;
class ItemStatusData;

class ItemAbstractUpdater : public QObject
{

    Q_OBJECT
    Q_ENUMS(ItemHierarchy)

public:

    enum ItemHierarchy { Parent,
                         Child
                       };

    ItemAbstractUpdater(StandardItemModel *, ItemHierarchy);
    ItemAbstractUpdater();

protected:

    StandardItemModel *mDownloadModel;
    ItemParentUpdater *mItemParentUpdater;

    int mDownloadItemNumber;
    int mProgressNumber;
    int mDownloadFinishItemNumber;
    int mInQueueItemNumber;
    int mPauseItemNumber;
    int mPausingItemNumber;
    int mDecodeFinishItemNumber;
    int mDecodeErrorItemNumber;
    int mDecodeItemNumber;
    int mScanItemNumber;
    int mVerifyItemNumber;
    int mVerifyFinishItemNumber;
    int mRepairItemNumber;
    int mArticleNotFoundNumber;
    int mArticleFoundNumber;
    int mPendingSegmentsOnBackupNumber;

    void clear();
    void countItemStatus(const int &);
    void setIcon(QStandardItem *, const UtilityNamespace::ItemStatus &);
    void setIcon(QStandardItem *, const QString &);

protected Q_SLOTS:

    void parentStatusIconUpdateSlot(QStandardItem *, const ItemStatusData &);
    void childStatusIconUpdateSlot(QStandardItem *, const ItemStatusData &);

};

#endif // ITEMABSTRACTUPDATER_H
