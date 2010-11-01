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

#include "segmentdata.h"

SegmentData::SegmentData() {
    this->parentUniqueIdentifier = QVariant();
    this->elementInList = -1;
    this->status = IdleStatus;
    this->segmentInfoData.reset();
}


SegmentData::SegmentData(const QString& bytes, const QString& number, const QString& part, const UtilityNamespace::ItemStatus status) {
    this->bytes = bytes;
    this->number = number;
    this->part = part;
    this->status = status;

    this->serverGroupTarget = MasterServer;
    this->articlePresence = Unknown;
    this->parentUniqueIdentifier = QVariant();
    this->elementInList = -1;
    this->segmentInfoData.reset();
}

void SegmentData::setReadyForNewServer(const int& nextServerGroup) {

    this->setProgress(PROGRESS_INIT);
    this->setStatus(IdleStatus);
    this->setArticlePresenceOnServer(Unknown);
    this->setServerGroupTarget(nextServerGroup);
}

void SegmentData::setDownloadFinished(const int& articlePresence) {

    this->setProgress(PROGRESS_COMPLETE);
    this->setStatus(DownloadFinishStatus);
    this->setArticlePresenceOnServer(articlePresence);
}

bool SegmentData::isInitialized() {
    return this->parentUniqueIdentifier != QVariant();
}

void SegmentData::setBytes(const QString& bytes){
    this->bytes = bytes;
}

QString SegmentData::getBytes() const{
    return this->bytes;
}

void SegmentData::setNumber(const QString& number){
    this->number = number;
}

QString SegmentData::getNumber() const{
    return this->number;
}

void SegmentData::setPart(const QString& part){
    this->part = part;
}

QString SegmentData::getPart() const{
    return this->part;
}

UtilityNamespace::ItemStatus SegmentData::getStatus() const{
    return this->status;
}

void SegmentData::setStatus(const UtilityNamespace::ItemStatus status){
    this->status = status;
}

int SegmentData::getServerGroupTarget() const{
    return this->serverGroupTarget;
}

void SegmentData::setServerGroupTarget(const int serverGroupTarget){
    this->serverGroupTarget = serverGroupTarget;
}


int SegmentData::getProgress() const{
    return this->progress;
}

void SegmentData::setProgress(const int progress){
    this->progress = progress;
}

void SegmentData::setElementInList(const int elementInList){
    this->elementInList = elementInList;
}

int SegmentData::getElementInList() const{
    return this->elementInList;
}

void SegmentData::setParentUniqueIdentifier(const QVariant& parentUniqueIdentifier){
    this->parentUniqueIdentifier = parentUniqueIdentifier;
}

QVariant SegmentData::getParentUniqueIdentifier() const{
    return this->parentUniqueIdentifier;
}


int SegmentData::getArticlePresenceOnServer() const{
    return this->articlePresence;
}

void SegmentData::setArticlePresenceOnServer(const int articlePresence){
    this->articlePresence = articlePresence;
}

SegmentInfoData SegmentData::getSegmentInfoData() const{
    return this->segmentInfoData;
}

void SegmentData::setSegmentInfoData(const SegmentInfoData& segmentInfoData){
    this->segmentInfoData = segmentInfoData;
}


QDataStream& operator<<(QDataStream& out, const SegmentData& segmentData) {

    out << segmentData.getBytes()
        << segmentData.getNumber()
        << segmentData.getPart()
        << segmentData.getElementInList()
        << (qint16)segmentData.getStatus()
        << segmentData.getProgress()
        << segmentData.getArticlePresenceOnServer();

    return out;
}


QDataStream& operator>>(QDataStream& in, SegmentData& segmentData) {

    QString bytes;
    QString number;
    QString part;
    qint16 status;
    int elementInList;
    int progress;
    int articlePresenceOnServer;

    in >> bytes
       >> number
       >> part
       >> elementInList
       >> status
       >> progress
       >> articlePresenceOnServer;


    segmentData.setBytes(bytes);
    segmentData.setNumber(number);
    segmentData.setPart(part);
    segmentData.setElementInList(elementInList);
    segmentData.setStatus((UtilityNamespace::ItemStatus)status);
    segmentData.setProgress(progress);
    segmentData.setArticlePresenceOnServer(articlePresenceOnServer);
    segmentData.setServerGroupTarget(MasterServer);

    return in;
}

