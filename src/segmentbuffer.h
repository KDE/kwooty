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


#ifndef SEGMENTBUFFER_H
#define SEGMENTBUFFER_H

#include <QObject>

#include "segmentsdecoderthread.h"
#include "data/segmentdata.h"

#include "utility.h"
using namespace UtilityNamespace;

class ServerManager;
class CentralWidget;


class SegmentBuffer : public QObject {

    Q_OBJECT

public:
    SegmentBuffer(ServerManager*, CentralWidget*);
    int segmentSavingQueued(const SegmentData&);

private:

    static const int MAX_BUFFER_SIZE = 100;

    QList<SegmentData> segmentDataList;
    CentralWidget* centralWidget;
    ServerManager* serverManager;
    int segmentDecoderIdle;
    int bufferFullCounter;

    void setupConnections();


signals:
    void saveDownloadedSegmentSignal(SegmentData);


public slots:
    void segmentDecoderIdleSlot();


private slots:


};

#endif // SEGMENTBUFFER_H
