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

#ifndef SEGMENTDECODER_H
#define SEGMENTDECODER_H

#include <QObject>
#include "utility.h"
#include "data/nzbfiledata.h"
//#include "data/segmentdata.h"
using namespace UtilityNamespace;

class CentralWidget;
class SegmentsDecoderThread;


class SegmentDecoder : public QObject
{

    Q_OBJECT

public:
    SegmentDecoder();
    ~SegmentDecoder();
    bool isDecoding();
    void decodeProgression(const int, UtilityNamespace::ItemStatus, const QString& decodedFileName = QString());


private:
    QString getErrorMessage(const int);
    QList<SegmentData> segmentDataList;
    QVariant parentIdentifer;
    bool isDecodingStatus;

    void scanSegmentFiles();
    void decodeSegmentFiles();
    void setupOptions(const QString&);

signals:
    void updateDecodeSignal(QVariant, int, UtilityNamespace::ItemStatus, QString);
    void saveFileErrorSignal(int);

public slots:
    void decodeSegments(NzbFileData);


private slots:


};

#endif // SEGMENTDECODER_H
