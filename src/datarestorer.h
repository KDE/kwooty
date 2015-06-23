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

#ifndef DATARESTORER_H
#define DATARESTORER_H

#include <QObject>
#include <QStandardItem>
#include <QTimer>
#include <QHash>
#include <QDataStream>

#include "utilities/utility.h"
#include "data/segmentdata.h"
#include "data/itemstatusdata.h"
using namespace UtilityNamespace;

class Core;
class StandardItemModel;
class NzbFileData;
class ItemStatusData;
class GlobalFileData;

class DataRestorer : public QObject
{

    Q_OBJECT

public:

    enum PendingDownloadsManagement { WithConfirmation,
                                      Automatically
                                    };

    explicit DataRestorer(Core *);
    DataRestorer();
    int saveQueueData(const SaveFileBehavior &);
    void setActive(const bool);

private:

    Core *mParent;
    StandardItemModel *mDownloadModel;
    QTimer *mDataSaverTimer;
    QHash<quint32, int> mVersionStreamMap;
    quint32 mMagicNumber;
    quint32 mApplicationVersion1;
    bool mActive;

    QString getPendingFileStr() const;
    bool isDataToSaveExist() const;
    bool isHeaderOk(QDataStream &) const;
    void setupConnections();
    void resetDataForDecodingFile(NzbFileData &, ItemStatusData &, int &);
    void resetDataForDownloadingFile(NzbFileData &, ItemStatusData &);
    void preprocessAndHandleData(const QList< QList<GlobalFileData> > &);
    void writeDataToDisk();
    void requestSuppressOldOrphanedSegments();
    void removePendingDataFile();

Q_SIGNALS:
    void suppressOldOrphanedSegmentsSignal();

public Q_SLOTS:
    void parentStatusItemChangedSlot(QStandardItem *, const ItemStatusData&);

private Q_SLOTS:
    void readDataFromDiskSlot();
    void saveQueueDataSilentlySlot();

};

#endif // DATARESTORER_H
