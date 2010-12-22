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

SegmentInfoData::SegmentInfoData() {

    this->reset();
}

SegmentInfoData::SegmentInfoData(const QString& nzbFileName, const int& nzbRowModelPosition) {

    this->nzbFileName = nzbFileName;
    this->nzbRowModelPosition = nzbRowModelPosition;
    this->bytesDownloaded = 0;

}


void SegmentInfoData::reset() {

    this->nzbFileName = QString();
    this->nzbRowModelPosition = -1;
    this->bytesDownloaded = 0;
}


void SegmentInfoData::setNzbFileName(const QString& nzbFileName) {
    this->nzbFileName = nzbFileName;
}

QString SegmentInfoData::getNzbFileName() const {
    return this->nzbFileName;
}


void SegmentInfoData::setNzbRowModelPosition(const int& nzbRowModelPosition) {
    this->nzbRowModelPosition = nzbRowModelPosition;
}

int SegmentInfoData::getNzbRowModelPosition() const {
    return this->nzbRowModelPosition;
}


void SegmentInfoData::setBytesDownloaded(const int& bytesDownloaded) {
    this->bytesDownloaded = bytesDownloaded;
}

int SegmentInfoData::getBytesDownloaded() const {
    return this->bytesDownloaded;
}

