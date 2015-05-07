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
#include <QAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <QIcon>
#include "categoriesmanual.h"
#include "core.h"
#include "mainwindow.h"
#include "standarditemmodel.h"
#include "widgets/centralwidget.h"
#include "widgets/mytreeview.h"
#include "kwooty_categoriessettings.h"

CategoriesManual::CategoriesManual(Categories *categories) : QObject(categories)
{

    mCore = categories->getCore();
    mDownloadModel = mCore->getDownloadModel();
    mTreeView = mCore->getTreeView();

    // create manualTransferFolderAction :
    QAction *manualTransferFolderAction = new QAction(this);
    manualTransferFolderAction->setText(i18n("Transfer folder..."));
    manualTransferFolderAction->setIcon(QIcon::fromTheme("folder-favorites"));
    manualTransferFolderAction->setToolTip(i18n("Select transfer folder"));
    manualTransferFolderAction->setShortcut(Qt::CTRL + Qt::Key_F);
    manualTransferFolderAction->setEnabled(true);
    manualTransferFolderAction->setCheckable(false);

    // add current action to actionCollection :
    mCore->getMainWindow()->actionCollection()->addAction("chooseFavoriteFolder", manualTransferFolderAction);

    setupConnections();

}

void CategoriesManual::setupConnections()
{

    // add action in context menu when requested :
    connect(mTreeView,
            &MyTreeView::addExternalActionSignal,
            this,
            &CategoriesManual::addExternalActionSlot);

    // action has been triggered by user :
    KActionCollection *actionCollection = mCore->getMainWindow()->actionCollection();
    connect(actionCollection->action("chooseFavoriteFolder"),
            SIGNAL(triggered(bool)),
            this,
            SLOT(manualTransferFolderSlot()));

}

void CategoriesManual::unload()
{

    // remove previous tooltips to nzb fileName column to notify user about the selected folder :
    QStandardItem *rootItem = mDownloadModel->invisibleRootItem();

    for (int i = 0; i < rootItem->rowCount(); ++i) {
        updateNzbFileNameToolTip(rootItem->child(i));
    }

    // remove action from context menu :
    KActionCollection *actionCollection = mCore->getMainWindow()->actionCollection();
    actionCollection->removeAction(actionCollection->action("chooseFavoriteFolder"));

}

bool CategoriesManual::isManualFolderSelected(const QString &currentUuidItem)
{

    if (CategoriesSettings::manualFolder()) {
        return mUuidFolderMap.contains(currentUuidItem);
    } else {
        return false;
    }

}

bool CategoriesManual::isActionAllowed(QStandardItem *item) const
{

    ItemStatusData itemStatusData = mDownloadModel->getStatusDataFromIndex(item->index());

    // if post process is not finished yet, allow to manually select transfert folder :
    return !itemStatusData.isPostProcessFinish();

}

QString CategoriesManual::getMoveFolderPath(const QString &currentUuidItem)
{

    return mUuidFolderMap.take(currentUuidItem);

}

void CategoriesManual::updateNzbFileNameToolTip(QStandardItem *nzbItem, const QString &directory)
{

    if (directory.isEmpty()) {
        nzbItem->setToolTip(directory);
    } else {
        nzbItem->setToolTip(i18n("Transfer folder: %1", directory));
    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//===========================================================================================================

void CategoriesManual::addExternalActionSlot(QMenu *contextMenu, QStandardItem *item)
{

    // FIXME: not yet the case but if several plugins need to insert actions in contextMenu,
    // think about displaying them always in the same order.

    // if item is a nzb item :
    if (item &&
            CategoriesSettings::manualFolder() &&
            mDownloadModel->isNzbItem(item)) {

        // if post process is not finished yet, allow to manually select transfert folder :
        if (isActionAllowed(item)) {

            contextMenu->addSeparator();
            contextMenu->addAction(mCore->getMainWindow()->actionCollection()->action("chooseFavoriteFolder"));

        }

    }

}

void CategoriesManual::manualTransferFolderSlot()
{

    if (CategoriesSettings::manualFolder()) {

        // get selected indexes :
        QList<QModelIndex> indexesList = mTreeView->selectionModel()->selectedRows();

        // one row should be selected when context menu action appears :
        if (!indexesList.isEmpty()) {

            QStandardItem *item = mDownloadModel->getFileNameItemFromIndex(indexesList.at(0));

            // first, be sure that item is a parent one (nzb) :
            if (mDownloadModel->isNzbItem(item) &&
                    isActionAllowed(item)) {

                QString uuidIndex = mDownloadModel->getUuidStrFromIndex(item->index());

                // get current file nzb save path :
                QString startDirectory = mDownloadModel->getParentFileSavePathFromIndex(item->index());

                // check if transfer folder has already been manually choosen :
                QString storedDirectory = mUuidFolderMap.value(uuidIndex);

                if (!storedDirectory.isEmpty()) {
                    startDirectory = storedDirectory;
                }

                QString directory = KFileDialog::getExistingDirectory(QUrl::fromLocalFile(startDirectory),
                                    static_cast<QWidget *>(mCore->getCentralWidget()),
                                    i18n("Select transfer folder"));
                // target folder has been selected :
                if (!directory.isEmpty()) {

                    // get the root item :
                    QStandardItem *rootItem = mDownloadModel->invisibleRootItem();

                    // remove old items currently stored in map :
                    for (int i = 0; i < rootItem->rowCount(); ++i) {

                        QModelIndex nzbFileNameIndex = rootItem->child(i)->index();

                        ItemStatusData itemStatusData = mDownloadModel->getStatusDataFromIndex(nzbFileNameIndex);

                        if (itemStatusData.isPostProcessFinish()) {

                            QString currentUuidIndex = mDownloadModel->getUuidStrFromIndex(nzbFileNameIndex);

                            // remove from map :
                            mUuidFolderMap.remove(currentUuidIndex);

                        }

                    }

                    // add new item :
                    mUuidFolderMap.insert(uuidIndex, directory);

                    // add tooltip to nzb fileName column to notify user about the selected folder :
                    updateNzbFileNameToolTip(item, directory);

                }

            }

        }

    }

}

