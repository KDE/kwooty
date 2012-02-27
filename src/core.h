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

#ifndef core_H
#define core_H

#include <KAboutData>

#include <QFile>
#include <QStandardItem>
#include "data/globalfiledata.h"
#include "kwooty_export.h"

#include "utilities/utility.h"
using namespace UtilityNamespace;

class ClientManagerConn;
class MyTreeView;
class CentralWidget;
class ItemParentUpdater;
class NzbFileHandler;
class StandardItemModel;
class StandardItemModelQuery;
class RepairDecompressThread;
class SegmentsDecoderThread;
class SegmentManager;
class DataRestorer;
class ShutdownManager;
class ClientsObserver;
class FileOperations;
class MainWindow;
class QueueFileObserver;
class NotificationManager;
class ServerManager;
class SideBar;
class ActionsManager;
class NzbFileData;
class ItemStatusData;



Q_DECLARE_METATYPE (QStandardItem*)
Q_DECLARE_METATYPE (ItemStatusData*)


class KWOOTY_EXPORT Core : public QObject {

    Q_OBJECT
    
public:
    Core(MainWindow*);
    ~Core();
    void handleNzbFile(QFile& file, const QList<GlobalFileData>& inGlobalFileDataList = QList<GlobalFileData>());
    void restoreDataFromPreviousSession(const QList<GlobalFileData>&);
    int savePendingDownloads(UtilityNamespace::SystemShutdownType systemShutdownType = UtilityNamespace::ShutdownMethodUnknown, const SaveFileBehavior = SaveNotSilently);

    MainWindow* getMainWindow() const;
    MyTreeView* getTreeView() const;
    DataRestorer* getDataRestorer() const;
    ServerManager* getServerManager() const;
    ActionsManager* getActionsManager() const;
    SegmentManager* getSegmentManager() const;
    NzbFileHandler* getNzbFileHandler() const;
    FileOperations* getFileOperations() const;
    ShutdownManager* getShutdownManager() const;
    StandardItemModel* getDownloadModel() const;
    ClientsObserver* getClientsObserver() const;
    StandardItemModelQuery* getModelQuery() const;
    QueueFileObserver* getQueueFileObserver() const;
    ItemParentUpdater* getItemParentUpdater() const;
    SegmentsDecoderThread* getSegmentsDecoderThread() const;
    SideBar* getSideBar() const;
    CentralWidget* getCentralWidget() const;


private:
    MainWindow* mainWindow;
    MyTreeView* treeView;
    DataRestorer* dataRestorer;
    ServerManager* serverManager;
    ActionsManager* actionsManager;
    NzbFileHandler* nzbFileHandler;
    SegmentManager* segmentManager;
    FileOperations* fileOperations;
    ShutdownManager* shutdownManager;
    StandardItemModel* downloadModel;
    ClientsObserver* clientsObserver;
    StandardItemModelQuery* modelQuery;
    QueueFileObserver* queueFileObserver;
    ItemParentUpdater* itemParentUpdater;
    NotificationManager* notificationManager;
    SegmentsDecoderThread* segmentsDecoderThread;
    RepairDecompressThread* repairDecompressThread;

    void setDataToModel(const QList<GlobalFileData>&, const QString&);
    void addParentItem (QStandardItem*, const GlobalFileData&);
    void statusBarFileSizeUpdate();
    void initFoldersSettings();

    
signals:
    void dataHasArrivedSignal();
    void settingsChangedSignal();
    void passwordEnteredByUserSignal(bool, QString password = QString());
    void changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus);


public slots:
    void saveFileErrorSlot(const int);
    void updateSettingsSlot();
    void downloadWaitingPar2Slot();
    void statusBarFileSizeUpdateSlot(StatusBarUpdateType);
    void serverStatisticsUpdateSlot(const int);
    void extractPasswordRequiredSlot(QString);

    
private slots:
    
    
    
};

#endif // core_H
