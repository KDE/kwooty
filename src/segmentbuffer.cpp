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

#include <KDebug>

#include "servermanager.h"
#include "core.h"


SegmentBuffer::SegmentBuffer(ServerManager* parent, Core* core) : QObject(parent) {

    this->serverManager = parent;
    this->core = core;

    this->segmentDecoderIdle = true;
    this->bufferFullCounter = 0;

    this->setupConnections();

}



void SegmentBuffer::setupConnections() {

    // send segment to decoder thread that handles decoding and saving :
    connect (this,
             SIGNAL(saveDownloadedSegmentSignal(SegmentData)),
             this->core->getSegmentsDecoderThread(),
             SLOT(saveDownloadedSegmentSlot(SegmentData)));

    // decoder thread will notify its current state (idle or busy) :
    connect (this->core->getSegmentsDecoderThread(),
             SIGNAL(segmentDecoderIdleSignal()),
             this,
             SLOT(segmentDecoderIdleSlot()));

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


int SegmentBuffer::segmentSavingQueued(const SegmentData& segmentData) {

    // if decoder is idle, send segment right now :
    if (this->segmentDecoderIdle) {

        this->segmentDecoderIdle = false;
        emit saveDownloadedSegmentSignal(segmentData);
    }
    // else store it to be processed lately :
    else {

        this->segmentDataList.append(segmentData);

    }

    // if list has reached its max size :
    if (this->segmentDataList.size() >= MAX_BUFFER_SIZE) {

        kDebug() << "segment buffer is full, request next segment with a short delay...";
        this->bufferFullCounter++;
    }
    else {
        this->bufferFullCounter = 0;
    }


    // next segment download will be delayed according to buffer size :
    return this->bufferFullCounter;

}


void SegmentBuffer::segmentDecoderIdleSlot() {

    this->segmentDecoderIdle = true;

    // if decoder is now idle and segments are pending :
    if (!this->segmentDataList.isEmpty()) {

        // send the first stored segment :
        this->segmentSavingQueued(this->segmentDataList.takeFirst());

    }

}




