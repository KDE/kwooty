/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
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


#ifndef QUEUEFILEOBSERVER_H
#define QUEUEFILEOBSERVER_H

#include <QObject>
#include <QStandardItem>
#include <QMap>
#include <QDateTime>
#include <QTimer>

#include "data/jobnotifydata.h"
#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class StandardItemModel;
class MyTreeView;


class QueueFileObserver : public QObject {

    Q_OBJECT

public:

    QueueFileObserver(CentralWidget* parent = 0);
    UtilityNamespace::ItemStatus getFocusedItemStatus() const;
    QStandardItem* searchParentItem(const UtilityNamespace::ItemStatus);
    ItemStatus isRetryDownloadAllowed(QStandardItem*, bool* = 0);
    int getFocusedProgressValue() const;
    bool areJobsFinished();
    bool haveItemsSameParent(const QList<QModelIndex>&);



private:

    static const int MAX_LIST_SIZE = 10;

    StandardItemModel* downloadModel;
    MyTreeView* treeView;
    QStandardItem* parentItem;
    QTimer* jobNotifyTimer;
    QList<JobNotifyData> jobNotifyDataList;
    UtilityNamespace::ItemStatus focusedItemStatus;
    int focusedProgressValue;
    int previousProgressValue;

    void setupConnections();

    void checkProgressItemValue(QStandardItem*);
    JobNotifyData retrieveJobNotifyData(QStandardItem*, UtilityNamespace::ItemStatus);
    void addToList(const JobNotifyData&);



signals:

    void progressUpdateSignal(const int);
    void statusUpdateSignal(const UtilityNamespace::ItemStatus);
    void jobFinishSignal(const UtilityNamespace::ItemStatus, const QString);

public slots:

    void parentItemChangedSlot();
    void jobFinishStatusSlot(QStandardItem*);


private slots:


    void checkJobFinishSlot();



};

#endif // QUEUEFILEOBSERVER_H
