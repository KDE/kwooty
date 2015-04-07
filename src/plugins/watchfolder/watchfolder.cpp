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


#include "watchfolder.h"

#include "kwooty_debug.h"
#include <kdirwatch.h>
#include <klocale.h>

#include <QFileInfo>
#include <QDir>

#include "mainwindow.h"
#include "core.h"
#include "watchfolderplugin.h"
#include "kwooty_watchfoldersettings.h"
#include "fileoperations.h"


WatchFolder::WatchFolder(WatchFolderPlugin* parent) :  QObject(parent) {

    this->core = parent->getMainWindow()->getCore();

    this->kDirWatch = new KDirWatch(this);

    // init folder to watch :
    this->settingsChanged();

    // start nzb file process timer
    this->fileCompleteTimer = new QTimer(this);
    this->fileCompleteTimer->start(500);

    // setup signals/slots connections :
    this->setupConnections();

}


WatchFolder::~WatchFolder() {

}


void WatchFolder::setupConnections() {

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


}



QSet<QString> WatchFolder::getNzbFileSetFromWatchFolder() {

    QDir watchFolderDir(WatchFolderSettings::watchFolder().path());
    QStringList filters;
    filters.append("*.nzb");
    filters.append("*.NZB");

    // order list of nzb files by date and return the last modified 10 last files :
    return watchFolderDir.entryList(filters, QDir::Files, QDir::Time).mid(0, MAX_LIST_SIZE).toSet();

}



void WatchFolder::appendFileToList(const QString& filePath) {

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


void WatchFolder::watchFileSlot(const QString& filePath) {

    // filter by .nzb extension :
    if (filePath.endsWith(".nzb", Qt::CaseInsensitive)) {

        this->firstEnqueueMethod = true;

        this->appendFileToList(filePath);
    }

    // nzb file name may not be returned if FAM or INotify is not used. filePath will only
    // contain the watch folder path, this is a workaround for this issue :
    else if (filePath == WatchFolderSettings::watchFolder().path()) {

        this->firstEnqueueMethod = false;

        QSet<QString> currentNzbFileInWatchFolderSet = this->getNzbFileSetFromWatchFolder();

        // filter with only new nzb files added from watch folder :
        QSet<QString> newNzbFiles = currentNzbFileInWatchFolderSet.subtract(this->nzbFileInWatchFolderSet);

        foreach (const QString& nzbFile, newNzbFiles) {

            QString nzbfilePath = Utility::buildFullPath(WatchFolderSettings::watchFolder().path(), nzbFile);
            this->appendFileToList(nzbfilePath);

        }

        // update list of nzb files in watch folder directory :
        this->nzbFileInWatchFolderSet = this->getNzbFileSetFromWatchFolder();

    }

}



void WatchFolder::fileCompleteTimerSlot() {

    QStringList pendingFileList;

    // check if nzb files contained is the list are complete :
    foreach (const QString& nzbFilePath, this->nzbFileList) {

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
                    if (WatchFolderSettings::copyNzbFromWatch()) {
                        openFileMode = UtilityNamespace::OpenWith;
                    }

                    // open the nzb file and launch download :
                    this->core->getFileOperations()->openFileWithFileMode(KUrl(nzbFilePath), openFileMode);

                    // check if .nzb has to be removed from watch folder:
                    if (WatchFolderSettings::suppressNzbFromWatch()) {
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
        this->nzbFileInWatchFolderSet = this->getNzbFileSetFromWatchFolder();
    }


    // update nzb pending list :
    this->nzbFileList = pendingFileList;

}



void WatchFolder::settingsChanged() {

    // reload settings from just saved config file :
    WatchFolderSettings::self()->load();


    // if directory to watch has been changed, update it :
    if (WatchFolderSettings::watchFolder().path() != this->currentWatchDir) {

        // remove previous watched folder :
        if (!this->currentWatchDir.isEmpty()) {
            this->kDirWatch->removeDir(this->currentWatchDir);
        }

        // get the new one :
        this->currentWatchDir = WatchFolderSettings::watchFolder().path();

        // update the kdirwatch with the new folder :
        this->kDirWatch->addDir(this->currentWatchDir, KDirWatch::WatchFiles);

        // retrieve nzb file list from watch folder at this stage :
        this->nzbFileInWatchFolderSet = this->getNzbFileSetFromWatchFolder();

        // by default consider that the first enqueue method is used :
        this->firstEnqueueMethod = true;

    }

    // start monitoring :
    this->kDirWatch->startScan();

}


