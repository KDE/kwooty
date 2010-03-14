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

    // set status icon :
    this->setIconToFileNameItem(parentModelIndex, status);

    // if status comes from *decode* process :
    if (status <= ScanStatus) {
        this->updateDecodeItems(parentModelIndex, progression, status);
    }
    // if status comes from *verify/repair* or *extract* process :
    else if ( (status >= VerifyStatus) &&
              (status <= SevenZipProgramMissing) ) {
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






void ItemPostDownloadUpdater::addFileTypeInfo(QStandardItem* fileNameItem, const QString& decodedFileName, const bool& crc32Match) {


    NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();

    // set the name of the decoded file :
    if (!decodedFileName.isEmpty()) {
        nzbFileData.setDecodedFileName(decodedFileName);

        // add info about type of file (par2 or rar file) :
        if (!nzbFileData.isPar2File() &&
            !nzbFileData.isArchiveFile()) {

            QFile decodedFile(nzbFileData.getFileSavePath() + nzbFileData.getDecodedFileName());
            if (decodedFile.exists()) {

                decodedFile.open(QIODevice::ReadOnly);

                // rar headers could be corrupted because repair process has not been proceeded yet at this stage
                // but assume that at least one rar file will have a correct header to launch decompress process later :
                if (decodedFile.peek(rarFileMagicNumber.size()) == rarFileMagicNumber) {
                    nzbFileData.setArchiveFormat(RarFormat);
                }

                if (decodedFile.peek(zipFileMagicNumber.size()) == zipFileMagicNumber) {
                    nzbFileData.setArchiveFormat(ZipFormat);
                }

                if (decodedFile.peek(sevenZipFileMagicNumber.size()) == sevenZipFileMagicNumber) {
                    nzbFileData.setArchiveFormat(SevenZipFormat);
                }

                // check if it is a par2 file :
                else if ( (decodedFile.peek(par2FileMagicNumber.size()) == par2FileMagicNumber) ||
                          decodedFileName.endsWith(par2FileExt, Qt::CaseInsensitive)) {

                    nzbFileData.setPar2File(true);
                }

                decodedFile.close();
            }
        }

    }


    // update the nzbFileData of current fileNameItem and its corresponding items :
    this->downloadModel->updateNzbFileDataToItem(fileNameItem, nzbFileData);


    // set the name of the decoded file :
    if (!decodedFileName.isEmpty()) {

        ItemStatusData itemStatusData = this->downloadModel->getStatusDataFromIndex(fileNameItem->index());

        // inidicate if crc32 of the archive is ok :
        if (!crc32Match) {
            itemStatusData.setCrc32Match(CrcKo);
        }

        QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(fileNameItem->index());
        this->downloadModel->storeStatusDataToItem(stateItem, itemStatusData);

    }

}
