/***************************************************************************
 *   Copyright (C) 2009 by Xavier Lefage                                   *
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

#ifndef CLIENTMANAGERCONN_H
#define CLIENTMANAGERCONN_H

#include "data/segmentdata.h"
#include "data/serverdata.h"

#include "utilities/utility.h"
using namespace UtilityNamespace;

class NntpClient;
class ServerGroup;

class ClientManagerConn : public QObject
{

    Q_OBJECT

public:
    ClientManagerConn(ServerGroup *, int, int);
    ~ClientManagerConn();
    NntpClient *getNntpClient();
    ServerGroup *getServerGroup();
    ServerData getServerData() const;
    int getClientId() const;
    int getConnectionDelay() const;
    bool isClientReady() const;
    bool isMasterServer() const;
    bool isDisabledBackupServer() const;
    bool isBandwidthNotNeeded() const;
    bool isBandwidthLimited() const;
    bool isBandwidthFull() const;
    void noSegmentAvailable();
    void processNextSegment(const SegmentData &);
    void setBandwidthMode(const BandwidthClientMode &);
    void handleFirstConnection();

private:
    NntpClient *nntpClient;
    ServerGroup *parent;
    BandwidthClientMode bandwidthClientMode;
    int clientId;
    int connectionDelay;

public Q_SLOTS:
    void dataHasArrivedSlot();
    void resetConnectionSlot();
    void disconnectRequestSlot();
    void connectRequestSlot();
    void limitDownloadSpeedSlot(BandwidthClientMode);
    void startupCompleteSlot();

private Q_SLOTS:
    void initSlot();

};

#endif // CLIENTMANAGERCONN_H

