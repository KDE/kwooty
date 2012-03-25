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

#ifndef ITEMCHILDRENMANAGER_H
#define ITEMCHILDRENMANAGER_H

#include <QObject>
#include <QModelIndex>
#include "itemabstractupdater.h"
#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class ItemParentUpdater;


class ItemChildrenManager : public ItemAbstractUpdater {

    Q_OBJECT

public:
    ItemChildrenManager(CentralWidget*, ItemParentUpdater*);
    bool resetItemStatusIfExtractFail(const QModelIndex);
    void resetFinishedChildrenItemToDecodeFinish(QStandardItem*);
    void resetItemStatusToTarget(QStandardItem*, const ItemStatus&);

private:
    CentralWidget* parent;
    bool smartPar2Download;

    void setupConnections();


signals:
    void downloadWaitingPar2Signal();


public slots:
    void changePar2FilesStatusSlot(const QModelIndex, UtilityNamespace::ItemStatus);
    void settingsChangedSlot();


private slots:


};

#endif // ITEMCHILDRENMANAGER_H
