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
#include <QTreeView>
#include "data/globalfiledata.h"
#include "utility.h"
using namespace UtilityNamespace;


class ClientManagerConn;
class MyStatusBar;
class ItemParentUpdater;
class StandardItemModel;
class RepairDecompressThread;
class SegmentsDecoderThread;
class SegmentManager;
class DataRestorer;
class NzbFileData;
class ItemStatusData;


Q_DECLARE_METATYPE (QStandardItem*)
Q_DECLARE_METATYPE (ItemStatusData*)

class CentralWidget : public QWidget
        
{
    Q_OBJECT
    
public:
    CentralWidget(QWidget* parent = 0, MyStatusBar* parentStatuBar = 0);
    ~CentralWidget();
    void handleNzbFile(QFile& file, const QList<GlobalFileData>& inGlobalFileDataList = QList<GlobalFileData>());
    void restoreDataFromPreviousSession(const QList<GlobalFileData>&);
    void savePendingDownloads();

    SegmentManager* getSegmentManager() const;
    StandardItemModel* getDownloadModel() const;
    MyStatusBar* getStatusBar() const;
    ItemParentUpdater* getItemParentUpdater() const;

private:
    QTreeView* treeView;
    MyStatusBar* statusBar;
    QList<ClientManagerConn*> clientManagerConnList;
    SegmentManager* segmentManager;
    DataRestorer* dataRestorer;
    SegmentsDecoderThread* segmentsDecoderThread;
    RepairDecompressThread* repairDecompressThread;
    ItemParentUpdater* itemParentUpdater;
    StandardItemModel* downloadModel;
    int saveErrorButtonCode;

    void setDataToModel(const QList<GlobalFileData>&, const QString&);
    void addParentItem (QStandardItem*, const GlobalFileData&);
    void setHeaderLabels();
    void createNntpClients();
    void setupConnections();
    void updateItemsInView(QModelIndex, QModelIndex);
    void moveRow(bool);
    void setStartPauseDownload(int, const QList<QModelIndex>&);
    void statusBarFileSizeUpdate();
    void initFoldersSettings();
    
signals:
    void dataHasArrivedSignal();
    void setMoveButtonEnabledSignal(bool);
    void setPauseButtonEnabledSignal(bool);
    void setStartButtonEnabledSignal(bool);
    void setRemoveButtonEnabledSignal(bool);
    void recalculateNzbSizeSignal(const QModelIndex);
    void settingsChangedSignal();
    void setIconToFileNameItemSignal(const QModelIndex);
    void changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus);


public slots:
    void removeRowSlot();
    void clearSlot();
    void moveToTopSlot();
    void moveToBottomSlot();
    void selectedItemSlot();
    void startDownloadSlot();
    void pauseDownloadSlot();
    void saveFileErrorSlot(int);
    void updateSettingsSlot();
    void downloadWaitingPar2Slot();

    // #QTBUG-5201
    #if QT_VERSION == 0x040503
    void dataChangedSlot(QStandardItem*);
    #endif

    
private slots:
    
    
    
};

#endif // CENTRALWIDGET_H
