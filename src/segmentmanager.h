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
#include "utility.h"
#include "data/segmentdata.h"
using namespace UtilityNamespace;

class CentralWidget;
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


    SegmentManager(CentralWidget* parent = 0);
    SegmentManager();
    void setIdlePauseSegments(QStandardItem*, const UtilityNamespace::ItemStatus);
    QStandardItem* searchItem(const QVariant&, const UtilityNamespace::ItemStatus);
    void updatePendingSegmentsToTargetServer(const int&, const int&, const PendingSegments = UpdateSegments);

private:
    StandardItemModel* downloadModel;
    ItemParentUpdater* itemParentUpdater;

    bool sendNextIdleSegment(QStandardItem*, ClientManagerConn*, const SegmentInfoData&);


signals:

public slots:
    void updateDownloadSegmentSlot(SegmentData);
    void getNextSegmentSlot(ClientManagerConn*);
    void updateDecodeSegmentSlot(QVariant, int, UtilityNamespace::ItemStatus, QString, bool, UtilityNamespace::ArticleEncodingType);
    void updateRepairExtractSegmentSlot(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget);


private slots:




};

#endif // SEGMENTMANAGER_H
