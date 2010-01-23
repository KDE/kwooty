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

#include "nzbfilehandler.h"
#include "utility.h"
#include "settings.h"
#include "clientmanagerconn.h"
#include "mystatusbar.h"
#include "mytreeview.h"
#include "segmentmanager.h"
#include "segmentsdecoderthread.h"
#include "repairdecompressthread.h"
#include "standarditemmodel.h"
#include "itemparentupdater.h"
#include "datarestorer.h"
#include "shutdownmanager.h"
#include "data/itemstatusdata.h"

using namespace UtilityNamespace;


CentralWidget::CentralWidget(QWidget* parent, MyStatusBar* parentStatusBar) : QWidget(parent)
{

    // init the downloadModel :
    downloadModel = new StandardItemModel(this);
    
    // init treeview :
    treeView = new MyTreeView(parent, this);
    
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

    // save and restore pending downloads from previous session :
    dataRestorer = new DataRestorer(this);

    // setup shutdown manager :
    shutdownManager = new ShutdownManager(this);

    this->setupConnections();

}

CentralWidget::~CentralWidget()
{
    
}





void CentralWidget::handleNzbFile(QFile& file, const QList<GlobalFileData>& inGlobalFileDataList){
    
    // remove .nzb extension to file name:
    QFileInfo fileInfo(file.fileName());
    QString nzbName = fileInfo.completeBaseName();
    
    QList<GlobalFileData> globalFileDataList;

    // if list is empty it corresponds to a normal nzb file processing :
    if (inGlobalFileDataList.isEmpty()) {

        // parse the xml file and add elements to the model :
        NzbFileHandler nzbFileHandler;
        globalFileDataList = nzbFileHandler.processNzbFile(this, file, nzbName);

    }
    // else it corresponds to a data restoration from a previous session :
    else {

        globalFileDataList = inGlobalFileDataList;

    }


    // insert the data from file into the download model :
    if (!globalFileDataList.isEmpty()) {

        this->setDataToModel(globalFileDataList, nzbName);

        // update the status bar :
        this->statusBarFileSizeUpdate();

        // resize the column according to file's name length :
        int widthInPixel = treeView->fontMetrics().width(nzbName) + 100;

        // if column width is lower than current width, ajust it :
        if (treeView->columnWidth(FILE_NAME_COLUMN) < widthInPixel) {
            treeView->setColumnWidth(FILE_NAME_COLUMN, widthInPixel);
        }

        // notify nntp clients that data has arrived :
        emit dataHasArrivedSignal();
    }


}



void CentralWidget::savePendingDownloads(bool saveSilently, UtilityNamespace::SystemShutdownType systemShutdownType) {

    dataRestorer->saveQueueData(saveSilently);

    // disable dataRestorer when shutdown is requested because the app will be closed automatically
    // data saving will be triggered and then data saving dialog box could prevent system shutdown :
    if (systemShutdownType == UtilityNamespace::Shutdown) {
        dataRestorer->setActive(false);
    }

}


void CentralWidget::restoreDataFromPreviousSession(const QList<GlobalFileData>& globalFileDataList) {

    // instanciate a QFile to only get the name of the nzb needed by handleNzbFile();
    NzbFileData nzbFileData = globalFileDataList.at(0).getNzbFileData();
    QFile nzbFile(nzbFileData.getNzbName());

    // populate treeView with saved data :
    this->handleNzbFile(nzbFile, globalFileDataList);

    // update parent status to the same value as previous session :
    for (int i = 0; i < downloadModel->rowCount(); i++) {
        // retrieve nzb parent item :
        QStandardItem* parentFileNameItem = this->downloadModel->item(i, FILE_NAME_COLUMN);
        this->itemParentUpdater->updateNzbItems(parentFileNameItem->index());

    }

}



void CentralWidget::setDataToModel(const QList<GlobalFileData>& globalFileDataList, const QString& nzbName){

    QStandardItem* nzbNameItem = new QStandardItem(nzbName);
    nzbNameItem->setIcon(KIcon("go-next-view"));

    quint64  nzbFilesSize = 0;
    int  par2FileNumber = 0;

    foreach (GlobalFileData currentGlobalFileData, globalFileDataList) {

        // populate children :
        this->addParentItem(nzbNameItem, currentGlobalFileData);
        
        // compute size of all files contained in the nzb :
        nzbFilesSize += currentGlobalFileData.getNzbFileData().getSize();

        // count number of par2 Files :
        if (currentGlobalFileData.getNzbFileData().isPar2File()) {
            par2FileNumber++;
        }
        
    }

    // set idle status by default :
    QStandardItem* nzbStateItem = new QStandardItem();
    nzbStateItem->setData(qVariantFromValue(ItemStatusData()), StatusRole);
    
    // set size :
    QStandardItem* nzbSizeItem = new QStandardItem();
    nzbSizeItem->setData(qVariantFromValue(nzbFilesSize), SizeRole);
    
    // add the nzb items to the model :
    int nzbNameItemRow = downloadModel->rowCount();
    
    downloadModel->setItem(nzbNameItemRow, FILE_NAME_COLUMN, nzbNameItem);
    downloadModel->setItem(nzbNameItemRow, SIZE_COLUMN, nzbSizeItem);
    downloadModel->setItem(nzbNameItemRow, STATE_COLUMN, nzbStateItem);
    downloadModel->setItem(nzbNameItemRow, PROGRESS_COLUMN, new QStandardItem());

    // expand treeView :
    treeView->setExpanded(nzbNameItem->index(), Settings::expandTreeView());

    // alternate row color :
    treeView->setAlternatingRowColors(Settings::alternateColors());

    // enable / disable smart par2 download :
    if ( Settings::smartPar2Download() && (par2FileNumber < globalFileDataList.size()) ) {
        emit changePar2FilesStatusSignal(nzbNameItem->index(), WaitForPar2IdleStatus);
    }


    // set status Icon near archive file name :
    emit setIconToFileNameItemSignal(nzbNameItem->index());


}



void CentralWidget::addParentItem (QStandardItem* nzbNameItem, const GlobalFileData& currentGlobalFileData) {

    // get the current row of the nzbName item :
    int nzbNameItemNextRow = nzbNameItem->rowCount();
    
    const NzbFileData currentNzbFileData  = currentGlobalFileData.getNzbFileData();

    // add the file name as parent's item :
    QString fileName = currentNzbFileData.getFileName();
    QStandardItem* fileNameItem = new QStandardItem(fileName);
    
    // set data to fileNameItem :
    QVariant variant;
    variant.setValue(currentNzbFileData);
    fileNameItem->setData(variant, NzbFileDataRole);
    
    // set unique identifier :
    fileNameItem->setData(currentNzbFileData.getUniqueIdentifier(), IdentifierRole);
    // set tool tip :
    fileNameItem->setToolTip(fileName);

    nzbNameItem->setChild(nzbNameItemNextRow, FILE_NAME_COLUMN, fileNameItem);

    // set idle status by default :
    QStandardItem* parentStateItem = new QStandardItem();
    parentStateItem->setData(qVariantFromValue(currentGlobalFileData.getItemStatusData()), StatusRole);
    nzbNameItem->setChild(nzbNameItemNextRow, STATE_COLUMN, parentStateItem);
    
    // set size :
    QStandardItem* parentSizeItem = new QStandardItem();
    parentSizeItem->setData(qVariantFromValue(currentNzbFileData.getSize()), SizeRole);
    nzbNameItem->setChild(nzbNameItemNextRow, SIZE_COLUMN, parentSizeItem);
    
    // set download progression (0 by default) :
    QStandardItem* parentProgressItem = new QStandardItem();
    parentProgressItem->setData(qVariantFromValue(currentGlobalFileData.getProgressValue()), ProgressRole);
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
             treeView,
             SLOT(selectedItemSlot()));

    // update info about decoding process :
    connect (segmentsDecoderThread,
             SIGNAL(updateDecodeSignal(QVariant, int, UtilityNamespace::ItemStatus, QString, bool)),
             segmentManager,
             SLOT(updateDecodeSegmentSlot(QVariant, int, UtilityNamespace::ItemStatus, QString, bool)));

    // suppress old segments if user have to chosen to not reload data from previous session :
    connect (dataRestorer,
             SIGNAL(suppressOldOrphanedSegmentsSignal()),
             segmentsDecoderThread,
             SLOT(suppressOldOrphanedSegmentsSlot()),Qt::QueuedConnection);


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




void CentralWidget::setStartPauseDownload(const UtilityNamespace::ItemStatus targetStatus, const QList<QModelIndex>& indexesList){


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
    treeView->selectedItemSlot();
}



void CentralWidget::setStartPauseDownloadAllItems(const UtilityNamespace::ItemStatus targetStatus){

    // select all rows in order to set them to paused or Idle :
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

    this->setStartPauseDownload(targetStatus, indexesList);


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

MyTreeView* CentralWidget::getTreeView() const{
    return this->treeView;
}

ItemParentUpdater* CentralWidget::getItemParentUpdater() const{
    return this->itemParentUpdater;
}

ShutdownManager* CentralWidget::getShutdownManager() const{
    return this->shutdownManager;
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void CentralWidget::statusBarFileSizeUpdateSlot(StatusBarUpdateType statusBarUpdateType){

    if (statusBarUpdateType == Reset) {
        // reset the status bar :
        statusBar->fullFileSizeUpdate(0, 0);
    }

    if (statusBarUpdateType == Incremental) {
        this->statusBarFileSizeUpdate();
    }
}


void CentralWidget::pauseDownloadSlot(){
    
    QList<QModelIndex> indexesList = treeView->selectionModel()->selectedRows();
    this->setStartPauseDownload(PauseStatus, indexesList);
}

void CentralWidget::startDownloadSlot(){
    
    QList<QModelIndex> indexesList = treeView->selectionModel()->selectedRows();
    this->setStartPauseDownload(IdleStatus, indexesList);
    emit dataHasArrivedSignal();
}



void CentralWidget::downloadWaitingPar2Slot(){

    this->statusBarFileSizeUpdate();
    emit dataHasArrivedSignal();

}



void CentralWidget::saveFileErrorSlot(int fromProcessing){
    
    // 1. set all Idle items to pause before notify the user :
    this->setStartPauseDownloadAllItems(UtilityNamespace::PauseStatus);

    
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

    
    // 2. delegate specific settings to concerned object  :
    emit settingsChangedSignal();
    
}



