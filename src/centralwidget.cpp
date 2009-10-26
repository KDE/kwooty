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

#include "centralwidget.h"

#include <KMessageBox>
#include <KDebug>
#include <KIcon>
#include <QtGui>

#include "itemdelegate.h"
#include "nzbfilehandler.h"
#include "data/itemstatusdata.h"
#include "utility.h"
#include "settings.h"
#include "clientmanagerconn.h"
#include "mystatusbar.h"
#include "segmentmanager.h"
#include "segmentsdecoderthread.h"
#include "repairdecompressthread.h"
#include "standarditemmodel.h"
#include "itemparentupdater.h"

using namespace UtilityNamespace;


CentralWidget::CentralWidget(QWidget *parent, MyStatusBar* parentStatusBar) : QWidget(parent)
{
    
    // setup layout :
    QVBoxLayout* vBoxLayout = new QVBoxLayout(parent);
    treeView = new QTreeView(parent);
    vBoxLayout->addWidget(treeView);
    
    // init the downloadModel
    downloadModel = new StandardItemModel(this);
    
    // add the labels to the header :
    this->setHeaderLabels();
    
    treeView->setModel(downloadModel);
    
    // delegate for item rendering / displaying :
    treeView->setItemDelegate(new ItemDelegate(this));
    
    // Avoid rows editing:
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Allowing mutiple row selection :
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->setAllColumnsShowFocus(true);
    treeView->setAnimated(Settings::animateTreeView());
    
    // setup segment decoder thread :
    segmentsDecoderThread = new SegmentsDecoderThread(this);
    
    // setup repairing and decompressing thread :
    repairDecompressThread = new RepairDecompressThread(this);
    
    // retrieve status bar :
    statusBar = parentStatusBar;
    
    // update view according to items data :
    itemParentUpdater = new ItemParentUpdater(this);
    
    // manage dispatching / updating segments related to one item :
    segmentManager = new SegmentManager(this);
    
    // create one nntp connection per client:
    this->createNntpClients();
    
    // set download ant temp folders into home dir if not specified by user :
    this->initFoldersSettings();
    
    // init button code that avoid to display one message box per nntp client instance error :
    saveErrorButtonCode = 0;
    
    this->setupConnections();
    
}

CentralWidget::~CentralWidget()
{
    
}



void CentralWidget::setHeaderLabels() {
    
    QStringList headerLabels;
    headerLabels.append(i18n("File Name"));
    headerLabels.append(i18n("Status"));
    headerLabels.append(i18n("Progress"));
    headerLabels.append(i18n("Size"));
    downloadModel->setHorizontalHeaderLabels(headerLabels);
}




void CentralWidget::handleNzbFile(QFile& file){
    
    //remove .nzb extension to file name:
    QFileInfo fileInfo(file.fileName());
    QString nzbName = fileInfo.fileName();
    nzbName.chop(4);
    
    // parse the xml file and add elements to the model :
    NzbFileHandler nzbFileHandler;
    QList<NzbFileData> nzbFilesList = nzbFileHandler.processNzbFile(this, file, nzbName);
    
    // insert the data from file into the download model :  
    if (!nzbFilesList.isEmpty()) {
        this->setDataToModel(nzbFilesList, nzbName);
        
        // resize the column according to file's name length :
        int widthInPixel = treeView->fontMetrics().width(nzbName) + 100;
        
        // if column width is lower than current width, ajust it :
        if (treeView->columnWidth(FILE_NAME_COLUMN) < widthInPixel) {
            treeView->setColumnWidth(FILE_NAME_COLUMN, widthInPixel);
        }
        
        treeView->setAlternatingRowColors(Settings::alternateColors());
        
        // notify nntp clients that data has arrived :
        emit dataHasArrivedSignal();
    }
    
}



void CentralWidget::setDataToModel(const QList<NzbFileData>& nzbFilesList, const QString& nzbName){

    QStandardItem* nzbNameItem = new QStandardItem(nzbName);
    nzbNameItem->setIcon(KIcon("arrow-right"));
    
    
    quint64  nzbFilesSize = 0;
    foreach (NzbFileData currentNzbFileData, nzbFilesList) {
    
        this->addParentItem(nzbNameItem, currentNzbFileData);
        
        // compute size of all files contained in the nzb :
        nzbFilesSize += currentNzbFileData.getSize();
        
    }
    
    // update status bar :
    statusBar->addSize(nzbFilesSize);
    statusBar->addFiles(nzbFilesList.size());
    
    // set idle status by default :
    QStandardItem* nzbStateItem = new QStandardItem();
    nzbStateItem->setData(qVariantFromValue(new ItemStatusData()), StatusRole);;
    
    // set size :
    QStandardItem* nzbSizeItem = new QStandardItem();
    nzbSizeItem->setData(qVariantFromValue(nzbFilesSize), SizeRole);
    
    // add the nzb items to the model :
    int nzbNameItemRow = downloadModel->rowCount();
    
    downloadModel->setItem(nzbNameItemRow, FILE_NAME_COLUMN, nzbNameItem);
    downloadModel->setItem(nzbNameItemRow, SIZE_COLUMN, nzbSizeItem);
    downloadModel->setItem(nzbNameItemRow, STATE_COLUMN, nzbStateItem);
    downloadModel->setItem(nzbNameItemRow, PROGRESS_COLUMN, new QStandardItem());

}



void CentralWidget::addParentItem (QStandardItem* nzbNameItem, const NzbFileData& currentNzbFileData) {

    // get the current row of the nzbName item :
    int nzbNameItemNextRow = nzbNameItem->rowCount();
    
    // add the file name as parent's item :
    QString fileName = currentNzbFileData.getFileName();
    QStandardItem* fileNameItem = new QStandardItem(fileName);
    
    // set data to fileNameItem :
    QVariant variant;
    variant.setValue(currentNzbFileData);
    fileNameItem->setData(variant, NzbFileDataRole);
    
    SegmentData segmentData = currentNzbFileData.getSegmentList().at(0);
    // set unique identifier :
    fileNameItem->setData(segmentData.getParentUniqueIdentifier(), IdentifierRole);
    // set tool tip :
    fileNameItem->setToolTip(fileName);
    nzbNameItem->setChild(nzbNameItemNextRow, FILE_NAME_COLUMN, fileNameItem);
    
    
    // set idle status by default :
    QStandardItem* parentStateItem = new QStandardItem();
    parentStateItem->setData(qVariantFromValue(new ItemStatusData()), StatusRole);
    nzbNameItem->setChild(nzbNameItemNextRow, STATE_COLUMN, parentStateItem);
    
    // set size :
    QStandardItem* parentSizeItem = new QStandardItem();
    parentSizeItem->setData(qVariantFromValue(currentNzbFileData.getSize()), SizeRole);
    nzbNameItem->setChild(nzbNameItemNextRow, SIZE_COLUMN, parentSizeItem);
    
    // set download progression (0 by default) :
    QStandardItem* parentProgressItem = new QStandardItem();
    parentProgressItem->setData(qVariantFromValue(PROGRESS_INIT), ProgressRole);
    nzbNameItem->setChild(nzbNameItemNextRow, PROGRESS_COLUMN, parentProgressItem);
    
}


void CentralWidget::createNntpClients(){

    //create the nntp clients tread manager :
    int connectionNumber = Settings::connectionNumber();
    
    // set a delay of +100 ms between each nntp client instance :
    int connectionDelay = 0;
    for (int i = 0; i < connectionNumber; i++){
        this->clientManagerConnList.append(new ClientManagerConn(this, i, connectionDelay));
        connectionDelay += 100;
    }
    
}




void CentralWidget::setupConnections() {

    // enable or disable buttons according to selected items :
    connect (treeView->selectionModel(),
             SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
             this,
             SLOT(selectedItemSlot()));
             
    // update info about decoding process :
    connect (segmentsDecoderThread,
             SIGNAL(updateDecodeSignal(QVariant, int, UtilityNamespace::ItemStatus, QString)),
             segmentManager,
             SLOT(updateDecodeSegmentSlot(QVariant, int, UtilityNamespace::ItemStatus, QString)));
             
    // update info about decoding repair process :
    connect (repairDecompressThread,
             SIGNAL(updateRepairSignal(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget)),
             segmentManager,
             SLOT(updateRepairExtractSegmentSlot(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget)));
             
    // update info about decoding extract process :
    connect (repairDecompressThread,
             SIGNAL(updateExtractSignal(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget)),
             segmentManager,
             SLOT(updateRepairExtractSegmentSlot(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget)));
             
}




void CentralWidget::moveRow(bool isMovedToTop){

    QStandardItem* parentItem = NULL;
    QList<QModelIndex> indexesList = treeView->selectionModel()->selectedRows();
    QList<int> rowList;
    
    //stores rows in a list only if index is valid :
    for (int i = 0; i < indexesList.size(); i++){
        rowList.append(indexesList.at(i).row());
    }
    
    if (isMovedToTop) {
        qSort(rowList.begin(), rowList.end(), qGreater<int>());
    }
    else {
        qSort(rowList.begin(), rowList.end());
    }
    
    
    // check selected rows :
    for (int i = 0; i < indexesList.size(); i++){
    
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
        
        treeView->selectionModel()->select(QItemSelection(topLeftIndex, bottomRightIndex), QItemSelectionModel::Select);
        
        // scroll to moved items :
        if (isMovedToTop) {
            treeView->scrollTo(downloadModel->index(topRow, 0, parentItem->index()));
        }
        else {
            treeView->scrollTo(downloadModel->index(bottomRow, 0, parentItem->index()));
        }
    }
    
    
}


void CentralWidget::statusBarFileSizeUpdate() {

    quint64 size = 0;
    quint64 files = 0;
    
    // get the root item :
    QStandardItem* rootItem = downloadModel->invisibleRootItem();
    
    // parse nzb items :
    for (int i = 0; i < rootItem->rowCount(); i++) {
    
        QStandardItem* nzbItem = rootItem->child(i);
        
        // parse nzb children :
        for (int j = 0; j < nzbItem->rowCount(); j++) {
        
            QStandardItem* statusItem = nzbItem->child(j, STATE_COLUMN);
            
            // if the item is pending (Idle, Download, Paused, Pausing states), processes it :            
            UtilityNamespace::ItemStatus status = downloadModel->getStatusFromStateItem(statusItem);
            
            if ( Utility::isReadyToDownload(status) ||
                 Utility::isPaused(status)          ||
                 Utility::isPausing(status) )       {
                 
                QStandardItem* sizeItem = nzbItem->child(j, SIZE_COLUMN);
                size += sizeItem->data(SizeRole).toULongLong();
                files++;
            }
        }
        
    }
    
    statusBar->fullFileSizeUpdate(size, files);
    
}




void CentralWidget::setStartPauseDownload(int targetStatus, const QList<QModelIndex>& indexesList){


    foreach (QModelIndex currentModelIndex, indexesList){
    
        // get file name item related to selected index :
        QStandardItem* fileNameItem = downloadModel->getFileNameItemFromIndex(currentModelIndex);
        
        // if the item is a nzbItem, retrieve their children :
        if (downloadModel->isNzbItem(fileNameItem)){
        
            for (int i = 0; i < fileNameItem->rowCount(); i++){
            
                QStandardItem* nzbChildrenItem = fileNameItem->child(i, FILE_NAME_COLUMN);
                segmentManager->setIdlePauseSegments(nzbChildrenItem, targetStatus);
            }
        }
        
        else {
            // update selected nzb children segments :
            segmentManager->setIdlePauseSegments(fileNameItem, targetStatus);
        }
        
    }
    
    //reset default buttons :
    this->selectedItemSlot();
}



void CentralWidget::initFoldersSettings() {

    // set default path for download and temporary folders if not filled by user :
    if (Settings::completedFolder().path().isEmpty()) {
        Settings::setCompletedFolder(QDir::homePath() + "/kwooty/Download");
    }
    
    if (Settings::temporaryFolder().path().isEmpty()) {
        Settings::setTemporaryFolder(QDir::homePath() + "/kwooty/Temp");
    }
}



SegmentManager* CentralWidget::getSegmentManager() const{
    return segmentManager;
}


StandardItemModel* CentralWidget::getDownloadModel() const{
    return this->downloadModel;
}


MyStatusBar* CentralWidget::getStatusBar() const{
    return this->statusBar;
}

ItemParentUpdater* CentralWidget::getItemParentUpdater() const{
    return this->itemParentUpdater;
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void CentralWidget::selectedItemSlot(){

    bool enableMoveButton = true;
    bool enableStartButton = false;
    bool enablePauseButton = false;
    
    // get selected items :
    QList<QModelIndex> indexesList = treeView->selectionModel()->selectedRows();
    
    if (!indexesList.isEmpty()){
    
        // get the parent of the first selected element :
        QModelIndex firstParentIndex = indexesList.at(0).parent();
        
        for (int i = 1; i < indexesList.size(); i++){
        
            QModelIndex currentModelIndex = indexesList.at(i);
            
            // if elements do not have the same parent :
            if (firstParentIndex != currentModelIndex.parent()){
                enableMoveButton = false;
            }
        }
    }
    
    // if no item selected :
    if (indexesList.isEmpty()){
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
        
        for (int i = 0; i < indexesList.size(); i++){
            
            QModelIndex currentModelIndex = indexesList.at(i);
            QStandardItem* stateItem = downloadModel->getStateItemFromIndex(currentModelIndex);
            UtilityNamespace::ItemStatus currentStatus = downloadModel->getStatusFromStateItem(stateItem);
            
            // enable start button if selected item is paused/pausing
            if (!enableStartButton){
                enableStartButton = ( Utility::isPaused(currentStatus) || Utility::isPausing(currentStatus) );
            }
            
            // enable pause button if selected item is idle/download
            if (!enablePauseButton){
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



void CentralWidget::removeRowSlot()
{

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
        QList<QModelIndex> indexesList = treeView->selectionModel()->selectedRows();

        //stores rows in a list
        for (int i = 0; i < indexesList.size(); i++){
            rowList.append(indexesList.at(i).row());
        }

        qSort(rowList.begin(), rowList.end(), qGreater<int>());


        for (int i = 0; i < indexesList.size(); i++){

            QModelIndex currentModelIndex = indexesList.at(i);
            if (currentModelIndex.isValid()) {

                // if the parent has been selected (a nzb item):
                if (currentModelIndex.parent() == QModelIndex()){
                    downloadModel->removeRow(rowList.at(i));
                }
                // else files of the parent (nzb item) has been selected :
                else{
                    QStandardItem* nzbItem = downloadModel->itemFromIndex(currentModelIndex.parent());
                    nzbItem->removeRow(rowList.at(i));

                    if (nzbItem->rowCount() > 0) {
                        // set nzb parent row up to date :
                        emit recalculateNzbSizeSignal(nzbItem->index());
                    }
                    // if the nzb item has no more child, remove it :
                    else{
                        downloadModel->invisibleRootItem()->removeRow(nzbItem->row());
                    }

                }
            }
        }

    }

    // update the status bar :
    this->statusBarFileSizeUpdate();


    
}



void CentralWidget::clearSlot()
{
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
        statusBar->fullFileSizeUpdate(0, 0);
    }


}


void CentralWidget::pauseDownloadSlot(){
    
    QList<QModelIndex> indexesList = treeView->selectionModel()->selectedRows();
    setStartPauseDownload(PauseStatus, indexesList);
}

void CentralWidget::startDownloadSlot(){
    
    QList<QModelIndex> indexesList = treeView->selectionModel()->selectedRows();
    setStartPauseDownload(IdleStatus, indexesList);
    emit dataHasArrivedSignal();
}



void CentralWidget::moveToTopSlot(){
    this->moveRow(true);
}



void CentralWidget::moveToBottomSlot(){
    this->moveRow(false);
}



void CentralWidget::saveFileErrorSlot(int fromProcessing){
    
    // 1. set all Idle items to pause before notify the user :
    QList<QModelIndex> indexesList;
    
    int rowNumber = downloadModel->rowCount();
    
    for (int i = 0; i < rowNumber; i++) {
        
        QModelIndex currentIndex = downloadModel->item(i)->index();
        QStandardItem* stateItem = downloadModel->getStateItemFromIndex(currentIndex);
        
        UtilityNamespace::ItemStatus currentStatus = downloadModel->getStatusFromStateItem(stateItem);
        
        if (Utility::isReadyToDownload(currentStatus)) {
            indexesList.append(currentIndex);
        }
    }
    
    this->setStartPauseDownload(PauseStatus, indexesList);
    
    
    
    // 2. notify user with a message box (and avoid multiple message box instances):
    if (this->saveErrorButtonCode == 0) {
        
        QString saveErrorFolder;
        
        if (fromProcessing == DuringDecode) {
            saveErrorFolder = i18n("download folder");
        }
        if (fromProcessing == DuringDownload) {
            saveErrorFolder = i18n("temporary folder");
        }
        
        
        this->saveErrorButtonCode = KMessageBox::Cancel;
        
        this->saveErrorButtonCode = KMessageBox::messageBox(this,
                                                            KMessageBox::Sorry,
                                                            i18n("Write error in <b>%1</b>: disk drive may be full.<br>Downloads have been suspended.",
                                                                 saveErrorFolder),
                                                            i18n("Write error"));
        
        
        if (this->saveErrorButtonCode == KMessageBox::Ok) {
            this->saveErrorButtonCode = 0;
        }
        
    }
    
}




void CentralWidget::updateSettingsSlot() {
    
    // 1. ajust connection objects according to value set in settings :
    // if more nntp connections are requested :
    int connectionNumber = Settings::connectionNumber();
    if (connectionNumber > clientManagerConnList.size()) {
        
        int connectionDelay = 0;
        for (int i = clientManagerConnList.size(); i < connectionNumber; i++){
            this->clientManagerConnList.append(new ClientManagerConn(this, i, connectionDelay));
            //set a delay of 100ms between each new connection :
            connectionDelay += 100;
        }
    }
    
    // if less nntp connections are requested :
    if (connectionNumber < clientManagerConnList.size()) {
        while (clientManagerConnList.size() > connectionNumber) {
            clientManagerConnList.takeLast()->deleteLater();
        }
    }
    
    // 2. change UI related settings :
    treeView->setAnimated(Settings::animateTreeView());
    treeView->setAlternatingRowColors(Settings::alternateColors());
    
    // 3. delegate specific settings to concerned object  :
    emit settingsChangedSignal();
    
}






