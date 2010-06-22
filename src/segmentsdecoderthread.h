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

#ifndef SEGMENTSDECODERTHREAD_H
#define SEGMENTSDECODERTHREAD_H

#include <QThread>
#include "data/nzbfiledata.h"
#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class SegmentDecoderBase;
class SegmentData;


class SegmentsDecoderThread : public QObject {

    Q_OBJECT

public:
    SegmentsDecoderThread(CentralWidget*);
    SegmentsDecoderThread();
    ~SegmentsDecoderThread();

private:

    QThread* dedicatedThread;
    CentralWidget* parent;
    QList<SegmentDecoderBase*> segmentDecoderList;
    QList<SegmentData> segmentDataList;
    QList<NzbFileData> nzbFileDataList;
    int currentDecoderElement;
    bool currentlyDecoding;

    void init();
    void setupConnections();
    void startDecoding();


signals:
    void updateDecodeSegmentSignal(SegmentData, int, int);
    void updateDecodeSignal(QVariant, int, UtilityNamespace::ItemStatus, QString, bool);

public slots:
    void decodeSegmentsSlot(NzbFileData);
    void suppressOldOrphanedSegmentsSlot();

private slots:


};

#endif // SEGMENTSDECODERTHREAD_H
