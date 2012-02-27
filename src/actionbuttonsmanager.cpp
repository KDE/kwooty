/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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


#include "actionbuttonsmanager.h"

#include "core.h"
#include "mainwindow.h"
#include "actionsmanager.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "widgets/mytreeview.h"


ActionButtonsManager::ActionButtonsManager(ActionsManager* actionsManager, Core* core) : QObject(actionsManager) {

    this->actionsManager = actionsManager;
    this->core = core;
    this->treeView = this->core->getTreeView();
    this->downloadModel = this->core->getDownloadModel();
    this->downloadModelQuery = this->core->getModelQuery();

}


void ActionButtonsManager::selectedItemSlot() {

    bool sameParents = true;
    bool enableStartButton = false;
    bool enablePauseButton = false;
    bool enableRetryButton = false;

    // get selected items :
    QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();

    // if no item selected :
    if (indexesList.isEmpty()) {
        emit setMoveButtonEnabledSignal(false);
    }

    else {
        sameParents = this->downloadModelQuery->haveItemsSameParent(indexesList);
        emit setMoveButtonEnabledSignal(sameParents);
    }


    // enable/disable start-pause buttons :
    if (!sameParents) {
        emit setPauseButtonEnabledSignal(false);
        emit setStartButtonEnabledSignal(false);
        emit setRetryButtonEnabledSignal(false);
    }
    else {

        for (int i = 0; i < indexesList.size(); i++) {

            QModelIndex currentModelIndex = indexesList.at(i);
            QStandardItem* stateItem = this->downloadModel->getStateItemFromIndex(currentModelIndex);
            UtilityNamespace::ItemStatus currentStatus = this->downloadModel->getStatusFromStateItem(stateItem);

            // enable start button if selected item is paused/pausing
            if (!enableStartButton) {
                enableStartButton = ( Utility::isPaused(currentStatus) || Utility::isPausing(currentStatus) );
            }

            // enable pause button if selected item is idle/download
            if (!enablePauseButton) {
                enablePauseButton = Utility::isReadyToDownload(currentStatus);
            }

            // disable remove button when download has been accomplished :
            if ( !this->downloadModel->isNzbItem(stateItem) &&
                 !Utility::isInDownloadProcess(currentStatus) &&
                 currentStatus != UtilityNamespace::WaitForPar2IdleStatus ) {

                emit setRemoveButtonEnabledSignal(false);

            }


            // enable retry button if current item is parent :
            if (this->downloadModel->isNzbItem(stateItem)) {

                QStandardItem* fileNameItem = this->downloadModel->getFileNameItemFromIndex(currentModelIndex);

                for (int i = 0; i < fileNameItem->rowCount(); i++) {

                    if (!enableRetryButton) {
                        QStandardItem* nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                        this->downloadModelQuery->isRetryDownloadAllowed(nzbChildrenItem, &enableRetryButton);
                    }
                }
            }
            // else enable retry button if current item is child :
            else if (!enableRetryButton){
                this->downloadModelQuery->isRetryDownloadAllowed(stateItem, &enableRetryButton);

            }


        } // end of loop

        // disable both buttons if paused and downloading items are both selected :
        if (enableStartButton && enablePauseButton) {
            emit setPauseButtonEnabledSignal(false);
            emit setStartButtonEnabledSignal(false);
        } else {
            emit setPauseButtonEnabledSignal(enablePauseButton);
            emit setStartButtonEnabledSignal(enableStartButton);
        }


        emit setRetryButtonEnabledSignal(enableRetryButton);

    }
}

