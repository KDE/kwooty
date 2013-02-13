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
#include <KInputDialog>

#include <kgenericfactory.h>
#include <kmimetypechooser.h>
#include <kmimetype.h>

#include <QHBoxLayout>
#include <QTreeView>

#include "categoriesfilehandler.h"
#include "categoriesmodel.h"
#include "utilitycategories.h"
#include "kwooty_categoriessettings.h"




K_PLUGIN_FACTORY(PluginFactory, registerPlugin<PreferencesCategories>();)
        K_EXPORT_PLUGIN(PluginFactory("kwooty_categoriessettings"))

        PreferencesCategories::PreferencesCategories(QWidget* parent, const QVariantList& args) : KCModule(PluginFactory::componentData(), parent, args) {

    this->saveChangesRequested = false;

    // set layout config layout :
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);

    // setup ui file :
    int defaultWidth = 500;
    QWidget* widget = new QWidget(this);
    widget->setMinimumSize(defaultWidth, 350);

    this->preferencesCategoriesUi.setupUi(widget);
    layout->addWidget(widget);

    // set text for transfer management combo box :
    this->preferencesCategoriesUi.kcfg_transferManagement->addItem(i18n("Rename automatically"));
    this->preferencesCategoriesUi.kcfg_transferManagement->addItem(i18n("Overwrite"));

    // set mode to folder mode :
    this->preferencesCategoriesUi.kurlrequester->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
    this->preferencesCategoriesUi.kcfg_defaultTransferFolder->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    // add main kconfigskeleton :
    this->addConfig(CategoriesSettings::self(), widget);


    // setup treeView behaviour :
    QTreeView* mimeTreeView = this->preferencesCategoriesUi.mimeTreeView;
    mimeTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mimeTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mimeTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mimeTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mimeTreeView->setUniformRowHeights(false);
    mimeTreeView->setAllColumnsShowFocus(true);


    // retrieve model from saved file :
    this->categoriesModel = CategoriesFileHandler().loadModelFromFile(this);
    mimeTreeView->setModel(this->categoriesModel);
    mimeTreeView->expandAll();

    // set header label :
    QStringList headerLabels;
    headerLabels.append(i18n("Category"));
    headerLabels.append(i18n("Folder"));

    this->categoriesModel->setHorizontalHeaderLabels(headerLabels);
    // resize columns in order that each column occupy half of the treeview width :
    mimeTreeView->header()->resizeSection(CategoriesModel::ColumnCategory, defaultWidth / 2) ;

    this->preferencesCategoriesUi.toolButtonAdd->setIcon(KIcon("list-add"));
    this->preferencesCategoriesUi.toolButtonAdd->setText(i18n("Add Category"));

    this->preferencesCategoriesUi.toolButtonRemove->setIcon(KIcon("list-remove"));
    this->preferencesCategoriesUi.toolButtonRemove->setText(i18n("Remove Category"));
    this->preferencesCategoriesUi.toolButtonRemove->setEnabled(false);

    this->preferencesCategoriesUi.toolButtonEditSubcategory->setIcon(KIcon("document-edit"));
    this->preferencesCategoriesUi.toolButtonEditSubcategory->setText(i18n("Edit Subcategory"));
    this->preferencesCategoriesUi.toolButtonEditSubcategory->setEnabled(false);

    this->preferencesCategoriesUi.groupBoxCategory->setDisabled(true);
    this->preferencesCategoriesUi.groupBoxCategory->setTitle(this->buildGroupBoxTitle());

    // enable/disable defaultTransferFolder widget accordingly :
    this->defaultTransferValueButtonToggledSlot();

    this->setupConnections();

}


PreferencesCategories::~PreferencesCategories() {

}


void PreferencesCategories::load() {
    KCModule::load();
}


void PreferencesCategories::save() {
    CategoriesFileHandler().saveModelToFile(this->categoriesModel);
    KCModule::save();
}


void PreferencesCategories::setupConnections() {

    // enable or disable buttons according to selected items :
    connect (this->preferencesCategoriesUi.mimeTreeView->selectionModel(),
             SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
             this,
             SLOT(categoryWidgetsSlot()));

    connect (this->preferencesCategoriesUi.toolButtonAdd,
             SIGNAL(clicked(bool)),
             this,
             SLOT(toolButtonAddClickSlot()));

    connect (this->preferencesCategoriesUi.toolButtonEditSubcategory,
             SIGNAL(clicked(bool)),
             this,
             SLOT(toolButtonEditSubcategoryClickSlot()));

    connect (this->preferencesCategoriesUi.toolButtonRemove,
             SIGNAL(clicked(bool)),
             this,
             SLOT(toolButtonRemoveClickSlot()));

    connect (this->preferencesCategoriesUi.kurlrequester,
             SIGNAL(textChanged(const QString&)),
             this,
             SLOT(urlChangedSlot(const QString&)));

    connect (this->preferencesCategoriesUi.kcfg_enableDefaultTransfer,
             SIGNAL(stateChanged(int)),
             this,
             SLOT(defaultTransferValueButtonToggledSlot()));

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



void PreferencesCategories::addMimeTypeToGroup(QStandardItem* parentItem) {

    QString mainCategory = this->categoriesModel->getMainCategory(parentItem);

    // display mimeTypeChooser dialog :
    KMimeTypeChooserDialog dialog (i18n("Mime Type Selection"),
                                   i18n("Add or Remove Subcategories"),
                                   this->retrieveSelectionList(parentItem),
                                   mainCategory,
                                   QStringList() << mainCategory,
                                   KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns,
                                   this);


    // retrieve all subCategories selected by user :
    QStringList subcategorySelectedList;
    MimeData firstMimeDataToDisplay(MimeData::SubCategory);

    // dialog has not been canceled, do process :
    if (dialog.exec() == KDialog::Accepted) {

        foreach (const QString& mimeType, dialog.chooser()->mimeTypes()) {

            QString subCategory = UtilityCategories::buildSubcategoryPattern(mimeType);

            if (!subCategory.isEmpty()) {
                subcategorySelectedList.append(subCategory);
            }
        }


        foreach (const QString& subCategorySelected, subcategorySelectedList) {

            if (!this->categoriesModel->isDuplicateSubCategory(parentItem, subCategorySelected)) {

                // build associated mimeData for current subcategory :
                MimeData mimeData(MimeData::SubCategory, mainCategory);
                mimeData.setSubCategory(subCategorySelected);
                UtilityCategories::builPartialMimeData(mimeData);

                // place the current item at the right lexical order :
                QStandardItem* categoryItem = this->categoriesModel->getCategoryItem(parentItem);
                int position = this->categoriesModel->retrieveLexicalTextPosition(mimeData.getDisplayedText(), categoryItem);

                // build category and target items :
                QList<QStandardItem*> childItemList;
                QStandardItem* childCategoryItem = new QStandardItem(mimeData.getDisplayedText());
                childItemList.append(childCategoryItem);
                childItemList.append(new QStandardItem(mimeData.getMoveFolderPath()));

                // insert them at the proper row in order to be displayed in alphabetical sorting order :
                parentItem->insertRow(position, childItemList);

                // set mime data to item :
                this->categoriesModel->storeMimeData(childCategoryItem, mimeData);

                // retrieve the first diplayed text sorted alphabetically :
                if (firstMimeDataToDisplay.getDisplayedText().isEmpty()) {
                    firstMimeDataToDisplay = mimeData;
                }

                else if (firstMimeDataToDisplay.getDisplayedText().localeAwareCompare(mimeData.getDisplayedText()) > 0) {
                    firstMimeDataToDisplay = mimeData;
                }

            }

        }


        // retrieve subCategories actually stored :
        QStandardItem* categoryItem = this->categoriesModel->getCategoryItem(parentItem);
        QList<MimeData> mimeDataChildList = this->categoriesModel->retrieveMimeDataListFromItem(categoryItem);

        // compare subCategories stored with subCategories selected :
        foreach (const MimeData& mimeDataChild, mimeDataChildList) {

            // if subCategory stored is no more selected, user has deselected it :
            if (!subcategorySelectedList.contains(mimeDataChild.getSubCategory())) {

                // remove it from model :
                QStandardItem* itemToRemove = this->categoriesModel->retrieveItemFromCategory(mimeDataChild.getSubCategory(), parentItem);

                if (itemToRemove) {
                    this->categoriesModel->removeRow(itemToRemove->row(), itemToRemove->parent()->index());
                }
            }
        }


        // expand item to display subcategories :
        this->preferencesCategoriesUi.mimeTreeView->setExpanded(parentItem->index(), true);

        // finally set the first item added as automatically selected :
        if (!firstMimeDataToDisplay.getSubCategory().isEmpty()) {

            QStandardItem* subcategorySelectItem = this->categoriesModel->retrieveItemFromCategory(firstMimeDataToDisplay.getSubCategory(), parentItem);

            if (subcategorySelectItem) {

                this->preferencesCategoriesUi.mimeTreeView->selectionModel()->clear();
                this->preferencesCategoriesUi.mimeTreeView->selectionModel()->select(subcategorySelectItem->index(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                this->preferencesCategoriesUi.mimeTreeView->scrollTo(subcategorySelectItem->index());

                // display subCategory groupBox accordingly :
                this->subCategoryWidgets(subcategorySelectItem->index());
            }
        }


    } //  end if (dialog.exec() == KDialog::Accepted)

}



QStringList PreferencesCategories::retrieveSelectionList(QStandardItem* selectedItem) {

    // retrieve all full gategories from parent item :
    QStringList selectionList;

    for (int i = 0; i < selectedItem->rowCount(); i++) {

        QStandardItem* childItem = selectedItem->child(i);
        MimeData mimeData = this->categoriesModel->loadMimeData(childItem);

        selectionList.append(UtilityCategories::buildFullCategoryPattern(mimeData.getMainCategory(), mimeData.getSubCategory()));

    }

    return selectionList;
}


QString PreferencesCategories::buildGroupBoxTitle(const QString& subCategoryComment) {

    QString text = subCategoryComment;

    if (subCategoryComment.isEmpty()) {
        text = i18n("n/a");
    }

    return i18nc("%1 = type of subcategory", "Subcategory: %1", text);
}



void PreferencesCategories::saveChanges() {

    if (!this->saveChangesRequested) {

        emit changed(true);
        this->saveChangesRequested = true;

    }
}

void PreferencesCategories::subCategoryWidgets(const QModelIndex& activatedIndex) {

    QModelIndex activatedCategoryIndex = this->categoriesModel->getCategoryItem(activatedIndex)->index();

    MimeData currentMimeData = this->categoriesModel->loadMimeData(activatedCategoryIndex);
    this->preferencesCategoriesUi.kurlrequester->setUrl(KUrl(currentMimeData.getMoveFolderPath()));


    if (this->categoriesModel->isSelectedItemParent(activatedCategoryIndex)) {
        this->preferencesCategoriesUi.groupBoxCategory->setDisabled(true);
        this->preferencesCategoriesUi.groupBoxCategory->setTitle(this->buildGroupBoxTitle());
    }
    else {
        this->preferencesCategoriesUi.groupBoxCategory->setEnabled(true);
        this->preferencesCategoriesUi.groupBoxCategory->setTitle(this->buildGroupBoxTitle(currentMimeData.getComments()));
    }

}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void PreferencesCategories::toolButtonAddClickSlot() {

    bool ok = false;

    QStringList selectedCategories = KInputDialog::getItemList(i18n("Mime Type Selection"),
                                                               i18n("Select Main Category"),
                                                               UtilityCategories::retrieveFilteredMainCategoryList(this->categoriesModel),
                                                               QStringList(),
                                                               true,
                                                               &ok,
                                                               this);

    // add main categories to model :
    this->categoriesModel->addParentCategoryListToModel(selectedCategories);

    // set the first item added as automatically selected in treeView :
    if (!selectedCategories.isEmpty()) {

        qSort(selectedCategories);
        QStandardItem* selectItem = this->categoriesModel->retrieveItemFromCategory(selectedCategories.at(0));

        if (selectItem) {

            // set selection on first item added :
            this->preferencesCategoriesUi.mimeTreeView->selectionModel()->clear();
            this->preferencesCategoriesUi.mimeTreeView->selectionModel()->select(selectItem->index(), QItemSelectionModel::Select | QItemSelectionModel::Rows);

            // then display dialog box for subCategory edit :
            this->toolButtonEditSubcategoryClickSlot();
        }
    }


    // the model has been updated, emit signal that will call save() when "ok" button will be pressed :
    this->saveChanges();

}



void PreferencesCategories::toolButtonEditSubcategoryClickSlot() {

    // get selected items :
    QStandardItem* selectedItem = this->getSelectedItem();

    if (selectedItem) {

        QStandardItem* categoryItem = this->categoriesModel->getCategoryItem(selectedItem);

        if (categoryItem) {

            // retrieve main category item :
            if (!this->categoriesModel->isSelectedItemParent(categoryItem)) {

                categoryItem = categoryItem->parent();

            }

            this->addMimeTypeToGroup(categoryItem);
        }

    }

    // the model has been updated, emit signal that will call save() when "ok" button will be pressed :
    this->saveChanges();
}


void PreferencesCategories::toolButtonRemoveClickSlot() {

    // get selected items :
    QStandardItem* selectedItem = this->getSelectedItem();

    // remove row :
    if (selectedItem) {
        this->categoriesModel->removeRow(selectedItem->row());
    }

    // the model has been updated, emit signal that will call save() when "ok" button will be pressed :
    this->saveChanges();

}



void PreferencesCategories::urlChangedSlot(const QString& changedUrl) {

    QStandardItem* selectedItem = this->getSelectedItem();

    MimeData currentMimeData = this->categoriesModel->loadMimeData(selectedItem);
    currentMimeData.setMoveFolderPath(changedUrl);

    this->categoriesModel->storeMimeData(selectedItem, currentMimeData);

    // the model has been updated, emit signal that will call save() when "ok" button will be pressed :
    this->saveChanges();

}


void PreferencesCategories::categoryWidgetsSlot() {

    // get selected items :
    QStandardItem* selectedItem = this->getSelectedItem();

    if (selectedItem) {

        QModelIndex selectedIndex = selectedItem->index();

        // check if main category has been selected :
        bool mainCategorySelected = this->categoriesModel->isSelectedItemParent(selectedIndex);

        this->preferencesCategoriesUi.toolButtonRemove->setEnabled(mainCategorySelected);
        this->preferencesCategoriesUi.toolButtonEditSubcategory->setEnabled(true);

        // enable/disable subCatagories related widgets :
        this->subCategoryWidgets(selectedIndex);

    }
    // no main category found, disable button for subCategory editing :
    else {
        this->preferencesCategoriesUi.toolButtonEditSubcategory->setEnabled(false);
    }

}


void PreferencesCategories::defaultTransferValueButtonToggledSlot() {

    // enable default folder widget if enableDefaultTransfer is checked :
    this->preferencesCategoriesUi.kcfg_defaultTransferFolder->setEnabled(this->preferencesCategoriesUi.kcfg_enableDefaultTransfer->isChecked());

}

