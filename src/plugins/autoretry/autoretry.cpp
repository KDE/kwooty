/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
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

#include "autoretry.h"

#include "kwooty_debug.h"

#include "mainwindow.h"
#include "core.h"
#include "standarditemmodel.h"
#include "autoretryplugin.h"
#include "standarditemmodelquery.h"
#include "actions/actionsmanager.h"
#include "data/itemstatusdata.h"
#include "kwooty_autoretrysettings.h"

AutoRetry::AutoRetry(AutoRetryPlugin *parent)
    :  QObject(parent)
{

    mCore = parent->getMainWindow()->getCore();

    // init folder to watch :
    settingsChanged();

    // setup signals/slots connections :
    setupConnections();

}

AutoRetry::~AutoRetry()
{

}

void AutoRetry::setupConnections()
{

    connect(mCore->getDownloadModel(),
            &StandardItemModel::parentStatusItemChangedSignal,
            this,
            &AutoRetry::parentStatusItemChangedSlot);

    connect(mCore->getDownloadModel(),
            &StandardItemModel::childStatusItemChangedSignal,
            this,
            &AutoRetry::childStatusItemChangedSlot);

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void AutoRetry::parentStatusItemChangedSlot(QStandardItem *stateItem)
{

    StandardItemModel *downloadModel = mCore->getDownloadModel();
    ItemStatusData itemStatusData = downloadModel->getStatusDataFromIndex(stateItem->index());

    //qCDebug(KWOOTY_LOG) <<  itemStatusData.isPostProcessFinish() << itemStatusData.getStatus() <<  itemStatusData.getDataStatus() ;

    if (itemStatusData.getDownloadRetryCounter() <= AutoRetrySettings::retryRepairFail()) {

        // avoid double parentStatusItemChangedSlot call by checking
        // if status has just not be set to be ready for downloading again :
        ItemStatus itemStatus = itemStatusData.getStatus();

        if (itemStatus == VerifyFinishedStatus ||
                itemStatus == ExtractFinishedStatus) {

            if (itemStatusData.isPostProcessFinish() &&
                    !itemStatusData.areAllPostProcessingCorrect()) {

                qDebug() << "post process finished, retry counter :" << itemStatusData.getDownloadRetryCounter();

                retryDownload(stateItem);

            }

        }

    }

}

void AutoRetry::childStatusItemChangedSlot(QStandardItem *stateItem)
{

    StandardItemModel *downloadModel = mCore->getDownloadModel();
    ItemStatusData itemStatusData = downloadModel->getStatusDataFromIndex(stateItem->index());

    if (itemStatusData.getDownloadRetryCounter() <= AutoRetrySettings::retryNoPar2Files()) {

        if (Utility::isDecodeFinish(itemStatusData.getStatus()) &&
                itemStatusData.getCrc32Match() != CrcOk) {

            // if nzb file does not contain any par2 files, reset in queue corrupted decoded file :
            if (!mCore->getModelQuery()->isParentContainsPar2File(stateItem)) {

                qDebug() << "Decode Finished No par2 files - retry!";

                // select all rows in order to set them to paused or Idle :
                retryDownload(stateItem);

            }

        }

        // no data found, try to download files again :
        else if (Utility::isFileNotFound(itemStatusData.getStatus(), itemStatusData.getDataStatus())) {

            // if nzb file does not contain any par2 files, reset in queue corrupted decoded file :
            if (!mCore->getModelQuery()->isParentContainsPar2File(stateItem)) {

                qDebug() << "Decode Finished No par2 files - retry!" << itemStatusData.getDownloadRetryCounter();

                // select all rows in order to set them to paused or Idle :
                retryDownload(stateItem);

            }
        }

    }

}

void AutoRetry::retryDownload(QStandardItem *stateItem)
{

    QList<QModelIndex> indexesList;
    indexesList.append(stateItem->index());

    mCore->getActionsManager()->retryDownload(indexesList);

}

void AutoRetry::settingsChanged()
{

    // reload settings from just saved config file :
    AutoRetrySettings::self()->load();

}

