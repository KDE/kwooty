/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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


#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QTimer>
#include <QStandardItemModel>
#include <QHash>

#include "widgets/mystatusbar.h"
#include "utilities/utility.h"
#include "schedulerfilehandler.h"
using namespace UtilityNamespace;
using namespace SchedulerNamespace;


class Core;
class SchedulerPlugin;
class ServerManager;


class Scheduler : public QObject {

    Q_OBJECT

public:
    Scheduler(SchedulerPlugin*);
    ~Scheduler();
    void settingsChanged();
    void disableSpeedLimit();


private:

    enum BypassSchedulerMethod { BypassItemsPause,
                                 BypassItemsStart,
                                 BypassItemsPauseOrStart,
                                 BypassNoItems
                               };


    static const int NO_SPEED_LIMIT = 0;

    QStandardItemModel* schedulerModel;
    Core* core;
    ServerManager* serverManager;
    MyStatusBar* statusBar;
    QTimer* schedulerTimer;
    DownloadLimitStatus downloadLimitStatus;
    QHash<QString, Scheduler::BypassSchedulerMethod> manuallyUuidStartPauseMap;

    DownloadLimitStatus getCurrentDownloadLimitStatus();
    void setupConnections();
    void applySpeedLimit();
    void checkDownloadStatus(const DownloadLimitStatus&);
    void suspendDownloads();
    void resumeDownloads();
    void scheduleStartPauseDownload(UtilityNamespace::ItemStatus);
    QList<QString> retrieveProperListFromMap(const UtilityNamespace::ItemStatus&) const;
    void initUuidStartPauseMap();



signals:


public slots:
    void serverManagerSettingsChangedSlot();
    void statusBarWidgetDblClickSlot(MyStatusBar::WidgetIdentity);

private slots:
    void schedulerTimerSlot();
    void dataAboutToArriveSlot(QModelIndex = QModelIndex());
    void startPauseAboutToBeTriggeredSlot(UtilityNamespace::ItemStatus, QList<QModelIndex>);

};

#endif // SCHEDULER_H
