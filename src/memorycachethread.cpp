/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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


#include "memorycachethread.h"

#include <KDebug>
#include <QBuffer>

#include "centralwidget.h"
#include "itemparentupdater.h"
#include "itemdownloadupdater.h"
#include "data/segmentdata.h"
#include "serverspeedmanager.h"
#include "kwootysettings.h"


MemoryCacheThread::MemoryCacheThread(CentralWidget* inParent) {

    this->parent = inParent;

    this->init();

    this->dedicatedThread = new QThread();
    this->moveToThread(this->dedicatedThread);

    // start current thread :
    this->dedicatedThread->start();

}


MemoryCacheThread::~MemoryCacheThread() {

    kDebug() << "WRITE TO DISK";
    this->writeDataToDisk(SaveSilently);
    this->dedicatedThread->quit();
    this->dedicatedThread->wait();

    delete this->dedicatedThread;

}


void MemoryCacheThread::init() {

    this->cacheSizeInBytes = 0;

    // setup connections :
    this->setupConnections();

}


void MemoryCacheThread::setupConnections() {

    // receive data to decode from decodeSegmentsSignal :
    qRegisterMetaType<NzbFileData>("NzbFileData");
    connect (parent->getItemParentUpdater()->getItemDownloadUpdater(),
             SIGNAL(decodeSegmentsSignal(NzbFileData)),
             this,
             SLOT(decodeSegmentsSlot(NzbFileData)));


    connect (this,
             SIGNAL(saveFileErrorSignal(const int)),
             parent,
             SLOT(saveFileErrorSlot(const int)));

}



void MemoryCacheThread::writeDataToDisk(const SaveFileBehavior saveFileBehavior) {


    foreach(const QString fileName, this->fileNameByteArrayMap.keys()) {

        QString temporaryFolder = Settings::temporaryFolder().path() + '/';

        QBuffer* downloadedSegment = qobject_cast<QBuffer*>(this->fileNameByteArrayMap.take(fileName));
        bool isSaved  = Utility::saveData(temporaryFolder, fileName, (*downloadedSegment).data());

        // send save error notification, only if kwooty is not being quitting :
        if ( !isSaved &&
             saveFileBehavior == SaveNotSilently ) {

            // send save error notification :
            emit saveFileErrorSignal(DuringDownload);
        }

        delete downloadedSegment;

    }

    this->cacheSizeInBytes = 0;

}



void MemoryCacheThread::decodeSegmentsSlot(NzbFileData nzbFileData) {

    QList<SegmentData> segmentList = nzbFileData.getSegmentList();

    for (int segmentIndex = 0; segmentIndex < segmentList.size(); segmentIndex++) {

        SegmentData segmentData = segmentList.value(segmentIndex);
        segmentData.setIoDevice(this->retrieveDataFile(nzbFileData.getFileSavePath(), segmentData.getPart()));

        // update parent item data :
        segmentList.replace(segmentIndex, segmentData);
        nzbFileData.setSegmentList(segmentList);

    }

    emit decodeSegmentsFromMemoryCacheSignal(nzbFileData);

}



QIODevice* MemoryCacheThread::retrieveDataFile(const QString& fileSavePath, const QString& fileName) {

    QIODevice* ioDevice;

    if (this->fileNameByteArrayMap.contains(fileName)) {

        ioDevice = this->fileNameByteArrayMap.take(fileName);

    }
    else {

        //kDebug() << "FILE LOADED FROM DISK !!!" << Settings::temporaryFolder().path() + '/' + fileName;
        ioDevice = qobject_cast<QIODevice*>(new QFile(Settings::temporaryFolder().path() + '/' + fileName));

    }

    return ioDevice;
}



void MemoryCacheThread::saveDownloadedSegmentSlot(QString fileName, QIODevice* downloadedBuffer) {

    this->fileNameByteArrayMap.insert(fileName, downloadedBuffer);
    this->cacheSizeInBytes += downloadedBuffer->size();

    // clear the cache if it reaches the limit :
    if (this->cacheSizeInBytes > static_cast<quint64>(200 * NBR_BYTES_IN_MB)) {

        kDebug() << "WRITE TO DISK";
        this->writeDataToDisk();

    }


}





