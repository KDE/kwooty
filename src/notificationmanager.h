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

#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;

class NotificationManager : public QObject
{

    Q_OBJECT

public:
    explicit NotificationManager(Core *);

public Q_SLOTS:
    void jobFinishSlot(const UtilityNamespace::ItemStatus, const QString &);
    void insufficientDiskSpaceSlot(const QString &);

private:
    void init();
    void setupConnections();
    void sendEvent(const QString &, const QString &, KNotification::NotificationFlags = KNotification::CloseOnTimeout);

    Core *mParent;
    QHash<UtilityNamespace::ItemStatus, QString> mFinishSatusTextMap;
};

#endif // NOTIFICATIONMANAGER_H
