/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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


#ifndef ACTIONSMANAGER_H
#define ACTIONSMANAGER_H

#include <QObject>
#include <QModelIndex>

#include "kwooty_export.h"

#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class MyTreeView;
class StandardItemModel;
class StandardItemModelQuery;
class ActionButtonsManager;
class ActionMergeManager;

class KWOOTY_EXPORT ActionsManager : public QObject {

    Q_OBJECT

public:
    ActionsManager(Core*);
    Core* getCore() const;
    ActionButtonsManager* getActionButtonsManager() const;
    ActionMergeManager* getActionMergeManager() const;
    void setStartPauseDownloadAllItems(const UtilityNamespace::ItemStatus);
    void setStartPauseDownload(const UtilityNamespace::ItemStatus, const QList<QModelIndex>&);
    void setStartPauseDownload(const UtilityNamespace::ItemStatus, const QModelIndex&);
    void retryDownload(const QModelIndexList&);
    void changePar2FilesStatus(const QModelIndex, UtilityNamespace::ItemStatus);


private:

    enum MoveRowType {
        MoveRowsUp,
        MoveRowsDown,
        MoveRowsTop,
        MoveRowsBottom
    };


    void setupConnections();
    void moveRow(ActionsManager::MoveRowType);

    Core* core;
    MyTreeView* treeView;
    StandardItemModel* downloadModel;
    StandardItemModelQuery* modelQuery;
    ActionMergeManager* actionMergeManager;
    ActionButtonsManager* actionButtonsManager;

    
signals:
    void statusBarFileSizeUpdateSignal(StatusBarUpdateType);
    void allRowRemovedSignal();
    void recalculateNzbSizeSignal(const QModelIndex);
    void changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus);
    void startPauseAboutToBeTriggeredSignal(UtilityNamespace::ItemStatus, QList<QModelIndex>);
    void startPauseTriggeredSignal(UtilityNamespace::ItemStatus);


public slots:
    void moveToTopSlot();
    void moveToBottomSlot();
    void moveUpSlot();
    void moveDownSlot();
    void clearSlot();
    void removeRowSlot();
    void openFolderSlot();
    void startDownloadSlot();
    void pauseDownloadSlot();
    void startAllDownloadSlot();
    void pauseAllDownloadSlot();
    void retryDownloadSlot();
    void manualExtractSlot();


};

#endif // ACTIONSMANAGER_H
