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

#include "itemabstractupdater.h"

#include "kwooty_debug.h"
#include "itemparentupdater.h"
#include "standarditemmodel.h"
#include "kwootysettings.h"
#include "utilities/utilityiconpainting.h"
#include "data/nzbfiledata.h"

ItemAbstractUpdater::ItemAbstractUpdater(StandardItemModel *downloadModel, ItemHierarchy itemHierarchy) : QObject(downloadModel)
{

    mDownloadModel = downloadModel;

    // setup connection in order to display proper icon near each file name according to its current status :
    // update icon near child file name item :
    if (itemHierarchy == ItemAbstractUpdater::Child) {

        connect(mDownloadModel,
                SIGNAL(childStatusItemChangedSignal(QStandardItem*,ItemStatusData)),
                this,
                SLOT(childStatusIconUpdateSlot(QStandardItem*,ItemStatusData)));

    }
    // update icon near parent file name item :
    else if (itemHierarchy == ItemAbstractUpdater::Parent) {

        connect(mDownloadModel,
                SIGNAL(parentStatusItemChangedSignal(QStandardItem*,ItemStatusData)),
                this,
                SLOT(parentStatusIconUpdateSlot(QStandardItem*,ItemStatusData)));

    }

}

ItemAbstractUpdater::ItemAbstractUpdater() { }

void ItemAbstractUpdater::clear()
{

    mDownloadItemNumber = 0;
    mPauseItemNumber = 0;
    mProgressNumber = 0;
    mDownloadFinishItemNumber = 0;
    mInQueueItemNumber = 0;
    mPausingItemNumber = 0;
    mDecodeFinishItemNumber = 0;
    mDecodeErrorItemNumber = 0;
    mArticleNotFoundNumber = 0;
    mArticleFoundNumber = 0;
    mVerifyItemNumber = 0;
    mVerifyFinishItemNumber = 0;
    mRepairItemNumber = 0;
    mPendingSegmentsOnBackupNumber = 0;

}

void ItemAbstractUpdater::countItemStatus(const int &status)
{

    switch (status) {

    case DownloadStatus: {
        mDownloadItemNumber++;
        break;
    };

    case DownloadFinishStatus: {
        mDownloadFinishItemNumber++;
        break;
    };
    case IdleStatus: {
        mInQueueItemNumber++;
        break;
    };
    case PauseStatus: {
        mPauseItemNumber++;
        break;
    };
    case PausingStatus: {
        mPausingItemNumber++;
        break;
    };
    case DecodeFinishStatus: {
        mDecodeFinishItemNumber++;
        break;
    };
    case DecodeErrorStatus: {
        mDecodeErrorItemNumber++;
        break;
    };
    case DecodeStatus: {
        mDecodeItemNumber++;
        break;
    };
    case ScanStatus: {
        mScanItemNumber++;
        break;
    };
    case WaitForPar2IdleStatus: {
        mDecodeFinishItemNumber++;
        break;
    };

    // /!\ during download process the status could be ExtractSuccessStatus when following condition occurs :
    // A multi nzb-set has been fully downloaded and par2 files were not needed (WaitForPar2IdleStatus)
    // then extract process if directly done : 1st nzb-set is correctly extracted
    // but extracting of 2nd nzb-set failed (due to a bad crc for eg.)
    // par2 files are then downloaded for repairing of 2nd nzb-set.
    // => consider previously extracted files from 1st nzb-set as decodeFinish files :
    case ExtractSuccessStatus: {
        mDecodeFinishItemNumber++;
        break;
    };

    }

}

void ItemAbstractUpdater::setIcon(QStandardItem *stateItem, const QString &iconName)
{

    QStandardItem *fileNameItem = mDownloadModel->getFileNameItemFromIndex(stateItem->index());

    QIcon icon;
    bool iconFound = UtilityIconPainting::getInstance()->retrieveIconFromString(iconName, icon);

    if (iconFound) {
        fileNameItem->setIcon(icon);
    }

}

void ItemAbstractUpdater::setIcon(QStandardItem *stateItem, const UtilityNamespace::ItemStatus &status)
{

    QStandardItem *fileNameItem = mDownloadModel->getFileNameItemFromIndex(stateItem->index());

    // if item is the parent item :
    if (mDownloadModel->isNzbItem(fileNameItem)) {

        // update its icon according to its current status :
        QIcon icon;
        bool iconFound = UtilityIconPainting::getInstance()->retrieveParentIconFromStatus(status, icon);

        if (iconFound) {
            fileNameItem->setIcon(icon);
        }

    }
    // else item is the child item :
    else {

        // update its icon according to its current status :
        QIcon icon;
        bool iconFound = UtilityIconPainting::getInstance()->retrieveChildIconFromStatus(status, icon);

        if (iconFound) {
            fileNameItem->setIcon(icon);
        }

    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ItemAbstractUpdater::parentStatusIconUpdateSlot(QStandardItem *stateItem, const ItemStatusData &itemStatusData)
{

    UtilityNamespace::ItemStatus status = itemStatusData.getStatus();

    // get final status :
    if (itemStatusData.isPostProcessFinish()) {

        if (itemStatusData.areAllPostProcessingCorrect()) {

            // get fileName item and set corresponding icon :
            setIcon(stateItem, status);
        }

        else {

            // get fileName item and set corresponding icon :
            setIcon(stateItem, "dialog-warning");

        }

    }

    else if (Utility::isInQueue(status)) {
        // get fileName item and set corresponding icon :
        setIcon(stateItem, "go-next-view-transparent");
    }

    // get fileName item and set corresponding icon :
    else if (Utility::isDownloadOrPausing(status)) {
        setIcon(stateItem, status);
    }

    else if (Utility::isDownloadFinish(status)) {

        if (itemStatusData.getDataStatus() == NoData) {
            setIcon(stateItem, "dialog-cancel");
        }
    }

}

void ItemAbstractUpdater::childStatusIconUpdateSlot(QStandardItem *stateItem, const ItemStatusData &itemStatusData)
{

    UtilityNamespace::ItemStatus status = itemStatusData.getStatus();

    if (Utility::isDownloadFinish(status)) {

        // get final status :
        if (itemStatusData.getDataStatus() == NoData) {
            // in this case the status is set to DecodeErrorStatus only to display the proper icon :
            status = DecodeErrorStatus;
        }
    }

    // set a special icon if segment is still pending and has not been found on master server :
    else if (Utility::isInQueue(status)) {

        // get final status :
        if (itemStatusData.getDataStatus() == DataPendingBackupServer) {
            setIcon(stateItem, "mail-reply-list");
            return;
        }

    } else if (Utility::isDecodeFinish(status)) {
        // get final status :

        if (Utility::isBadCrcForYencArticle(itemStatusData.getCrc32Match(), itemStatusData.getArticleEncodingType())) {
            setIcon(stateItem, "mail-mark-important");
            return;

        }

    }

    // get fileName item and set corresponding icon :
    setIcon(stateItem, status);

}

