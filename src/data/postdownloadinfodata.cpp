/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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


#include "postdownloadinfodata.h"



PostDownloadInfoData::PostDownloadInfoData() {

    this->init();
}


void PostDownloadInfoData::initRepairDecompress(const QVariant& parentIdentifer, const int& progression, const UtilityNamespace::ItemStatus& status, const UtilityNamespace::ItemTarget& itemTarget) {

    this->init();

    this->parentIdentifer = parentIdentifer;
    this->progression = progression;
    this->status = status;
    this->itemTarget = itemTarget;
}


void PostDownloadInfoData::initDecode(const int& progression, const UtilityNamespace::ItemStatus& status, const QString& decodedFileName) {

    this->init();

    this->progression = progression;
    this->status = status;
    this->decodedFileName = decodedFileName;
    this->articleEncodingType = ArticleEncodingUUEnc;
}




void PostDownloadInfoData::init() {

    this->postProcessFinish = false;
    this->allPostProcessingCorrect = true;

}



void PostDownloadInfoData::setModelIndex(const QModelIndex& modelIndex) {
   this->modelIndex = modelIndex;
}
QModelIndex PostDownloadInfoData::getModelIndex() const {
     return this->modelIndex;
}


void PostDownloadInfoData::setParentIdentifer(const QVariant& parentIdentifer) {
    this->parentIdentifer = parentIdentifer;
}
QVariant PostDownloadInfoData::getParentIdentifer() const {
    return this->parentIdentifer;
}


void PostDownloadInfoData::setProgression(const int& progression) {
    this->progression = progression;
}
int PostDownloadInfoData::getProgression() const{
    return this->progression;
}

void PostDownloadInfoData::setStatus(const UtilityNamespace::ItemStatus& status) {
    this->status = status;
}
UtilityNamespace::ItemStatus PostDownloadInfoData::getStatus() const {
    return this->status;
}

void PostDownloadInfoData::setItemTarget(const UtilityNamespace::ItemTarget& itemTarget) {
    this->itemTarget = itemTarget;
}
UtilityNamespace::ItemTarget PostDownloadInfoData::getItemTarget() const{
    return this->itemTarget;
}

void PostDownloadInfoData::setDecodedFileName(const QString& decodedFileName) {
    this->decodedFileName = decodedFileName;
}
QString PostDownloadInfoData::getDecodedFileName() const{
    return this->decodedFileName;
}


void PostDownloadInfoData::setCrc32Match(const bool& crc32Match) {
    this->crc32Match = crc32Match;
}
bool PostDownloadInfoData::isCrc32Match() const{
    return this->crc32Match;
}

bool PostDownloadInfoData::areAllPostProcessingCorrect() const {
    return this->allPostProcessingCorrect;
}

void PostDownloadInfoData::setAllPostProcessingCorrect(const bool& allPostProcessingCorrect) {
    this->allPostProcessingCorrect = allPostProcessingCorrect;
}


void PostDownloadInfoData::setPostProcessFinish(const bool& postProcessFinish) {
    this->postProcessFinish = postProcessFinish;
}
bool PostDownloadInfoData::isPostProcessFinish() const{
    return this->postProcessFinish;
}




void PostDownloadInfoData::setArticleEncodingType(const UtilityNamespace::ArticleEncodingType& articleEncodingType) {
    this->articleEncodingType = articleEncodingType;
}
UtilityNamespace::ArticleEncodingType PostDownloadInfoData::getArticleEncodingType() const{
    return this->articleEncodingType;
}



