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

PostDownloadInfoData::PostDownloadInfoData()
{

    this->init();
}

void PostDownloadInfoData::initRepairDecompress(const QVariant &parentIdentifer, const int &progression, const UtilityNamespace::ItemStatus &status, const UtilityNamespace::ItemTarget &itemTarget)
{

    this->init();

    this->mParentIdentifer = parentIdentifer;
    this->mProgression = progression;
    this->mStatus = status;
    this->mItemTarget = itemTarget;
}

void PostDownloadInfoData::initDecode(const QVariant &parentIdentifer, const int &progression, const UtilityNamespace::ItemStatus &status, const QString &decodedFileName)
{

    this->init();

    this->mParentIdentifer = parentIdentifer;
    this->mProgression = progression;
    this->mStatus = status;
    this->mDecodedFileName = decodedFileName;
    this->mArticleEncodingType = ArticleEncodingUUEnc;
}

void PostDownloadInfoData::init()
{

    this->mPostProcessFinish = false;
    this->mAllPostProcessingCorrect = true;

}

void PostDownloadInfoData::setModelIndex(const QModelIndex &modelIndex)
{
    this->mModelIndex = modelIndex;
}
QModelIndex PostDownloadInfoData::getModelIndex() const
{
    return this->mModelIndex;
}

void PostDownloadInfoData::setParentIdentifer(const QVariant &parentIdentifer)
{
    this->mParentIdentifer = parentIdentifer;
}
QVariant PostDownloadInfoData::getParentIdentifer() const
{
    return this->mParentIdentifer;
}

void PostDownloadInfoData::setProgression(const int &progression)
{
    this->mProgression = progression;
}
int PostDownloadInfoData::getProgression() const
{
    return this->mProgression;
}

void PostDownloadInfoData::setStatus(const UtilityNamespace::ItemStatus &status)
{
    this->mStatus = status;
}
UtilityNamespace::ItemStatus PostDownloadInfoData::getStatus() const
{
    return this->mStatus;
}

void PostDownloadInfoData::setItemTarget(const UtilityNamespace::ItemTarget &itemTarget)
{
    this->mItemTarget = itemTarget;
}
UtilityNamespace::ItemTarget PostDownloadInfoData::getItemTarget() const
{
    return this->mItemTarget;
}

void PostDownloadInfoData::setDecodedFileName(const QString &decodedFileName)
{
    this->mDecodedFileName = decodedFileName;
}
QString PostDownloadInfoData::getDecodedFileName() const
{
    return this->mDecodedFileName;
}

void PostDownloadInfoData::setCrc32Match(const bool &crc32Match)
{
    this->mCrc32Match = crc32Match;
}
bool PostDownloadInfoData::isCrc32Match() const
{
    return this->mCrc32Match;
}

bool PostDownloadInfoData::areAllPostProcessingCorrect() const
{
    return this->mAllPostProcessingCorrect;
}

void PostDownloadInfoData::setAllPostProcessingCorrect(const bool &allPostProcessingCorrect)
{
    this->mAllPostProcessingCorrect = allPostProcessingCorrect;
}

void PostDownloadInfoData::setPostProcessFinish(const bool &postProcessFinish)
{
    this->mPostProcessFinish = postProcessFinish;
}
bool PostDownloadInfoData::isPostProcessFinish() const
{
    return this->mPostProcessFinish;
}

void PostDownloadInfoData::setArticleEncodingType(const UtilityNamespace::ArticleEncodingType &articleEncodingType)
{
    this->mArticleEncodingType = articleEncodingType;
}
UtilityNamespace::ArticleEncodingType PostDownloadInfoData::getArticleEncodingType() const
{
    return this->mArticleEncodingType;
}

