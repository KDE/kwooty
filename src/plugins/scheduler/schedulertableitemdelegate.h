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


#ifndef SCHEDULERTABLEITEMDELEGATE_H
#define SCHEDULERTABLEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QHash>

#include "schedulerfilehandler.h"
using namespace SchedulerNamespace;

class SchedulerTableItemDelegate : public QStyledItemDelegate {

    Q_OBJECT

public:
    SchedulerTableItemDelegate(QObject* parent = 0);
    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;


private:
    bool isSchedulerEnabled() const;
    QHash<DownloadLimitStatus, QColor> statusColorMap;

signals:


public slots:


};

#endif // SCHEDULERTABLEITEMDELEGATE_H
