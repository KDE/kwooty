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

#include <KDebug>
#include <KMenu>
#include <KActionCollection>


#include "itemdelegate.h"
#include "mainwindow.h"
#include "core.h"
#include "fileoperations.h"
#include "widgets/centralwidget.h"
#include "actionsmanager.h"
#include "actionbuttonsmanager.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "kwootysettings.h"




MyTreeView::MyTreeView(MainWindow* mainWindow) : QTreeView(mainWindow->getCentralWidget()) {

    this->mainWindow = mainWindow;

    // delegate for item rendering / displaying :
    this->setItemDelegate(new ItemDelegate(this));

    // Avoid rows editing:
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Allowing mutiple row selection :
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setUniformRowHeights(false);
    this->setAllColumnsShowFocus(true);
    this->setAnimated(Settings::animateTreeView());
    this->setAcceptDrops(true);


}

void MyTreeView::achieveInit() {

    this->setModel(this->getDownloadModel());

    // add the labels to the header :
    this->setHeaderLabels();

    // setup connections :
    this->setupConnections();

}



StandardItemModel* MyTreeView::getDownloadModel() {
    return this->getCore()->getDownloadModel();
}

Core* MyTreeView::getCore() {
    return this->mainWindow->getCore();
}


void MyTreeView::setupConnections() {

    // enable or disable buttons according to selected items :
    connect (this->selectionModel(),
             SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
             this->getCore()->getActionsManager()->getActionButtonsManager(),
             SLOT(selectedItemSlot()));


    // settings have changed :
    connect (this->getCore(),
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));


    // treeview has been expanded :
    connect (this,
             SIGNAL(expanded(const QModelIndex&)),
             this,
             SLOT(expandedSlot(const QModelIndex&)));


#if QT_VERSION == 0x040503
    // fixes #QTBUG-5201
    connect(this->getDownloadModel(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(dataChangedSlot(QStandardItem*)));
#endif


}


void MyTreeView::contextMenuEvent(QContextMenuEvent* event) {

    KMenu contextMenu(this);
    KActionCollection* actionCollection = this->mainWindow->actionCollection();

    QStandardItem* stateItem;

    // search for pause parents :
    stateItem = this->getCore()->getModelQuery()->searchParentItem(PauseStatus);
    if (stateItem) {
        contextMenu.addAction(actionCollection->action("startAll"));
    }

    // search for downloading parents :
    stateItem = this->getCore()->getModelQuery()->searchParentItem(DownloadStatus);
    if (stateItem) {
        contextMenu.addAction(actionCollection->action("pauseAll"));
    }

    // get item under mouse :
    QStandardItem* item = this->getDownloadModel()->itemFromIndex(this->indexAt(event->pos()));

    // add actions if right-clicked an item :
    if (item) {

        // get item status :
        UtilityNamespace::ItemStatus currentStatus = this->getDownloadModel()->getStatusDataFromIndex(item->index()).getStatus();

        // if item is in pause :
        if (Utility::isPaused(currentStatus)) {
            contextMenu.addAction(actionCollection->action("start"));
        }
        // if item is in download process :
        else if (Utility::isReadyToDownload(currentStatus)) {
            contextMenu.addAction(actionCollection->action("pause"));
        }

        contextMenu.addAction(actionCollection->action("retryDownload"));
        contextMenu.addSeparator();
        contextMenu.addAction(actionCollection->action("remove"));
        contextMenu.addSeparator();
        contextMenu.addAction(actionCollection->action("moveTop"));
        contextMenu.addAction(actionCollection->action("moveUp"));
        contextMenu.addAction(actionCollection->action("moveDown"));
        contextMenu.addAction(actionCollection->action("moveBottom"));
    }

    // if menu is not empty display it :
    if (!contextMenu.actions().isEmpty()) {
        contextMenu.exec(event->globalPos());
    }
}



void MyTreeView::dragEnterEvent(QDragEnterEvent* event) {
    event->acceptProposedAction();
}

void MyTreeView::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
}

void MyTreeView::dropEvent(QDropEvent* event) {

    // add drag and drop support :
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {

        // get urls of dropped files :
        QList<QUrl> urlList = mimeData->urls();

        foreach (KUrl nzbUrl, urlList) {

            // filter by .nzb extension :
            if (nzbUrl.url().endsWith(".nzb", Qt::CaseInsensitive)) {

                // handle nzb file from drag and drop event :
                this->getCore()->getFileOperations()->openFileWithFileMode(nzbUrl, UtilityNamespace::OpenNormal);

            }

        }

    }
    event->acceptProposedAction();
}




void MyTreeView::setHeaderLabels() {

    QStringList headerLabels;
    headerLabels.append(i18n("File Name"));
    headerLabels.append(i18n("Status"));
    headerLabels.append(i18n("Progress"));
    headerLabels.append(i18n("Size"));
    this->getDownloadModel()->setHorizontalHeaderLabels(headerLabels);
}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void MyTreeView::expandedSlot(const QModelIndex& index) {

    int maxFileNameSize = 0;
    int maxSizeTextSize = 0;
    int offset = 0;

    // retrieve maximum child fileName and fileSize text size :
    QStandardItem* parentItem = this->getDownloadModel()->itemFromIndex(index);

    for (int i = 0; i < parentItem->rowCount(); i++) {

        // get corresponding file name index :
        QModelIndex fileNameIndex = index.child(i, FILE_NAME_COLUMN);
        QString fileNameText = this->getDownloadModel()->itemFromIndex(fileNameIndex)->text();
        int currentfileNameTextSize = this->fontMetrics().width(fileNameText);


        if (currentfileNameTextSize > maxFileNameSize) {
            maxFileNameSize = currentfileNameTextSize;
        }


        // get corresponding file size index :
        QModelIndex sizeIndex = this->getDownloadModel()->getSizeItemFromIndex(fileNameIndex)->index();
        int sizeTextSize = this->fontMetrics().width(Utility::convertByteHumanReadable(sizeIndex.data(SizeRole).toULongLong()));

        if (sizeTextSize > maxSizeTextSize) {
            maxSizeTextSize = sizeTextSize;
        }

        // compute child offset position :
        offset = this->visualRect(fileNameIndex).left() + 2 * KIconLoader::SizeSmall;
    }



    // compute available space :
    int availableSpace = this->width() -
                         this->columnWidth(PROGRESS_COLUMN) -
                         this->columnWidth(STATE_COLUMN) -
                         maxSizeTextSize -
                         offset;

    // add offset (child branch + icon widths) to fileName size :
    maxFileNameSize += offset;

    // adjust column width according to free space available :
    if (maxFileNameSize < availableSpace) {

        if (maxFileNameSize > this->columnWidth(FILE_NAME_COLUMN))  {
            this->setColumnWidth(FILE_NAME_COLUMN, maxFileNameSize);
        }
    }
    else if (availableSpace > this->columnWidth(FILE_NAME_COLUMN)) {
        this->setColumnWidth(FILE_NAME_COLUMN, availableSpace);
    }


}




void MyTreeView::settingsChangedSlot() {

    // change UI related settings :
    this->setAnimated(Settings::animateTreeView());
    this->setAlternatingRowColors(Settings::alternateColors());
}



#if QT_VERSION == 0x040503
void MyTreeView::dataChangedSlot(QStandardItem* item){
    // items at row 0 are not updated in qt 4.5.3,
    // this fixes update issue :
    QModelIndex index = item->index();
    if (index.isValid()) {
        if (this->getDownloadModel()->isNzbItem(item) && (item->row() == 0) ) {
            const QRect rect = this->visualRect(index);
            if (this->viewport()->rect().intersects(rect)){
                this->viewport()->update(rect);
            }
        }
    }

}
#endif

