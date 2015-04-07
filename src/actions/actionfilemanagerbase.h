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


#ifndef ACTIONFILEMANAGERBASE_H
#define ACTIONFILEMANAGERBASE_H

#include <QObject>
#include <QStandardItem>

#include <kjob.h>

#include "kwooty_debug.h"
#include <KAction>

#include "core.h"
#include "mainwindow.h"
#include "actionsmanager.h"
#include "standarditemmodel.h"
#include "servermanager.h"
#include "segmentbuffer.h"
#include "widgets/mytreeview.h"
#include "widgets/centralwidget.h"

#include "utilities/utility.h"
using namespace UtilityNamespace;


class ActionFileManagerBase : public QObject {

    Q_OBJECT

public:

    ActionFileManagerBase(ActionsManager*);

protected:

    enum ActionFileStep {
        ActionFileRequested,
        ActionFileProcessing,
        ActionFileIdle
    };

    Core* core;
    MyTreeView* treeView;
    SegmentBuffer* segmentBuffer;
    ActionsManager* actionsManager;
    StandardItemModel* downloadModel;
    ActionFileStep actionFileStep;

    void setupConnections();
    void displayMessage(const QString&);
    virtual void launchProcess() = 0;


signals:

public slots:

    void processFileSlot();
    virtual void actionTriggeredSlot() = 0;

protected slots:

    virtual void handleResultSlot(KJob*) = 0;

};

#endif // ACTIONFILEMANAGERBASE_H

