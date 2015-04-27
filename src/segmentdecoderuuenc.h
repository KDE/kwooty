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


#ifndef SEGMENTDECODERUUENC_H
#define SEGMENTDECODERUUENC_H

#include "segmentdecoderbase.h"

class SegmentDecoderUUEnc : public SegmentDecoderBase
{

    Q_OBJECT

public:
    explicit SegmentDecoderUUEnc(SegmentsDecoderThread*);
    ~SegmentDecoderUUEnc();
    void decodeSegments(NzbFileData, const QString&);


private:
    enum UUValues {
                    UUMaxDecodedLineSize = 45,
                    UUMaxEncodedLineSize = 61
                  };

    void decodeEncodedData(QFile&, SegmentData&, int&, const QByteArray& , bool&, bool&);
    QString searchPattern(QIODevice* segmentFile);
    bool decodeUUenc(const QByteArray&, QFile&, const int&);
    bool isUUEncodedLine(QByteArray&);
    void decodeProgression(PostDownloadInfoData&);
    bool decodeSegmentFiles(QFile&);


};

#endif // SEGMENTDECODERUUENC_H
