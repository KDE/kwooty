#ifndef SCHEDULERTABLEITEMDELEGATE_H
#define SCHEDULERTABLEITEMDELEGATE_H

#include <QStyledItemDelegate>

class SchedulerTableItemDelegate : public QStyledItemDelegate {

    Q_OBJECT

public:
    SchedulerTableItemDelegate(QObject *parent = 0);
    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;


signals:


public slots:


};

#endif // SCHEDULERTABLEITEMDELEGATE_H
