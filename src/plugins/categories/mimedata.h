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

#ifndef MIMEDATA_H
#define MIMEDATA_H

#include "utilities/utility.h"
using namespace UtilityNamespace;

class MimeData
{

public:

    // category hierarchy :
    enum HierarchyCategory {
        MainCategory,
        SubCategory
    };

    MimeData(MimeData::HierarchyCategory, const QString & = QString());
    MimeData();

    QString getMainCategory() const;
    void setMainCategory(const QString &);

    QString getSubCategory() const;
    void setSubCategory(const QString &);

    void setPatterns(const QString &);
    QString getPatterns() const;

    void setMoveFolderPath(const QString &);
    QString getMoveFolderPath() const;

    void setComments(const QString &);
    QString getComments() const;

    void setDisplayedText(const QString &);
    QString getDisplayedText();

    bool isCategoryMatch(const QString &);

private:

    HierarchyCategory mHierarchyCategory;
    QString mMainCategory;
    QString mSubCategory;
    QString mPatterns;
    QString mMoveFolderPath;
    QString mComments;
    QString mDisplayedText;

};

Q_DECLARE_METATYPE(MimeData)

#endif // MIMEDATA_H
