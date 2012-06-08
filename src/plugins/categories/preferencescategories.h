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

#ifndef PREFERENCESCATEGORIES_H
#define PREFERENCESCATEGORIES_H

#include <QWidget>
#include "ui_preferencescategories.h"

#include <KCModule>

#include <KPluginFactory>
#include <KPluginLoader>

#include <QStandardItemModel>

#include "mimedata.h"

class PreferencesCategories : public KCModule {


    Q_OBJECT

    // custom roles used for storing data in items :
    enum CategoriesRoles{
        MimeRole  = Qt::UserRole + 1,
    };

public:
    PreferencesCategories(QWidget* = 0, const QVariantList& = QVariantList());
    ~PreferencesCategories();

    virtual void save();
    virtual void load();

private:
    Ui_PreferencesCategories preferencesCategoriesUi;
    QStandardItemModel* categoriesModel;

    QStringList retrieveMainTypeList();
    void addMimeTypeToGroup(QStandardItem*);
    void setupConnections();
    QStandardItem* getSelectedItem();
    MimeData loadMimeData(QStandardItem*);
    MimeData loadMimeData(const QModelIndex&);
    void storeMimeData(QStandardItem*, MimeData);



signals:



public slots:



private slots:

    void pushButtonAddClickSlot();
    void indexActivatedSlot(const QModelIndex&);
    void urlChangedSlot(const QString&);


};

#endif // PREFERENCESCATEGORIES_H
