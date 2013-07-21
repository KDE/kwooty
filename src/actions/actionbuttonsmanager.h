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


#ifndef ACTIONBUTTONSMANAGER_H
#define ACTIONBUTTONSMANAGER_H

#include <QObject>

#include "utilities/utility.h"
using namespace UtilityNamespace;


class ActionsManager;
class Core;
class MyTreeView;
class StandardItemModel;
class StandardItemModelQuery;

class ActionButtonsManager : public QObject {

    Q_OBJECT

public:
    ActionButtonsManager(ActionsManager*);


private:
    ActionsManager* actionsManager;
    Core* core;
    MyTreeView* treeView;
    StandardItemModel* downloadModel;
    StandardItemModelQuery* downloadModelQuery;

    
signals:
    void setMoveButtonEnabledSignal(bool);
    void setPauseButtonEnabledSignal(bool);
    void setPauseAllButtonEnabledSignal(bool);
    void setStartButtonEnabledSignal(bool);
    void setStartAllButtonEnabledSignal(bool);
    void setRemoveButtonEnabledSignal(bool);
    void setRetryButtonEnabledSignal(bool);
    void setMergeNzbButtonEnabledSignal(bool);
    void setRenameNzbButtonEnabledSignal(bool);
    void setManualExtractActionSignal(bool);
    void setRemoveDeleteFileButtonEnabledSignal(bool);

    
public slots:
    void selectedItemSlot();
    
};

#endif // ACTIONBUTTONSMANAGER_H
