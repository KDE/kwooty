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


#ifndef INFOCOLLECTORDISPATCHER_H
#define INFOCOLLECTORDISPATCHER_H


#include <QTimer>

#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class StandardItemModel;

class InfoCollectorDispatcher : public QObject
{
    Q_OBJECT

public:
    InfoCollectorDispatcher(CentralWidget* parent = 0);

     void fullFileSizeUpdate(const quint64, const quint64);

private:

    static const int SPEED_AVERAGE_SECONDS = 2;

    CentralWidget* parent;
    StandardItemModel* downloadModel;

    QTimer* downloadSpeedTimer;
    quint64 totalFiles;
    quint64 totalSize;
    quint64 totalBytesDownloaded;
    int meanDownloadSpeedKB;
    int downloadSpeedKB;
    int timeoutCounter;
    int meanSpeedActiveCounter;
    QModelIndex parentStateIndex;

    void resetVariables();
    void setupConnections();
    void computeTimeInfo();
    void retrieveFreeDiskSpace();
    void retrieveQueuedFilesInfo(bool&, bool&);
    QString calculateArrivalTime(const quint32&);
    QString calculateRemainingTime(const quint32&);


signals:

    void updateSizeInfoSignal(const QString);
    void updateFileInfoSignal(const QString);
    void updateDownloadSpeedInfoSignal(const QString);
    void updateTimeInfoSignal(const QString);
    void updateFreeSpaceSignal(const UtilityNamespace::FreeDiskSpace, const QString = QString(), const int = 0);



public slots:

    void decrementSlot(const quint64, const int);
    void nntpClientspeedSlot(const int);
    void updateDownloadSpeedSlot();
    void settingsChangedSlot();




};

#endif // INFOCOLLECTORDISPATCHER_H
