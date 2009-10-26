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

#include "standarditemmodel.h"

#include "centralwidget.h"
#include "data/itemstatusdata.h"
#include "data/nzbfiledata.h"


StandardItemModel::StandardItemModel(CentralWidget* parent) : QStandardItemModel (parent)
{
}

StandardItemModel::StandardItemModel()
{
}



QStandardItem* StandardItemModel::getParentItem(const QModelIndex& index){

    QStandardItem* parentItem = NULL;

    if (index.isValid()) {
        // if the parent has been selected (a nzb item):
        if (index.parent() == QModelIndex()){
            parentItem = this->invisibleRootItem();
        }
        // else files of the parent (nzb item) has been selected :
        else{
            parentItem = this->itemFromIndex(index.parent());
        }

    }

    return parentItem;
}


QStandardItem* StandardItemModel::getColumnItem(const QModelIndex& index, const int column){

    QStandardItem* item = NULL;

    if (index.isValid()) {

        // get parent item :
        QStandardItem* parentItem = this->getParentItem(index);

        // get child at the corresponding column :
        item = parentItem->child(index.row(), column);
    }

    return item;
}




void StandardItemModel::updateSateItem(QStandardItem* stateItem, const UtilityNamespace::ItemStatus status) {
    // get itemstatusdata from stateItem :
    ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();
    // set status :
    itemStatusData.setStatus(status);
    // reinject itemstatusdata in stateItem :
    this->storeStatusDataToItem(stateItem, itemStatusData);
}


void StandardItemModel::updateProgressItem(const QModelIndex& index, const int progressNumber) {

    QStandardItem* progressItem = this->getProgressItemFromIndex(index);
    // set progression :
    progressItem->setData(progressNumber, ProgressRole);
}





void StandardItemModel::storeStatusDataToItem(QStandardItem* stateItem, const ItemStatusData& itemStatusData) {
    // set status data to item :
    QVariant variant;
    variant.setValue(itemStatusData);
    stateItem->setData(variant, StatusRole);
}


void StandardItemModel::updateNzbFileDataToItem(QStandardItem* item, const NzbFileData& nzbFileData) {
    // set nzbFileData to item :
    QVariant variant;
    variant.setValue(nzbFileData);
    item->setData(variant, NzbFileDataRole);
}



UtilityNamespace::ItemStatus StandardItemModel::getStatusFromStateItem(QStandardItem* stateItem) const {
    ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();
    return itemStatusData.getStatus();
}



QStandardItem* StandardItemModel::getStateItemFromIndex(const QModelIndex& index) {
    return this->getColumnItem(index, STATE_COLUMN);
}


QStandardItem* StandardItemModel::getFileNameItemFromIndex(const QModelIndex& index) {
    return this->getColumnItem(index, FILE_NAME_COLUMN);
}


QStandardItem* StandardItemModel::getProgressItemFromIndex(const QModelIndex& index) {
    return this->getColumnItem(index, PROGRESS_COLUMN);
}


QStandardItem* StandardItemModel::getSizeItemFromIndex(const QModelIndex& index) {
    return this->getColumnItem(index, SIZE_COLUMN);
}




bool StandardItemModel::isNzbItem(QStandardItem* item){

    bool isNzb = false;
    if (item->parent() == 0){
        isNzb = true;
    }
    return isNzb;
}
