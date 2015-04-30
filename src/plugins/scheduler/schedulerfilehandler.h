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

#ifndef SCHEDULERFILEHANDLER_H
#define SCHEDULERFILEHANDLER_H

#include <QObject>
#include <QStandardItemModel>

namespace SchedulerNamespace
{

enum DownloadLimitStatus {
    NoLimitDownload,
    LimitDownload,
    DisabledDownload
};

// custom roles used for storing data in items :
enum SchedulerRoles {
    DownloadLimitRole = Qt::UserRole + 1
};

static const int HEADER_ROW_SCHEDULER = 0;
static const int ROW_NUMBER_SCHEDULER = 8;
static const int COLUMN_NUMBER_SCHEDULER = 48;

}

class SchedulerFileHandler : public QObject
{

    Q_OBJECT

public:
    SchedulerFileHandler(QObject *);
    SchedulerFileHandler();

    QStandardItemModel *loadModelFromFile(QObject *);
    void reloadModel(QStandardItemModel *);
    void saveModelToFile(QStandardItemModel *);

private:
    QString retrieveSchedulerFilePath();
    void fillModel(QStandardItemModel *);

Q_SIGNALS:

public Q_SLOTS:

};

#endif // SCHEDULERFILEHANDLER_H
