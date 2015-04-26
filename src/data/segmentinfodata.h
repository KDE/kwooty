/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
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


#ifndef SEGMENTINFODATA_H
#define SEGMENTINFODATA_H

#include <QObject>
#include <QString>

class SegmentInfoData {

public:
    SegmentInfoData();
    SegmentInfoData(const QString&, const int&, const int&);
    void reset();

    void setNzbFileName(const QString&);
    QString getNzbFileName() const;

    QString getTemporaryFileName() const;
    void setTemporaryFileName(const QString&);

    QString getDestinationFileSavePath() const;
    void setDestinationFileSavePath(const QString&);

    void setNzbRowModelPosition(const int&);
    int getNzbRowModelPosition() const;

    void setFileNameItemRowModelPosition(const int&);
    int getFileNameItemRowModelPosition() const;

    void setBytesDownloaded(const int&);
    int getBytesDownloaded() const;


private:
    QString nzbFileName;
    QString temporaryFileName;
    QString destinationFileSavePath;
    int nzbRowModelPosition;
    int fileNameItemRowModelPosition;
    int bytesDownloaded;
};

#endif // SEGMENTINFODATA_H
