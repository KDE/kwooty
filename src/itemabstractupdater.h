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

#ifndef ITEMABSTRACTUPDATER_H
#define ITEMABSTRACTUPDATER_H

#include <QObject>
#include <QStandardItem>
#include <QModelIndex>
#include <QHash>

#include "utility.h"
using namespace UtilityNamespace;

class ItemParentUpdater;
class StandardItemModel;
class ItemStatusData;

class ItemAbstractUpdater : public QObject
{


    Q_OBJECT

public:
    ItemAbstractUpdater(QObject* parent = 0);
    ItemAbstractUpdater();

protected:
    StandardItemModel* downloadModel;
    ItemParentUpdater* itemParentUpdater;
    QHash<int, QString> statusIconStrMap;
    int downloadItemNumber;
    int progressNumber;
    int downloadFinishItemNumber;
    int inQueueItemNumber;
    int pauseItemNumber;
    int pausingItemNumber;
    int decodeFinishItemNumber;
    int decodeErrorItemNumber;
    int decodeItemNumber;
    int scanItemNumber;
    int verifyItemNumber;
    int verifyFinishItemNumber;
    int repairItemNumber;
    int articleNotFoundNumber;
    int articleFoundNumber;

    void clear();
    void countItemStatus(const int);
    void setIconToFileNameItem(const QModelIndex&, UtilityNamespace::ItemStatus);

};

#endif // ITEMABSTRACTUPDATER_H
