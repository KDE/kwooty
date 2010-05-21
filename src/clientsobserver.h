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


#include <QObject>

#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class StatsInfoBuilder;


class ClientsObserver : public QObject
{
    Q_OBJECT


public:
    ClientsObserver(CentralWidget* parent = 0);
    StatsInfoBuilder* getStatsInfoBuilder() const;
    void fullFileSizeUpdate(const quint64, const quint64);
    void sendFullUpdate();

    int getTotalConnections() const;
    bool isSslActive() const;
    bool isCertificateVerified() const;
    int getNttpErrorStatus() const;
    QString getEncryptionMethod() const;
    QString getIssuerOrgranisation() const;
    quint64 getTotalBytesDownloaded() const;
    void resetTotalBytesDownloaded();
    quint64 getTotalSize() const;

private:

    CentralWidget* parent;
    StatsInfoBuilder* statsInfoBuilder;
    quint64 totalFiles;
    quint64 totalSize;
    quint64 totalBytesDownloaded;


    // from status bar :
    QString encryptionMethod;
    QString issuerOrgranisation;
    int totalConnections;
    int nttpErrorStatus;
    bool sslActive;
    bool certificateVerified;

    void resetVariables();



signals:
    void updateConnectionStatusSignal();
    void updateFileSizeInfoSignal(const quint64, const quint64);


public slots:

    void nntpClientSpeedSlot(const int);
    void connectionStatusSlot(const int);
    void encryptionStatusSlot(const bool, const QString, const bool, const QString);
    void nntpErrorSlot(const int);
    void decrementSlot(const quint64, const int);





};

#endif // CLIENTSOBSERVER_H
