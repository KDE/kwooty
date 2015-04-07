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


#include "segmentbuffer.h"

#include "kwooty_debug.h"

#include "servermanager.h"
#include "itemparentupdater.h"
#include "itemdownloadupdater.h"
#include "core.h"


SegmentBuffer::SegmentBuffer(ServerManager* parent, Core* core) : QObject(parent) {

    this->serverManager = parent;
    this->core = core;

    this->segmentDecoderIdle = true;
    this->finalizeDecodeIdle = true;
    this->finalizeLocked = false;
    this->bufferFull = false;
    this->dataSizeCounter = 0;
    this->requestNextSegmentDelaySec = 0;

    this->setupConnections();

}



void SegmentBuffer::setupConnections() {

    // send segment to decoder thread that finalize decoding :
    connect (this,
             SIGNAL(saveDownloadedSegmentSignal(SegmentData)),
             this->core->getSegmentsDecoderThread(),
             SLOT(saveDownloadedSegmentSlot(SegmentData)));

    // send segment to decoder thread that handles decoding and saving :
    connect (this->core->getItemParentUpdater()->getItemDownloadUpdater(),
             SIGNAL(decodeSegmentsSignal(NzbFileData)),
             this,
             SLOT(finalizeDecodeQueuedSlot(const NzbFileData&)));

    // store it it the buffer until segmentDecoderThread is not idle :
    connect (this,
             SIGNAL(decodeSegmentsSignal(NzbFileData)),
             this->core->getSegmentsDecoderThread(),
             SLOT(decodeSegmentsSlot(NzbFileData)));


    // decoder thread will notify its current state (idle or busy) :
    connect (this->core->getSegmentsDecoderThread(),
             SIGNAL(segmentDecoderIdleSignal()),
             this,
             SLOT(segmentDecoderIdleSlot()));

    // be notified when segmentDecoderThread is Idle :
    connect (this->core->getSegmentsDecoderThread(),
             SIGNAL(finalizeDecoderIdleSignal()),
             this,
             SLOT(finalizeDecoderIdleSlot()));


}

void SegmentBuffer::lockFinalizeDecode() {
    qCDebug(KWOOTY_LOG);
    this->finalizeLocked = true;
}

void SegmentBuffer::unlockFinalizeDecode() {
    qCDebug(KWOOTY_LOG);

    this->finalizeLocked = false;
    this->sendDataToFinalizeDecode();
}

bool SegmentBuffer::isBufferFull() const {

    if (this->bufferFull){
        qCDebug(KWOOTY_LOG) << "buffer is full...";
    }

    return this->bufferFull;
}

void SegmentBuffer::removeDataFromDecodeWaitingQueue(const NzbFileData& selectedNzbFileData) {

    QList<NzbFileData> nzbFileDataTempList;
    for (int i = 0; i < this->nzbFileDataList.size(); i++) {

        NzbFileData currentNzbFileData = this->nzbFileDataList.at(i);

        if (currentNzbFileData.getFileSavePath() != selectedNzbFileData.getFileSavePath()) {

            nzbFileDataTempList.append(currentNzbFileData);
            qCDebug(KWOOTY_LOG) << "pending files to decode updated";
        }

    }

    this->nzbFileDataList = nzbFileDataTempList;
}



void SegmentBuffer::updateDecodeWaitingQueue(const NzbFileData& selectedNzbFileData, const NzbFileData& targetNzbFileData) {

    for (int i = 0; i < this->nzbFileDataList.size(); i++) {

        NzbFileData currentNzbFileData = this->nzbFileDataList.at(i);

        if (currentNzbFileData.getFileSavePath() == selectedNzbFileData.getFileSavePath()) {

            currentNzbFileData.updateFileSavePath(targetNzbFileData);
            this->nzbFileDataList.replace(i, currentNzbFileData);

            qCDebug(KWOOTY_LOG) << "pending files to decode updated";
        }

    }

}



int SegmentBuffer::segmentSavingQueued(const SegmentData& segmentData) {

    // if decoder is idle, send segment right now :
    if (this->segmentDecoderIdle) {

        this->segmentDecoderIdle = false;
        emit saveDownloadedSegmentSignal(segmentData);
    }
    // else store it to be processed lately :
    else {

        this->dataSizeCounter += segmentData.getDataSize();
        this->segmentDataList.append(segmentData);

    }


    // check if buffer can not allow more pending segments :
    this->bufferFull = false;

    if ( (this->dataSizeCounter > UtilityNamespace::BufferMaxSizeMB) ||
         (this->segmentDataList.size() > UtilityNamespace::BufferMaxSizeList) ) {

        this->bufferFull = true;

    }

    // request next segment to be downloaded from 1 to 30 seconds at max :
    if (this->bufferFull) {

        this->requestNextSegmentDelaySec = qMin(this->requestNextSegmentDelaySec + 1, 30);

        qCDebug(KWOOTY_LOG) << "segment buffer is full, buffer size:" << Utility::convertByteHumanReadable(qAbs(this->dataSizeCounter)) << ",list size:"  << this->segmentDataList.size();
        qCDebug(KWOOTY_LOG) << "request next segment in" << this->requestNextSegmentDelaySec << "seconds...";

    }
    else {
        this->requestNextSegmentDelaySec = 0;
    }

    // next segment download will be delayed according to buffer size :
    return this->requestNextSegmentDelaySec;

}


bool SegmentBuffer::isfinalizeDecodeIdle() const {
    return this->finalizeDecodeIdle;
}


void SegmentBuffer::sendDataToFinalizeDecode() {

    // if decoder is Idle and as not been locked by "merging items" action :
    if ( this->finalizeDecodeIdle &&
         !this->finalizeLocked &&
         !this->nzbFileDataList.isEmpty() ) {

        this->finalizeDecodeIdle = false;
        emit decodeSegmentsSignal(this->nzbFileDataList.takeFirst());

    }
    // notify action merge manager that files decode finalize is suspended :
    else if (this->finalizeLocked) {

        emit finalizeDecoderLockedSignal();

    }
    else if (!this->nzbFileDataList.isEmpty()) {
        qCDebug(KWOOTY_LOG) << "finalize decode delayed...";
    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void SegmentBuffer::finalizeDecodeQueuedSlot(const NzbFileData& nzbFileData) {

    this->nzbFileDataList.append(nzbFileData);

    this->sendDataToFinalizeDecode();

}


void SegmentBuffer::segmentDecoderIdleSlot() {

    this->segmentDecoderIdle = true;

    // if decoder is now idle and segments are pending :
    if (!this->segmentDataList.isEmpty()) {

        // send the first stored segment :
        const SegmentData currentSegmentData = this->segmentDataList.takeFirst();
        this->dataSizeCounter -= currentSegmentData.getDataSize();
        this->segmentSavingQueued(currentSegmentData);

    }

}


void SegmentBuffer::finalizeDecoderIdleSlot() {

    this->finalizeDecodeIdle = true;

    // finish file decoding :
    this->sendDataToFinalizeDecode();
}
