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

#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class StandardItemModel;
class MyTreeView;


class QueueFileObserver : public QObject{

    Q_OBJECT

public:

    QueueFileObserver(CentralWidget* parent = 0);
    UtilityNamespace::ItemStatus getFocusedItemStatus() const;
    int getFocusedProgressValue() const;


private:
    StandardItemModel* downloadModel;
    MyTreeView* treeView;
    QStandardItem* parentItem;
    UtilityNamespace::ItemStatus focusedItemStatus;
    int focusedProgressValue;
    int previousProgressValue;

    void setupConnections();
    QStandardItem* searchParentItem(const UtilityNamespace::ItemStatus);
    void checkProgressItemValue(QStandardItem*);

signals:

    void progressUpdateSignal(const int);
    void statusUpdateSignal(const UtilityNamespace::ItemStatus);

public slots:

    void parentItemChangedSlot();


private slots:



};

#endif // QUEUEFILEOBSERVER_H
