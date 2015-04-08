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

#include "kwooty_debug.h"

#include <QFileInfo>

#include "data/itemstatusdata.h"
#include "data/nzbfiledata.h"
#include "data/postdownloadinfodata.h"
#include "core.h"
#include "itemparentupdater.h"
#include "standarditemmodel.h"
#include "widgets/mytreeview.h"
#include "fileoperations.h"


ItemPostDownloadUpdater::ItemPostDownloadUpdater(ItemParentUpdater* itemParentUpdater) : ItemAbstractUpdater(itemParentUpdater->getDownloadModel(), ItemAbstractUpdater::Child) {

    this->itemParentUpdater = itemParentUpdater;

}




void ItemPostDownloadUpdater::updateItems(const PostDownloadInfoData& postDownloadInfoData) {

    UtilityNamespace::ItemStatus status = postDownloadInfoData.getStatus();

    // if status comes from *decode* process :
    if (status <= ScanStatus) {

        this->updateDecodeItems(postDownloadInfoData);

    }
    // if status comes from *verify/repair* or *extract* process :
    else if ( (status >= VerifyStatus) &&
              (status <= SevenZipProgramMissing) ) {

        this->updateRepairExtractItems(postDownloadInfoData);

    }

}



void ItemPostDownloadUpdater::updateDecodeItems(const PostDownloadInfoData& repairDecompressInfoData) {

    UtilityNamespace::ItemStatus status = repairDecompressInfoData.getStatus() ;
    QModelIndex parentModelIndex = repairDecompressInfoData.getModelIndex();
    int progression = repairDecompressInfoData.getProgression();


    // if item has been decoded store this status (useful for download retry feature) :
    if (status == DecodeFinishStatus) {

        ItemStatusData itemStatusData = this->downloadModel->getStatusDataFromIndex(parentModelIndex);
        itemStatusData.setDecodeFinish(true);
        this->downloadModel->updateStatusDataFromIndex(parentModelIndex, itemStatusData);
    }


    // set progress to item :
    this->downloadModel->updateProgressItem(parentModelIndex, progression);

    // set status to items :
    this->downloadModel->updateStateItem(parentModelIndex, status);


    // move item to bottom of the list when decoding is finished :
    if (progression == PROGRESS_COMPLETE) {

        // retrieve state item :
        QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(parentModelIndex);

        QStandardItem* nzbItem = stateItem->parent();
        nzbItem->appendRow(nzbItem->takeRow(stateItem->row()));


    }

    // update parent (nzb item) status :
    itemParentUpdater->updateNzbItems(parentModelIndex.parent());

}




void ItemPostDownloadUpdater::updateRepairExtractItems(const PostDownloadInfoData& repairDecompressInfoData) {

    UtilityNamespace::ItemStatus status = repairDecompressInfoData.getStatus() ;
    QModelIndex parentModelIndex = repairDecompressInfoData.getModelIndex();
    int progression = repairDecompressInfoData.getProgression();
    UtilityNamespace::ItemTarget itemTarget = repairDecompressInfoData.getItemTarget();


    // update child and parent items :
    if (itemTarget == UtilityNamespace::BothItemsTarget) {

        // update item status :
        this->downloadModel->updateStateItem(parentModelIndex, status);
        // set progress to item :
        this->downloadModel->updateProgressItem(parentModelIndex, progression);

        // update parent (nzb item) status :
        this->updateRepairExtractParentItems(repairDecompressInfoData);

    }
    // update only child item :
    else if (itemTarget == UtilityNamespace::ChildItemTarget) {

        // update item status :
        this->downloadModel->updateStateItem(parentModelIndex, status);

        // set progress to item :
        this->downloadModel->updateProgressItem(parentModelIndex, progression);
    }
    // update only parent item :
    else if (itemTarget == UtilityNamespace::ParentItemTarget) {

        // update parent (nzb item) status :
        this->updateRepairExtractParentItems(repairDecompressInfoData);


    }

}


void ItemPostDownloadUpdater::updateRepairExtractParentItems(const PostDownloadInfoData& repairDecompressInfoData) {

    PostDownloadInfoData repairDecompressParentInfoData = repairDecompressInfoData;

    // retrieve parent model index :
    repairDecompressParentInfoData.setModelIndex(repairDecompressInfoData.getModelIndex().parent());

    itemParentUpdater->updateNzbItemsPostDecode(repairDecompressParentInfoData);

}



void ItemPostDownloadUpdater::addFileTypeInfo(const PostDownloadInfoData& decodeInfoData) {


    QStandardItem* fileNameItem = this->downloadModel->itemFromIndex(decodeInfoData.getModelIndex());
    QString decodedFileName = decodeInfoData.getDecodedFileName();

    NzbFileData nzbFileData = fileNameItem->data(NzbFileDataRole).value<NzbFileData>();

    // set the name of the decoded file :
    if (!decodedFileName.isEmpty()) {

        nzbFileData.setDecodedFileName(decodedFileName);

        // add info about type of file (par2 or rar file) :
        if ( !nzbFileData.isPar2File() &&
             !nzbFileData.isArchiveFile() ) {

            QFile decodedFile(Utility::buildFullPath(nzbFileData.getFileSavePath(), nzbFileData.getDecodedFileName()));
            if (decodedFile.exists()) {

                decodedFile.open(QIODevice::ReadOnly);

                // rar headers could be corrupted because repair process has not been proceeded yet at this stage
                // but assume that at least one rar file will have a correct header to launch decompress process later :
                if (decodedFile.peek(rarFileMagicNumber.size()) == rarFileMagicNumber) {
                    nzbFileData.setArchiveFormat(RarFormat);
                }

                // check if it is a zip file :
                else if (decodedFile.peek(zipFileMagicNumber.size()) == zipFileMagicNumber) {
                    QString fileExtension = QFileInfo(decodedFile).suffix().toLower();

                    // set a filter to file name extension in order to not extract zipped files
                    // that are not intended to be extracted as .jar, .war, ... files :
                    if ( (fileExtension == UtilityNamespace::zipFileExt) ||
                         (fileExtension == UtilityNamespace::sevenZipFileExt) ||
                         FileOperations::isSplitFileFormat(decodedFile) ) {

                        nzbFileData.setArchiveFormat(ZipOrSevenZipFormat);
                    }

                }                

                // check if it is a 7z file :
                else if (decodedFile.peek(sevenZipFileMagicNumber.size()) == sevenZipFileMagicNumber) {

                    nzbFileData.setArchiveFormat(ZipOrSevenZipFormat);
                }


                // check if it is a par2 file :
                else if ( (decodedFile.peek(par2FileMagicNumber.size()) == par2FileMagicNumber) ||
                          decodedFileName.endsWith(par2FileExt, Qt::CaseInsensitive)) {

                    nzbFileData.setPar2File(true);
                }

                // check if it is a splitted file :
                else if (FileOperations::isSplitFileFormat(decodedFile)) {
                    nzbFileData.setArchiveFormat(SplitFileFormat);

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

        // indicate if crc32 of the archive is ok :
        if (!decodeInfoData.isCrc32Match()) {
            itemStatusData.setCrc32Match(CrcKo);
        }

        // indicate type of article encoding :
        itemStatusData.setArticleEncodingType(decodeInfoData.getArticleEncodingType());


        QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(fileNameItem->index());
        this->downloadModel->storeStatusDataToItem(stateItem, itemStatusData);

    }

}
