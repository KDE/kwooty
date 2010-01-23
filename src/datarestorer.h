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

#include "utility.h"
#include "data/segmentdata.h"
using namespace UtilityNamespace;

class CentralWidget;
class StandardItemModel;
class NzbFileData;
class ItemStatusData;
class GlobalFileData;

class DataRestorer : public QObject
{

    Q_OBJECT

public:

    DataRestorer(CentralWidget* parent = 0);
    DataRestorer();
    void saveQueueData(const bool);
    void setActive(const bool);

private:
    CentralWidget* parent;
    StandardItemModel* downloadModel;
    QTimer* dataSaverTimer;
    QHash<quint32, int> versionStreamMap;
    quint32 magicNumber;
    quint32 applicationVersion1;
    bool active;

    void setupConnections();
    void resetDataForDecodingFile(NzbFileData&, ItemStatusData&, int&);
    void resetDataForDownloadingFile(NzbFileData&, ItemStatusData&);
    void preprocessAndHandleData(const QList< QList<GlobalFileData> >&);
    void writeDataToDisk();
    int displayRestoreMessageBox() const;
    int displaySaveMessageBox(const bool) const;
    bool isDataToSaveExist() const;
    bool isHeaderOk(QDataStream&) const;


signals:
    void suppressOldOrphanedSegmentsSignal();


public slots:


private slots:
    void readDataFromDiskSlot();
    void saveQueueDataSilentlySlot();


};

#endif // DATARESTORER_H
