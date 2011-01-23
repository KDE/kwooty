#include "schedulertableitemdelegate.h"

#include <KDebug>

#include <QApplication>
#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QStyleOptionHeader>
#include <QTime>


SchedulerTableItemDelegate::SchedulerTableItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {

}





void SchedulerTableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {


    if (index.row() == 0) {

        kDebug();
        painter->save();

        QStyleOptionHeader headerOption;
        headerOption.rect = option.rect;
        int currentHourLabel = index.column() / 2;
        headerOption.text  = QTime(currentHourLabel, 0).toString("hh:mm");

        //headerOption.state = QStyle::State_Selected;
        //headerOption.palette.setCurrentColorGroup(QPalette::Disabled);
        QApplication::style()->drawControl(QStyle::CE_Header, &headerOption, painter);

        painter->restore();

    } else {
        QStyledItemDelegate::paint(painter, option, index);

    }

}

