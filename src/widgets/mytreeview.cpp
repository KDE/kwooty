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

#include "mytreeview.h"

#include "kwooty_debug.h"
#include <QMenu>
#include <KActionCollection>
#include <KLocalizedString>
#include <KIconLoader>
#include <QMimeData>
#include "itemdelegate.h"
#include "mainwindow.h"
#include "core.h"
#include "fileoperations.h"
#include "widgets/centralwidget.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "actions/actionbuttonsmanager.h"
#include "actions/actionsmanager.h"
#include "kwootysettings.h"

MyTreeView::MyTreeView(MainWindow *_mainWindow) : QTreeView(_mainWindow->getCentralWidget())
{

    mainWindow = _mainWindow;

    // delegate for item rendering / displaying :
    setItemDelegate(new ItemDelegate(this));

    // Avoid rows editing:
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Allowing mutiple row selection :
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setUniformRowHeights(false);
    setAllColumnsShowFocus(true);
    setAnimated(Settings::animateTreeView());
    setAcceptDrops(true);

    // retrieve setting about short or normal display for file names :
    displayTinyFileName = Settings::displayTinyFileName();

}

void MyTreeView::achieveInit()
{

    setModel(getDownloadModel());

    // add the labels to the header :
    setHeaderLabels();

    // setup connections :
    setupConnections();

}

StandardItemModel *MyTreeView::getDownloadModel()
{
    return getCore()->getDownloadModel();
}

Core *MyTreeView::getCore()
{
    return mainWindow->getCore();
}

void MyTreeView::setupConnections()
{

    // enable or disable buttons according to selected items :
    connect(selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            getCore()->getActionsManager()->getActionButtonsManager(),
            SLOT(selectedItemSlot()));

    // settings have changed :
    connect(getCore(),
            SIGNAL(settingsChangedSignal()),
            this,
            SLOT(settingsChangedSlot()));

    // treeview has been expanded :
    connect(this,
            SIGNAL(expanded(QModelIndex)),
            this,
            SLOT(expandedSlot(QModelIndex)));

}

void MyTreeView::contextMenuEvent(QContextMenuEvent *event)
{

    QMenu contextMenu(this);
    KActionCollection *actionCollection = mainWindow->actionCollection();

    // search for pause parents :
    if (getCore()->getModelQuery()->searchParentItemPause()) {
        contextMenu.addAction(actionCollection->action("startAll"));
    }

    // search for downloading parents :
    if (getCore()->getModelQuery()->searchParentItemDownloadOrPausing()) {
        contextMenu.addAction(actionCollection->action("pauseAll"));
    }

    // get item under mouse :
    QStandardItem *item = getDownloadModel()->itemFromIndex(indexAt(event->pos()));

    // add actions if right-clicked an item :
    if (item) {

        // get item status :
        UtilityNamespace::ItemStatus currentStatus = getDownloadModel()->getStatusDataFromIndex(item->index()).getStatus();

        // if item is in pause :
        if (Utility::isPaused(currentStatus)) {
            contextMenu.addAction(actionCollection->action("start"));
        }
        // if item is in download process :
        else if (Utility::isReadyToDownload(currentStatus)) {
            contextMenu.addAction(actionCollection->action("pause"));
        }

        if (!contextMenu.isEmpty()) {
            contextMenu.addSeparator();
        }

        // allow manual extract process from menu if auto post process disabled :
        if (!Settings::groupBoxAutoDecompress() ||
                !Settings::groupBoxAutoRepair()) {
            contextMenu.addAction(actionCollection->action("manualExtract"));
        }

    }

    contextMenu.addAction(actionCollection->action("retryDownload"));
    contextMenu.addSeparator();
    contextMenu.addAction(actionCollection->action("remove"));
    contextMenu.addAction(actionCollection->action("removeItemDeleteFile"));
    contextMenu.addSeparator();
    contextMenu.addAction(actionCollection->action("moveTop"));
    contextMenu.addAction(actionCollection->action("moveUp"));
    contextMenu.addAction(actionCollection->action("moveDown"));
    contextMenu.addAction(actionCollection->action("moveBottom"));
    contextMenu.addSeparator();
    contextMenu.addAction(actionCollection->action("mergeNzb"));
    contextMenu.addAction(actionCollection->action("renameNzb"));

    emit addExternalActionSignal(&contextMenu, item);

    // if menu is not empty display it :
    if (!contextMenu.actions().isEmpty()) {
        contextMenu.exec(event->globalPos());
    }

}

void MyTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MyTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MyTreeView::dropEvent(QDropEvent *event)
{

    // add drag and drop support :
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {

        // get urls of dropped files :
        QList<QUrl> urlList = mimeData->urls();

        foreach (const KUrl &nzbUrl, urlList) {

            // filter by .nzb extension :
            if (nzbUrl.url().endsWith(".nzb", Qt::CaseInsensitive)) {

                // handle nzb file from drag and drop event :
                getCore()->getFileOperations()->openFileWithFileMode(nzbUrl, UtilityNamespace::OpenNormal);

            }

        }

    }
    event->acceptProposedAction();
}

void MyTreeView::setHeaderLabels()
{

    QStringList headerLabels;
    headerLabels.append(i18n("File Name"));
    headerLabels.append(i18n("Status"));
    headerLabels.append(i18n("Progress"));
    headerLabels.append(i18n("Size"));
    getDownloadModel()->setHorizontalHeaderLabels(headerLabels);
}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void MyTreeView::expandedSlot(const QModelIndex &index)
{

    int maxFileNameSize = 0;
    int maxSizeTextSize = 0;
    int offset = 0;

    // retrieve maximum child fileName and fileSize text size :
    QStandardItem *parentItem = getDownloadModel()->itemFromIndex(index);

    for (int i = 0; i < parentItem->rowCount(); ++i) {

        // get corresponding file name index :
        QModelIndex fileNameIndex = index.child(i, FILE_NAME_COLUMN);
        QString fileNameText = getDownloadModel()->itemFromIndex(fileNameIndex)->text();
        int currentfileNameTextSize = fontMetrics().width(fileNameText);

        if (currentfileNameTextSize > maxFileNameSize) {
            maxFileNameSize = currentfileNameTextSize;
        }

        // get corresponding file size index :
        QModelIndex sizeIndex = getDownloadModel()->getSizeItemFromIndex(fileNameIndex)->index();
        int sizeTextSize = fontMetrics().width(Utility::convertByteHumanReadable(sizeIndex.data(SizeRole).toULongLong()));

        if (sizeTextSize > maxSizeTextSize) {
            maxSizeTextSize = sizeTextSize;
        }

        // compute child offset position :
        offset = visualRect(fileNameIndex).left() + 2 * KIconLoader::SizeSmall;
    }

    // compute available space :
    int availableSpace = width() -
                         columnWidth(PROGRESS_COLUMN) -
                         columnWidth(STATE_COLUMN) -
                         maxSizeTextSize -
                         offset;

    // add offset (child branch + icon widths) to fileName size :
    maxFileNameSize += offset;

    // adjust column width according to free space available :
    if (maxFileNameSize < availableSpace) {

        if (maxFileNameSize > columnWidth(FILE_NAME_COLUMN))  {
            setColumnWidth(FILE_NAME_COLUMN, maxFileNameSize);
        }
    } else if (availableSpace > columnWidth(FILE_NAME_COLUMN)) {
        setColumnWidth(FILE_NAME_COLUMN, availableSpace);
    }

}

void MyTreeView::displayLongOrTinyFileName()
{

    if (displayTinyFileName != Settings::displayTinyFileName()) {

        StandardItemModel *downloadModel = getDownloadModel();
        // retrieve parent nzb :
        QStandardItem *rootItem = downloadModel->invisibleRootItem();

        for (int i = 0; i < downloadModel->invisibleRootItem()->rowCount(); ++i) {

            QStandardItem *nzbItem = rootItem->child(i);

            // retrieve nzb children :
            for (int j = 0; j < nzbItem->rowCount(); j++) {

                // get corresponding file name index :
                QStandardItem *fileNameItem = nzbItem->child(j, FILE_NAME_COLUMN);
                NzbFileData nzbFileData = downloadModel->getNzbFileDataFromIndex(fileNameItem->index());

                // display tiny or long file name according to settings :
                fileNameItem->setText(getDisplayedFileName(nzbFileData));

            }
        }

        displayTinyFileName = Settings::displayTinyFileName();

    }

}

QString MyTreeView::getDisplayedFileName(const NzbFileData &currentNzbFileData) const
{

    QString fileName;

    if (Settings::displayTinyFileName()) {
        fileName = currentNzbFileData.getReducedFileName();

    } else {
        fileName = currentNzbFileData.getFileName();
    }

    return fileName;

}

void MyTreeView::settingsChangedSlot()
{

    // change UI related settings :
    setAnimated(Settings::animateTreeView());
    setAlternatingRowColors(Settings::alternateColors());
    displayLongOrTinyFileName();
}

