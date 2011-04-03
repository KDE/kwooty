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


#include "schedulertableitemdelegate.h"

#include <KDebug>
#include <KColorUtils>

#include <QApplication>
#include <QTableView>
#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QStyleOptionHeader>
#include <QTime>



SchedulerTableItemDelegate::SchedulerTableItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {

    // associate color to display according to item status :
    this->statusColorMap.insert(NoLimitDownload,    KColorUtils::lighten(QColor(Qt::green), 0.40));
    this->statusColorMap.insert(LimitDownload,      KColorUtils::lighten(QColor(Qt::darkBlue), 0.40));
    this->statusColorMap.insert(DisabledDownload,   KColorUtils::lighten(QColor(Qt::darkRed), 0.40));

}



void SchedulerTableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    // draw horizontal header :
    if (index.row() == 0) {

        painter->save();

        QStyleOptionHeader headerOption;
        headerOption.rect = option.rect;
        int currentHourLabel = index.column() / 2;
        headerOption.text  = QTime(currentHourLabel, 0).toString("hh:mm");

        if (!this->isSchedulerEnabled()) {
            headerOption.palette.setCurrentColorGroup(QPalette::Disabled);
        }

        QApplication::style()->drawControl(QStyle::CE_Header, &headerOption, painter);

        painter->restore();

    }
    // draw cell with proper background :
    else {

        QStyleOptionViewItem opt = option;

        if (!this->isSchedulerEnabled()) {
            opt.palette.setCurrentColorGroup(QPalette::Disabled);
            QStyledItemDelegate::paint(painter, opt, index);
        }

        else {
            DownloadLimitStatus downloadLimit = static_cast<DownloadLimitStatus>(index.data(DownloadLimitRole).toInt());
            painter->fillRect(option.rect, this->statusColorMap.value(downloadLimit));
        }


    }

}


bool SchedulerTableItemDelegate::isSchedulerEnabled() const {
    return (static_cast<QTableView*>(this->parent()))->isEnabled();
}

