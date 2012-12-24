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


#ifndef ACTIONMERGEMANAGER_H
#define ACTIONMERGEMANAGER_H

#include <QObject>
#include <QAction>
#include <QStandardItem>

#include <kio/copyjob.h>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class MyTreeView;
class ActionsManager;
class StandardItemModel;


class ActionMergeManager : public QObject {

    Q_OBJECT

public:
    ActionMergeManager(ActionsManager*);
    QList<QStandardItem*> checkMergeCandidates(bool&);

private:
    Core* core;
    MyTreeView* treeView;
    ActionsManager* actionsManager;
    StandardItemModel* downloadModel;

    bool isMergeAllowed(QStandardItem*) const;
    void setupConnections();
    void processMerge(QStandardItem*, QStandardItem*);



signals:
    void recalculateNzbSizeSignal(const QModelIndex);


public slots:
    void mergeSubMenuAboutToShowSlot();
    void mergeNzbActionTriggeredSlot(QAction*);
    void handleResultSlot(KJob*);


};

#endif // ACTIONMERGEMANAGER_H
