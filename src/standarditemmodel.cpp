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

#include "core.h"
#include "data/nzbfiledata.h"


StandardItemModel::StandardItemModel(Core* parent) : QStandardItemModel (parent)
{
}

StandardItemModel::StandardItemModel()
{
}



QStandardItem* StandardItemModel::getParentItem(const QModelIndex& index) const{

    QStandardItem* parentItem = 0;

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


QStandardItem* StandardItemModel::getColumnItem(const QModelIndex& index, const int column) const {

    QStandardItem* item = 0;

    if (index.isValid()) {

        // get parent item :
        QStandardItem* parentItem = this->getParentItem(index);

        // get child at the corresponding column :
        item = parentItem->child(index.row(), column);
    }

    return item;
}



void StandardItemModel::updateStateItem(const QModelIndex& index, const UtilityNamespace::ItemStatus status) {

    // retrieve current progression and status related items :
    QStandardItem* stateItem = this->getStateItemFromIndex(index);

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



void StandardItemModel::initStatusDataToItem(QStandardItem* stateItem, const ItemStatusData& itemStatusData) {

    // convenience method to force signal to be emitted upon first data storage :
    this->storeStatusDataToItem(stateItem, itemStatusData);

    if (this->isNzbItem(stateItem)) {
        emit parentStatusItemChangedSignal(stateItem, itemStatusData);
    }
    else {
        emit childStatusItemChangedSignal(stateItem, itemStatusData);
    }

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


UtilityNamespace::ItemStatus StandardItemModel::getChildStatusFromNzbIndex(const QModelIndex& index, int i) const {

    // get corresponding file name index :
    QModelIndex fileNameIndex = index.child(i, FILE_NAME_COLUMN);

    // get current item status :
    QStandardItem* stateItem = this->getStateItemFromIndex(fileNameIndex);

    return this->getStatusFromStateItem(stateItem);

}

QStandardItem* StandardItemModel::getNzbItem(const QModelIndex& index) const {

    return this->getNzbItem(this->itemFromIndex(index));

}

QStandardItem* StandardItemModel::getNzbItem(QStandardItem* item) const {

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

ItemStatusData StandardItemModel::getStatusDataFromIndex(const QModelIndex& index) const {

    QStandardItem* stateItem = this->getStateItemFromIndex(index);
    return stateItem->data(StatusRole).value<ItemStatusData>();

}


NzbFileData StandardItemModel::getNzbFileDataFromIndex(const QModelIndex& index) const {

    QStandardItem* fileNameItem = this->getFileNameItemFromIndex(index);
    return fileNameItem->data(NzbFileDataRole).value<NzbFileData>();

}


int StandardItemModel::getProgressValueFromIndex(const QModelIndex& index) const {

    QStandardItem* progressItem = this->getProgressItemFromIndex(index);
    return progressItem->data(ProgressRole).toInt();

}


quint64 StandardItemModel::getSizeValueFromIndex(const QModelIndex& index) const {
    QStandardItem* sizeItem = this->getSizeItemFromIndex(index);
    return sizeItem->data(SizeRole).toULongLong();
}

QString StandardItemModel::getUuidStrFromIndex(const QModelIndex& index) const {

    QStandardItem* nzbFileName = this->getFileNameItemFromIndex(index);
    return nzbFileName->data(IdentifierRole).toString();

}

QString StandardItemModel::getParentFileSavePathFromIndex(const QModelIndex& index) const {

    QStandardItem* parentNzbFileNameItem = this->getNzbItem(index);
    return this->getNzbFileDataFromIndex(parentNzbFileNameItem->index()).getFileSavePath();

}

void StandardItemModel::updateParentFileSavePathFromIndex(const QModelIndex& index, const QString& nzbFileSavePath) {

    QStandardItem* parentNzbFileNameItem = this->getNzbItem(index);

    NzbFileData parentNzbFileData = this->getNzbFileDataFromIndex(parentNzbFileNameItem->index());
    parentNzbFileData.setFileSavePath(nzbFileSavePath);

    this->updateNzbFileDataToItem(parentNzbFileNameItem, parentNzbFileData);

}



QStandardItem* StandardItemModel::getStateItemFromIndex(const QModelIndex& index) const {
    return this->getColumnItem(index, STATE_COLUMN);
}


QStandardItem* StandardItemModel::getFileNameItemFromIndex(const QModelIndex& index) const {
    return this->getColumnItem(index, FILE_NAME_COLUMN);
}


QStandardItem* StandardItemModel::getProgressItemFromIndex(const QModelIndex& index) const {
    return this->getColumnItem(index, PROGRESS_COLUMN);
}


QStandardItem* StandardItemModel::getSizeItemFromIndex(const QModelIndex& index) const {
    return this->getColumnItem(index, SIZE_COLUMN);
}


QStandardItem* StandardItemModel::getFileNameItemFromRowNumber(const int& row) const {
    return this->item(row, FILE_NAME_COLUMN);
}



bool StandardItemModel::isNzbItem(QStandardItem* item) const {
    return (item->parent() == 0);
}
