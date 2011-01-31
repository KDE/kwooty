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

#include <QApplication>
#include <QPainter>
#include <QTableWidget>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QStyleOptionHeader>
#include <QTime>


SchedulerTableItemDelegate::SchedulerTableItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {

}





void SchedulerTableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {


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

    } else {

        QStyleOptionViewItemV4 opt = option;

        if (!this->isSchedulerEnabled()) {
            opt.palette.setCurrentColorGroup(QPalette::Disabled);
        }

        QStyledItemDelegate::paint(painter, opt, index);

    }

}


bool SchedulerTableItemDelegate::isSchedulerEnabled() const {

    return (static_cast<QTableWidget*>(this->parent()))->isEnabled();
}

