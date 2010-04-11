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


#include "folderwatcher.h"

#include <KDebug>
#include <kdirwatch.h>
#include <klocale.h>

#include <QFileInfo>

#include "centralwidget.h"
#include "settings.h"
#include "fileoperations.h"


FolderWatcher::FolderWatcher(CentralWidget* centralWidget) :  QObject(centralWidget) {

    this->centralWidget = centralWidget;

    this->kDirWatch = new KDirWatch(this);

    // init folder to watch :
    this->settingsChangedSlot();

    // start nzb file process timer
    this->fileCompleteTimer = new QTimer(this);
    this->fileCompleteTimer->start(500);

    // setup signals/slots connections :
    this->setupConnections();

}


void FolderWatcher::setupConnections() {

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

    // when time out occurs, check if nzb files from list can be processed
    connect(this->fileCompleteTimer,
            SIGNAL(timeout()),
            this,
            SLOT(fileCompleteTimerSlot()));


    // parent notify all settings have changed :
    connect (this->centralWidget,
             SIGNAL(settingsChangedSignal()),
             this,
             SLOT(settingsChangedSlot()));

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void FolderWatcher::watchFileSlot(const QString& filePath) {

    // filter by .nzb extension :
    if (filePath.endsWith(".nzb", Qt::CaseInsensitive)) {

        // if nzb file is not contained in the list :
        if (!this->nzbFileList.contains(filePath)) {

            // keep a list with a max size of 10 :
            if (this->nzbFileList.size() > MAX_LIST_SIZE) {
                this->nzbFileList.takeFirst();
            }

            // append the nzb file to the list :
            this->nzbFileList.append(filePath);

        }
    }
}



void FolderWatcher::fileCompleteTimerSlot() {

    QStringList pendingFileList;

    // check if nzb files contained is the list are complete :
    while (!this->nzbFileList.isEmpty()) {

        QString nzbFilePath = this->nzbFileList.takeFirst();

        // get the last modified time of the current nzb file :
        QFileInfo fileInfo(nzbFilePath);
        QDateTime lastModifiedDateTime = fileInfo.lastModified();

        // if file has not been modified since 1 second :
        if ( lastModifiedDateTime.secsTo(QDateTime::currentDateTime()) > 1 ) {

            // open the nzb file :
            QFile file(nzbFilePath);
            if (file.open(QIODevice::ReadOnly)){

                // if the end of the file pattern has been found :
                if (file.readAll().contains("</nzb>")) {

                    UtilityNamespace::OpenFileMode openFileMode = UtilityNamespace::OpenNormal;

                    // check if .nzb file has to be copied in destination download folder :
                    if (Settings::copyNzbFromWatch()) {
                        openFileMode = UtilityNamespace::OpenWith;
                    }

                    // open the nzb file and launch download :
                    this->centralWidget->getFileOperations()->openFileWithFileMode(KUrl(nzbFilePath), openFileMode);

                    // check if .nzb has to be removed from watch folder:
                    if (Settings::suppressNzbFromWatch()) {
                        QFile::remove(nzbFilePath);
                    }

                }
                // .nzb end of file pattern has not been found yet, add it to pending list again :
                else {

                    pendingFileList.append(nzbFilePath);
                }
            }

            // file open has failed :
            else {
                // only set a debug message :
                kDebug() << "File " << nzbFilePath << " can not be opened !";

            }
        }
        // file has been modified before 1 second elapsed, append it again :
        else {

            pendingFileList.append(nzbFilePath);
        }

    }

    // update nzb pending list :
    this->nzbFileList = pendingFileList;


}



void FolderWatcher::settingsChangedSlot() {

    // if directory to watch has been changed, update it :
    if (Settings::watchFolder().path() != this->currentWatchDir) {

        // remove previous watched folder :
        this->kDirWatch->removeDir(this->currentWatchDir);

        // get the new one :
        this->currentWatchDir = Settings::watchFolder().path();

        // update the kdirwatch with the new folder :
        this->kDirWatch->addDir(this->currentWatchDir, KDirWatch::WatchFiles);

        kDebug() << this->currentWatchDir;

    }

    // if watch folder check box is enabled :
    if (Settings::groupBoxWatch()) {
        this->kDirWatch->startScan();
    }

    else {
        this->kDirWatch->stopScan();
    }

}
