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

class KWOOTY_EXPORT StandardItemModel : public QStandardItemModel {

    Q_OBJECT

public:
    StandardItemModel(Core*);
    StandardItemModel();
    QStandardItem* getParentItem(const QModelIndex&);
    QStandardItem* getStateItemFromIndex(const QModelIndex&);
    QStandardItem* getFileNameItemFromIndex(const QModelIndex&);
    QStandardItem* getProgressItemFromIndex(const QModelIndex&);
    QStandardItem* getSizeItemFromIndex(const QModelIndex&);
    ItemStatusData getStatusDataFromIndex(const QModelIndex&);
    QStandardItem* getNzbItem(QStandardItem* item);
    QStandardItem* getNzbItem(const QModelIndex&);
    NzbFileData getNzbFileDataFromIndex(const QModelIndex&);
    int getProgressValueFromIndex(const QModelIndex&);
    quint64 getSizeValueFromIndex(const QModelIndex&);
    QString getUuidStrFromIndex(const QModelIndex&);
    QString getParentFileSavePathFromIndex(const QModelIndex&);
    UtilityNamespace::ItemStatus getStatusFromStateItem(QStandardItem*) const;
    UtilityNamespace::ItemStatus getChildStatusFromNzbIndex(const QModelIndex&, int);
    QStandardItem* getFileNameItemFromRowNumber(const int&);
    bool isNzbItem(QStandardItem*);
    void updateStateItem(QStandardItem*, const UtilityNamespace::ItemStatus);
    void updateProgressItem(const QModelIndex&, const int);
    void storeStatusDataToItem(QStandardItem*, const ItemStatusData&);
    void updateNzbFileDataToItem(QStandardItem*, const NzbFileData&);
    void updateStatusDataFromIndex(const QModelIndex&, const ItemStatusData&);
    void updateParentFileSavePathFromIndex(const QModelIndex&, const QString&);


private:
    QStandardItem* getColumnItem(const QModelIndex&, const int);


signals:
    void parentStatusItemChangedSignal(QStandardItem*, ItemStatusData);
    void childStatusItemChangedSignal(QStandardItem*, ItemStatusData);
    void parentProgressItemChangedSignal();


public slots:

private slots:


};

#endif // STANDARDITEMMODEL_H
