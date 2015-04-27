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


#ifndef SERVERSPEEDMANAGER_H
#define SERVERSPEEDMANAGER_H

#include <QObject>
#include <QTimer>

#include "utilities/utility.h"
using namespace UtilityNamespace;


class ServerGroup;

class ServerSpeedManager : public QObject {

    Q_OBJECT

public:
    ServerSpeedManager(ServerGroup* parent);

    void setDownloadSpeedLimitInBytes(const qint64&);
    qint64 getDownloadSpeedLimitInBytes() const;
    int getEnabledClientNumber() const;
    void setBandwidthMode(const BandwidthClientMode&);

private:

    enum ClientSpeedPriority {
        NoPriorityClient,
        LowPriorityClient,
        HighPriorityClient
    };

    enum SpeedManagementStatus {
        NoChangeSpeed,
        ReduceSpeed,
        IncreaseSpeed
    };

    static const int SPEED_MONITORING_TIME = 15;

    ServerGroup* parent;
    QTimer* downloadSpeedTimer;
    qint64 downloadSpeedLimitInBytes;
    ClientSpeedPriority clientSpeedPriority;
    SpeedManagementStatus speedManagementStatus;
    int speedTooLowCounter;

    bool disableClientForRateControl() const;
    void setupConnections();
    void manageClientsNumber(const SpeedManagementStatus&);
    void resetVariables();


Q_SIGNALS:
    void limitDownloadSpeedSignal(BandwidthClientMode);
private Q_SLOTS:
    void adjustDownloadSpeedSlot();


};

#endif // SERVERSPEEDMANAGER_H
