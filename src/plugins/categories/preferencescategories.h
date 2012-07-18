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
#include <QStandardItemModel>

#include "ui_preferencescategories.h"

#include <KCModule>
#include <KPluginFactory>
#include <KPluginLoader>

#include "mimedata.h"

class CategoriesModel;

class PreferencesCategories : public KCModule {

    Q_OBJECT

public:
    PreferencesCategories(QWidget* = 0, const QVariantList& = QVariantList());
    ~PreferencesCategories();

    virtual void save();
    virtual void load();

private:

    QStandardItem* getSelectedItem();
    QStringList retrieveSelectionList(QStandardItem*);
    QString buildGroupBoxTitle(const QString& = QString());
    void subCategoryWidgets(const QModelIndex&);
    void addMimeTypeToGroup(QStandardItem*);
    void setupConnections();
    void saveChanges();

    Ui_PreferencesCategories preferencesCategoriesUi;
    CategoriesModel* categoriesModel;
    bool saveChangesRequested;

signals:


public slots:


private slots:

    void toolButtonAddClickSlot();
    void toolButtonRemoveClickSlot();
    void toolButtonEditSubcategoryClickSlot();
    void urlChangedSlot(const QString&);
    void categoryWidgetsSlot();

};

#endif // PREFERENCESCATEGORIES_H
