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


#ifndef CATEGORIESMODEL_H
#define CATEGORIESMODEL_H

#include <QStandardItemModel>

#include "mimedata.h"

class CategoriesModel : public QStandardItemModel {

    Q_OBJECT

public:

    // custom roles used for storing data in items :
    enum CategoriesRoles {
        MimeRole  = Qt::UserRole + 1,
    };

    // column definitions :
    enum ColumnNumber {
        ColumnCategory,
        ColumnTarget
    };


    CategoriesModel(QObject*);

    MimeData loadMimeData(QStandardItem*);
    MimeData loadMimeData(const QModelIndex&);
    QList<MimeData> retrieveMimeDataListFromItem(QStandardItem*);
    QStandardItem* retrieveItemFromCategory(const QString&, QStandardItem* = 0);
    QString getMainCategory(const QModelIndex&);
    QString getMainCategory(QStandardItem*);
    int retrieveLexicalTextPosition(const QString&, QStandardItem*);
    bool isDuplicateSubCategory(QStandardItem*, const QString&);
    bool isSelectedItemParent(QStandardItem*);
    bool isSelectedItemParent(const QModelIndex&);
    void storeMimeData(QStandardItem*, MimeData);
    void addParentCategoryListToModel(const QStringList&);
    void init();

    QStandardItem* getCategoryItem(QStandardItem*);
    QStandardItem* getCategoryItem(const QModelIndex&);
    QStandardItem* getTargetItem(QStandardItem*);
    QStandardItem* getColumnItem(const QModelIndex&, CategoriesModel::ColumnNumber);
    QStandardItem* getParentItem(const QModelIndex&);


private:

    bool stringPos(const QString&, const QString&, int&);

Q_SIGNALS:

public Q_SLOTS:

};

#endif // CATEGORIESMODEL_H
