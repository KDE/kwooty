#include "mytreeview.h"

#include <KMessageBox>
#include <KDebug>

#include <QVBoxLayout>

#include "itemdelegate.h"
#include "centralwidget.h"
#include "standarditemmodel.h"
#include "settings.h"


MyTreeView::MyTreeView(QWidget* parent, CentralWidget* centralWidget) : QTreeView(parent) {

    this->centralWidget = centralWidget;
    this->downloadModel = centralWidget->getDownloadModel();

    // setup layout :
    QVBoxLayout* vBoxLayout = new QVBoxLayout(parent);
    vBoxLayout->addWidget(this);

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

    connect( this,
             SIGNAL(recalculateNzbSizeSignal(const QModelIndex)),
             centralWidget,
             SIGNAL(recalculateNzbSizeSignal(const QModelIndex)) );

    connect( this,
             SIGNAL(changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus)),
             centralWidget,
             SIGNAL(changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus)) );

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

    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        
        foreach (KUrl nzbUrl, urlList) {

            if (nzbUrl.url().endsWith(".nzb", Qt::CaseInsensitive)) {
                emit openFileByDragAndDropSignal(nzbUrl);
            }

        }
        
    }
    event->acceptProposedAction();
}


void MyTreeView::moveRow(bool isMovedToTop) {

    QStandardItem* parentItem = NULL;
    QList<QModelIndex> indexesList = this->selectionModel()->selectedRows();
    QList<int> rowList;

    //stores rows in a list only if index is valid :
    for (int i = 0; i < indexesList.size(); i++) {
        rowList.append(indexesList.at(i).row());
    }

    if (isMovedToTop) {
        qSort(rowList.begin(), rowList.end(), qGreater<int>());
    }
    else {
        qSort(rowList.begin(), rowList.end());
    }


    // check selected rows :
    for (int i = 0; i < indexesList.size(); i++) {

        QModelIndex currentModelIndex = indexesList.at(i);

        if (currentModelIndex.isValid()) {

            parentItem = downloadModel->getParentItem(currentModelIndex);

            // remove item at the given row and add it to the first one
            if (isMovedToTop) {
                parentItem->insertRow(0, parentItem->takeRow(rowList.at(i) + i));
            }
            else {
                // remove item at the given row and add it to the last one
                parentItem->appendRow(parentItem->takeRow(rowList.at(i) - i));
            }
        }
    }


    // highlight moved items :
    if (parentItem != NULL) {
        int topRow;
        int bottomRow;
        int lastColumn;

        if (isMovedToTop) {
            topRow = 0;
            bottomRow = rowList.size() - 1;
            lastColumn = parentItem->columnCount() - 1;
        }
        else {
            topRow = parentItem->rowCount() - rowList.size();
            bottomRow = parentItem->rowCount() - 1;
            lastColumn = parentItem->columnCount() - 1;
        }

        QModelIndex topLeftIndex = downloadModel->index(topRow, 0, parentItem->index());
        QModelIndex bottomRightIndex = downloadModel->index(bottomRow, lastColumn, parentItem->index());

        this->selectionModel()->select(QItemSelection(topLeftIndex, bottomRightIndex), QItemSelectionModel::Select);

        // scroll to moved items :
        if (isMovedToTop) {
            this->scrollTo(downloadModel->index(topRow, 0, parentItem->index()));
        }
        else {
            this->scrollTo(downloadModel->index(bottomRow, 0, parentItem->index()));
        }
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
    this->moveRow(true);
}

void MyTreeView::moveToBottomSlot() {
    this->moveRow(false);
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
            if (!downloadModel->isNzbItem(stateItem) && !Utility::isInDownloadProcess(currentStatus)) {
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
        if (downloadModel->isNzbItem(item) && (item->row() == 0) ) {
            const QRect rect = this->visualRect(index);
            if (this->viewport()->rect().intersects(rect)){
                this->viewport()->update(rect);
            }
        }
    }
}
#endif

