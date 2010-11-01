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


#ifndef STATSINFOBUILDER_H
#define STATSINFOBUILDER_H

#include <QObject>
#include <QTimer>

#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class ClientsObserver;
class StandardItemModel;

class StatsInfoBuilder : public QObject
{

    Q_OBJECT

public:
    static const int SPEED_AVERAGE_SECONDS = 2;

    StatsInfoBuilder(ClientsObserver* clientsObserver = 0, CentralWidget* parent = 0);
    void sendFullUpdate();
    QString getTotalTimeValue() const;
    QString getCurrentTimeValue() const;
    QString getTimeLabel() const;
    QString getNzbNameDownloading() const;
    QString getDownloadSpeedReadableStr() const;
    QTimer* getDownloadSpeedTimer() const;


private:   
    CentralWidget* parent;
    ClientsObserver* clientsObserver;
    StandardItemModel* downloadModel;
    QTimer* downloadSpeedTimer;
    QString nzbNameDownloading;
    QString currentTimeValue;
    QString totalTimeValue;
    QString timeLabel;
    QString downloadSpeedReadableStr;
    QModelIndex parentStateIndex;
    int timeoutCounter;
    int meanSpeedActiveCounter;
    int meanDownloadSpeedTotal;
    int downloadSpeedTotal;
    int meanDownloadSpeedCurrent;
    int downloadSpeedCurrent;
    UtilityNamespace::FreeDiskSpace previousDiskSpaceStatus;

    void setupConnections();
    void resetVariables();
    void computeTimeInfo();
    void retrieveFreeDiskSpace();
    void retrieveQueuedFilesInfo(bool&, bool&);
    QString calculateArrivalTime(const quint32&);
    QString calculateRemainingTime(const quint32&);
    void computeMeanSpeed(const int&, int&);



signals:

    void updateDownloadSpeedInfoSignal(const QString);
    void updateTimeInfoSignal(const bool);
    void updateFreeSpaceSignal(const UtilityNamespace::FreeDiskSpace, const QString = QString(), const int = 0);
    void insufficientDiskSpaceSignal(const QString);

public slots:

    void settingsChangedSlot();


private slots:

    void updateDownloadSpeedSlot();


};

#endif // STATSINFOBUILDER_H
