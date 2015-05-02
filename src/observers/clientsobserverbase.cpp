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

#include "clientsobserverbase.h"

ClientsObserverBase::ClientsObserverBase(QObject *parent) : QObject(parent)
{

}

void ClientsObserverBase::resetVariables()
{

    mTotalBytesDownloaded = 0;
    mTotalConnections = 0;
    mSslActive = false;
    mCertificateVerified = false;
    mSslErrors = QStringList();
    mNttpErrorStatus = NoError;

}

void ClientsObserverBase::addBytesDownloaded(const int &bytesDownloaded)
{
    mTotalBytesDownloaded += bytesDownloaded;
}

void ClientsObserverBase::updateTotalConnections(const int &connectionStatus)
{

    if (connectionStatus == Connected) {
        mTotalConnections++;
    }

    if (connectionStatus == Disconnected) {
        mTotalConnections--;
    }
}

void ClientsObserverBase::setNntpErrorStatus(const int &nttpErrorStatus)
{
    mNttpErrorStatus = nttpErrorStatus;
}

void ClientsObserverBase::setSslHandshakeParameters(const bool &sslActive, const QString &encryptionMethod, const bool &certificateVerified, const QString &issuerOrgranisation, const QStringList &sslErrors)
{
    mEncryptionMethod = encryptionMethod;
    mSslActive = sslActive;
    mCertificateVerified = certificateVerified;
    mIssuerOrgranisation = issuerOrgranisation;
    mSslErrors = sslErrors;
}

bool ClientsObserverBase::isConnected() const
{

    return (mTotalConnections > 0);
}

QString ClientsObserverBase::getEncryptionMethod() const
{
    return mEncryptionMethod;
}

QString ClientsObserverBase::getIssuerOrgranisation() const
{
    return mIssuerOrgranisation;
}

QStringList ClientsObserverBase::getSslErrors() const
{
    return mSslErrors;
}

int ClientsObserverBase::getTotalConnections() const
{
    return mTotalConnections;
}

bool ClientsObserverBase::isCertificateVerified() const
{
    return mCertificateVerified;
}

int ClientsObserverBase::getNttpErrorStatus() const
{
    return mNttpErrorStatus;
}

quint64 ClientsObserverBase::getTotalBytesDownloaded() const
{
    return mTotalBytesDownloaded;
}

