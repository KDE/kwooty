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

#include "preferencescategories.h"


#include <KDebug>
#include <KGlobal>

#include <kmimetype.h>
#include <kgenericfactory.h>
#include <kmimetypechooser.h>

#include <QHBoxLayout>
#include <QTreeView>

#include "kwooty_categoriessettings.h"




K_PLUGIN_FACTORY(PluginFactory, registerPlugin<PreferencesCategories>();)
        K_EXPORT_PLUGIN(PluginFactory("kwooty_categoriessettings"))

        PreferencesCategories::PreferencesCategories(QWidget* parent, const QVariantList& args) : KCModule(PluginFactory::componentData(), parent, args) {

    // set layout config layout :
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);

    // setup ui file :
    QWidget* widget = new QWidget(this);
    widget->setMinimumSize(400, 300);
    this->preferencesCategoriesUi.setupUi(widget);
    layout->addWidget(widget);   


    //set mode to folder mode :
    this->preferencesCategoriesUi.kurlrequester->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    // add main kconfigskeleton :
    this->addConfig(CategoriesSettings::self(), widget);


    QTreeView* mimeTreeView = this->preferencesCategoriesUi.mimeTreeView;
    mimeTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    this->categoriesModel = new QStandardItemModel(this);
    mimeTreeView->setModel(this->categoriesModel);

    foreach (QString parentType, this->retrieveMainTypeList()) {

        this->categoriesModel->appendRow(new QStandardItem(parentType));
    }


    this->preferencesCategoriesUi.pushButtonAdd->setIcon(KIcon("list-add"));
    this->preferencesCategoriesUi.pushButtonAdd->setMaximumSize(24, 24);

    this->preferencesCategoriesUi.pushButtonRemove->setIcon(KIcon("list-remove"));
    this->preferencesCategoriesUi.pushButtonRemove->setMaximumSize(24, 24);

    this->preferencesCategoriesUi.pushButtonEdit->setIcon(KIcon("document-edit"));
    this->preferencesCategoriesUi.pushButtonEdit->setMaximumSize(24, 24);


    this->setupConnections();

}


PreferencesCategories::~PreferencesCategories() {

}


void PreferencesCategories::load() {
    KCModule::load();
}


void PreferencesCategories::save() {
    KCModule::save();

}


void PreferencesCategories::setupConnections() {

    connect (this->preferencesCategoriesUi.pushButtonAdd,
             SIGNAL(clicked(bool)),
             this,
             SLOT(pushButtonAddClickSlot()));


    connect (this->preferencesCategoriesUi.mimeTreeView,
             SIGNAL(clicked(const QModelIndex&)),
             this,
             SLOT(indexActivatedSlot(const QModelIndex&)));

    connect (this->preferencesCategoriesUi.kurlrequester,
             SIGNAL(textChanged(const QString&)),
             this,
             SLOT(urlChangedSlot(const QString&)));

}

void PreferencesCategories::pushButtonAddClickSlot() {

    // get selected items :
    QStandardItem* selectedItem = this->getSelectedItem();

    if (selectedItem) {
        this->addMimeTypeToGroup(selectedItem);
    }


}


QStandardItem* PreferencesCategories::getSelectedItem() {

    QStandardItem* selectedItem = 0;
    // get selected items :
    QList<QModelIndex> indexesList = this->preferencesCategoriesUi.mimeTreeView->selectionModel()->selectedRows();

    if (indexesList.size() > 0) {
        selectedItem = this->categoriesModel->itemFromIndex(indexesList.at(0));
    }

    return selectedItem;
}



void PreferencesCategories::indexActivatedSlot(const QModelIndex& activatedIndex) {

    kDebug();

    MimeData currentMimeData = this->loadMimeData(activatedIndex);
    this->preferencesCategoriesUi.kurlrequester->setUrl(KUrl(currentMimeData.getMoveFolderPath()));

}


void PreferencesCategories::urlChangedSlot(const QString& changedUrl) {

    kDebug();

    QStandardItem* selectedItem = this->getSelectedItem();

    MimeData currentMimeData = this->loadMimeData(selectedItem);
    currentMimeData.setMoveFolderPath(changedUrl);
    this->storeMimeData(selectedItem, currentMimeData);

}


MimeData PreferencesCategories::loadMimeData(QStandardItem* selectedItem) {
    return selectedItem->data(PreferencesCategories::MimeRole).value<MimeData>();
}

MimeData PreferencesCategories::loadMimeData(const QModelIndex&  selectedIndex) {
    return selectedIndex.data(PreferencesCategories::MimeRole).value<MimeData>();
}

void PreferencesCategories::storeMimeData(QStandardItem* selectedItem, MimeData mimeData) {
    // set mime data to item :
    QVariant variant;
    variant.setValue(mimeData);
    selectedItem->setData(variant, PreferencesCategories::MimeRole);
}


void PreferencesCategories::addMimeTypeToGroup(QStandardItem* selectedItem) {

    QString group = selectedItem->text();

    KMimeTypeChooserDialog dialog (i18n("Select Mime Types"),
                                   i18n("Add extensions categories"),
                                   QStringList(),
                                   group,
                                   QStringList() << group,
                                   KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns,
                                   this);


    if (dialog.exec() == KDialog::Accepted ) {

        foreach (QString mimeType, dialog.chooser()->mimeTypes()) {

            QStringList tempList = mimeType.split("/");

            if (tempList.size() > 1) {

                selectedItem->setChild(selectedItem->rowCount(), 0, new QStandardItem(tempList.at(1)));

                MimeData mimeData;
                mimeData.setMimeType(tempList.at(1));
                mimeData.setPatterns(dialog.chooser()->patterns().join(""));
                mimeData.setMoveFolderPath("/mnt/toto/target");

                // set mime data to item :
                this->storeMimeData(selectedItem, mimeData);

            }

        }

        kDebug() << dialog.chooser()->patterns();
        kDebug() << dialog.chooser()->mimeTypes();
    }

    kDebug() << group;

}



QStringList PreferencesCategories::retrieveMainTypeList() {

    QStringList parentTypeList;
    foreach (KSharedPtr< KMimeType >  mimeType,  KMimeType::allMimeTypes()) {

        QStringList tempList = mimeType->name().split("/");

        if ( (tempList.size() > 1 )&&
             !parentTypeList.contains(tempList.at(0))) {

            parentTypeList.append(tempList.at(0));
        }


    }

    return parentTypeList;
}




