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

#include "segmentinfodata.h"

SegmentInfoData::SegmentInfoData()
{

    reset();
}

SegmentInfoData::SegmentInfoData(const QString &nzbFileName, const int &nzbRowModelPosition, const int &fileNameItemRowModelPosition)
{

    mNzbFileName = nzbFileName;
    mNzbRowModelPosition = nzbRowModelPosition;
    mFileNameItemRowModelPosition = fileNameItemRowModelPosition;
    mBytesDownloaded = 0;

}

void SegmentInfoData::reset()
{

    mNzbFileName = QString();
    mTemporaryFileName = QString();
    mDestinationFileSavePath = QString();
    mNzbRowModelPosition = -1;
    mFileNameItemRowModelPosition = -1;
    mBytesDownloaded = 0;
}

void SegmentInfoData::setNzbFileName(const QString &nzbFileName)
{
    mNzbFileName = nzbFileName;
}

QString SegmentInfoData::getNzbFileName() const
{
    return mNzbFileName;
}

QString SegmentInfoData::getTemporaryFileName() const
{
    return mTemporaryFileName;
}

void SegmentInfoData::setTemporaryFileName(const QString &temporaryFileName)
{
    mTemporaryFileName = temporaryFileName;
}

QString SegmentInfoData::getDestinationFileSavePath() const
{
    return mDestinationFileSavePath;
}

void SegmentInfoData::setDestinationFileSavePath(const QString &destinationFileSavePath)
{
    mDestinationFileSavePath = destinationFileSavePath;
}

void SegmentInfoData::setNzbRowModelPosition(const int &nzbRowModelPosition)
{
    mNzbRowModelPosition = nzbRowModelPosition;
}

int SegmentInfoData::getNzbRowModelPosition() const
{
    return mNzbRowModelPosition;
}

void SegmentInfoData::setFileNameItemRowModelPosition(const int &fileNameItemRowModelPosition)
{
    mFileNameItemRowModelPosition = fileNameItemRowModelPosition;
}

int SegmentInfoData::getFileNameItemRowModelPosition() const
{
    return mFileNameItemRowModelPosition;
}

void SegmentInfoData::setBytesDownloaded(const int &bytesDownloaded)
{
    mBytesDownloaded = bytesDownloaded;
}

int SegmentInfoData::getBytesDownloaded() const
{
    return mBytesDownloaded;
}

