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
#include <KDebug>

MimeData::MimeData(MimeData::HierarchyCategory hierarchyCategory, const QString& mainCategory) {

    this->hierarchyCategory = hierarchyCategory;
    this->mainCategory = mainCategory;
}

MimeData::MimeData() {

    kDebug() << "ooops, this constructor should not be called";
}


QString MimeData::getMainCategory() const {
    return this->mainCategory;
}

void MimeData::setMainCategory(const QString& mainCategory) {
    this->mainCategory = mainCategory;
}


QString MimeData::getSubCategory() const {
    return this->subCategory;
}

void MimeData::setSubCategory(const QString& subCategory) {
    this->subCategory = subCategory;
}



QString MimeData::getPatterns() const {
    return this->patterns;
}

void MimeData::setPatterns(const QString& patterns) {
    this->patterns = patterns;
}


QString MimeData::getMoveFolderPath() const {
    return this->moveFolderPath;
}

void MimeData::setMoveFolderPath(const QString& moveFolderPath) {
    this->moveFolderPath = moveFolderPath;
}


QString MimeData::getComments() const {
    return this->comments;
}

void MimeData::setComments(const QString& comments) {
    this->comments = comments;
}

QString MimeData::getDisplayedText() {

    if (this->hierarchyCategory == MainCategory) {
        this->displayedText = this->mainCategory;
    }

    return this->displayedText;
}

void MimeData::setDisplayedText(const QString& displayedText) {
    this->displayedText = displayedText;
}


bool MimeData::isCategoryMatch(const QString& compareCategory) {

    if (this->hierarchyCategory == MainCategory) {
        return (this->mainCategory == compareCategory);
    }
    else {
        return (this->subCategory == compareCategory);
    }

}


