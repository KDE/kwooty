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

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>

#include "kwooty_export.h"
#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class ServerGroup;
class SegmentBuffer;


class KWOOTY_EXPORT ServerManager : public QObject {

    Q_OBJECT

public:
    ServerManager(CentralWidget*);
    ServerGroup* getNextTargetServer(ServerGroup*);
    ServerGroup* getServerGroupById(const int&);
    SegmentBuffer* getSegmentBuffer();
    quint64 retrieveServerDownloadSpeed(const int&) const;
    quint64 retrieveCumulatedDownloadSpeed(const int&) const;
    int getServerNumber() const;
    bool areAllServersEncrypted() const;
    bool currentIsFirstMasterAvailable(const ServerGroup*) const;    
    void downloadWithAnotherBackupServer(ServerGroup*);
    void masterServerAvailabilityChanges();
    void setLimitServerDownloadSpeed(const int&, const qint64&);
    void setBandwidthMode(const BandwidthClientMode&);


private:
    CentralWidget* parent;
    QMap<int, ServerGroup*> idServerGroupMap;
    ServerGroup* currentMasterServer;
    SegmentBuffer* segmentBuffer;

    void setupConnections();

signals:
    void serverManagerSettingsChangedSignal();


public slots:
    void settingsChangedSlot();
    void requestClientConnectionSlot();


};

#endif // SERVERMANAGER_H
