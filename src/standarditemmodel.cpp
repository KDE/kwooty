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




void StandardItemModel::updateStateItem(QStandardItem* stateItem, const UtilityNamespace::ItemStatus status) {

    // get itemstatusdata from stateItem :
    ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();

    // set status :
    itemStatusData.setStatus(status);

    // reinject itemstatusdata in stateItem :
    this->storeStatusDataToItem(stateItem, itemStatusData);

}


void StandardItemModel::updateProgressItem(const QModelIndex& index, const int progressNumber) {

    QStandardItem* progressItem = this->getProgressItemFromIndex(index);

    int currentDownloadProgress = progressItem->data(ProgressRole).toInt();

    if (currentDownloadProgress != progressNumber) {

        // set progression :
        progressItem->setData(progressNumber, ProgressRole);

        if (this->isNzbItem(progressItem)) {
            emit parentProgressItemChangedSignal();
        }

    }

}



void StandardItemModel::updateStatusDataFromIndex(const QModelIndex& index, const ItemStatusData& itemStatusData) {

    QStandardItem* stateItem = this->getStateItemFromIndex(index);
    this->storeStatusDataToItem(stateItem, itemStatusData);

}



void StandardItemModel::storeStatusDataToItem(QStandardItem* stateItem, const ItemStatusData& itemStatusData) {

    // get itemstatusdata from stateItem :
    ItemStatusData currentItemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();

    if (currentItemStatusData != itemStatusData) {

        // set status data to item :
        QVariant variant;
        variant.setValue(itemStatusData);
        stateItem->setData(variant, StatusRole);

        // if state changed and is a parent, emit signal :
        if (this->isNzbItem(stateItem)) {
            emit parentStatusItemChangedSignal(stateItem, itemStatusData);
        }
        // else emit signal if child status have changed :
        else {
            emit childStatusItemChangedSignal(stateItem, itemStatusData);
        }

    }

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


UtilityNamespace::ItemStatus StandardItemModel::getChildStatusFromNzbIndex(const QModelIndex& index, int i) {

    // get corresponding file name index :
    QModelIndex fileNameIndex = index.child(i, FILE_NAME_COLUMN);

    // get current item status :
    QStandardItem* stateItem = this->getStateItemFromIndex(fileNameIndex);

    return this->getStatusFromStateItem(stateItem);

}

QStandardItem* StandardItemModel::getNzbItem(QStandardItem* item) {

    QStandardItem* parentFileNameItem;

    // if current item is a child, retrieve its parent :
    if (!this->isNzbItem(item)) {
        parentFileNameItem = this->getFileNameItemFromIndex(item->parent()->index());
    }
    else {
        parentFileNameItem = this->getFileNameItemFromIndex(item->index());
    }

    return parentFileNameItem;

}

ItemStatusData StandardItemModel::getStatusDataFromIndex(const QModelIndex& index) {

    QStandardItem* stateItem = this->getStateItemFromIndex(index);
    ItemStatusData itemStatusData = stateItem->data(StatusRole).value<ItemStatusData>();
    return itemStatusData;
}


NzbFileData StandardItemModel::getNzbFileDataFromIndex(const QModelIndex& index) {

    QStandardItem* fileNameItem = this->getFileNameItemFromIndex(index);
    NzbFileData currentNzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();
    return currentNzbFileData;
}


int StandardItemModel::getProgressValueFromIndex(const QModelIndex& index) {

    QStandardItem* progressItem = this->getProgressItemFromIndex(index);
    int currentDownloadProgress = progressItem->data(ProgressRole).toInt();
    return currentDownloadProgress;
}


quint64 StandardItemModel::getSizeValueFromIndex(const QModelIndex& index) {
    QStandardItem* sizeItem = this->getSizeItemFromIndex(index);
    quint64 nzbSize = sizeItem->data(SizeRole).toULongLong();
    return nzbSize;
}

QString StandardItemModel::getUuidStrFromIndex(const QModelIndex& index) {

    QStandardItem* nzbFileName = this->getFileNameItemFromIndex(index);
    return nzbFileName->data(IdentifierRole).toString();

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


QStandardItem* StandardItemModel::getFileNameItemFromRowNumber(const int& row) {
    return this->item(row, FILE_NAME_COLUMN);
}



bool StandardItemModel::isNzbItem(QStandardItem* item){
    return (item->parent() == 0);
}
