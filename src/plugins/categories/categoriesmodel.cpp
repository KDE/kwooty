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

#include "categoriesmodel.h"

#include "kwooty_debug.h"

CategoriesModel::CategoriesModel(QObject *parent) : QStandardItemModel(parent)
{

}

MimeData CategoriesModel::loadMimeData(QStandardItem *selectedItem)
{
    return selectedItem->data(CategoriesModel::MimeRole).value<MimeData>();
}

MimeData CategoriesModel::loadMimeData(const QModelIndex  &selectedIndex)
{
    return selectedIndex.data(CategoriesModel::MimeRole).value<MimeData>();
}

QString CategoriesModel::getMainCategory(QStandardItem *selectedItem)
{
    return this->loadMimeData(selectedItem).getMainCategory();
}

QString CategoriesModel::getMainCategory(const QModelIndex &selectedIndex)
{
    return this->loadMimeData(selectedIndex).getMainCategory();
}

void CategoriesModel::init()
{

    this->clear();
    this->setColumnCount(2);

}

void CategoriesModel::storeMimeData(QStandardItem *selectedItem, MimeData mimeData)
{

    // set mime data to item :
    QVariant variant;
    variant.setValue(mimeData);
    selectedItem->setData(variant, CategoriesModel::MimeRole);

    // update target folder path when mimeData is stored :
    QStandardItem *targetItem = this->getTargetItem(selectedItem);

    if (targetItem) {
        targetItem->setText(mimeData.getMoveFolderPath());
    }

}

bool CategoriesModel::isDuplicateSubCategory(QStandardItem *selectedItem, const QString &compareCategory)
{

    bool duplicate = false;

    for (int i = 0; i < selectedItem->rowCount(); ++i) {

        QStandardItem *childItem = selectedItem->child(i);
        MimeData mimeData = this->loadMimeData(childItem);

        if (mimeData.isCategoryMatch(compareCategory)) {

            duplicate = true;
            break;

        }
    }

    return duplicate;
}

void CategoriesModel::addParentCategoryListToModel(const QStringList &categoryList)
{

    foreach (const QString &parentType, categoryList) {

        bool categoryInserted = false;

        for (int i = 0; i < this->rowCount(); ++i) {

            // check if category has previously been added :
            if (parentType == this->getMainCategory(item(i))) {

                categoryInserted = true;
                break;
            }

        }

        // if category not already inserted, add it in the model :
        if (!categoryInserted) {

            MimeData groupMimeData(MimeData::MainCategory);
            // retrieve the name of the group :
            groupMimeData.setMainCategory(parentType);

            QStandardItem *groupItem = new QStandardItem(groupMimeData.getDisplayedText());
            this->storeMimeData(groupItem, groupMimeData);

            int groupPosition = this->retrieveLexicalTextPosition(groupMimeData.getDisplayedText(), this->invisibleRootItem());
            this->insertRow(groupPosition, groupItem);

        }

    }

}

bool CategoriesModel::isSelectedItemParent(QStandardItem *item)
{
    return (item->parent() == 0);
}

bool CategoriesModel::isSelectedItemParent(const QModelIndex &index)
{
    return this->isSelectedItemParent(this->itemFromIndex(index));
}

QStandardItem *CategoriesModel::getParentItem(const QModelIndex &index)
{

    QStandardItem *parentItem = 0;

    if (index.isValid()) {

        // if the parent has been selected (a main category item):
        if (index.parent() == QModelIndex()) {
            parentItem = this->invisibleRootItem();
        }
        // else subcategory of the parent has been selected :
        else {
            parentItem = this->itemFromIndex(index.parent());
        }

    }

    return parentItem;
}

QStandardItem *CategoriesModel::getCategoryItem(QStandardItem *selectedItem)
{
    return this->getColumnItem(selectedItem->index(), CategoriesModel::ColumnCategory);
}

QStandardItem *CategoriesModel::getCategoryItem(const QModelIndex &index)
{
    return this->getColumnItem(index, CategoriesModel::ColumnCategory);
}

QStandardItem *CategoriesModel::getTargetItem(QStandardItem *selectedItem)
{
    return this->getColumnItem(selectedItem->index(), CategoriesModel::ColumnTarget);
}

QStandardItem *CategoriesModel::getColumnItem(const QModelIndex &index, CategoriesModel::ColumnNumber columnNumber)
{

    QStandardItem *item = 0;

    if (index.isValid()) {

        // get child at the corresponding column :
        item = this->getParentItem(index)->child(index.row(), columnNumber);
    }

    return item;
}

QList<MimeData> CategoriesModel::retrieveMimeDataListFromItem(QStandardItem *selectedItem)
{

    QList<MimeData> mimeDataList;

    for (int i = 0; i < selectedItem->rowCount(); ++i) {
        mimeDataList.append(this->loadMimeData(selectedItem->child(i)));
    }

    return mimeDataList;
}

QStandardItem *CategoriesModel::retrieveItemFromCategory(const QString &category, QStandardItem *selectedItem)
{

    QStandardItem *categoryItem = 0;
    QStandardItem *parentItem = selectedItem;

    // if selectedItem is null, search in main category is requested :
    if (!selectedItem) {
        parentItem = this->invisibleRootItem();
    }

    // else selectedItem exists, search in sub category is requested
    if (!this->isSelectedItemParent(parentItem)) {
        parentItem = this->getCategoryItem(parentItem);
    }

    // look for category pattern stored in each mimeData :
    for (int i = 0; i < parentItem->rowCount(); ++i) {

        QStandardItem *currentCategoryItem = parentItem->child(i);

        if (this->loadMimeData(currentCategoryItem).isCategoryMatch(category)) {

            categoryItem = currentCategoryItem;
            break;

        }
    }

    return categoryItem;
}

bool CategoriesModel::stringPos(const QString &targetString, const QString &currentString, int &position)
{

    bool positionFound = false;

    if (targetString.toLower().localeAwareCompare(currentString.toLower()) <= 0) {
        positionFound = true;
    }

    if (!positionFound) {
        position++;
    }

    return positionFound;
}

int CategoriesModel::retrieveLexicalTextPosition(const QString &targetDisplayedText, QStandardItem *selectedItem)
{

    int position = 0;

    for (int i = 0; i < selectedItem->rowCount(); ++i) {

        QString currentDisplayedText = this->loadMimeData(selectedItem->child(i)).getDisplayedText();

        if (this->stringPos(targetDisplayedText, currentDisplayedText, position)) {
            break;
        }

    }

    return position;

}
