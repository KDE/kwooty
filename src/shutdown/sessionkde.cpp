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


#include "sessionkde.h"

#include <KDebug>
#include <KJob>
#include <kworkspace/kworkspace.h>


SessionKde::SessionKde(ShutdownManager* parent) : SessionBase(parent) {

    kDebug();

}



QList<UtilityNamespace::SystemShutdownType> SessionKde::retrieveAvailableShutdownMethods() {

    QList<UtilityNamespace::SystemShutdownType> indexShutdownTypeList;

    // at first add system shutdown :
    indexShutdownTypeList.append(UtilityNamespace::Shutdown);

    // then add supported sleep types by system :
    foreach (const SleepState& sleepState, Solid::PowerManagement::supportedSleepStates()) {

        // add standby :
        if (sleepState == StandbyState) {
            indexShutdownTypeList.append(UtilityNamespace::Standby);
        }
        // add suspend :
        if (sleepState == SuspendState) {
            indexShutdownTypeList.append(UtilityNamespace::Suspend);
        }
        // add hibernate :
        if (sleepState == HibernateState) {
            indexShutdownTypeList.append(UtilityNamespace::Hibernate);
        }
    }

    return indexShutdownTypeList;

}


void SessionKde::requestShutdown() {

    // check if shutdown has any chance of succeeding :
    bool canShutDown = KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmNo,
                                               KWorkSpace::ShutdownTypeHalt,
                                               KWorkSpace::ShutdownModeForceNow);


    // if shutdown is possible :
    if (canShutDown) {

        // halt the system now :
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmNo,
                                    KWorkSpace::ShutdownTypeHalt,
                                    KWorkSpace::ShutdownModeForceNow);
    }
    else {
        this->shutdownManager->handleShutdownError(i18n("Shutdown has failed (session manager can not be contacted)."));
    }


}


void SessionKde::requestSuspend() {

    Solid::PowerManagement::SleepState suspendMethod = StandbyState;

    // get type of system shutdown :
    switch (this->getChosenShutdownType()) {


    case UtilityNamespace::Standby: {
        suspendMethod = Solid::PowerManagement::StandbyState;
        break;
    }

    case UtilityNamespace::Suspend: {
        suspendMethod = Solid::PowerManagement::SuspendState;
        break;
    }

    case UtilityNamespace::Hibernate: {
        suspendMethod = Solid::PowerManagement::HibernateState;
        break;
    }
    default: {
        break;
    }

    }
    // requests a suspend of the system :
#if KDE_IS_VERSION(4, 5, 82)
    Solid::PowerManagement::requestSleep(suspendMethod, 0, 0);
#else
    Solid::Control::PowerManager::suspend(static_cast<Solid::Control::PowerManager::SuspendMethod>(suspendMethod))->start();
#endif


}

