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

#include <QFileInfo>

#include "centralwidget.h"
#include "fileoperations.h"


FolderWatcher::FolderWatcher(CentralWidget* centralWidget) :  QObject(centralWidget) {

    this->centralWidget = centralWidget;

    this->kDirWatch = new KDirWatch(this);

    this->kDirWatch->addDir("/mnt/Potam/Download", KDirWatch::WatchFiles);


    this->fileCompleteTimer = new QTimer(this);
    this->fileCompleteTimer->start(1500);


    // send remaining files to status bar :
    connect (this->kDirWatch,
             SIGNAL(created(const QString&)),
             this,
             SLOT(watchFileSlot(const QString& )));


    connect (this->kDirWatch,
             SIGNAL(dirty(const QString&)),
             this,
             SLOT(watchFileSlot(const QString& )));

    connect(this->fileCompleteTimer, SIGNAL(timeout()), this, SLOT(fileCompleteTimerSlot()));



}




void FolderWatcher::watchFileSlot(const QString& filePath) {


    kDebug() << filePath;

    // filter by .nzb extension :
    if (filePath.endsWith(".nzb", Qt::CaseInsensitive)) {

        if (!this->nzbFileList.contains(filePath)) {


            if (this->nzbFileList.size() > 10) {
                this->nzbFileList.takeFirst();
            }

            this->nzbFileList.append(filePath);


        }

    }

}



void FolderWatcher::fileCompleteTimerSlot() {

    QStringList pendingFileList;

    while (!this->nzbFileList.isEmpty()) {


        QString nzbFilePath = this->nzbFileList.takeFirst();

        QFileInfo fileInfo(nzbFilePath);


        QDateTime lastModifiedDateTime = fileInfo.lastModified();

        if ( lastModifiedDateTime.secsTo(QDateTime::currentDateTime()) > 1 ) {

            QFile file(nzbFilePath);

            // open the nzb file :
            if (!file.open(QIODevice::ReadOnly)){

                // do not display messages as it could be used as a service, user would not be informed
                // about that
                // only set a debug message :
                kDebug() << "File " << nzbFilePath << " can not be open !";

            }
            else {

                if (file.readAll().contains("</nzb>")) {

                    this->centralWidget->getFileOperations()->openFileWithFileMode(KUrl(nzbFilePath), UtilityNamespace::OpenWith);

                }
                else {

                    pendingFileList.append(nzbFilePath);
                }


            }


        }

        else {

            pendingFileList.append(nzbFilePath);
        }

    }

    this->nzbFileList = pendingFileList;


}
