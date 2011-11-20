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
#include "centralwidget.h"


SegmentBuffer::SegmentBuffer(ServerManager* parent, CentralWidget* centralWidget) : QObject(parent) {

    this->serverManager = parent;
    this->centralWidget = centralWidget;

    this->segmentDecoderIdle = true;
    this->pauseRequested = false;

    this->setupConnections();

}



void SegmentBuffer::setupConnections() {

    // send segment to decoder thread that handles decoding and saving :
    connect (this,
             SIGNAL(saveDownloadedSegmentSignal(SegmentData)),
             this->centralWidget->getSegmentsDecoderThread(),
             SLOT(saveDownloadedSegmentSlot(SegmentData)));

    // decoder thread will notify its current state (idle or busy) :
    connect (this->centralWidget->getSegmentsDecoderThread(),
             SIGNAL(segmentDecoderIdleSignal(bool)),
             this,
             SLOT(segmentDecoderIdleSlot(bool)));

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void SegmentBuffer::saveDownloadedSegmentSlot(SegmentData segmentData) {

    // if decoder is idle, send it segment right now :
    if (this->segmentDecoderIdle) {

        emit saveDownloadedSegmentSignal(segmentData);
    }
    // else store it to be processed lately :
    else {

        this->segmentDataList.append(segmentData);

    }

    // if list has reached its max size :
    if ( this->segmentDataList.size() >= MAX_BUFFER_SIZE &&
         !this->pauseRequested ) {

        // pause all downloads until decoder becomes ready again :
        this->pauseRequested = true;
        this->centralWidget->pauseAllDownloadSlot();

        kDebug() << "segment buffer is full, set on pause until empty...";
    }

    // if no more segments are waiting and pause was requested, restart downloads :
    else if ( this->pauseRequested &&
              this->segmentDataList.isEmpty() ) {

        this->pauseRequested = false;
        this->centralWidget->startAllDownloadSlot();
    }


}


void SegmentBuffer::segmentDecoderIdleSlot(bool segmentDecoderIdle) {

    this->segmentDecoderIdle = segmentDecoderIdle;

    // if decoder is now idle and segments are waiting :
    if ( this->segmentDecoderIdle &&
         !this->segmentDataList.isEmpty() ) {

        // send the first stored segment :
        this->saveDownloadedSegmentSlot(this->segmentDataList.takeFirst());

    }

}




