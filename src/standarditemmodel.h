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

#ifndef STANDARDITEMMODEL_H
#define STANDARDITEMMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>

#include "data/itemstatusdata.h"
#include "kwooty_export.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class ItemStatusData;
class NzbFileData;

class KWOOTY_EXPORT StandardItemModel : public QStandardItemModel
{

    Q_OBJECT

public:
    explicit StandardItemModel(Core *);
    StandardItemModel();
    QStandardItem *getParentItem(const QModelIndex &) const;
    QStandardItem *getStateItemFromIndex(const QModelIndex &) const;
    QStandardItem *getFileNameItemFromIndex(const QModelIndex &) const;
    QStandardItem *getProgressItemFromIndex(const QModelIndex &) const;
    QStandardItem *getSizeItemFromIndex(const QModelIndex &) const;
    ItemStatusData getStatusDataFromIndex(const QModelIndex &) const;
    QStandardItem *getNzbItem(QStandardItem *item) const;
    QStandardItem *getNzbItem(const QModelIndex &) const;
    NzbFileData getNzbFileDataFromIndex(const QModelIndex &) const;
    int getProgressValueFromIndex(const QModelIndex &) const;
    quint64 getSizeValueFromIndex(const QModelIndex &) const;
    QString getUuidStrFromIndex(const QModelIndex &) const;
    QString getParentFileSavePathFromIndex(const QModelIndex &) const;
    UtilityNamespace::ItemStatus getStatusFromStateItem(QStandardItem *) const;
    UtilityNamespace::ItemStatus getChildStatusFromNzbIndex(const QModelIndex &, int) const;
    QStandardItem *getFileNameItemFromRowNumber(const int &) const;
    bool isNzbItem(QStandardItem *) const;
    void updateStateItem(const QModelIndex &index, const UtilityNamespace::ItemStatus);
    void updateProgressItem(const QModelIndex &, const int);
    void storeStatusDataToItem(QStandardItem *, const ItemStatusData &);
    void initStatusDataToItem(QStandardItem *, const ItemStatusData &);
    void updateNzbFileDataToItem(QStandardItem *, const NzbFileData &);
    void updateStatusDataFromIndex(const QModelIndex &, const ItemStatusData &);
    void updateParentFileSavePathFromIndex(const QModelIndex &, const NzbFileData &);

private:
    QStandardItem *getColumnItem(const QModelIndex &, const int) const;

Q_SIGNALS:
    void parentStatusItemChangedSignal(QStandardItem *, ItemStatusData);
    void childStatusItemChangedSignal(QStandardItem *, ItemStatusData);
    void parentProgressItemChangedSignal();
};

#endif // STANDARDITEMMODEL_H
