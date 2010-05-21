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

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <KAboutData>
#include <QFile>
#include <QStandardItem>
#include "mytreeview.h"
#include "data/globalfiledata.h"
#include "utility.h"

using namespace UtilityNamespace;


class ClientManagerConn;
class MyTreeView;
class ItemParentUpdater;
class StandardItemModel;
class RepairDecompressThread;
class SegmentsDecoderThread;
class SegmentManager;
class DataRestorer;
class ShutdownManager;
class ClientsObserver;
class FileOperations;
class FolderWatcher;
class MainWindow;
class QueueFileObserver;
class NzbFileData;
class ItemStatusData;



Q_DECLARE_METATYPE (QStandardItem*)
Q_DECLARE_METATYPE (ItemStatusData*)


class CentralWidget : public QWidget

{
    Q_OBJECT
    
public:
    CentralWidget(MainWindow* parent = 0);
    ~CentralWidget();
    void handleNzbFile(QFile& file, const QList<GlobalFileData>& inGlobalFileDataList = QList<GlobalFileData>());
    void restoreDataFromPreviousSession(const QList<GlobalFileData>&);
    int savePendingDownloads(UtilityNamespace::SystemShutdownType systemShutdownType = UtilityNamespace::ShutdownMethodUnknown, bool saveSilently = false);

    SegmentManager* getSegmentManager() const;
    StandardItemModel* getDownloadModel() const;
    ItemParentUpdater* getItemParentUpdater() const;
    MyTreeView* getTreeView() const;
    ShutdownManager* getShutdownManager() const;
    ClientsObserver* getClientsObserver() const;
    FileOperations* getFileOperations() const;
    QueueFileObserver* getQueueFileObserver() const;


private:

    MyTreeView* treeView;
    QList<ClientManagerConn*> clientManagerConnList;
    SegmentManager* segmentManager;
    DataRestorer* dataRestorer;
    ShutdownManager* shutdownManager;
    SegmentsDecoderThread* segmentsDecoderThread;
    RepairDecompressThread* repairDecompressThread;
    ItemParentUpdater* itemParentUpdater;
    StandardItemModel* downloadModel;
    ClientsObserver* clientsObserver;
    FileOperations* fileOperations;
    FolderWatcher* folderWatcher;
    QueueFileObserver* queueFileObserver;
    int saveErrorButtonCode;
    

    void setDataToModel(const QList<GlobalFileData>&, const QString&);
    void addParentItem (QStandardItem*, const GlobalFileData&);
    void setupConnections();
    void updateItemsInView(QModelIndex, QModelIndex);
    void setStartPauseDownloadAllItems(const UtilityNamespace::ItemStatus);
    void setStartPauseDownload(const UtilityNamespace::ItemStatus, const QList<QModelIndex>&);
    void statusBarFileSizeUpdate();
    void initFoldersSettings();
    void createNntpClients();


    
signals:
    void dataHasArrivedSignal();
    void setPauseButtonEnabledSignal(bool);
    void setStartButtonEnabledSignal(bool);
    void setRemoveButtonEnabledSignal(bool);
    void recalculateNzbSizeSignal(const QModelIndex);
    void settingsChangedSignal();
    void setIconToFileNameItemSignal(const QModelIndex);
    void changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus);


public slots:
    void startDownloadSlot();
    void pauseDownloadSlot();
    void saveFileErrorSlot(int);
    void updateSettingsSlot();
    void downloadWaitingPar2Slot();
    void statusBarFileSizeUpdateSlot(StatusBarUpdateType);
    void startAllDownloadSlot();
    void pauseAllDownloadSlot();

    
private slots:
    
    
    
};

#endif // CENTRALWIDGET_H
