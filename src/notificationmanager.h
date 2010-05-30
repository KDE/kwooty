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


#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <KNotification>

#include <QString>
#include <QHash>

#include "utility.h"
using namespace UtilityNamespace;


class NotificationManager
{


public:
    NotificationManager();

    static void init();
    static void sendJobFinishedEvent(const UtilityNamespace::ItemStatus&, const QString&);
    static void sendInsufficientDiskSpaceEvent(const QString&);

private:

    static void sendEvent(const QString&, const QString&, KNotification::NotificationFlags = KNotification::CloseOnTimeout);
    static QHash<UtilityNamespace::ItemStatus, QString> finishSatusTextMap;

};

#endif // NOTIFICATIONMANAGER_H
