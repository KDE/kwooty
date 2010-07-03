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
#include "segmentdecoderyenc.h"
#include "segmentdecoderuuenc.h"
#include "itemdownloadupdater.h"
#include "data/segmentdata.h"
#include "itemparentupdater.h"
#include "segmentmanager.h"
#include "datarestorer.h"
#include "kwootysettings.h"


SegmentsDecoderThread::SegmentsDecoderThread(){}


SegmentsDecoderThread::SegmentsDecoderThread(CentralWidget* inParent) {

    this->parent = inParent;

    this->init();

    this->dedicatedThread = new QThread();
    this->moveToThread(this->dedicatedThread);

    // start current thread :
    this->dedicatedThread->start();

}


SegmentsDecoderThread::~SegmentsDecoderThread() {

    this->dedicatedThread->quit();
    this->dedicatedThread->wait();

    delete this->dedicatedThread;

}


void SegmentsDecoderThread::init() {

    // create Yenc decoder instance :
    this->segmentDecoderList.append(new SegmentDecoderYEnc(this));

    // create UU decoder instance :
    this->segmentDecoderList.append(new SegmentDecoderUUEnc(this));

    // init encoder instance :
    this->currentDecoderElement = 0;

    // no decoding at startup :
    this->currentlyDecoding = false;

    // setup connections :
    this->setupConnections();

}


void SegmentsDecoderThread::setupConnections() {
        
    // suppress old segments if user have to chosen to not reload data from previous session :
    connect (parent->getDataRestorer(),
             SIGNAL(suppressOldOrphanedSegmentsSignal()),
             this,
             SLOT(suppressOldOrphanedSegmentsSlot()));


    // receive data to decode from decodeSegmentsSignal :
    qRegisterMetaType<NzbFileData>("NzbFileData");
    connect (parent->getItemParentUpdater()->getItemDownloadUpdater(),
             SIGNAL(decodeSegmentsSignal(NzbFileData)),
             this,
             SLOT(decodeSegmentsSlot(NzbFileData)));


    // update user interface about current decoding status :
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<UtilityNamespace::ItemStatus>("UtilityNamespace::ItemStatus");


    // for each decoders connect update signals :
    foreach (SegmentDecoderBase* currentSegmentDecoder, this->segmentDecoderList) {


        // update info about decoding process :
        connect (currentSegmentDecoder,
                 SIGNAL(updateDecodeSignal(QVariant, int, UtilityNamespace::ItemStatus, QString, bool)),
                 this->parent->getSegmentManager(),
                 SLOT(updateDecodeSegmentSlot(QVariant, int, UtilityNamespace::ItemStatus, QString, bool)));


        connect (currentSegmentDecoder,
                 SIGNAL(saveFileErrorSignal(int)),
                 parent,
                 SLOT(saveFileErrorSlot(int)));

    }


}



void SegmentsDecoderThread::startDecoding() {

    // if pending segments are present and check if decoding is currently processed :
    if (!this->currentlyDecoding) {

        while (!nzbFileDataList.isEmpty()) {

            // decoding begins :
            this->currentlyDecoding = true;

            // get nzbfiledata to decode from list
            NzbFileData currentFileDataToDecode = nzbFileDataList.takeFirst();

            // decode data :
            int decoderNumber = 0;
            QString fileNameStr;

            // apply a round robin in order to select the proper decoder (YDec or UUDec) :
            while ( fileNameStr.isEmpty() && (decoderNumber++ < this->segmentDecoderList.size()) ) {

                // scan segments with the current decoder :
                fileNameStr = this->segmentDecoderList.at(this->currentDecoderElement)->scanSegmentFiles(currentFileDataToDecode);

                // if fileName is not empty, decode segments with the current decoder :
                if (!fileNameStr.isEmpty()) {
                    this->segmentDecoderList.at(this->currentDecoderElement)->decodeSegments(currentFileDataToDecode, fileNameStr);
                }
                // else scan segments with other decoder(s) at next iteration :
                else {
                    this->currentDecoderElement = (this->currentDecoderElement + 1) % this->segmentDecoderList.size();
                }

            }

            // if fileName is empty after trying all decoders, decoding failed :
            if (fileNameStr.isEmpty()) {
                emit updateDecodeSignal(currentFileDataToDecode.getUniqueIdentifier(), PROGRESS_COMPLETE, DecodeErrorStatus, QString(), false);
            }


            // decoding is over :
            this->currentlyDecoding = false;

        }
    }

}




void SegmentsDecoderThread::decodeSegmentsSlot(NzbFileData nzbFileData) {

    // add nzbfiledata to the queue :
    nzbFileDataList.append(nzbFileData);

    this->startDecoding();

}



void SegmentsDecoderThread::suppressOldOrphanedSegmentsSlot() {


    // get temporary path :
    QString tempPathStr = Settings::temporaryFolder().path();
    QDir temporaryFolder(tempPathStr);

    // get file list from temporary path :
    QStringList segmentFilelist = temporaryFolder.entryList();

    // if file is a previous segment, suppress it :
    QFile currentSegment;
    foreach (QString currentFileStr, segmentFilelist) {


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




