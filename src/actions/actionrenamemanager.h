/***************************************************************************
 *   Copyright (C) 2013 by Xavier Lefage                                   *
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


#ifndef ACTIONRENAMEMANAGER_H
#define ACTIONRENAMEMANAGER_H

#include <QObject>
#include <QStandardItem>

#include <kio/copyjob.h>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class SegmentBuffer;
class ActionsManager;
class StandardItemModel;
class MyTreeView;


class ActionRenameManager : public QObject {

    Q_OBJECT

public:
    ActionRenameManager(ActionsManager*);
    void checkRenameCandidates(bool&);


private:

    enum ActionRenameStep {
        ActionRenameRequested,
        ActionRenameProcessing,
        ActionRenameIdle
    };

    Core* core;
    MyTreeView* treeView;
    SegmentBuffer* segmentBuffer;
    ActionsManager* actionsManager;
    StandardItemModel* downloadModel;
    QString input;
    QString selectedItemUuid;
    ActionRenameStep actionRenameStep;

    void setupConnections();
    void processRename(QStandardItem*);
    void displayMessage(const QString&);
    bool validateNewFolderName(QStandardItem*) const;
    bool isRenameAllowed(QStandardItem*) const;


signals:


public slots:

    void processRenameSlot();
    void renameNzbActionSlot();
    void handleResultSlot(KJob*);

};

#endif // ACTIONRENAMEMANAGER_H
