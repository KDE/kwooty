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


#include "nzbfiledata.h"
#include <KDebug>

NzbFileData::NzbFileData()
{

    this->par2File = false;
    this->rarFile = false;

}

NzbFileData::NzbFileData(const QString& fileName, const QStringList& groupList, const QList<SegmentData>& segmentList){

    this->setFileName(fileName);
    this->setGroupList(groupList);
    this->setSegmentList(segmentList);
    this->par2File = false;
    this->rarFile = false;
}

NzbFileData::~NzbFileData(){ }



QString NzbFileData::getFileName() const {
    return this->fileName;
}

void NzbFileData::setFileName(const QString& fileName) {
    this->fileName = fileName;
}

QString NzbFileData::getDecodedFileName() const {
    return this->decodedFileName;
}

void NzbFileData::setDecodedFileName(const QString& decodedFileName) {

    this->decodedFileName = decodedFileName;

    // add decoded fileName to list of candidate file names
    // (this fileName will always be the first element of the list) :
    if (!this->possibleFileNameList.contains(decodedFileName)) {
        this->possibleFileNameList.append(decodedFileName);
    }

}

bool NzbFileData::match(const QString& fileNameStr, const QString& originalFileNameStr) {

    return ( this->possibleFileNameList.contains(fileNameStr) ||
             this->possibleFileNameList.contains(originalFileNameStr) );

}

void NzbFileData::setRenamedFileName(const QString& fileNameStr, const QString& originalFileNameStr)  {

    if (!this->possibleFileNameList.contains(fileNameStr) && !fileNameStr.isEmpty()) {
        this->possibleFileNameList.append(fileNameStr);
    }

    if (!this->possibleFileNameList.contains(originalFileNameStr) && !originalFileNameStr.isEmpty()) {
        this->possibleFileNameList.append(originalFileNameStr);
    }

    if (this->possibleFileNameList.size() > 1) {
        this->renamedFileName = this->possibleFileNameList.at(1);
    }

}

QString NzbFileData::getRenamedFileName() const {
    return this->renamedFileName;
}

QString NzbFileData::getNzbName() const {
    return this->nzbName;
}

void NzbFileData::setNzbName(const QString& nzbName) {
    this->nzbName = nzbName;
}


bool NzbFileData::isPar2File() const {
    return this->par2File;
}

void NzbFileData::setPar2File(const bool par2File) {
    this->par2File = par2File;
}

bool NzbFileData::isRarFile() const {
    return this->rarFile;
}

void NzbFileData::setRarFile(const bool rarFile) {
    this->rarFile = rarFile;
}

void NzbFileData::setVerifyProgressionStep(const UtilityNamespace::ItemStatus verifyProgressionStep) {
    this->verifyProgressionStep = verifyProgressionStep;
}

UtilityNamespace::ItemStatus NzbFileData::getVerifyProgressionStep() const {
    return this->verifyProgressionStep;
}

void NzbFileData::setExtractProgressionStep(const UtilityNamespace::ItemStatus extractProgressionStep) {
    this->extractProgressionStep = extractProgressionStep;
}

UtilityNamespace::ItemStatus NzbFileData::getExtractProgressionStep() const {
    return this->extractProgressionStep;
}

quint64 NzbFileData::getSize() const {
    return this->size;
}

void NzbFileData::setSize(const quint64 size) {
    this->size = size;
}

QString NzbFileData::getFileSavePath() const {
    return this->fileSavePath;
}

void NzbFileData::setFileSavePath(const QString& fileSavePath){
    this->fileSavePath = fileSavePath;
}

void NzbFileData::setBaseName(const QString& baseName){
    this->baseName = baseName;
}

QString NzbFileData::getBaseName() const {
    return this->baseName;
}

QStringList NzbFileData::getGroupList() const {
    return this->groupList;
}

void NzbFileData::setGroupList(const QStringList& groupList){
    this->groupList = groupList;
}

QList<SegmentData> NzbFileData::getSegmentList() const {
    return this->segmentList;
}

void NzbFileData::setSegmentList(const QList<SegmentData>& segmentList){
    this->segmentList = segmentList;
}

void NzbFileData::setUniqueIdentifier(const QVariant& uniqueIdentifier){
    this->uniqueIdentifier = uniqueIdentifier;
}

QVariant NzbFileData::getUniqueIdentifier() const {
    return this->uniqueIdentifier;
}

bool NzbFileData::operator==(const NzbFileData& nzbFileDataToCompare) {
      return (this->getUniqueIdentifier() == nzbFileDataToCompare.getUniqueIdentifier());
}
