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
#include <QBuffer>
#include "core.h"
#include "segmentdecoderyenc.h"
#include "segmentdecoderuuenc.h"
#include "itemdownloadupdater.h"
#include "data/segmentdata.h"
#include "data/postdownloadinfodata.h"
#include "itemparentupdater.h"
#include "segmentmanager.h"
#include "datarestorer.h"
#include "kwootysettings.h"


SegmentsDecoderThread::SegmentsDecoderThread(){}


SegmentsDecoderThread::SegmentsDecoderThread(Core* inParent) {

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
    this->yencDecoder = new SegmentDecoderYEnc(this);

    // create UU decoder instance :
    this->uuencDecoder = new SegmentDecoderUUEnc(this);

    // setup connections :
    this->setupConnections();

}


void SegmentsDecoderThread::setupConnections() {

    // suppress old segments if user have to chosen to not reload data from previous session :
    connect (parent->getDataRestorer(),
             SIGNAL(suppressOldOrphanedSegmentsSignal()),
             this,
             SLOT(suppressOldOrphanedSegmentsSlot()));


    // update info about decoding process :
    qRegisterMetaType<PostDownloadInfoData>("PostDownloadInfoData");
    connect (this,
             SIGNAL(updateDecodeSignal(PostDownloadInfoData)),
             this->parent->getSegmentManager(),
             SLOT(updateDecodeSegmentSlot(PostDownloadInfoData)));


    connect (this,
             SIGNAL(saveFileErrorSignal(const int)),
             parent,
             SLOT(saveFileErrorSlot(const int)));


    // send to core segment data update :
    qRegisterMetaType<SegmentData>("SegmentData");
    connect (this,
             SIGNAL(updateDownloadSegmentSignal(SegmentData, QString)),
             this->parent->getSegmentManager(),
             SLOT(updateDownloadSegmentSlot(SegmentData, QString)));


}


void SegmentsDecoderThread::emitDecodeProgression(const PostDownloadInfoData& decodeInfoData) {
    emit updateDecodeSignal(decodeInfoData);
}

void SegmentsDecoderThread::emitSaveFileError() {
    emit saveFileErrorSignal(DuringDecode);
}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void SegmentsDecoderThread::saveDownloadedSegmentSlot(SegmentData segmentData) {

    QString temporaryFolder = Settings::temporaryFolder().path();
    bool writeSuccess = true;

    // check if data is --yEncoded-- :
    QString decodedfileName = this->yencDecoder->scanCurrentSegment(segmentData);

    // file name found, decode data on the fly :
    if (!decodedfileName.isEmpty()) {

        bool crc32Match = this->yencDecoder->decodeEncodedData(temporaryFolder, segmentData, decodedfileName, writeSuccess);


        // by default, crc value is set to CrcUnknown :
        if (crc32Match) {
            segmentData.setCrc32Match(CrcOk);
        }
        else {
            segmentData.setCrc32Match(CrcKo);
        }

    }
    // else, save the segment right now and decoding will try to be done at the end of file download (uuenc):
    else {

        segmentData.getIoDevice()->open(QIODevice::ReadOnly);
        writeSuccess = Utility::saveData(temporaryFolder, segmentData.getPart(), segmentData.getIoDevice()->readAll());
        segmentData.getIoDevice()->close();
    }

    if (!writeSuccess) {

        this->emitSaveFileError();

        // segment saving has failed, reset current segment to Idle :
        segmentData.setReadyForNewServer(MasterServer);
    }

    // once processed, delete data pointer :
    delete segmentData.getIoDevice();

    emit updateDownloadSegmentSignal(segmentData, decodedfileName);

    // decoder is ready again :
    emit segmentDecoderIdleSignal();
}




void SegmentsDecoderThread::decodeSegmentsSlot(NzbFileData nzbFileData) {

    // if yenc decoding has already been performed :
    if (!nzbFileData.getDecodedFileName().isEmpty()) {

        this->yencDecoder->finishDecodingJob(nzbFileData);
    }

    else {

        // check if data is --uuEncoded-- :
        QString decodedfileName = this->uuencDecoder->scanSegmentFiles(nzbFileData);

        // if fileName is not empty, decode segments with uudecoder :
        if (!decodedfileName.isEmpty()) {

            this->uuencDecoder->decodeSegments(nzbFileData, decodedfileName);
        }

        // if fileName is empty after trying all decoders, decoding failed :
        else {

            PostDownloadInfoData decodeInfoData;
            decodeInfoData.initDecode(nzbFileData.getUniqueIdentifier(), PROGRESS_COMPLETE, DecodeErrorStatus);
            decodeInfoData.setCrc32Match(false);

            this->emitDecodeProgression(decodeInfoData);

        }

    }


    // decoder is ready again :
    emit finalizeDecoderIdleSignal();

}



void SegmentsDecoderThread::suppressOldOrphanedSegmentsSlot() {

    // get temporary path :
    QString tempPathStr = Settings::temporaryFolder().path();
    QDir temporaryFolder(tempPathStr);

    // get file list from temporary path :
    QStringList temporaryFilelist = temporaryFolder.entryList(QDir::Files | QDir::NoDotAndDotDot);

    // if file is a previous segment, suppress it :
    QFile temporaryFile;
    foreach (const QString& currentFileStr, temporaryFilelist) {

        temporaryFile.setFileName( Utility::buildFullPath(tempPathStr, currentFileStr) );

        if (temporaryFile.exists()) {

            // open file
            temporaryFile.open(QIODevice::ReadOnly);
            bool removeFile = false;

            // check that the file has been downloaded by this application, if this is the case suppress it :
            if (temporaryFile.peek(applicationFileOwner.size()) == applicationFileOwner) {
                removeFile = true;
            }

            else {

                // check at the of the file if tag exists (for yenc files that are decoded on the fly) :
                temporaryFile.seek(temporaryFile.size() - applicationFileOwner.size());

                if (temporaryFile.peek(applicationFileOwner.size()) == applicationFileOwner) {
                    removeFile = true;
                }
            }

            temporaryFile.close();

            if (removeFile) {
                temporaryFile.remove();
            }

        }

    }

}
