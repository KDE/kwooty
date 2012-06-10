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


#ifndef SESSIONGNOME3_H
#define SESSIONGNOME3_H


#include "sessionbase.h"

#include <QDBusInterface>

class SessionGnome3 : public SessionBase {


public:

    // type of Dbus methods called :
    enum DbusMethodCall {
        DbusStopAllowed,
        DbusSuspendAllowed,
        DbusHibernateAllowed,
        DbusStop,
        DbusSuspend,
        DbusHibernate
    };


    SessionGnome3(ShutdownManager*);
    QList<UtilityNamespace::SystemShutdownType> retrieveAvailableShutdownMethods();

private:

    QMap<SessionGnome3::DbusMethodCall, QString> methodCallEnumStringMap;
    QDBusInterface* dbusUpowerInterface;

    void requestShutdown();
    void requestSuspend();
    bool callDbusMethod(const SessionGnome3::DbusMethodCall);

};

#endif // SESSIONGNOME3_H
