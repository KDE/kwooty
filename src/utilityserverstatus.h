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


#ifndef UTILITYSERVERSTATUS_H
#define UTILITYSERVERSTATUS_H

#include <KIcon>

#include "observers/clientsobserverbase.h"


namespace UtilityServerStatusNamespace
{

// process priority :
enum ServerConnectionIcon {
    DisconnectedIcon,
    ConnectedNormalIcon,
    ConnectedEncryptedIcon,
    ConnectedEncryptedOverlayIcon,
    ConnectedDownloadingIcon
};


}

using namespace UtilityServerStatusNamespace;


class UtilityServerStatus {

public:

    UtilityServerStatus();

    enum EncryptionMethodDisplay {
        DisplayEncryptionMethod,
        DoNotDisplayEncryptionMethod
    };

    static KIcon getConnectionIcon(const ServerConnectionIcon&);
    static QPixmap getConnectionPixmap(const ServerConnectionIcon&);
    static ServerConnectionIcon buildConnectionStringFromStatus(const ClientsObserverBase*, QString&, EncryptionMethodDisplay = DisplayEncryptionMethod);
    static QString buildSslHandshakeStatus(const ClientsObserverBase*);
    static QString buildConnectionToolTip(const ClientsObserverBase*, const QString&, const QString&);
    static QString getServerModeString(UtilityNamespace::BackupServerMode);


private:


};

#endif // UTILITYSERVERSTATUS_H
