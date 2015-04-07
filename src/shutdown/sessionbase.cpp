/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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


#include "sessionbase.h"

#include "kwooty_debug.h"
#include "kwootysettings.h"


SessionBase::SessionBase(ShutdownManager* shutdownManager) : QObject(shutdownManager) {

    this->shutdownManager = shutdownManager;
}


void SessionBase::launchSystemShutdown() {


    // get type of system shutdown :
    switch (this->getChosenShutdownType()) {

    case UtilityNamespace::Shutdown: {
        this->requestShutdown();
        break;
    }

    case UtilityNamespace::Standby:
    case UtilityNamespace::Suspend:
    case UtilityNamespace::Hibernate:{

        this->requestSuspend();
        break;
    }


    default: {
        this->shutdownManager->handleShutdownError(i18n("System shutdown type unknown, shutdown is not possible!"));
        break;
    }

    }

}


UtilityNamespace::SystemShutdownType SessionBase::getChosenShutdownType() {

    UtilityNamespace::SystemShutdownType systemShutdownType = UtilityNamespace::ShutdownMethodUnknown;

    QList<UtilityNamespace::SystemShutdownType> indexShutdownTypeList = this->retrieveAvailableShutdownMethods();

    // ensure that list contains element :
    if (indexShutdownTypeList.size() > Settings::shutdownMethods()) {
        systemShutdownType = indexShutdownTypeList.at(Settings::shutdownMethods());
    }

    return systemShutdownType;

}



