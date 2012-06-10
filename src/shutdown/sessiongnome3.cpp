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


#include "sessiongnome3.h"

#include <KDebug>
#include <KStandardDirs>
#include <KProcess>

#include <QDBusReply>


SessionGnome3::SessionGnome3(ShutdownManager* parent) : SessionBase(parent) {

    kDebug();

    this->methodCallEnumStringMap.insert(SessionGnome3::DbusSuspend,              "Suspend");
    this->methodCallEnumStringMap.insert(SessionGnome3::DbusSuspendAllowed,       "SuspendAllowed");
    this->methodCallEnumStringMap.insert(SessionGnome3::DbusHibernate,            "Hibernate");
    this->methodCallEnumStringMap.insert(SessionGnome3::DbusHibernateAllowed,     "HibernateAllowed");

    this->dbusUpowerInterface = 0;

}



QList<UtilityNamespace::SystemShutdownType> SessionGnome3::retrieveAvailableShutdownMethods() {

    QList<UtilityNamespace::SystemShutdownType> indexShutdownTypeList;

    // at first add system shutdown :
    indexShutdownTypeList.append(UtilityNamespace::Shutdown);

    // add suspend :
    if (this->callDbusMethod(SessionGnome3::DbusSuspendAllowed)) {
        indexShutdownTypeList.append(UtilityNamespace::Suspend);
    }

    // add hibernate :
    if (this->callDbusMethod(SessionGnome3::DbusHibernateAllowed)) {
        indexShutdownTypeList.append(UtilityNamespace::Hibernate);
    }


    return indexShutdownTypeList;
}


void SessionGnome3::requestShutdown() {

    // try to shutdown system with dbus and ConsoleKit :
    QDBusInterface* dbusConsoleKitInterface = new QDBusInterface ("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", QDBusConnection::systemBus());

    // call stop method :
    dbusConsoleKitInterface->call("Stop");

    // if an error has been detected :
    if (dbusConsoleKitInterface->lastError().isValid()) {

        // last chance to shutdown system if previous command fails :
        QDBusInterface* dbusManagerInterface = new QDBusInterface ("org.gnome.SessionManager", "/org/gnome/SessionManager", "org.gnome.SessionManager", QDBusConnection::systemBus());

        // call stop method :
        dbusManagerInterface->call("Stop");

        delete dbusManagerInterface;
    }

    delete dbusConsoleKitInterface;

}


void SessionGnome3::requestSuspend() {

    SessionGnome3::DbusMethodCall suspendMethod = SessionGnome3::DbusSuspend;

    // get type of system shutdown :
    switch (this->getChosenShutdownType()) {

    case UtilityNamespace::Suspend: {
        suspendMethod = SessionGnome3::DbusSuspend;
        break;
    }

    case UtilityNamespace::Hibernate: {
        suspendMethod = SessionGnome3::DbusHibernate;
        break;
    }
    default: {
        break;
    }

    }

    this->callDbusMethod(suspendMethod);

}


bool SessionGnome3::callDbusMethod(const SessionGnome3::DbusMethodCall dbusMethodCall) {

    bool methodAvailable = false;

    // create dbus interface :
    if (!this->dbusUpowerInterface) {

        this->dbusUpowerInterface = new QDBusInterface ("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus(), this);

    }

    // call method :
    QDBusReply<bool> reply = this->dbusUpowerInterface->call(this->methodCallEnumStringMap.value(dbusMethodCall));

    // if reply is valid, get answer value to know if method is allowed :
    if (reply.isValid()) {
        methodAvailable = reply.value();
    }

    //kDebug() << "methodAvailable :" << methodAvailable;

    return methodAvailable;

}



