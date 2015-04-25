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


#ifndef CLIENTSOBSERVERBASE_H
#define CLIENTSOBSERVERBASE_H

#include <QObject>
#include <QStringList>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class ClientsObserverBase : public QObject
{

    Q_OBJECT
public:

    ClientsObserverBase(QObject*);
    int getTotalConnections() const;
    bool isCertificateVerified() const;
    int getNttpErrorStatus() const;
    QString getEncryptionMethod() const;
    QString getIssuerOrgranisation() const;
    QStringList getSslErrors() const;
    quint64 getTotalBytesDownloaded() const;
    bool isConnected() const;
    virtual bool isSslActive() const = 0;


protected:
    quint64 totalBytesDownloaded;
    QStringList sslErrors;
    QString encryptionMethod;
    QString issuerOrgranisation;
    int totalConnections;
    int nttpErrorStatus;
    bool sslActive;
    bool certificateVerified;

    void resetVariables();
    void addBytesDownloaded(const int&);
    void updateTotalConnections(const int&);
    void setNntpErrorStatus(const int&);
    void setSslHandshakeParameters(const bool&, const QString&, const bool&, const QString&, const QStringList&);


private:


Q_SIGNALS:

public Q_SLOTS:


protected Q_SLOTS:


};

#endif // CLIENTSOBSERVERBASE_H
