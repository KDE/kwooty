/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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

#include "mimedata.h"
#include "kwooty_categories_plugins_debug.h"

MimeData::MimeData(MimeData::HierarchyCategory hierarchyCategory, const QString &mainCategory)
{

    this->mHierarchyCategory = hierarchyCategory;
    this->mMainCategory = mainCategory;
}

MimeData::MimeData()
{

    qCDebug(KWOOTY_CATEGORIES_PLUGIN_LOG) << "ooops, this constructor should not be called";
}

QString MimeData::getMainCategory() const
{
    return this->mMainCategory;
}

void MimeData::setMainCategory(const QString &mainCategory)
{
    this->mMainCategory = mainCategory;
}

QString MimeData::getSubCategory() const
{
    return this->mSubCategory;
}

void MimeData::setSubCategory(const QString &subCategory)
{
    this->mSubCategory = subCategory;
}

QString MimeData::getPatterns() const
{
    return this->mPatterns;
}

void MimeData::setPatterns(const QString &patterns)
{
    this->mPatterns = patterns;
}

QString MimeData::getMoveFolderPath() const
{
    return this->mMoveFolderPath;
}

void MimeData::setMoveFolderPath(const QString &moveFolderPath)
{
    this->mMoveFolderPath = moveFolderPath;
}

QString MimeData::getComments() const
{
    return this->mComments;
}

void MimeData::setComments(const QString &comments)
{
    this->mComments = comments;
}

QString MimeData::getDisplayedText()
{

    if (this->mHierarchyCategory == MainCategory) {
        this->mDisplayedText = this->mMainCategory;
    }

    return this->mDisplayedText;
}

void MimeData::setDisplayedText(const QString &displayedText)
{
    this->mDisplayedText = displayedText;
}

bool MimeData::isCategoryMatch(const QString &compareCategory)
{

    if (this->mHierarchyCategory == MainCategory) {
        return (this->mMainCategory == compareCategory);
    } else {
        return (this->mSubCategory == compareCategory);
    }

}

