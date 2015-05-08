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

#include "kwooty_debug.h"
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

Core::Core(MainWindow *mainWindow) : QObject(mainWindow)
{

    this->mMainWindow = mainWindow;

    this->mNzbFileHandler = new NzbFileHandler(this);

    // init the downloadModel :
    this->mDownloadModel = new StandardItemModel(this);

    // query model according to items status :
    this->mModelQuery = new StandardItemModelQuery(this);

    // init queue file observer :
    this->mQueueFileObserver = new QueueFileObserver(this);

    // collect connection statuses from clients and build stats info :
    this->mClientsObserver = new ClientsObserver(this);

    // update view according to items data :
    this->mItemParentUpdater = new ItemParentUpdater(this);

    // manage dispatching / updating segments related to one item :
    this->mSegmentManager = new SegmentManager(this);

    // save and restore pending downloads from previous session :
    this->mDataRestorer = new DataRestorer(this);

    // setup segment decoder thread :
    this->mSegmentsDecoderThread = new SegmentsDecoderThread(this);

    // setup repairing and decompressing thread :
    this->mRepairDecompressThread = new RepairDecompressThread(this);

    // handle nntp clients with one or several servers :
    this->mServerManager = new ServerManager(this);

    // set download and temp folders into home dir if not specified by user :
    this->initFoldersSettings();

    // setup shutdown manager :
    this->mShutdownManager = new ShutdownManager(this);

    // setup nzb file opening closing :
    this->mFileOperations = new FileOperations(this);

    // setup notification events :
    this->mNotificationManager = new NotificationManager(this);

    // setup actions manager :
    this->mActionsManager = new ActionsManager(this);

}

Core::~Core()
{

    delete this->mSegmentsDecoderThread;
    delete this->mRepairDecompressThread;

}

void Core::emitDataHasArrived(const QModelIndex &appendedIndex)
{

    // first notify that data have just be appended for pre-processing :
    emit dataAboutToArriveSignal(appendedIndex);

    // notify nntp clients that data is now ready to be downloaded :
    emit dataHasArrivedSignal();

}

void Core::handleNzbFile(QFile &file, const QString &nzbName, const QList<GlobalFileData> &inGlobalFileDataList)
{

    bool normalNzbFileProcessing = inGlobalFileDataList.isEmpty();

    QList<GlobalFileData> globalFileDataList;

    // if list is empty it corresponds to a normal nzb file processing :
    if (normalNzbFileProcessing) {

        // parse the xml file and add elements to the model :
        globalFileDataList = this->mNzbFileHandler->processNzbFile(file, nzbName);

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
        int widthInPixel = this->mMainWindow->getTreeView()->fontMetrics().width(nzbName) + 100;

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

int Core::savePendingDownloads(UtilityNamespace::SystemShutdownType systemShutdownType, const SaveFileBehavior saveFileBehavior)
{

    int answer = this->mDataRestorer->saveQueueData(saveFileBehavior);

    // disable dataRestorer when shutdown is requested because the app will be closed automatically
    // data saving will be triggered and then data saving dialog box could prevent system shutdown :
    if (systemShutdownType == UtilityNamespace::Shutdown) {
        this->mDataRestorer->setActive(false);
    }

    return answer;
}

void Core::restoreDataFromPreviousSession(const QList<GlobalFileData> &globalFileDataList)
{

    // instantiate a QFile to only get the name of the nzb needed by handleNzbFile();
    NzbFileData nzbFileData = globalFileDataList.at(0).getNzbFileData();
    QFile nzbFile(nzbFileData.getNzbName());

    // populate treeView with saved data :
    this->handleNzbFile(nzbFile, nzbFileData.getNzbName(), globalFileDataList);

    // update parent status to the same value as previous session :
    for (int i = 0; i < this->mDownloadModel->rowCount(); ++i) {

        // retrieve nzb parent item :
        QStandardItem *parentFileNameItem = this->mDownloadModel->item(i, FILE_NAME_COLUMN);
        this->mItemParentUpdater->updateNzbItems(parentFileNameItem->index());

    }

    // notify that data has just been restored :
    this->emitDataHasArrived();

}

QModelIndex Core::setDataToModel(const QList<GlobalFileData> &globalFileDataList, const QString &nzbName)
{

    QStandardItem *nzbNameItem = new QStandardItem(nzbName);
    QStandardItem *nzbStateItem = new QStandardItem();
    QStandardItem *nzbSizeItem = new QStandardItem();

    // add the nzb items to the model :
    int nzbNameItemRow = this->mDownloadModel->rowCount();

    this->mDownloadModel->setItem(nzbNameItemRow, FILE_NAME_COLUMN, nzbNameItem);
    this->mDownloadModel->setItem(nzbNameItemRow, SIZE_COLUMN, nzbSizeItem);
    this->mDownloadModel->setItem(nzbNameItemRow, STATE_COLUMN, nzbStateItem);
    this->mDownloadModel->setItem(nzbNameItemRow, PROGRESS_COLUMN, new QStandardItem());

    quint64 nzbFilesSize = 0;
    int par2FileNumber = 0;
    bool badCrc = false;
    NzbFileData nzbFileData;

    // add children to parent (nzbNameItem) :
    foreach (const GlobalFileData &currentGlobalFileData, globalFileDataList) {

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
    this->mDownloadModel->updateParentFileSavePathFromIndex(nzbNameItem->index(), nzbFileData);

    // set idle status by default :
    this->mDownloadModel->initStatusDataToItem(nzbStateItem, ItemStatusData());

    // set size :
    nzbSizeItem->setData(qVariantFromValue(nzbFilesSize), SizeRole);

    // expand treeView :
    this->mMainWindow->getTreeView()->setExpanded(nzbNameItem->index(), Settings::expandTreeView());

    // alternate row color :
    this->mMainWindow->getTreeView()->setAlternatingRowColors(Settings::alternateColors());

    // enable / disable smart par2 download :
    if (!badCrc && Settings::smartPar2Download() && (par2FileNumber < globalFileDataList.size())) {

        this->mActionsManager->changePar2FilesStatus(nzbNameItem->index(), WaitForPar2IdleStatus);

    }

    return nzbNameItem->index();

}

void Core::addParentItem(QStandardItem *nzbNameItem, const GlobalFileData &currentGlobalFileData)
{

    // get the current row of the nzbName item :
    int nzbNameItemNextRow = nzbNameItem->rowCount();

    const NzbFileData currentNzbFileData  = currentGlobalFileData.getNzbFileData();

    // add the (consice) file name as parent's item :
    QStandardItem *fileNameItem = new QStandardItem(this->getTreeView()->getDisplayedFileName(currentNzbFileData));
    nzbNameItem->setChild(nzbNameItemNextRow, FILE_NAME_COLUMN, fileNameItem);

    QStandardItem *parentStateItem = new QStandardItem();
    nzbNameItem->setChild(nzbNameItemNextRow, STATE_COLUMN, parentStateItem);

    QStandardItem *parentSizeItem = new QStandardItem();
    nzbNameItem->setChild(nzbNameItemNextRow, SIZE_COLUMN, parentSizeItem);

    QStandardItem *parentProgressItem = new QStandardItem();
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
    this->mDownloadModel->initStatusDataToItem(parentStateItem, currentGlobalFileData.getItemStatusData());

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

void Core::statusBarFileSizeUpdate()
{

    quint64 size = 0;
    quint64 files = 0;

    // get the root item :
    QStandardItem *rootItem = mDownloadModel->invisibleRootItem();

    // parse nzb items :
    for (int i = 0; i < rootItem->rowCount(); ++i) {

        QStandardItem *nzbItem = rootItem->child(i);

        // parse nzb children :
        for (int j = 0; j < nzbItem->rowCount(); j++) {

            QStandardItem *statusItem = nzbItem->child(j, STATE_COLUMN);

            // if the item is pending (Idle, Download, Paused, Pausing states), processes it :
            UtilityNamespace::ItemStatus status = mDownloadModel->getStatusFromStateItem(statusItem);

            if (Utility::isInDownloadProcess(status))  {

                QStandardItem *sizeItem = nzbItem->child(j, SIZE_COLUMN);
                size += sizeItem->data(SizeRole).toULongLong();
                files++;
            }
        }

    }

    this->mClientsObserver->fullFileSizeUpdate(size, files);

}

void Core::initFoldersSettings()
{

    // set default path for download and temporary folders if not filled by user :
    if (Settings::completedFolder().path().isEmpty()) {
        Settings::setCompletedFolder(Utility::buildFullPath(QDir::homePath(), "kwooty/Download"));
    }

    if (Settings::temporaryFolder().path().isEmpty()) {
        Settings::setTemporaryFolder(Utility::buildFullPath(QDir::homePath(), "kwooty/Temp"));
    }

}

MainWindow *Core::getMainWindow() const
{
    return this->mMainWindow;
}

SegmentManager *Core::getSegmentManager() const
{
    return this->mSegmentManager;
}

NzbFileHandler *Core::getNzbFileHandler() const
{
    return this->mNzbFileHandler;
}

StandardItemModel *Core::getDownloadModel() const
{
    return this->mDownloadModel;
}

StandardItemModelQuery *Core::getModelQuery() const
{
    return this->mModelQuery;
}

ItemParentUpdater *Core::getItemParentUpdater() const
{
    return this->mItemParentUpdater;
}

ShutdownManager *Core::getShutdownManager() const
{
    return this->mShutdownManager;
}

ClientsObserver *Core::getClientsObserver() const
{
    return this->mClientsObserver;
}

FileOperations *Core::getFileOperations() const
{
    return this->mFileOperations;
}

QueueFileObserver *Core::getQueueFileObserver() const
{
    return this->mQueueFileObserver;
}

DataRestorer *Core::getDataRestorer() const
{
    return this->mDataRestorer;
}

ServerManager *Core::getServerManager() const
{
    return this->mServerManager;
}

SegmentsDecoderThread *Core::getSegmentsDecoderThread() const
{
    return this->mSegmentsDecoderThread;
}

ActionsManager *Core::getActionsManager() const
{
    return this->mActionsManager;
}

SideBar *Core::getSideBar() const
{
    return this->mMainWindow->getSideBar();
}

MyTreeView *Core::getTreeView() const
{
    return this->mMainWindow->getTreeView();
}

CentralWidget *Core::getCentralWidget() const
{
    return this->mMainWindow->getCentralWidget();
}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void Core::statusBarFileSizeUpdateSlot(StatusBarUpdateType statusBarUpdateType)
{

    if (statusBarUpdateType == Reset) {
        // reset the status bar :
        this->mClientsObserver->fullFileSizeUpdate(0, 0);
    }

    if (statusBarUpdateType == Incremental) {
        this->statusBarFileSizeUpdate();
    }
}

void Core::serverStatisticsUpdateSlot(const int serverId)
{

    this->getSideBar()->serverStatisticsUpdate(serverId);
}

void Core::downloadWaitingPar2Slot()
{

    this->statusBarFileSizeUpdate();
    this->emitDataHasArrived();

}

void Core::saveFileErrorSlot(const int fromProcessing)
{

    this->mActionsManager->setStartPauseDownloadAllItems(UtilityNamespace::PauseStatus);
    this->getCentralWidget()->saveFileError(fromProcessing);
}

void Core::extractPasswordRequiredSlot(const QString &currentArchiveFileName)
{

    bool passwordEntered;
    QString password = this->getCentralWidget()->extractPasswordRequired(currentArchiveFileName, passwordEntered);

    emit passwordEnteredByUserSignal(passwordEntered, password);

}

void Core::updateSettingsSlot()
{

    // delegate specific settings to concerned object  :
    emit settingsChangedSignal();

}
