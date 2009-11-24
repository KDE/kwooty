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

#include "segmentsdecoderthread.h"

#include <KDebug>
#include <QDir>
#include "centralwidget.h"
#include "segmentdecoder.h"
#include "itemdownloadupdater.h"
#include "data/segmentdata.h"
#include "itemparentupdater.h"
#include "segmentmanager.h"
#include "settings.h"


SegmentsDecoderThread::SegmentsDecoderThread(){}


SegmentsDecoderThread::SegmentsDecoderThread(CentralWidget* inParent) : QThread(inParent) {

    this->parent = inParent;

    // start current thread :
    QTimer::singleShot(0, this, SLOT(start()));
}


SegmentsDecoderThread::~SegmentsDecoderThread() {
    quit();
    wait();
    delete segmentDecoder;
}


void SegmentsDecoderThread::run() {

    // create decoder instance :
    segmentDecoder = new SegmentDecoder();

    // create incoming data monitoring timer :
    decoderTimer = new QTimer();

    // setup connections :
    this->setupConnections();

    // start timer :
    decoderTimer->start(100);

    this->exec();
}


void SegmentsDecoderThread::setupConnections() {
    
    // check if there are data to decode :
    connect (this->decoderTimer ,
             SIGNAL(timeout()),
             this,
             SLOT(startDecodingSlot()),
             Qt::DirectConnection);

    // receive data to decode from decodeSegmentsSignal :
    qRegisterMetaType<NzbFileData>("NzbFileData");
    connect (parent->getItemParentUpdater()->getItemDownloadUpdater(),
             SIGNAL(decodeSegmentsSignal(NzbFileData)),
             this,
             SLOT(decodeSegmentsSlot(NzbFileData)),
             Qt::QueuedConnection);


    // update user interface about current decoding status :
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<UtilityNamespace::ItemStatus>("UtilityNamespace::ItemStatus");
    connect (segmentDecoder ,
             SIGNAL(updateDecodeSignal(QVariant, int, UtilityNamespace::ItemStatus, QString)),
             this,
             SIGNAL(updateDecodeSignal(QVariant, int, UtilityNamespace::ItemStatus, QString)));


    connect (segmentDecoder,
             SIGNAL(saveFileErrorSignal(int)),
             parent,
             SLOT(saveFileErrorSlot(int)),
             Qt::QueuedConnection);



}



void SegmentsDecoderThread::startDecodingSlot() {

    // if no data is currently being decoded :
    if (!segmentDecoder->isDecoding() && !nzbFileDataList.isEmpty()) {

        // get nzbfiledata to decode from list
        mutex.lock();
        NzbFileData currentFileDataToDecode = nzbFileDataList.takeFirst();
        mutex.unlock();

        // decode data :
        segmentDecoder->decodeSegments(currentFileDataToDecode);

    }

}




void SegmentsDecoderThread::decodeSegmentsSlot(NzbFileData nzbFileData) {

    // add nzbfiledata to the queue :
    QMutexLocker locker(&mutex);
    nzbFileDataList.append(nzbFileData);

}



void SegmentsDecoderThread::suppressOldOrphanedSegmentsSlot() {

    // get temporary path :
    QString tempPathStr = Settings::temporaryFolder().path();
    QDir temporaryFolder(tempPathStr);

    // get file list from temporary path :
    QStringList segmentFilelist = temporaryFolder.entryList();

    // if file is a previous segment, suppress it :
    QFile currentSegment;
    foreach (QString currentFileStr , segmentFilelist) {

        currentSegment.setFileName(tempPathStr + "/" + currentFileStr);

        if (currentSegment.exists()) {
            // open file
            currentSegment.open(QIODevice::ReadOnly);

            // check that the file has been downloaded by this application, if this is the case suppress it :
            if (currentSegment.peek(applicationFileOwner.size()) == applicationFileOwner) {
                currentSegment.close();
                currentSegment.remove();
            }
            currentSegment.close();
        }

    }

}




