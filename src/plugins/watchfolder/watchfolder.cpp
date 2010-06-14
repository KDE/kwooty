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

#include <KDebug>
#include <kdirwatch.h>
#include <klocale.h>

#include <QFileInfo>

#include "centralwidget.h"
#include "watchfoldersettings.h"
#include "fileoperations.h"


WatchFolder::WatchFolder(CentralWidget* centralWidget) :  QObject(centralWidget) {

    this->centralWidget = centralWidget;

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

    kDebug();
    delete this->kDirWatch;
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



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void WatchFolder::watchFileSlot(const QString& filePath) {

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



void WatchFolder::fileCompleteTimerSlot() {

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
                    if (WatchFolderSettings::copyNzbFromWatch()) {
                        openFileMode = UtilityNamespace::OpenWith;
                    }

                    // open the nzb file and launch download :
                    this->centralWidget->getFileOperations()->openFileWithFileMode(KUrl(nzbFilePath), openFileMode);

                    // check if .nzb has to be removed from watch folder:
                    if (WatchFolderSettings::suppressNzbFromWatch()) {
                        QFile::remove(nzbFilePath);
                    }

                    // file has been enqueued :
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

    // update nzb pending list :
    this->nzbFileList = pendingFileList;


}



void WatchFolder::settingsChanged() {

    // reload settings from just saved config file :
    WatchFolderSettings::self()->readConfig();


    // if directory to watch has been changed, update it :
    if (WatchFolderSettings::watchFolder().path() != this->currentWatchDir) {

        // remove previous watched folder :
        this->kDirWatch->removeDir(this->currentWatchDir);

        // get the new one :
        this->currentWatchDir = WatchFolderSettings::watchFolder().path();

        // update the kdirwatch with the new folder :
        this->kDirWatch->addDir(this->currentWatchDir, KDirWatch::WatchFiles);

    }

    // start monitoring :
    this->kDirWatch->startScan();

}
