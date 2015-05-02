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

#ifndef CLIENTSOBSERVER_H
#define CLIENTSOBSERVER_H

#include "clientsobserverbase.h"

class Core;
class StatsInfoBuilder;

class ClientsObserver : public ClientsObserverBase
{
    Q_OBJECT

public:
    explicit ClientsObserver(Core *);
    StatsInfoBuilder *getStatsInfoBuilder() const;
    void fullFileSizeUpdate(const quint64, const quint64);
    void sendFullUpdate();
    quint64 getTotalSize() const;
    void resetTotalBytesDownloaded();
    bool isSingleServer(QString &) const;
    bool isSslActive() const;

private:

    Core *mParent;
    StatsInfoBuilder *mStatsInfoBuilder;
    quint64 mTotalFiles;
    quint64 mTotalSize;

    void resetVariables();

Q_SIGNALS:
    void updateConnectionStatusSignal();
    void updateFileSizeInfoSignal(const quint64, const quint64);

public Q_SLOTS:

    void nntpClientSpeedSlot(const int);
    void connectionStatusSlot(const int);
    void encryptionStatusSlot(const bool, const QString &, const bool, const QString &, const QStringList &);
    void nntpErrorSlot(const int);
    void decrementSlot(const quint64, const int);

};

#endif // CLIENTSOBSERVER_H
