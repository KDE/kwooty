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


#ifndef UTILITYCATEGORIES_H
#define UTILITYCATEGORIES_H

#include <kmimetype.h>

#include <QString>
#include "categoriesmodel.h"
#include "mimedata.h"

class UtilityCategories {

public:
    UtilityCategories();

    static QString buildSubcategoryPattern(const QString&);
    static QString buildMaincategoryPattern(const QString&);
    static QString buildFullCategoryPattern(CategoriesModel*, QStandardItem*, const QString&);
    static QString builExtensionStringFromMimeType(KSharedPtr< KMimeType>);
    static QString buildFullCategoryPattern(const QString&, const QString&);
    static QString buildTextToDisplay(const QString&, const QString&);
    static QString buildTextToDisplayFromFullCategoryPattern(const QString&);
    static void builPartialMimeData(MimeData&);
    static QStringList retrieveMainCategoryList();
    static QStringList retrieveFilteredMainCategoryList(CategoriesModel*);


private:
    static QString buildPattern(const QString&, const int&);

};

#endif // UTILITYCATEGORIES_H
