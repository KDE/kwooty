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


#ifndef FOLDERWATCHER_H
#define FOLDERWATCHER_H

#include <KUrl>
#include <kdirwatch.h>


#include <QObject>
#include <QStringList>
#include <QTimer>

#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;


class FolderWatcher : public QObject
{

    Q_OBJECT

public:
    FolderWatcher(CentralWidget* parent);


private:
    static const int MAX_LIST_SIZE = 10;

    KDirWatch* kDirWatch;
    CentralWidget* centralWidget;
    QTimer* fileCompleteTimer;
    QStringList nzbFileList;
    QString currentWatchDir;

    void setupConnections();


signals:


public slots:


private slots:
    void watchFileSlot(const QString&);
    void fileCompleteTimerSlot();
    void settingsChangedSlot();


};

#endif // FOLDERWATCHER_H
