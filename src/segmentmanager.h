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

#ifndef SEGMENTMANAGER_H
#define SEGMENTMANAGER_H

#include <QStandardItem>
#include "utilities/utility.h"
#include "data/segmentdata.h"
#include "data/postdownloadinfodata.h"
using namespace UtilityNamespace;

class Core;
class ClientManagerConn;
class ItemParentUpdater;
class StandardItemModel;
class SegmentData;
class SegmentInfoData;


class SegmentManager : public QObject
{

    Q_OBJECT
    Q_ENUMS(PendingSegments)

public:

    enum PendingSegments { UpdateSegments,
                           ResetSegments
                         };


    SegmentManager(Core*);
    SegmentManager();
    void setIdlePauseSegments(QStandardItem*, const UtilityNamespace::ItemStatus);
    void setIdleDownloadFailSegments(QStandardItem*);
    QStandardItem* searchItem(const QVariant&, const UtilityNamespace::ItemStatus);
    QStandardItem* searchItem(const SegmentData&);
    void updatePendingSegmentsToTargetServer(const int&, const int&, const PendingSegments = UpdateSegments);

private:
    StandardItemModel* downloadModel;
    ItemParentUpdater* itemParentUpdater;

    bool sendNextIdleSegment(QStandardItem*, ClientManagerConn*, SegmentInfoData);


Q_SIGNALS:

public Q_SLOTS:
    void updateDownloadSegmentSlot(SegmentData,  QString = QString());
    void getNextSegmentSlot(ClientManagerConn*);
    void updateDecodeSegmentSlot(PostDownloadInfoData repairDecompressInfoData);
    void updateRepairExtractSegmentSlot(PostDownloadInfoData repairDecompressInfoData);


private Q_SLOTS:




};

#endif // SEGMENTMANAGER_H
