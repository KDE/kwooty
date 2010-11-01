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

#include <KLocale>
#include "utilityserverstatus.h"

#include "utility.h"
using namespace UtilityNamespace;


UtilityServerStatus::UtilityServerStatus() {

}




bool UtilityServerStatus::buildConnectionStringFromStatus(const ClientsObserverBase* clientsObserver, QString& connectionIconStr, QString& connection, EncryptionMethodDisplay encryptionMethodDisplay) {

    bool displayOverlay = false;

    int totalConnections = clientsObserver->getTotalConnections();

    if (totalConnections == 0) {

        connectionIconStr = "weather-clear-night";
        connection = i18n("Disconnected");

        int nttpErrorStatus = clientsObserver->getNttpErrorStatus();
        // detail disconnection issues to user :
        if (nttpErrorStatus == HostNotFound){
            connection = i18n("Disconnected (Host not found)");
        }

        if (nttpErrorStatus == ConnectionRefused) {
            connection = i18n("Disconnected (Connection refused)");
        }

        if (nttpErrorStatus == RemoteHostClosed) {
            connection = i18n("Disconnected (Closed by remote host)");
        }

        if (nttpErrorStatus == SslHandshakeFailed) {
            connection = i18n("Disconnected (SSL handshake failed)");
        }

        //kDebug() << "nttpErrorStatus = " << nttpErrorStatus;
        if (nttpErrorStatus == AuthenticationNeeded) {
            connection = i18n("Disconnected (Authentication required)");
        }


        if (nttpErrorStatus == AuthenticationFailed) {
            connection = i18n("Disconnected (Authentication Denied)");
        }

    }
    else {
        // set connection icon :
        connectionIconStr = "applications-internet";
        connection = i18n("Connected: <numid>%1</numid>", totalConnections);

        if (clientsObserver->isSslActive()) {

            // if SSL active use another connection icon :
            connectionIconStr = "document-encrypt";

            // display type of encryption method used by server :
            QString encryptionMethod = clientsObserver->getEncryptionMethod();

            if (!encryptionMethod.isEmpty() &&
                (encryptionMethodDisplay == DisplayEncryptionMethod) ) {
                connection = connection + " :: " + encryptionMethod;
            }

            // display overlay only if connected to server with ssl connection and with certificate not verified :
            if (!clientsObserver->isCertificateVerified()) {
                displayOverlay = true;
            }

        }

    }

    return displayOverlay;

}



QString UtilityServerStatus::buildConnectionToolTip(const ClientsObserverBase* clientsObserver, const QString& connection, const QString& serverName) {

    QString toolTipStr;

    // if totalConnections == 0, client is disconnected :
    if (clientsObserver->getTotalConnections() == 0) {
        toolTipStr.append(connection);
    }

    else {
        // set host name info :
        toolTipStr.append(i18n("Connected to %1<br>", serverName));

        // set SSL connection info :
        if (clientsObserver->isSslActive()) {

            toolTipStr.append(i18n("Connection is SSL encrypted"));

            QString encryptionMethod = clientsObserver->getEncryptionMethod();
            if (!encryptionMethod.isEmpty()) {
                toolTipStr.append(i18nc("type of ssl encryption method", ": %1", encryptionMethod));
            }

            toolTipStr.append("<br>");

            if (clientsObserver->isCertificateVerified()) {
                toolTipStr.append(i18n("Certificate <b>verified</b> by %1", clientsObserver->getIssuerOrgranisation()));
            }
            else {
                toolTipStr.append(i18n("Certificate <b>can not be verified</b> "));

                // add ssl errors encountered :
                QStringList sslErrorList = clientsObserver->getSslErrors();

                if (!sslErrorList.isEmpty()) {

                    QString errorListSeparator = "<li>";
                    toolTipStr.append(i18np("(%1 error during SSL handshake): %2",
                                            "(%1 errors during SSL handshake): %2",
                                            sslErrorList.size(),
                                            "<ul style=\"margin-top:0px; margin-bottom:0px;\">" +
                                            errorListSeparator + sslErrorList.join(errorListSeparator)) +
                                      "</ul>");
                }

            }

        }
        else {

            toolTipStr.append(i18n("Connection is not encrypted"));
        }

    }

    return toolTipStr;

}



QString UtilityServerStatus::getServerModeString(UtilityNamespace::BackupServerMode backupServerMode) {

    QString serverMode;

    switch (backupServerMode) {

    case PassiveServer: {
            serverMode = i18n("Passive");
            break;
        }
    case ActiveServer: {
            serverMode = i18n("Active");
            break;
        }
    case FailoverServer: {
            serverMode = i18n("Failover");
            break;
        }
    case DisabledServer: {
            serverMode = i18n("Server Disabled");
            break;
        }

    }

    return serverMode;

}

