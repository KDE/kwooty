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

#include <KMessageBox>
#include <KRun>
#include <KDebug>

#include "itemdelegate.h"
#include "centralwidget.h"
#include "fileoperations.h"
#include "standarditemmodel.h"
#include "kwootysettings.h"



MyTreeView::MyTreeView(CentralWidget* centralWidget) : QTreeView(centralWidget) {

    this->centralWidget = centralWidget;
    this->downloadModel = centralWidget->getDownloadModel();

    this->setModel(downloadModel);

    // delegate for item rendering / displaying :
    this->setItemDelegate(new ItemDelegate(this));

    // Avoid rows editing:
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Allowing mutiple row selection :
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setUniformRowHeights(true);
    this->setAllColumnsShowFocus(true);
    this->setAnimated(Settings::animateTreeView());
    this->setAcceptDrops(true);

    // add the labels to the header :
    this->setHeaderLabels();

    // setup connections :
    this->setupConnections();

}


void MyTreeView::setupConnections() {

    connect( this,
             SIGNAL(statusBarFileSizeUpdateSignal(StatusBarUpdateType)),
             centralWidget,
             SLOT(statusBarFileSizeUpdateSlot(StatusBarUpdateType)) );

    // propagate signal to centralwidget to finally reach itemparentupdater's slot :
    connect( this,
             SIGNAL(recalculateNzbSizeSignal(const QModelIndex)),
             centralWidget,
             SIGNAL(recalculateNzbSizeSignal(const QModelIndex)) );

    // propagate signal to centralwidget to finally reach itemparentupdater's slot :
    connect( this,
             SIGNAL(changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus)),
             centralWidget,
             SIGNAL(changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus)) );


    // enable or disable buttons according to selected items :
    connect (this->selectionModel(),
             SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
             this,
             SLOT(selectedItemSlot()));


    // settings have changed :
    connect (centralWidget,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));


#if QT_VERSION == 0x040503
    // fixes #QTBUG-5201
    connect(downloadModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(dataChangedSlot(QStandardItem*)));
#endif


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
                this->centralWidget->getFileOperations()->openFileWithFileMode(nzbUrl, UtilityNamespace::OpenNormal);

            }

        }

    }
    event->acceptProposedAction();
}



void MyTreeView::moveRow(MyTreeView::MoveRowType moveRowType) {

    // get selected indexes :
    QList<QModelIndex> indexesList = this->selectionModel()->selectedRows();     

    // get parent item :
    QStandardItem* parentItem = NULL;
    if (!indexesList.isEmpty()) {
        parentItem = downloadModel->getParentItem(indexesList.at(0));
    }

    // sort indexes by decremental order
    qSort(indexesList.begin(), indexesList.end(), qGreater<QModelIndex>());


    // remove selected indexes from model
    QMap< int, QList<QStandardItem*> > itemRowsMap;

    foreach (QModelIndex index, indexesList) {

        int rowNumber = index.row();
        QList<QStandardItem*> rowItems = parentItem->takeRow(rowNumber);

        itemRowsMap.insert(rowNumber, rowItems);
    }


    QList<int> rowNumberList = itemRowsMap.keys();

    // sort indexes by incremental order
    qSort(rowNumberList);
    QList<int> updatedRowNumberList;

    // then replace removed indexes to the proper position :
    foreach (int currentRow , rowNumberList) {

        QList<QStandardItem*> itemRows = itemRowsMap.value(currentRow);

        int updatedRowNumber;

        if (moveRowType == MoveRowsUp) {
            updatedRowNumber = currentRow - 1;
        }

        if (moveRowType == MoveRowsDown) {
            updatedRowNumber = currentRow + 1;
        }

        if (moveRowType == MoveRowsTop) {
            updatedRowNumber = 0;
        }

        if (moveRowType == MoveRowsBottom) {
            updatedRowNumber = parentItem->rowCount();
        }

        // control out of range and multiple row selection :
        if ( (updatedRowNumber < 0) ||
             (updatedRowNumber > parentItem->rowCount()) ||
            updatedRowNumberList.contains(updatedRowNumber) ) {


            if ( (moveRowType == MoveRowsUp) ||
                 (moveRowType == MoveRowsDown) )  {

                updatedRowNumber = currentRow;
            }

            if ( (moveRowType == MoveRowsTop) ||
                 (moveRowType == MoveRowsBottom) ) {

                updatedRowNumber = updatedRowNumberList.at(updatedRowNumberList.size() - 1) + 1;
            }


        }


        // insert row to the model :
        parentItem->insertRow(updatedRowNumber, itemRows);

        // keep row number of inserted items :
        updatedRowNumberList.append(updatedRowNumber);

        this->selectionModel()->select(QItemSelection(itemRows.at(0)->index(), itemRows.at(itemRows.size() - 1)->index()),
                                       QItemSelectionModel::Select);
    }



    // ensure that moved rows are still visible :
    if (!rowNumberList.isEmpty()) {

        QModelIndex visibleIndex;

        if ( (moveRowType == MoveRowsUp) ||
             (moveRowType == MoveRowsTop) ) {

            visibleIndex = itemRowsMap.value(rowNumberList.takeFirst()).at(0)->index();
        }

        if ( (moveRowType == MoveRowsDown) ||
             (moveRowType == MoveRowsBottom) ){

            visibleIndex = itemRowsMap.value(rowNumberList.takeLast()).at(0)->index();
        }


        this->scrollTo(visibleIndex);

    }

}


void MyTreeView::setHeaderLabels() {

    QStringList headerLabels;
    headerLabels.append(i18n("File Name"));
    headerLabels.append(i18n("Status"));
    headerLabels.append(i18n("Progress"));
    headerLabels.append(i18n("Size"));
    downloadModel->setHorizontalHeaderLabels(headerLabels);
}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void MyTreeView::moveToTopSlot() {
    this->moveRow(MoveRowsTop);
}

void MyTreeView::moveToBottomSlot() {
    this->moveRow(MoveRowsBottom);
}

void MyTreeView::moveUpSlot() {
    this->moveRow(MoveRowsUp);
}

void MyTreeView::moveDownSlot() {
    this->moveRow(MoveRowsDown);
}



void MyTreeView::selectedItemSlot() {

    bool enableMoveButton = true;
    bool enableStartButton = false;
    bool enablePauseButton = false;

    // get selected items :
    QList<QModelIndex> indexesList = this->selectionModel()->selectedRows();

    if (!indexesList.isEmpty()) {

        // get the parent of the first selected element :
        QModelIndex firstParentIndex = indexesList.at(0).parent();

        for (int i = 1; i < indexesList.size(); i++) {

            QModelIndex currentModelIndex = indexesList.at(i);

            // if elements do not have the same parent :
            if (firstParentIndex != currentModelIndex.parent()) {
                enableMoveButton = false;
            }
        }
    }

    // if no item selected :
    if (indexesList.isEmpty()) {
        emit setMoveButtonEnabledSignal(false);
    }
    else {
        emit setMoveButtonEnabledSignal(enableMoveButton);
    }


    // enable/disable start-pause buttons :
    if (!enableMoveButton) {
        emit setPauseButtonEnabledSignal(false);
        emit setStartButtonEnabledSignal(false);
    }
    else{

        for (int i = 0; i < indexesList.size(); i++) {

            QModelIndex currentModelIndex = indexesList.at(i);
            QStandardItem* stateItem = downloadModel->getStateItemFromIndex(currentModelIndex);
            UtilityNamespace::ItemStatus currentStatus = downloadModel->getStatusFromStateItem(stateItem);

            // enable start button if selected item is paused/pausing
            if (!enableStartButton) {
                enableStartButton = ( Utility::isPaused(currentStatus) || Utility::isPausing(currentStatus) );
            }

            // enable pause button if selected item is idle/download
            if (!enablePauseButton) {
                enablePauseButton = Utility::isReadyToDownload(currentStatus);
            }

            // disable remove button when download has been accomplished :
            if (!downloadModel->isNzbItem(stateItem) &&
                !Utility::isInDownloadProcess(currentStatus) &&
                currentStatus != UtilityNamespace::WaitForPar2IdleStatus) {

                emit setRemoveButtonEnabledSignal(false);

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

    }
}



void MyTreeView::removeRowSlot() {

    // remove selected rows by default :
    int answer = KMessageBox::Yes;

    // display confirm Dialog if checked in settings :
    if (Settings::confirmRemove()) {

        if (downloadModel->rowCount() != 0) {
            answer = KMessageBox::messageBox(this,
                                             KMessageBox::QuestionYesNo,
                                             i18n("Remove selected files from queue ?"));
        }

    }

    // if selected rows has not been canceled :
    if (answer == KMessageBox::Yes) {

        QList<int> rowList;
        QList<QModelIndex> indexesList = this->selectionModel()->selectedRows();

        //stores rows in a list
        for (int i = 0; i < indexesList.size(); i++) {
            rowList.append(indexesList.at(i).row());
        }

        qSort(rowList.begin(), rowList.end(), qGreater<int>());


        for (int i = 0; i < indexesList.size(); i++) {

            QModelIndex currentModelIndex = indexesList.at(i);
            if (currentModelIndex.isValid()) {

                // if the parent has been selected (a nzb item):
                if (currentModelIndex.parent() == QModelIndex()) {
                    downloadModel->removeRow(rowList.at(i));
                }
                // else files of the parent (nzb item) has been selected :
                else {
                    QStandardItem* nzbItem = downloadModel->itemFromIndex(currentModelIndex.parent());
                    nzbItem->removeRow(rowList.at(i));

                    if (nzbItem->rowCount() > 0) {

                        // set nzb parent row up to date :
                        emit recalculateNzbSizeSignal(nzbItem->index());

                        // item has been removed extract could fail, download Par2 files :
                        emit changePar2FilesStatusSignal(nzbItem->index(), IdleStatus);
                    }
                    // if the nzb item has no more child, remove it :
                    else {
                        downloadModel->invisibleRootItem()->removeRow(nzbItem->row());

                    }

                }
            }
        }

    }


    // disable shutdown if all rows have been removed by the user :
    if (this->downloadModel->invisibleRootItem()->rowCount() == 0) {
        // disable shutdown scheduler :
        emit allRowRemovedSignal();
    }

    // update the status bar :
    emit statusBarFileSizeUpdateSignal(Incremental);


}


void MyTreeView::clearSlot() {

    // clear rows by default :
    int answer = KMessageBox::Yes;

    // display confirm Dialog if checked in settings :
    if (Settings::confirmClear()) {

        if (downloadModel->rowCount() != 0) {
            answer = KMessageBox::messageBox(this,
                                             KMessageBox::QuestionYesNo,
                                             i18n("Remove all files from queue ?"));
        }
    }

    // if selected rows has not been canceled :
    if (answer == KMessageBox::Yes) {

        downloadModel->clear();
        // add the labels to the header :
        this->setHeaderLabels();

        //reset default buttons :
        this->selectedItemSlot();

        // reset the status bar :
        emit statusBarFileSizeUpdateSignal(Reset);

        // disable shutdown scheduler :
        emit allRowRemovedSignal();

    }


}


void MyTreeView::openFolderSlot() {

    // get selected indexes :
    QList<QModelIndex> indexesList = this->selectionModel()->selectedRows();
    qSort(indexesList);

    // open download folder by default :
    QString fileSavePath = Settings::completedFolder().path();

    // if a row has been selected, open folder below download folder :
    if (!indexesList.isEmpty()) {

        QModelIndex index = indexesList.at(0);

        // if parent has been selected, get its first child to retrieve file save path :
        if ( this->downloadModel->isNzbItem(this->downloadModel->itemFromIndex(index)) ) {

            index = index.child(FILE_NAME_COLUMN, 0);
        }

        fileSavePath = this->downloadModel->getNzbFileDataFromIndex(index).getFileSavePath();

    }

    // do not manage delete as KRun uses auto deletion by default :
    new KRun(KUrl(fileSavePath), this);

}





bool MyTreeView::areJobsFinished() {

    bool jobFinished = true;

    // get the root model :
    QStandardItem* rootItem = this->downloadModel->invisibleRootItem();

    // for each parent item, get its current status :
    for (int i = 0; i < rootItem->rowCount(); i++) {

        QStandardItem* parentStateItem = rootItem->child(i, STATE_COLUMN);
        UtilityNamespace::ItemStatus currentStatus = this->downloadModel->getStatusFromStateItem(parentStateItem);

        // check parent status activity :
        if ( Utility::isReadyToDownload(currentStatus)       ||
             Utility::isPausing(currentStatus)               ||
             Utility::isDecoding(currentStatus)              ||
             Utility::isPostDownloadProcessing(currentStatus) ) {

            jobFinished = false;
            break;
        }

        // if do not shutdown system if paused items found :
        if ( Settings::pausedShutdown() && Utility::isPaused(currentStatus) ) {

            jobFinished = false;
            break;
        }

    }

    return jobFinished;
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
        if (downloadModel->isNzbItem(item) && (item->row() == 0) ) {
            const QRect rect = this->visualRect(index);
            if (this->viewport()->rect().intersects(rect)){
                this->viewport()->update(rect);
            }
        }
    }

}
#endif

