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

#include <QHBoxLayout>
#include <QHeaderView>

#include "kwooty_schedulersettings.h"
#include <kgenericfactory.h>

#include "schedulertableitemdelegate.h"



K_PLUGIN_FACTORY(PluginFactory, registerPlugin<PreferencesScheduler>();)
        K_EXPORT_PLUGIN(PluginFactory("kwooty_schedulersettings"))


        PreferencesScheduler::PreferencesScheduler(QWidget* parent, const QVariantList& args) :
        KCModule(PluginFactory::componentData(), parent, args) {

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



    QTableWidget* schedulerTableWidget = this->preferencesSchedulerUi.schedulerTableWidget;

    schedulerTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    schedulerTableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    schedulerTableWidget->setColumnCount(48);
    schedulerTableWidget->setRowCount(8);
    schedulerTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    schedulerTableWidget->setItemDelegate(new SchedulerTableItemDelegate(schedulerTableWidget));

    QHeaderView* horizontalHeader = schedulerTableWidget->horizontalHeader();
    horizontalHeader->setResizeMode(QHeaderView::Stretch);
    horizontalHeader->setDefaultSectionSize(10);
    horizontalHeader->setMinimumSectionSize(10);
    horizontalHeader->hide();

    QHeaderView* verticalHeader = schedulerTableWidget->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Stretch);
    verticalHeader->setDefaultSectionSize(5);
    verticalHeader->setMinimumSectionSize(5);


    QStringList verticalHeaderLabels;
    verticalHeaderLabels << QString();

    for (int i = 1; i < 8; i++) {
        verticalHeaderLabels.append(QDate::longDayName(i));
    }
    schedulerTableWidget->setVerticalHeaderLabels(verticalHeaderLabels);


    for (int i = 0; i < 48; i++) {

        int index = i * 4;
        schedulerTableWidget->setSpan(0, index, 1, 4);

    }


    for (int i = 1; i < 8; i++) {

        for (int j = 0; j < 48; j++) {
            schedulerTableWidget->setItem(i, j, new QTableWidgetItem());
            schedulerTableWidget->item(i, j)->setData(Qt::BackgroundColorRole, QColor(this->getRateLimitColor(NoLimitDownload)));
        }

    }


    connect (this->preferencesSchedulerUi.kcfg_downloadLimitSpinBox, SIGNAL(valueChanged(int)), this, SLOT(downloadLimitValueChangedSlot(int)));

    connect (schedulerTableWidget, SIGNAL(cellEntered(int, int)), this, SLOT(cellEnteredSlot(int, int)));
    connect (schedulerTableWidget, SIGNAL(cellPressed(int, int)), this, SLOT(cellPressedSlot(int, int)));


    this->preferencesSchedulerUi.noLimitRadioButton->setChecked(true);
    this->downloadLimitValueChangedSlot(this->preferencesSchedulerUi.kcfg_downloadLimitSpinBox->value());


    QPixmap pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    pixmap.fill(this->getRateLimitColor(NoLimitDownload));
    this->preferencesSchedulerUi.noLimitRadioButton->setIcon(pixmap);

    pixmap.fill(this->getRateLimitColor(LimitDownload));
    this->preferencesSchedulerUi.donwloadLimitRadioButton->setIcon(pixmap);

    pixmap.fill(this->getRateLimitColor(DisabledDownload));
    this->preferencesSchedulerUi.downloadDisabledRadioButton->setIcon(pixmap);

    //schedulerTableWidget->setDisabled(true);



    kDebug() << QDate::longDayName(1);
    // set folder mode :
    //this->preferencesSchedulerUi.kcfg_watchFolder->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);


}


QColor PreferencesScheduler::getRateLimitColor(PreferencesScheduler::DownloadLimit downloadLimit) {

    QColor color;
    if (downloadLimit == NoLimitDownload) {
        color = KColorUtils::lighten(QColor(Qt::green));
    }
    else if (downloadLimit == LimitDownload) {
        color = KColorUtils::lighten(QColor(Qt::darkBlue));
    }
    else if (downloadLimit == DisabledDownload) {
        color = KColorUtils::lighten(QColor(Qt::darkRed));
    }

    return color;

}

void PreferencesScheduler::cellPressedSlot(int row, int column) {

    this->mousePressedRow = row;
    this->mousePressedColumn = column;

    this->assignDownloadRateToCell(row, column);

}


void PreferencesScheduler::cellEnteredSlot(int row, int column) {

    kDebug() << row << column;

    if (row > 0) {  

        if (this->mousePressedRow == row) {

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



void PreferencesScheduler::assignDownloadRateToCell(int row, int column) {

    QColor cellBackgroundColor;

    if (this->preferencesSchedulerUi.noLimitRadioButton->isChecked()) {
        cellBackgroundColor = this->getRateLimitColor(NoLimitDownload);
    }
    else if (this->preferencesSchedulerUi.donwloadLimitRadioButton->isChecked()){
        cellBackgroundColor = this->getRateLimitColor(LimitDownload);

    }
    else if (this->preferencesSchedulerUi.downloadDisabledRadioButton->isChecked()){
        cellBackgroundColor = this->getRateLimitColor(DisabledDownload);

    }

//    KColorScheme kColorScheme(QPalette::Text);

    kDebug() <<  this->preferencesSchedulerUi.schedulerTableWidget->item(row, column)->data(Qt::BackgroundColorRole);

    //this->preferencesSchedulerUi.schedulerTableWidget->item(row, column)->setData(Qt::BackgroundColorRole, kColorScheme.foreground(KColorScheme::InactiveText).color());

    cellBackgroundColor = QPalette().color(QPalette::Disabled, QPalette::Base);

    this->preferencesSchedulerUi.schedulerTableWidget->item(row, column)->setData(Qt::BackgroundColorRole, cellBackgroundColor);

}


void PreferencesScheduler::downloadLimitValueChangedSlot(int downloadRate) {

    if (downloadRate == 0) {
        this->preferencesSchedulerUi.donwloadLimitRadioButton->setText(i18n("No download limit"));
    }

    else {
        this->preferencesSchedulerUi.donwloadLimitRadioButton->setText(i18n("Download limit to %1 KiB/s", downloadRate));
    }


}


PreferencesScheduler::~PreferencesScheduler() {

}


void PreferencesScheduler::load(){
    KCModule::load();
}


void PreferencesScheduler::save(){
    KCModule::save();

}
