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


#include "autoretry.h"

#include <KDebug>
#include <kdirwatch.h>
#include <klocale.h>

#include <QFileInfo>
#include <QDir>

#include "mainwindow.h"
#include "centralwidget.h"
#include "standarditemmodel.h"
#include "autoretryplugin.h"
#include "observers/queuefileobserver.h"
#include "data/itemstatusdata.h"
#include "kwooty_autoretrysettings.h"
#include "fileoperations.h"


AutoRetry::AutoRetry(AutoRetryPlugin* parent) :  QObject(parent) {

    this->centralWidget = parent->getCore()->getCentralWidget();

    this->kDirWatch = new KDirWatch(this);

    // init folder to watch :
    this->settingsChanged();

    // start nzb file process timer
    this->fileCompleteTimer = new QTimer(this);
    this->fileCompleteTimer->start(500);

    // setup signals/slots connections :
    this->setupConnections();

}


AutoRetry::~AutoRetry() {

}


void AutoRetry::setupConnections() {

    // kDirWatch notify that a file has been created :
    connect (this->kDirWatch,
             SIGNAL(created(const QString&)),
             this,
             SLOT(watchFileSlot(const QString& )));


    // kDirWatch notify that a file has changed :
    connect (this->kDirWatch,
             SIGNAL(dirty(const QString&)),
             this,
             SLOT(watchFileSlot(const QString& )));

    // when time-out occurs, check if nzb files from list can be processed
    connect(this->fileCompleteTimer,
            SIGNAL(timeout()),
            this,
            SLOT(fileCompleteTimerSlot()));


    connect(this->centralWidget->getDownloadModel(),
            SIGNAL(parentStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
            this,
            SLOT(parentStatusItemChangedSlot(QStandardItem*)));

    connect(this->centralWidget->getDownloadModel(),
            SIGNAL(childStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
            this,
            SLOT(childStatusItemChangedSlot(QStandardItem*)));


}



QSet<QString> AutoRetry::getNzbFileSetFromAutoRetry() {

    QDir watchFolderDir(AutoRetrySettings::watchFolder().path());
    QStringList filters;
    filters.append("*.nzb");
    filters.append("*.NZB");

    // order list of nzb files by date and return the last modified 10 last files :
    return watchFolderDir.entryList(filters, QDir::Files, QDir::Time).mid(0, MAX_LIST_SIZE).toSet();

}



void AutoRetry::appendFileToList(const QString& filePath) {

    bool allowEnqueue = true;

    // if file has already been enqueued :
    if (this->nzbFilePathlastEnqueuedMap.keys().contains(filePath)) {

        // check that the file has been previously enqueued more than 2 seconds in order
        // to avoid double file enqueue :
        if (this->nzbFilePathlastEnqueuedMap.value(filePath).secsTo(QDateTime::currentDateTime()) < 2)  {
            allowEnqueue = false;
        }
        // else file can be removed from map :
        else {
            this->nzbFilePathlastEnqueuedMap.remove(filePath);

        }
    }


    // if nzb file is not contained in the list :
    if (!this->nzbFileList.contains(filePath) && allowEnqueue) {

        // keep a list with a max size of 10 :
        if (this->nzbFileList.size() > MAX_LIST_SIZE) {
            this->nzbFileList.takeFirst();
        }

        // append the nzb file to the list :
        this->nzbFileList.append(filePath);

    }

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void AutoRetry::parentStatusItemChangedSlot(QStandardItem* stateItem) {

    StandardItemModel* downloadModel = this->centralWidget->getDownloadModel();
//    ItemStatusData itemStatusData = downloadModel->getStatusDataFromIndex(stateItem->index());


//    if ( itemStatusData.getStatus() == VerifyFinishedStatus ||
//         itemStatusData.getStatus() == ExtractFinishedStatus ) {

//        bool triggerRetry = false;

//        // check for children whose post processing has failed :
//        for (int i = 0; i < stateItem->rowCount(); i++) {

//            QStandardItem* nzbChildrenItem = stateItem->child(i, FILE_NAME_COLUMN);
//            ItemStatusData itemStatusData = downloadModel->getStatusDataFromIndex(nzbChildrenItem->index());

//            if (Utility::isPostDownloadFailed(itemStatusData.getStatus())/* &&
//                    Utility::isProgramMissing(itemStatusData.getStatus()) */) {

//                triggerRetry = true;
//            }


//        }


//        if (triggerRetry) {

//            // select all rows in order to set them to paused or Idle :
//            QList<QModelIndex> indexesList;
//            indexesList.append(stateItem->index());

//            this->centralWidget->retryDownload(indexesList);

//        }

//    }


}


void AutoRetry::childStatusItemChangedSlot(QStandardItem* stateItem) {


//    StandardItemModel* downloadModel = this->centralWidget->getDownloadModel();
//    ItemStatusData itemStatusData = downloadModel->getStatusDataFromIndex(stateItem->index());


//    if ( Utility::isDecodeFinish(itemStatusData.getStatus()) &&
//         itemStatusData.getCrc32Match() != CrcOk ) {

//        // if nzb file does not contain any par2 files, reset in queue corrupted decoded file :
//        if (!this->centralWidget->getQueueFileObserver()->isParentContainsPar2File(stateItem)) {

//            // select all rows in order to set them to paused or Idle :
//            QList<QModelIndex> indexesList;
//            indexesList.append(stateItem->index());

//            this->centralWidget->retryDownload(indexesList);
//            kDebug();

//        }

//    }

}




























void AutoRetry::watchFileSlot(const QString& filePath) {

    // filter by .nzb extension :
    if (filePath.endsWith(".nzb", Qt::CaseInsensitive)) {

        this->firstEnqueueMethod = true;

        this->appendFileToList(filePath);
    }

    // nzb file name may not be returned if FAM or INotify is not used. filePath will only
    // contain the watch folder path, this is a workaround for this issue :
    else if (filePath == AutoRetrySettings::watchFolder().path()) {

        this->firstEnqueueMethod = false;

        QSet<QString> currentNzbFileInAutoRetrySet = this->getNzbFileSetFromAutoRetry();

        // filter with only new nzb files added from watch folder :
        QSet<QString> newNzbFiles = currentNzbFileInAutoRetrySet.subtract(this->nzbFileInAutoRetrySet);

        foreach (const QString& nzbFile, newNzbFiles) {

            QString nzbfilePath = AutoRetrySettings::watchFolder().path() + "/" +  nzbFile;
            this->appendFileToList(nzbfilePath);

        }

        // update list of nzb files in watch folder directory :
        this->nzbFileInAutoRetrySet = this->getNzbFileSetFromAutoRetry();

    }

}



void AutoRetry::fileCompleteTimerSlot() {

    QStringList pendingFileList;

    // check if nzb files contained is the list are complete :
    foreach(QString nzbFilePath, this->nzbFileList) {

        bool fileEnqueued = false;

        // get the last modified time of the current nzb file :
        QFileInfo fileInfo(nzbFilePath);
        QDateTime lastModifiedDateTime = fileInfo.lastModified();

        // if file has not been modified since 1 second :
        if ( lastModifiedDateTime.secsTo(QDateTime::currentDateTime()) > 1 ) {

            // open the nzb file :
            QFile file(nzbFilePath);
            if (file.open(QIODevice::ReadOnly)) {

                // if the end of the file pattern has been found :
                if (file.readAll().contains("</nzb>")) {

                    UtilityNamespace::OpenFileMode openFileMode = UtilityNamespace::OpenNormal;

                    // check if .nzb file has to be copied in destination download folder :
                    if (AutoRetrySettings::copyNzbFromWatch()) {
                        openFileMode = UtilityNamespace::OpenWith;
                    }

                    // open the nzb file and launch download :
                    this->centralWidget->getFileOperations()->openFileWithFileMode(KUrl(nzbFilePath), openFileMode);

                    // check if .nzb has to be removed from watch folder:
                    if (AutoRetrySettings::suppressNzbFromWatch()) {
                        QFile::remove(nzbFilePath);
                    }

                    // file has been enqueued :
                    this->nzbFilePathlastEnqueuedMap.insert(nzbFilePath, QDateTime::currentDateTime());

                    fileEnqueued = true;

                }

                // close the current file :
                file.close();
            }

        }


        // file has not been enqueued yet :
        if (!fileEnqueued) {
            pendingFileList.append(nzbFilePath);

        }

    }

    // if nzbname is not sent by dirty or created signal, manage nzb enqueuing diffrently :
    if (!this->firstEnqueueMethod) {
        this->nzbFileInAutoRetrySet = this->getNzbFileSetFromAutoRetry();
    }


    // update nzb pending list :
    this->nzbFileList = pendingFileList;

}



void AutoRetry::settingsChanged() {

    // reload settings from just saved config file :
    AutoRetrySettings::self()->readConfig();


    // if directory to watch has been changed, update it :
    if (AutoRetrySettings::watchFolder().path() != this->currentWatchDir) {

        // remove previous watched folder :
        if (!this->currentWatchDir.isEmpty()) {
            this->kDirWatch->removeDir(this->currentWatchDir);
        }

        // get the new one :
        this->currentWatchDir = AutoRetrySettings::watchFolder().path();

        // update the kdirwatch with the new folder :
        this->kDirWatch->addDir(this->currentWatchDir, KDirWatch::WatchFiles);

        // retrieve nzb file list from watch folder at this stage :
        this->nzbFileInAutoRetrySet = this->getNzbFileSetFromAutoRetry();

        // by default consider that the first enqueue method is used :
        this->firstEnqueueMethod = true;

    }

    // start monitoring :
    this->kDirWatch->startScan();

}


