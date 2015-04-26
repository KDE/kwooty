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


#ifndef CLIENTSPERSERVEROBSERVER_H
#define CLIENTSPERSERVEROBSERVER_H


#include "clientsobserverbase.h"
#include "data/segmentinfodata.h"

class ServerGroup;

class ClientsPerServerObserver : public ClientsObserverBase
{
    Q_OBJECT


public:
    ClientsPerServerObserver(ServerGroup*);

    quint64 getDownloadSpeed() const;
    quint64 getAverageDownloadSpeed() const;
    quint64 getEffectiveMeanDownloadSpeed() const;
    SegmentInfoData getSegmentInfoData() const;
    quint64 getBytesDownloadedForCurrentSession() const;
    bool isSslActive() const;
    bool isDownloading() const;


private:
    ServerGroup* parent;
    SegmentInfoData segmentInfoData;
    quint64 downloadSpeed;
    quint64 averageDownloadSpeed;
    quint64 effectiveMeanDownloadSpeed;
    quint64 bytesDownloadedForCurrentSession;
    quint64 meanDownloadSpeedCounter;

    void setupConnections();
    void resetVariables();


Q_SIGNALS:

    void connectionStatusSignal(int);
    void encryptionStatusSignal(const bool, const QString & = QString(), const bool = false, const QString  & = QString(), const QStringList & = QStringList());
    void speedSignal(int);
    void nntpErrorSignal(const int);
    void serverStatisticsUpdateSignal(const int);

public Q_SLOTS:

    void nntpClientSpeedPerServerSlot(const SegmentInfoData);
    void connectionStatusPerServerSlot(const int);
    void encryptionStatusPerServerSlot(const bool, const QString &, const bool, const QString& , const QStringList& );
    void nntpErrorPerServerSlot(const int);

private Q_SLOTS:
    void updateDownloadSpeedSlot();

};

#endif // CLIENTSPERSERVEROBSERVER_H
