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

#include "core.h"

#include <KDebug>
#include <QIcon>
#include <KPasswordDialog>
#include <QtGui>

#include "nzbfilehandler.h"
#include "utilities/utility.h"
#include "utilities/utilityiconpainting.h"
#include "kwootysettings.h"
#include "clientmanagerconn.h"
#include "widgets/mytreeview.h"
#include "widgets/centralwidget.h"
#include "segmentmanager.h"
#include "segmentsdecoderthread.h"
#include "repairdecompressthread.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "itemparentupdater.h"
#include "itemchildrenmanager.h"
#include "datarestorer.h"
#include "fileoperations.h"
#include "mainwindow.h"
#include "sidebar.h"
#include "notificationmanager.h"
#include "servermanager.h"
#include "actions/actionbuttonsmanager.h"
#include "actions/actionsmanager.h"
#include "shutdown/shutdownmanager.h"
#include "observers/clientsobserver.h"
#include "observers/queuefileobserver.h"
#include "data/itemstatusdata.h"



Core::Core(MainWindow* mainWindow) : QObject(mainWindow) {

    this->mainWindow = mainWindow;

    this->nzbFileHandler = new NzbFileHandler(this);

    // init the downloadModel :
    this->downloadModel = new StandardItemModel(this);

    // query model according to items status :
    this->modelQuery = new StandardItemModelQuery(this);

    // init queue file observer :
    this->queueFileObserver = new QueueFileObserver(this);

    // collect connection statuses from clients and build stats info :
    this->clientsObserver = new ClientsObserver(this);

    // update view according to items data :
    this->itemParentUpdater = new ItemParentUpdater(this);

    // manage dispatching / updating segments related to one item :
    this->segmentManager = new SegmentManager(this);

    // save and restore pending downloads from previous session :
    this->dataRestorer = new DataRestorer(this);

    // setup segment decoder thread :
    this->segmentsDecoderThread = new SegmentsDecoderThread(this);

    // setup repairing and decompressing thread :
    this->repairDecompressThread = new RepairDecompressThread(this);

    // handle nntp clients with one or several servers :
    this->serverManager = new ServerManager(this);

    // set download and temp folders into home dir if not specified by user :
    this->initFoldersSettings();

    // setup shutdown manager :
    this->shutdownManager = new ShutdownManager(this);

    // setup nzb file opening closing :
    this->fileOperations = new FileOperations(this);

    // setup notification events :
    this->notificationManager = new NotificationManager(this);

    // setup actions manager :
    this->actionsManager = new ActionsManager(this);

}

Core::~Core() {

    delete this->segmentsDecoderThread;
    delete this->repairDecompressThread;

}


void Core::emitDataHasArrived(const QModelIndex& appendedIndex) {

    // first notify that data have just be appended for pre-processing :
    emit dataAboutToArriveSignal(appendedIndex);

    // notify nntp clients that data is now ready to be downloaded :
    emit dataHasArrivedSignal();

}



void Core::handleNzbFile(QFile& file, const QString& nzbName, const QList<GlobalFileData>& inGlobalFileDataList) {

    bool normalNzbFileProcessing = inGlobalFileDataList.isEmpty();

    QList<GlobalFileData> globalFileDataList;

    // if list is empty it corresponds to a normal nzb file processing :
    if (normalNzbFileProcessing) {

        // parse the xml file and add elements to the model :
        globalFileDataList = this->nzbFileHandler->processNzbFile(file, nzbName);

    }
    // else it corresponds to a data restoration from a previous session :
    else {

        globalFileDataList = inGlobalFileDataList;

    }


    // insert the data from file into the download model :
    if (!globalFileDataList.isEmpty()) {

        QModelIndex nzbNameIndex = this->setDataToModel(globalFileDataList, nzbName);

        // if it is restauration, do not send the new appended index :
        if (!normalNzbFileProcessing) {
            nzbNameIndex = QModelIndex();
        }

        // update the status bar :
        this->statusBarFileSizeUpdate();

        // resize the column according to file's name length :
        int widthInPixel = this->mainWindow->getTreeView()->fontMetrics().width(nzbName) + 100;

        // if column width is lower than current width, ajust it :
        if (this->getTreeView()->columnWidth(FILE_NAME_COLUMN) < widthInPixel) {
            this->getTreeView()->setColumnWidth(FILE_NAME_COLUMN, widthInPixel);
        }

        // select the row newly appended :
        this->getTreeView()->selectionModel()->select(nzbNameIndex, QItemSelectionModel::Clear | QItemSelectionModel::Select | QItemSelectionModel::Rows);

        // notify nntp clients that data has arrived :
        this->emitDataHasArrived(nzbNameIndex);

    }


}


int Core::savePendingDownloads(UtilityNamespace::SystemShutdownType systemShutdownType, const SaveFileBehavior saveFileBehavior) {

    int answer = this->dataRestorer->saveQueueData(saveFileBehavior);

    // disable dataRestorer when shutdown is requested because the app will be closed automatically
    // data saving will be triggered and then data saving dialog box could prevent system shutdown :
    if (systemShutdownType == UtilityNamespace::Shutdown) {
        this->dataRestorer->setActive(false);
    }

    return answer;
}


void Core::restoreDataFromPreviousSession(const QList<GlobalFileData>& globalFileDataList) {

    // instanciate a QFile to only get the name of the nzb needed by handleNzbFile();
    NzbFileData nzbFileData = globalFileDataList.at(0).getNzbFileData();
    QFile nzbFile(nzbFileData.getNzbName());

    // populate treeView with saved data :
    this->handleNzbFile(nzbFile, nzbFileData.getNzbName(), globalFileDataList);

    // update parent status to the same value as previous session :
    for (int i = 0; i < this->downloadModel->rowCount(); i++) {

        // retrieve nzb parent item :
        QStandardItem* parentFileNameItem = this->downloadModel->item(i, FILE_NAME_COLUMN);
        this->itemParentUpdater->updateNzbItems(parentFileNameItem->index());

    }

    // notify that data has just been restored :
    this->emitDataHasArrived();

}



QModelIndex Core::setDataToModel(const QList<GlobalFileData>& globalFileDataList, const QString& nzbName) {

    QStandardItem* nzbNameItem = new QStandardItem(nzbName);
    QStandardItem* nzbStateItem = new QStandardItem();
    QStandardItem* nzbSizeItem = new QStandardItem();

    // add the nzb items to the model :
    int nzbNameItemRow = this->downloadModel->rowCount();

    this->downloadModel->setItem(nzbNameItemRow, FILE_NAME_COLUMN, nzbNameItem);
    this->downloadModel->setItem(nzbNameItemRow, SIZE_COLUMN, nzbSizeItem);
    this->downloadModel->setItem(nzbNameItemRow, STATE_COLUMN, nzbStateItem);
    this->downloadModel->setItem(nzbNameItemRow, PROGRESS_COLUMN, new QStandardItem());


    quint64 nzbFilesSize = 0;
    int par2FileNumber = 0;
    bool badCrc = false;
    NzbFileData nzbFileData;

    // add children to parent (nzbNameItem) :
    foreach (const GlobalFileData& currentGlobalFileData, globalFileDataList) {

        // populate children :
        this->addParentItem(nzbNameItem, currentGlobalFileData);

        // compute size of all files contained in the nzb :
        nzbFilesSize += currentGlobalFileData.getNzbFileData().getSize();

        // count number of par2 Files :
        if (currentGlobalFileData.getNzbFileData().isPar2File()) {
            par2FileNumber++;
        }

        // if it is data restoring, check overall crc value in order to enable or disable smart par2 feature :
        if (currentGlobalFileData.getItemStatusData().getCrc32Match() != CrcOk) {
            badCrc = true;
        }

        // retrieve the path where nzb files are saved :
        if (nzbFileData.getFileSavePath().isEmpty()) {
            nzbFileData = currentGlobalFileData.getNzbFileData();
        }

    }

    // set parent unique identifier :
    nzbNameItem->setData(QVariant(QUuid::createUuid().toString()), IdentifierRole);

    // set parent file save path :
    this->downloadModel->updateParentFileSavePathFromIndex(nzbNameItem->index(), nzbFileData);

    // set idle status by default :
    this->downloadModel->initStatusDataToItem(nzbStateItem, ItemStatusData());

    // set size :
    nzbSizeItem->setData(qVariantFromValue(nzbFilesSize), SizeRole);


    // expand treeView :
    this->mainWindow->getTreeView()->setExpanded(nzbNameItem->index(), Settings::expandTreeView());

    // alternate row color :
    this->mainWindow->getTreeView()->setAlternatingRowColors(Settings::alternateColors());


    // enable / disable smart par2 download :
    if ( !badCrc && Settings::smartPar2Download() && (par2FileNumber < globalFileDataList.size()) ) {

        this->actionsManager->changePar2FilesStatus(nzbNameItem->index(), WaitForPar2IdleStatus);

    }

    return nzbNameItem->index();

}



void Core::addParentItem (QStandardItem* nzbNameItem, const GlobalFileData& currentGlobalFileData) {

    // get the current row of the nzbName item :
    int nzbNameItemNextRow = nzbNameItem->rowCount();

    const NzbFileData currentNzbFileData  = currentGlobalFileData.getNzbFileData();

    // add the (consice) file name as parent's item :
    QStandardItem* fileNameItem = new QStandardItem(this->getTreeView()->getDisplayedFileName(currentNzbFileData));
    nzbNameItem->setChild(nzbNameItemNextRow, FILE_NAME_COLUMN, fileNameItem);

    QStandardItem* parentStateItem = new QStandardItem();
    nzbNameItem->setChild(nzbNameItemNextRow, STATE_COLUMN, parentStateItem);

    QStandardItem* parentSizeItem = new QStandardItem();
    nzbNameItem->setChild(nzbNameItemNextRow, SIZE_COLUMN, parentSizeItem);

    QStandardItem* parentProgressItem = new QStandardItem();
    nzbNameItem->setChild(nzbNameItemNextRow, PROGRESS_COLUMN, parentProgressItem);


    // set data to fileNameItem :
    QVariant variant;
    variant.setValue(currentNzbFileData);
    fileNameItem->setData(variant, NzbFileDataRole);

    // set unique identifier :
    fileNameItem->setData(currentNzbFileData.getUniqueIdentifier(), IdentifierRole);
    // set tool tip :
    fileNameItem->setToolTip(currentNzbFileData.getFileName());

    // set idle status by default :
    nzbNameItem->setChild(nzbNameItemNextRow, STATE_COLUMN, parentStateItem);
    this->downloadModel->initStatusDataToItem(parentStateItem, currentGlobalFileData.getItemStatusData());

    // set size :
    nzbNameItem->setChild(nzbNameItemNextRow, SIZE_COLUMN, parentSizeItem);
    parentSizeItem->setData(qVariantFromValue(currentNzbFileData.getSize()), SizeRole);

    // set download progression (0 by default) :
    nzbNameItem->setChild(nzbNameItemNextRow, PROGRESS_COLUMN, parentProgressItem);
    parentProgressItem->setData(qVariantFromValue(currentGlobalFileData.getProgressValue()), ProgressRole);


    // if the current file name corresponds to a par2 file :
    if (currentNzbFileData.isPar2File()) {

        // display lighter color text :
        UtilityIconPainting::getInstance()->displayLighterText(fileNameItem);

    }

}



void Core::statusBarFileSizeUpdate() {

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

            if (Utility::isInDownloadProcess(status))  {

                QStandardItem* sizeItem = nzbItem->child(j, SIZE_COLUMN);
                size += sizeItem->data(SizeRole).toULongLong();
                files++;
            }
        }

    }


    this->clientsObserver->fullFileSizeUpdate(size, files);

}


void Core::initFoldersSettings() {

    // set default path for download and temporary folders if not filled by user :
    if (Settings::completedFolder().path().isEmpty()) {
        Settings::setCompletedFolder(Utility::buildFullPath(QDir::homePath(), "kwooty/Download"));
    }

    if (Settings::temporaryFolder().path().isEmpty()) {
        Settings::setTemporaryFolder(Utility::buildFullPath(QDir::homePath(), "kwooty/Temp"));
    }

}



MainWindow* Core::getMainWindow() const {
    return this->mainWindow;
}

SegmentManager* Core::getSegmentManager() const {
    return this->segmentManager;
}

NzbFileHandler* Core::getNzbFileHandler() const {
    return this->nzbFileHandler;
}

StandardItemModel* Core::getDownloadModel() const {
    return this->downloadModel;
}

StandardItemModelQuery* Core::getModelQuery() const {
    return this->modelQuery;
}


ItemParentUpdater* Core::getItemParentUpdater() const {
    return this->itemParentUpdater;
}

ShutdownManager* Core::getShutdownManager() const {
    return this->shutdownManager;
}

ClientsObserver* Core::getClientsObserver() const {
    return this->clientsObserver;
}

FileOperations* Core::getFileOperations() const {
    return this->fileOperations;
}

QueueFileObserver* Core::getQueueFileObserver() const {
    return this->queueFileObserver;
}

DataRestorer* Core::getDataRestorer() const {
    return this->dataRestorer;
}

ServerManager* Core::getServerManager() const {
    return this->serverManager;
}

SegmentsDecoderThread* Core::getSegmentsDecoderThread() const {
    return this->segmentsDecoderThread;
}

ActionsManager* Core::getActionsManager() const {
    return this->actionsManager;
}

SideBar* Core::getSideBar() const {
    return this->mainWindow->getSideBar();
}

MyTreeView* Core::getTreeView() const {
    return this->mainWindow->getTreeView();
}

CentralWidget* Core::getCentralWidget() const {
    return this->mainWindow->getCentralWidget();
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void Core::statusBarFileSizeUpdateSlot(StatusBarUpdateType statusBarUpdateType) {

    if (statusBarUpdateType == Reset) {
        // reset the status bar :
        this->clientsObserver->fullFileSizeUpdate(0, 0);
    }

    if (statusBarUpdateType == Incremental) {
        this->statusBarFileSizeUpdate();
    }
}


void Core::serverStatisticsUpdateSlot(const int serverId) {

    this->getSideBar()->serverStatisticsUpdate(serverId);
}


void Core::downloadWaitingPar2Slot() {

    this->statusBarFileSizeUpdate();
    this->emitDataHasArrived();

}


void Core::saveFileErrorSlot(const int fromProcessing) {

    this->actionsManager->setStartPauseDownloadAllItems(UtilityNamespace::PauseStatus);
    this->getCentralWidget()->saveFileError(fromProcessing);
}


void Core::extractPasswordRequiredSlot(QString currentArchiveFileName) {

    bool passwordEntered;
    QString password = this->getCentralWidget()->extractPasswordRequired(currentArchiveFileName, passwordEntered);

    emit passwordEnteredByUserSignal(passwordEntered, password);

}


void Core::updateSettingsSlot() {

    // delegate specific settings to concerned object  :
    emit settingsChangedSignal();

}
