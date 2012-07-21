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


#ifndef CATEGORIES_H
#define CATEGORIES_H

#include <QObject>
#include <QHash>
#include <QStandardItem>
#include <QStandardItemModel>

#include <kio/copyjob.h>
#include <kmimetype.h>

#include "mimedata.h"
#include "data/itemstatusdata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class CategoriesPlugin;
class CategoriesModel;


class Categories : public QObject {

    Q_OBJECT

public:
    Categories(CategoriesPlugin*);
    ~Categories();
    void settingsChanged();


private:

    // category hierarchy :
    enum MoveJobStatus {
        NoMoveStatus,
        MovingStatus,
        MoveSuccessStatus,
        MoveUserCanceledErrorStatus,
        MoveDiskFullErrorStatus,
        MoveCouldNotMkdirErrorStatus,
        MoveInsufficientDiskSpaceErrorStatus,
        MoveUnknownErrorStatus
    };

    KSharedPtr<KMimeType> retrieveFileMimeType(const QString&, const QString&);
    QHash<QString, quint64> scanDownloadedFiles(const QString&);
    QString guessMainMimeName(const QHash<QString, quint64>&);
    bool checkDiskSpace(const MimeData&, const QString&, const QList<quint64>&);
    void setupConnections();
    void launchMoveProcess(const MimeData&, const QString&);
    void notifyMoveProcessing(int = UtilityNamespace::PROGRESS_UNKNOWN);
    void launchPreProcess();
    void setJobProcessing(const bool&);

    Core* core;
    CategoriesModel* categoriesModel;
    MoveJobStatus moveJobStatus;
    QHash<int, QString> moveStatusTextMap;
    QHash<int, QColor> moveStatusColorMap;
    QStringList uuidItemList;
    QString currentUuidItem;
    bool jobProcessing;

signals:
    void pluginJobRunningSignal(bool);

public slots:
    void handleResultSlot(KJob*);
    void jobProgressionSlot(KIO::Job*);

private slots:
    void parentStatusItemChangedSlot(QStandardItem*, ItemStatusData);

};

#endif // CATEGORIES_H
