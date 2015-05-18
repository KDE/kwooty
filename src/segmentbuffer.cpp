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

SegmentBuffer::SegmentBuffer(ServerManager *parent, Core *core) : QObject(parent)
{

    this->mServerManager = parent;
    this->mCore = core;

    this->mSegmentDecoderIdle = true;
    this->mFinalizeDecodeIdle = true;
    this->mFinalizeLocked = false;
    this->mBufferFull = false;
    this->mDataSizeCounter = 0;
    this->mRequestNextSegmentDelaySec = 0;

    this->setupConnections();

}

void SegmentBuffer::setupConnections()
{

    // send segment to decoder thread that finalize decoding :
    connect(this,
            SIGNAL(saveDownloadedSegmentSignal(SegmentData)),
            this->mCore->getSegmentsDecoderThread(),
            SLOT(saveDownloadedSegmentSlot(SegmentData)));

    // send segment to decoder thread that handles decoding and saving :
    connect(this->mCore->getItemParentUpdater()->getItemDownloadUpdater(),
            SIGNAL(decodeSegmentsSignal(NzbFileData)),
            this,
            SLOT(finalizeDecodeQueuedSlot(NzbFileData)));

    // store it it the buffer until segmentDecoderThread is not idle :
    connect(this,
            SIGNAL(decodeSegmentsSignal(NzbFileData)),
            this->mCore->getSegmentsDecoderThread(),
            SLOT(decodeSegmentsSlot(NzbFileData)));

    // decoder thread will notify its current state (idle or busy) :
    connect(this->mCore->getSegmentsDecoderThread(),
            SIGNAL(segmentDecoderIdleSignal()),
            this,
            SLOT(segmentDecoderIdleSlot()));

    // be notified when segmentDecoderThread is Idle :
    connect(this->mCore->getSegmentsDecoderThread(),
            SIGNAL(finalizeDecoderIdleSignal()),
            this,
            SLOT(finalizeDecoderIdleSlot()));

}

void SegmentBuffer::lockFinalizeDecode()
{
    qCDebug(KWOOTY_LOG);
    this->mFinalizeLocked = true;
}

void SegmentBuffer::unlockFinalizeDecode()
{
    qCDebug(KWOOTY_LOG);

    this->mFinalizeLocked = false;
    this->sendDataToFinalizeDecode();
}

bool SegmentBuffer::isBufferFull() const
{

    if (this->mBufferFull) {
        qCDebug(KWOOTY_LOG) << "buffer is full...";
    }

    return this->mBufferFull;
}

void SegmentBuffer::removeDataFromDecodeWaitingQueue(const NzbFileData &selectedNzbFileData)
{

    QList<NzbFileData> nzbFileDataTempList;
    for (int i = 0; i < this->mnZbFileDataList.size(); ++i) {

        NzbFileData currentNzbFileData = this->mnZbFileDataList.at(i);

        if (currentNzbFileData.getFileSavePath() != selectedNzbFileData.getFileSavePath()) {

            nzbFileDataTempList.append(currentNzbFileData);
            qCDebug(KWOOTY_LOG) << "pending files to decode updated";
        }

    }

    this->mnZbFileDataList = nzbFileDataTempList;
}

void SegmentBuffer::updateDecodeWaitingQueue(const NzbFileData &selectedNzbFileData, const NzbFileData &targetNzbFileData)
{

    for (int i = 0; i < this->mnZbFileDataList.size(); ++i) {

        NzbFileData currentNzbFileData = this->mnZbFileDataList.at(i);

        if (currentNzbFileData.getFileSavePath() == selectedNzbFileData.getFileSavePath()) {

            currentNzbFileData.updateFileSavePath(targetNzbFileData);
            this->mnZbFileDataList.replace(i, currentNzbFileData);

            qCDebug(KWOOTY_LOG) << "pending files to decode updated";
        }

    }

}

int SegmentBuffer::segmentSavingQueued(const SegmentData &segmentData)
{

    // if decoder is idle, send segment right now :
    if (this->mSegmentDecoderIdle) {

        this->mSegmentDecoderIdle = false;
        emit saveDownloadedSegmentSignal(segmentData);
    }
    // else store it to be processed lately :
    else {

        this->mDataSizeCounter += segmentData.getDataSize();
        this->mSegmentDataList.append(segmentData);

    }

    // check if buffer can not allow more pending segments :
    this->mBufferFull = false;

    if ((this->mDataSizeCounter > UtilityNamespace::BufferMaxSizeMB) ||
            (this->mSegmentDataList.size() > UtilityNamespace::BufferMaxSizeList)) {

        this->mBufferFull = true;

    }

    // request next segment to be downloaded from 1 to 30 seconds at max :
    if (this->mBufferFull) {

        this->mRequestNextSegmentDelaySec = qMin(this->mRequestNextSegmentDelaySec + 1, 30);

        qCDebug(KWOOTY_LOG) << "segment buffer is full, buffer size:" << Utility::convertByteHumanReadable(qAbs(this->mDataSizeCounter)) << ",list size:"  << this->mSegmentDataList.size();
        qCDebug(KWOOTY_LOG) << "request next segment in" << this->mRequestNextSegmentDelaySec << "seconds...";

    } else {
        this->mRequestNextSegmentDelaySec = 0;
    }

    // next segment download will be delayed according to buffer size :
    return this->mRequestNextSegmentDelaySec;

}

bool SegmentBuffer::isfinalizeDecodeIdle() const
{
    return this->mFinalizeDecodeIdle;
}

void SegmentBuffer::sendDataToFinalizeDecode()
{

    // if decoder is Idle and as not been locked by "merging items" action :
    if (this->mFinalizeDecodeIdle &&
            !this->mFinalizeLocked &&
            !this->mnZbFileDataList.isEmpty()) {

        this->mFinalizeDecodeIdle = false;
        emit decodeSegmentsSignal(this->mnZbFileDataList.takeFirst());

    }
    // notify action merge manager that files decode finalize is suspended :
    else if (this->mFinalizeLocked) {

        emit finalizeDecoderLockedSignal();

    } else if (!this->mnZbFileDataList.isEmpty()) {
        qCDebug(KWOOTY_LOG) << "finalize decode delayed...";
    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void SegmentBuffer::finalizeDecodeQueuedSlot(const NzbFileData &nzbFileData)
{

    this->mnZbFileDataList.append(nzbFileData);

    this->sendDataToFinalizeDecode();

}

void SegmentBuffer::segmentDecoderIdleSlot()
{

    this->mSegmentDecoderIdle = true;

    // if decoder is now idle and segments are pending :
    if (!this->mSegmentDataList.isEmpty()) {

        // send the first stored segment :
        const SegmentData currentSegmentData = this->mSegmentDataList.takeFirst();
        this->mDataSizeCounter -= currentSegmentData.getDataSize();
        this->segmentSavingQueued(currentSegmentData);

    }

}

void SegmentBuffer::finalizeDecoderIdleSlot()
{

    this->mFinalizeDecodeIdle = true;

    // finish file decoding :
    this->sendDataToFinalizeDecode();
}
