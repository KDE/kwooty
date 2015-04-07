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


#include "kwooty_debug.h"
#include <KFileDialog>
#include <KAction>
#include <KActionCollection>

#include "categoriesmanual.h"
#include "core.h"
#include "mainwindow.h"
#include "standarditemmodel.h"
#include "widgets/centralwidget.h"
#include "widgets/mytreeview.h"
#include "kwooty_categoriessettings.h"



CategoriesManual::CategoriesManual(Categories* categories) : QObject(categories) {

    this->core = categories->getCore();
    this->downloadModel = this->core->getDownloadModel();
    this->treeView = this->core->getTreeView();

    // create manualTransferFolderAction :
    KAction* manualTransferFolderAction = new KAction(this);
    manualTransferFolderAction->setText(i18n("Transfer folder..."));
    manualTransferFolderAction->setIcon(KIcon("folder-favorites"));
    manualTransferFolderAction->setToolTip(i18n("Select transfer folder"));
    manualTransferFolderAction->setShortcut(Qt::CTRL + Qt::Key_F);
    manualTransferFolderAction->setEnabled(true);
    manualTransferFolderAction->setCheckable(false);

    // add current action to actionCollection :
    this->core->getMainWindow()->actionCollection()->addAction("chooseFavoriteFolder", manualTransferFolderAction);

    this->setupConnections();

}


void CategoriesManual::setupConnections() {

    // add action in context menu when requested :
    connect (this->treeView,
             SIGNAL(addExternalActionSignal(KMenu*, QStandardItem*)),
             this,
             SLOT(addExternalActionSlot(KMenu*, QStandardItem*)));

    // action has been triggered by user :
    KActionCollection* actionCollection = this->core->getMainWindow()->actionCollection();
    connect(actionCollection->action("chooseFavoriteFolder"),
            SIGNAL(triggered(bool)),
            this,
            SLOT(manualTransferFolderSlot()));

}


void CategoriesManual::unload() {

    // remove previous tooltips to nzb fileName column to notify user about the selected folder :
    QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

    for (int i = 0; i < rootItem->rowCount(); i++) {
        this->updateNzbFileNameToolTip(rootItem->child(i));
    }

    // remove action from context menu :
    KActionCollection* actionCollection = this->core->getMainWindow()->actionCollection();
    actionCollection->removeAction(actionCollection->action("chooseFavoriteFolder"));

}



bool CategoriesManual::isManualFolderSelected(const QString& currentUuidItem) {

    if (CategoriesSettings::manualFolder()) {
        return this->uuidFolderMap.contains(currentUuidItem);
    }
    else {
        return false;
    }

}


bool CategoriesManual::isActionAllowed(QStandardItem* item) const {

    ItemStatusData itemStatusData = this->downloadModel->getStatusDataFromIndex(item->index());

    // if post process is not finished yet, allow to manually select transfert folder :
    return !itemStatusData.isPostProcessFinish();

}


QString CategoriesManual::getMoveFolderPath(const QString& currentUuidItem) {

    return this->uuidFolderMap.take(currentUuidItem);

}

void CategoriesManual::updateNzbFileNameToolTip(QStandardItem* nzbItem, const QString& directory) {

    if (directory.isEmpty()) {
        nzbItem->setToolTip(directory);
    }
    else {
        nzbItem->setToolTip(i18n("Transfer folder: %1", directory));
    }

}



//============================================================================================================//
//                                               SLOTS                                                        //
//===========================================================================================================


void CategoriesManual::addExternalActionSlot(KMenu* contextMenu, QStandardItem* item) {

    // FIXME: not yet the case but if several plugins need to insert actions in contextMenu,
    // think about displaying them always in the same order.

    // if item is a nzb item :
    if ( item &&
         CategoriesSettings::manualFolder() &&
         this->downloadModel->isNzbItem(item) ) {

        // if post process is not finished yet, allow to manually select transfert folder :
        if (this->isActionAllowed(item)) {

            contextMenu->addSeparator();
            contextMenu->addAction(this->core->getMainWindow()->actionCollection()->action("chooseFavoriteFolder"));

        }

    }

}


void CategoriesManual::manualTransferFolderSlot() {

    if (CategoriesSettings::manualFolder()) {

        // get selected indexes :
        QList<QModelIndex> indexesList = this->treeView->selectionModel()->selectedRows();

        // one row should be selected when context menu action appears :
        if (!indexesList.isEmpty()) {

            QStandardItem* item = this->downloadModel->getFileNameItemFromIndex(indexesList.at(0));

            // first, be sure that item is a parent one (nzb) :
            if ( this->downloadModel->isNzbItem(item) &&
                 this->isActionAllowed(item) ) {

                QString uuidIndex = this->downloadModel->getUuidStrFromIndex(item->index());

                // get current file nzb save path :
                QString startDirectory = this->downloadModel->getParentFileSavePathFromIndex(item->index());

                // check if transfer folder has already been manually choosen :
                QString storedDirectory = this->uuidFolderMap.value(uuidIndex);

                if (!storedDirectory.isEmpty()) {
                    startDirectory = storedDirectory;
                }


                QString directory = KFileDialog::getExistingDirectory ( KUrl(startDirectory),
                                                                        static_cast<QWidget*>(this->core->getCentralWidget()),
                                                                        i18n("Select transfer folder") );
                // target folder has been selected :
                if (!directory.isEmpty()) {

                    // get the root item :
                    QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

                    // remove old items currently stored in map :
                    for (int i = 0; i < rootItem->rowCount(); i++) {

                        QModelIndex nzbFileNameIndex = rootItem->child(i)->index();

                        ItemStatusData itemStatusData = this->downloadModel->getStatusDataFromIndex(nzbFileNameIndex);

                        if (itemStatusData.isPostProcessFinish()) {

                            QString currentUuidIndex = this->downloadModel->getUuidStrFromIndex(nzbFileNameIndex);

                            // remove from map :
                            this->uuidFolderMap.remove(currentUuidIndex);

                        }

                    }

                    // add new item :
                    this->uuidFolderMap.insert(uuidIndex, directory);

                    // add tooltip to nzb fileName column to notify user about the selected folder :
                    this->updateNzbFileNameToolTip(item, directory);

                }

            }

        }

    }

}

