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

#include "utilityserverstatus.h"

#include <KDebug>
#include <KLocalizedString>

#include "utilityiconpainting.h"
#include "utility.h"
using namespace UtilityNamespace;


UtilityServerStatus::UtilityServerStatus() {

}


KIcon UtilityServerStatus::getConnectionIcon(const ServerConnectionIcon& serverConnectionIcon) {
    return KIcon(UtilityServerStatus::getConnectionPixmap(serverConnectionIcon));
}


QPixmap UtilityServerStatus::getConnectionPixmap(const ServerConnectionIcon& serverConnectionIcon) {

    QPixmap pixmap;
    QString iconStr;

    switch (serverConnectionIcon) {

    case InitIcon: case DisconnectedIcon: case ConnectedNormalIcon: {
        iconStr = "applications-internet";
        break;
    }

    case ConnectedEncryptedIcon: case ConnectedEncryptedOverlayIcon: {
        iconStr = "document-encrypt";
        break;
    }

    case ConnectedDownloadingIcon: {
        iconStr = "mail-receive";
        break;
    }

    }

    // if server is disconnected, build a grayed icon :
    if (serverConnectionIcon == DisconnectedIcon) {
        pixmap = UtilityIconPainting::getInstance()->buildGrayIcon(iconStr);
    }
    // if server is connected with a not trusted ssl connection, add an overlay warning icon :
    else if (serverConnectionIcon == ConnectedEncryptedOverlayIcon) {
        pixmap = UtilityIconPainting::getInstance()->blendOverLayEmblem("emblem-important", KIcon(iconStr));
    }
    else {
        pixmap = SmallIcon(iconStr);
    }

    return pixmap;
}




ServerConnectionIcon UtilityServerStatus::buildConnectionStringFromStatus(const ClientsObserverBase* clientsObserver, QString& connection, EncryptionMethodDisplay encryptionMethodDisplay) {

    // by default consider that client is not connected to host :
    ServerConnectionIcon serverConnectionIcon = DisconnectedIcon;

    int totalConnections = clientsObserver->getTotalConnections();

    if (totalConnections == 0) {

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
        serverConnectionIcon = ConnectedNormalIcon;

        connection = i18n("Connected: <numid>%1</numid>", totalConnections);

        if (clientsObserver->isSslActive()) {

            // if SSL active use another connection icon :
            serverConnectionIcon = ConnectedEncryptedIcon;

            // display type of encryption method used by server :
            QString encryptionMethod = clientsObserver->getEncryptionMethod();

            if ( !encryptionMethod.isEmpty() &&
                 (encryptionMethodDisplay == DisplayEncryptionMethod) ) {

                connection = connection + " :: " + encryptionMethod;

            }

            // display overlay only if connected to server with ssl connection and with certificate not verified :
            if (!clientsObserver->isCertificateVerified()) {

                serverConnectionIcon = ConnectedEncryptedOverlayIcon;

            }

        }

    }

    return serverConnectionIcon;

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
            toolTipStr.append(UtilityServerStatus::buildSslHandshakeStatus(clientsObserver));

        }
        else {

            toolTipStr.append(i18n("Connection is not encrypted"));
        }

    }

    return toolTipStr;

}



QString UtilityServerStatus::buildSslHandshakeStatus(const ClientsObserverBase* clientsObserver) {

    QString sslHandshakeStatus;

    if (clientsObserver->isCertificateVerified()) {
        sslHandshakeStatus.append(i18n("Certificate <b>verified</b> by %1", clientsObserver->getIssuerOrgranisation()));
    }
    else {
        sslHandshakeStatus.append(i18n("Certificate <b>can not be verified</b> "));

        // add ssl errors encountered :
        QStringList sslErrorList = clientsObserver->getSslErrors();

        if (!sslErrorList.isEmpty()) {

            QString errorListSeparator = "<li>";
            sslHandshakeStatus.append(i18np("(%1 error during SSL handshake): %2",
                                            "(%1 errors during SSL handshake): %2",
                                            sslErrorList.size(),
                                            "<ul style=\"margin-top:0px; margin-bottom:0px;\">" +
                                            errorListSeparator + sslErrorList.join(errorListSeparator)) +
                                            "</ul>");
        }

    }   

    return sslHandshakeStatus;

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

