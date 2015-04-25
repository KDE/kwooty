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


#ifndef SEGMENTDECODERBASE_H
#define SEGMENTDECODERBASE_H

#include <QObject>
#include <QFile>
#include <QIODevice>

#include "data/nzbfiledata.h"
#include "data/segmentdata.h"
#include "data/postdownloadinfodata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class SegmentsDecoderThread;


class SegmentDecoderBase : public QObject {

  Q_OBJECT

public:
    SegmentDecoderBase(SegmentsDecoderThread*);


    QString scanSegmentFiles(const NzbFileData&);
    QString scanCurrentSegment(SegmentData&);


protected:
    virtual QString searchPattern(QIODevice*) = 0;
    virtual void decodeProgression(PostDownloadInfoData& decodeInfoData) = 0;

    SegmentsDecoderThread* segmentsDecoderThread;
    QList<SegmentData> segmentDataList;
    QVariant parentIdentifer;
    bool crc32Match;


Q_SIGNALS:

};

#endif // SEGMENTDECODERBASE_H
