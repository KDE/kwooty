/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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

#include "preferencesscheduler.h"

#include <KDebug>
#include <KGlobal>
#include <KIconLoader>
#include <KColorUtils>
#include <kcolorscheme.h>
#include <kgenericfactory.h>

#include <QHBoxLayout>
#include <QHeaderView>
#include <QModelIndex>

#include "kwooty_schedulersettings.h"
#include "schedulertableitemdelegate.h"


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<PreferencesScheduler>();)
K_EXPORT_PLUGIN(PluginFactory("kwooty_schedulersettings"))


PreferencesScheduler::PreferencesScheduler(QWidget* parent, const QVariantList& args) : KCModule(PluginFactory::componentData(), parent, args) {

    // set layout config layout :
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);

    // setup ui file :
    QWidget* widget = new QWidget(this);
    widget->setMinimumSize(600, 200);
    this->preferencesSchedulerUi.setupUi(widget);
    layout->addWidget(widget);

    // add main kconfigskeleton :
    this->addConfig(SchedulerSettings::self(), widget);


    this->setupConnections();


    QTableView* schedulerTableView = this->preferencesSchedulerUi.schedulerTableView;

    // retrieve model from saved file :
    this->schedulerModel = SchedulerFileHandler().loadModelFromFile(this);
    schedulerTableView->setModel(this->schedulerModel);

    // disable table edit and select :
    schedulerTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    schedulerTableView->setSelectionMode(QAbstractItemView::NoSelection);

    // set delegate used to paint horizontal label :
    schedulerTableView->setItemDelegate(new SchedulerTableItemDelegate(schedulerTableView));

    // build horizontal header :
    QHeaderView* horizontalHeader = schedulerTableView->horizontalHeader();
    horizontalHeader->setResizeMode(QHeaderView::Stretch);
    horizontalHeader->setDefaultSectionSize(10);
    horizontalHeader->setMinimumSectionSize(10);
    horizontalHeader->hide();

    // build vertical header :
    QHeaderView* verticalHeader = schedulerTableView->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Stretch);
    verticalHeader->setDefaultSectionSize(5);
    verticalHeader->setMinimumSectionSize(5);


    // setup vertical header containing days of week :
    QStringList verticalHeaderLabels;
    verticalHeaderLabels << QString();

    for (int i = HEADER_ROW_SCHEDULER + 1; i < ROW_NUMBER_SCHEDULER; i++) {
        verticalHeaderLabels.append(QDate::longDayName(i));
    }

    this->schedulerModel->setVerticalHeaderLabels(verticalHeaderLabels);


    // span first row with 4 cells (it will be used to display a spanned horizontal header with itemDelegate) :
    for (int i = 0; i < COLUMN_NUMBER_SCHEDULER; i++) {

        int index = i * 4;
        schedulerTableView->setSpan(HEADER_ROW_SCHEDULER, index, 1, 4);

    }


    // by default, check the "no limit" radio button from scheduler group box :
    this->preferencesSchedulerUi.noLimitRadioButton->setChecked(true);
    this->downloadLimitValueChangedSlot(this->preferencesSchedulerUi.kcfg_downloadLimitSpinBox->value());


    // setup proper pixmaps next to radio buttons :
    QPixmap pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    pixmap.fill(KColorUtils::lighten(QColor(Qt::green), 0.40));
    this->preferencesSchedulerUi.noLimitRadioButton->setIcon(pixmap);

    pixmap.fill(KColorUtils::lighten(QColor(Qt::darkBlue), 0.40));
    this->preferencesSchedulerUi.donwloadLimitRadioButton->setIcon(pixmap);

    pixmap.fill(KColorUtils::lighten(QColor(Qt::darkRed), 0.40));
    this->preferencesSchedulerUi.downloadDisabledRadioButton->setIcon(pixmap);


    // enable or disable tableWidget :
    this->schedulerToggledSlot(this->preferencesSchedulerUi.kcfg_enableScheduler->isChecked());


}


PreferencesScheduler::~PreferencesScheduler() {

}


void PreferencesScheduler::load(){
    KCModule::load();
}


void PreferencesScheduler::save(){
    SchedulerFileHandler().saveModelToFile(this->schedulerModel);
    KCModule::save();
}


void PreferencesScheduler::setupConnections() {

    connect (this->preferencesSchedulerUi.kcfg_downloadLimitSpinBox, SIGNAL(valueChanged(int)), this, SLOT(downloadLimitValueChangedSlot(int)));
    connect (this->preferencesSchedulerUi.kcfg_enableScheduler, SIGNAL(toggled(bool)), this, SLOT(schedulerToggledSlot(bool)));

    connect (this->preferencesSchedulerUi.schedulerTableView, SIGNAL(entered (const QModelIndex&)), this, SLOT(cellEnteredSlot(const QModelIndex&)));
    connect (this->preferencesSchedulerUi.schedulerTableView, SIGNAL(pressed (const QModelIndex&)), this, SLOT(cellPressedSlot(const QModelIndex&)));

}




void PreferencesScheduler::assignDownloadRateToCell(int row, int column) {

    DownloadLimitStatus downloadLimit = NoLimitDownload;

    // get download limit status :
    if (this->preferencesSchedulerUi.noLimitRadioButton->isChecked()) {
        downloadLimit = NoLimitDownload;
    }
    else if (this->preferencesSchedulerUi.donwloadLimitRadioButton->isChecked()){
        downloadLimit = LimitDownload;

    }
    else if (this->preferencesSchedulerUi.downloadDisabledRadioButton->isChecked()){
        downloadLimit = DisabledDownload;

    }


    // apply status to cell :
    QStandardItem* item = this->schedulerModel->itemFromIndex(this->schedulerModel->index(row, column));
    item->setData(static_cast<int>(downloadLimit), DownloadLimitRole);

}





//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void PreferencesScheduler::cellPressedSlot(const QModelIndex& index) {

    this->mousePressedRow = index.row();
    this->mousePressedColumn = index.column();

    // set download rate value to table item :
    this->assignDownloadRateToCell(index.row(), index.column());

    // the model has been updated, emit signal that will call save() when "ok" button will be pressed :
    emit changed(true);
}


void PreferencesScheduler::cellEnteredSlot(const QModelIndex& index) {

    int row = index.row();
    int column = index.column();

    // the first row is dedicated to paint the horizontal header, be sure that
    // this row has not been selected :
    if (row > HEADER_ROW_SCHEDULER) {

        // if mouse has moved from pressed action :
        if (this->mousePressedRow == row) {

            // if column has changed, fill item gaps with the same download status :
            if (column - this->mousePressedColumn > 0) {

                for (int currentColumn = this->mousePressedColumn; currentColumn <= column; currentColumn++) {
                    this->assignDownloadRateToCell(row,currentColumn);
                }

            }
            else if (column - this->mousePressedColumn < 0) {

                for (int currentColumn = this->mousePressedColumn; currentColumn >= column; currentColumn--) {
                    this->assignDownloadRateToCell(row,currentColumn);
                }

            }

        }

    }

}


void  PreferencesScheduler::schedulerToggledSlot(bool toggled) {

    // scheduler has been disabled :
    if (!toggled) {

        // disable groupbox and table :
        this->preferencesSchedulerUi.schedulerGroupBox->setDisabled(true);
        this->preferencesSchedulerUi.schedulerTableView->setDisabled(true);
    }

    // scheduler has been enabled :
    else {

        // enable groupbox and table :
        this->preferencesSchedulerUi.schedulerGroupBox->setDisabled(false);
        this->preferencesSchedulerUi.schedulerTableView->setDisabled(false);

    }

}



void PreferencesScheduler::downloadLimitValueChangedSlot(int downloadRate) {

    // if speed limit is set to 0 :
    if (downloadRate == 0) {
        this->preferencesSchedulerUi.donwloadLimitRadioButton->setText(i18n("No limit"));
    }
    // else display speed limit :
    else {
        this->preferencesSchedulerUi.donwloadLimitRadioButton->setText(i18n("Limit to %1 KiB/s", downloadRate));
    }


}


