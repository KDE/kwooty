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

#include "itempostdownloadupdater.h"

#include <KDebug>
#include "data/itemstatusdata.h"
#include "data/nzbfiledata.h"
#include "centralwidget.h"
#include "itemparentupdater.h"
#include "standarditemmodel.h"


ItemPostDownloadUpdater::ItemPostDownloadUpdater(ItemParentUpdater* itemParentUpdater) : ItemAbstractUpdater(itemParentUpdater)
{
    this->itemParentUpdater = itemParentUpdater;
    this->downloadModel = itemParentUpdater->getDownloadModel();
}




void ItemPostDownloadUpdater::updateItems(const QModelIndex& parentModelIndex, const int progression, const UtilityNamespace::ItemStatus status, UtilityNamespace::ItemTarget itemTarget) {

    // if status comes from *decode* process :
    if (status <= ScanStatus) {
        this->updateDecodeItems(parentModelIndex, progression, status);
    }
    // if status comes from *verify/repair* or *extract* process :
    else if ( (status >= VerifyStatus) &&
              (status <= UnrarProgramMissing) ) {
        this->updateRepairExtractItems(parentModelIndex, progression, status, itemTarget);
    }

}



void ItemPostDownloadUpdater::updateDecodeItems(const QModelIndex& parentModelIndex, const int progression, const UtilityNamespace::ItemStatus status) {

    // set progress to item :
    this->downloadModel->updateProgressItem(parentModelIndex, progression);

    // retrieve current progression and status related items :
    QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(parentModelIndex);

    // set status to items :
    this->downloadModel->updateSateItem(stateItem, status);

    // move item to bottom of the list when decoding is finished :
    if (progression == PROGRESS_COMPLETE) {
        QStandardItem* nzbItem = stateItem->parent();
        nzbItem->appendRow(nzbItem->takeRow(stateItem->row()));
    }

    // update parent (nzb item) status :
    itemParentUpdater->updateNzbItems(parentModelIndex.parent());

}




void ItemPostDownloadUpdater::updateRepairExtractItems(const QModelIndex& parentModelIndex, const int progression, const UtilityNamespace::ItemStatus status, UtilityNamespace::ItemTarget itemTarget) {

    // retrieve current progression and status related items :
    QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(parentModelIndex);

    NzbFileData currentNzbFileData = parentModelIndex.data(NzbFileDataRole).value<NzbFileData>();

    // update child and parent items :
    if (itemTarget == UtilityNamespace::BothItemsTarget) {

        // update item status :
        this->downloadModel->updateSateItem(stateItem, status);
        // set progress to item :
        this->downloadModel->updateProgressItem(parentModelIndex, progression);

        // update parent (nzb item) status :
        itemParentUpdater->updateNzbItemsPostDecode(parentModelIndex.parent(), progression, status);
    }
    // update only child item :
    else if (itemTarget == UtilityNamespace::ChildItemTarget) {

        // update item status :
        this->downloadModel->updateSateItem(stateItem, status);

        // set progress to item :
        this->downloadModel->updateProgressItem(parentModelIndex, progression);
    }
    // update only parent item :
    else if (itemTarget == UtilityNamespace::ParentItemTarget) {

        // update parent (nzb item) status :
        itemParentUpdater->updateNzbItemsPostDecode(parentModelIndex.parent(), progression, status);
    }

}




