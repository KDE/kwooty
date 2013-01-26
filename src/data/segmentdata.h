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

#ifndef SEGMENTDATA_H
#define SEGMENTDATA_H

#include <QStandardItem>

#include "segmentinfodata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class SegmentData {


public:
    SegmentData(const QString&, const QString&, const QString&, const UtilityNamespace::ItemStatus);
    SegmentData();
    void setReadyForNewServer(const int&);
    void setDownloadFinished(const int&);
    bool isInitialized();


    void setBytes(const QString&);
    QString getBytes() const;

    void setNumber(const QString&);
    QString getNumber() const;

    void setPart(const QString&);
    QString getPart() const;

    void setStatus(const UtilityNamespace::ItemStatus);
    UtilityNamespace::ItemStatus getStatus() const;

    int getServerGroupTarget() const;
    void setServerGroupTarget(const int);

    int getProgress() const;
    void setProgress(const int);

    void setElementInList(const int);
    int getElementInList() const;

    void setParentUniqueIdentifier(const QVariant&);
    QVariant getParentUniqueIdentifier() const;

    void setArticlePresenceOnServer(const int);
    int getArticlePresenceOnServer() const;

    SegmentInfoData getSegmentInfoData() const;
    void setSegmentInfoData(const SegmentInfoData&);

    QIODevice* getIoDevice();
    void setIoDevice(QIODevice*);

    int getDataSize() const;
    void setDataSize(const int&);

    UtilityNamespace::CrcNotify getCrc32Match() const;
    void setCrc32Match(const UtilityNamespace::CrcNotify);


private:
    QString bytes;
    QString number;
    QString part;
    QVariant parentUniqueIdentifier;
    SegmentInfoData segmentInfoData;
    UtilityNamespace::ItemStatus status;
    QIODevice* ioDevice;
    int elementInList;
    int serverGroupTarget;
    int progress;
    int articlePresence;
    int dataSize;
    UtilityNamespace::CrcNotify crc32Match;

};

QDataStream& operator<<(QDataStream&, const SegmentData&);
QDataStream& operator>>(QDataStream&, SegmentData&);


Q_DECLARE_METATYPE(SegmentData);

#endif // SEGMENTDATA_H
