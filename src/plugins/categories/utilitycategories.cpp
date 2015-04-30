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

#include "utilitycategories.h"

#include "kwooty_debug.h"

#include "mimedata.h"

UtilityCategories::UtilityCategories()
{

}

QString UtilityCategories::buildMaincategoryPattern(const QString &mimeType)
{

    return UtilityCategories::buildPattern(mimeType, 0);
}

QString UtilityCategories::buildSubcategoryPattern(const QString &mimeType)
{

    return UtilityCategories::buildPattern(mimeType, 1);

}

QString UtilityCategories::buildPattern(const QString &mimeType, const int &position)
{

    QString subcategory;

    QStringList tempList = mimeType.split("/");

    if (tempList.size() > position) {

        subcategory = tempList.at(position);
    }

    return subcategory;
}

QString UtilityCategories::buildFullCategoryPattern(CategoriesModel *categoriesModel, QStandardItem *selectedItem, const QString &subcategory)
{

    QString fullCategoryPattern;

    if (selectedItem->index().isValid()) {
        fullCategoryPattern = UtilityCategories::buildFullCategoryPattern(categoriesModel->getMainCategory(selectedItem), subcategory);
    }

    return fullCategoryPattern;

}

QString UtilityCategories::buildFullCategoryPattern(const QString &category, const QString &subcategory)
{

    return QString(Utility::buildFullPath(category, subcategory));
}

QString UtilityCategories::buildTextToDisplay(const QString &category, const QString &subcategory)
{
#if 0 //PORT KF5
    QString textToDisplay;
    KSharedPtr<KMimeType> mimeType = KMimeType::mimeType(UtilityCategories::buildFullCategoryPattern(category, subcategory));

    if (!mimeType.isNull()) {
        textToDisplay = mimeType->comment() + " (" + UtilityCategories::builExtensionStringFromMimeType(mimeType) + ")";
    }

    return textToDisplay;
#else
    return QString();
#endif
}

QString UtilityCategories::buildTextToDisplayFromFullCategoryPattern(const QString &fullCategory)
{
#if 0 //PORT KF5
    QString textToDisplay;
    KSharedPtr<KMimeType> mimeType = KMimeType::mimeType(fullCategory);

    if (!mimeType.isNull()) {
        textToDisplay = mimeType->comment() + " (" + UtilityCategories::builExtensionStringFromMimeType(mimeType) + ")";
    }

    return textToDisplay;
#else
    return QString();
#endif
}

QString UtilityCategories::builExtensionStringFromMimeType(KSharedPtr<KMimeType> mimeType)
{

    QString extensionString;

    if (!mimeType.isNull()) {
        extensionString = mimeType->patterns().join("; ");
    }

    return extensionString;
}

void UtilityCategories::builPartialMimeData(MimeData &mimeData)
{

#if 0 //PORT KF5
    QString fullCategory = UtilityCategories::buildFullCategoryPattern(mimeData.getMainCategory(), mimeData.getSubCategory());

    KSharedPtr<KMimeType> mimeType = KMimeType::mimeType(fullCategory);

    if (!mimeType.isNull()) {

        mimeData.setComments(mimeType->comment());
        mimeData.setPatterns(UtilityCategories::builExtensionStringFromMimeType(mimeType));
        mimeData.setDisplayedText(UtilityCategories::buildTextToDisplayFromFullCategoryPattern(fullCategory));

    }
#endif
}

QStringList UtilityCategories::retrieveMainCategoryList()
{

    QStringList parentTypeList;
#if 0 //PORT KF5
    foreach (const KSharedPtr<KMimeType> &mimeType,  KMimeType::allMimeTypes()) {

        QStringList tempList = mimeType->name().split("/");

        if ((tempList.size() > 1) &&
                !parentTypeList.contains(tempList.at(0))) {

            // do not include "all" main category that include all mime types :
            if (tempList.at(0) != "all") {
                parentTypeList.append(tempList.at(0));
            }
        }

    }

    if (parentTypeList.isEmpty()) {
        qCDebug(KWOOTY_LOG) << "error retrieving mime type list !";
    }

    // return main category list sorted by alphabetical order :
    qSort(parentTypeList);
#endif
    return parentTypeList;
}

QStringList UtilityCategories::retrieveFilteredMainCategoryList(CategoriesModel *categoriesModel)
{

    QStringList filteredMainCategoryList = UtilityCategories::retrieveMainCategoryList();

    for (int i = 0; i < categoriesModel->rowCount(); ++i) {

        QString currentMainCategory = categoriesModel->getMainCategory(categoriesModel->item(i));

        // if main category is already present in model, does not take it into account :
        if (filteredMainCategoryList.contains(currentMainCategory)) {
            filteredMainCategoryList.removeAll(currentMainCategory);
        }

    }
    return filteredMainCategoryList;
}
