/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
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


#ifndef WATCHFOLDER_H
#define WATCHFOLDER_H

#include <KUrl>
#include <kdirwatch.h>

#include <QObject>
#include <QStringList>
#include <QSet>
#include <QTimer>
#include <QHash>
#include <QDateTime>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class WatchFolderPlugin;

class WatchFolder : public QObject
{

    Q_OBJECT

public:
    WatchFolder(WatchFolderPlugin*);
    ~WatchFolder();
    void settingsChanged();


private:
    static const int MAX_LIST_SIZE = 10;

    KDirWatch* kDirWatch;
    Core* core;
    QTimer* fileCompleteTimer;
    QStringList nzbFileList;
    QHash<QString, QDateTime> nzbFilePathlastEnqueuedMap;
    QSet<QString> nzbFileInWatchFolderSet;
    QString currentWatchDir;
    bool firstEnqueueMethod;

    void setupConnections();
    QSet<QString> getNzbFileSetFromWatchFolder();
    void appendFileToList(const QString&);


Q_SIGNALS:


public Q_SLOTS:


private Q_SLOTS:
    void watchFileSlot(const QString&);
    void fileCompleteTimerSlot();


};

#endif // WATCHFOLDER_H
