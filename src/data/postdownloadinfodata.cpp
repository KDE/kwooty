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

    init();
}

void PostDownloadInfoData::initRepairDecompress(const QVariant &parentIdentifer, const int &progression, const UtilityNamespace::ItemStatus &status, const UtilityNamespace::ItemTarget &itemTarget)
{

    init();

    mParentIdentifer = parentIdentifer;
    mProgression = progression;
    mStatus = status;
    mItemTarget = itemTarget;
}

void PostDownloadInfoData::initDecode(const QVariant &parentIdentifer, const int &progression, const UtilityNamespace::ItemStatus &status, const QString &decodedFileName)
{

    init();

    mParentIdentifer = parentIdentifer;
    mProgression = progression;
    mStatus = status;
    mDecodedFileName = decodedFileName;
    mArticleEncodingType = ArticleEncodingUUEnc;
}

void PostDownloadInfoData::init()
{

    mPostProcessFinish = false;
    mAllPostProcessingCorrect = true;

}

void PostDownloadInfoData::setModelIndex(const QModelIndex &modelIndex)
{
    mModelIndex = modelIndex;
}
QModelIndex PostDownloadInfoData::getModelIndex() const
{
    return mModelIndex;
}

void PostDownloadInfoData::setParentIdentifer(const QVariant &parentIdentifer)
{
    mParentIdentifer = parentIdentifer;
}
QVariant PostDownloadInfoData::getParentIdentifer() const
{
    return mParentIdentifer;
}

void PostDownloadInfoData::setProgression(const int &progression)
{
    mProgression = progression;
}
int PostDownloadInfoData::getProgression() const
{
    return mProgression;
}

void PostDownloadInfoData::setStatus(const UtilityNamespace::ItemStatus &status)
{
    mStatus = status;
}
UtilityNamespace::ItemStatus PostDownloadInfoData::getStatus() const
{
    return mStatus;
}

void PostDownloadInfoData::setItemTarget(const UtilityNamespace::ItemTarget &itemTarget)
{
    mItemTarget = itemTarget;
}
UtilityNamespace::ItemTarget PostDownloadInfoData::getItemTarget() const
{
    return mItemTarget;
}

void PostDownloadInfoData::setDecodedFileName(const QString &decodedFileName)
{
    mDecodedFileName = decodedFileName;
}
QString PostDownloadInfoData::getDecodedFileName() const
{
    return mDecodedFileName;
}

void PostDownloadInfoData::setCrc32Match(const bool &crc32Match)
{
    mCrc32Match = crc32Match;
}
bool PostDownloadInfoData::isCrc32Match() const
{
    return mCrc32Match;
}

bool PostDownloadInfoData::areAllPostProcessingCorrect() const
{
    return mAllPostProcessingCorrect;
}

void PostDownloadInfoData::setAllPostProcessingCorrect(const bool &allPostProcessingCorrect)
{
    mAllPostProcessingCorrect = allPostProcessingCorrect;
}

void PostDownloadInfoData::setPostProcessFinish(const bool &postProcessFinish)
{
    mPostProcessFinish = postProcessFinish;
}
bool PostDownloadInfoData::isPostProcessFinish() const
{
    return mPostProcessFinish;
}

void PostDownloadInfoData::setArticleEncodingType(const UtilityNamespace::ArticleEncodingType &articleEncodingType)
{
    mArticleEncodingType = articleEncodingType;
}
UtilityNamespace::ArticleEncodingType PostDownloadInfoData::getArticleEncodingType() const
{
    return mArticleEncodingType;
}

