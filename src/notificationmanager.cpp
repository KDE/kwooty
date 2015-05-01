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

#include "notificationmanager.h"

#include <KLocalizedString>
#include "kwooty_debug.h"

#include "core.h"
#include "statsinfobuilder.h"
#include "observers/clientsobserver.h"
#include "observers/queuefileobserver.h"
#include "kwootysettings.h"

NotificationManager::NotificationManager(Core *parent) : QObject(parent)
{

    this->mParent = parent;

    this->init();
    this->setupConnections();
}

void NotificationManager::init()
{

    this->mFinishSatusTextMap.insert(DownloadFinishStatus,      i18n("Download finished"));
    this->mFinishSatusTextMap.insert(DecodeFinishStatus,        i18n("Decoding finished"));
    this->mFinishSatusTextMap.insert(VerifyFinishedStatus,      i18n("Verify finished"));
    this->mFinishSatusTextMap.insert(RepairFinishedStatus,      i18n("Repair complete"));
    this->mFinishSatusTextMap.insert(ExtractFinishedStatus,     i18n("Extract finished"));

}

void NotificationManager::setupConnections()
{

    // queueFileObserver emit signal when a job is finished :
    connect(this->mParent->getQueueFileObserver(),
            SIGNAL(jobFinishSignal(UtilityNamespace::ItemStatus,QString)),
            this,
            SLOT(jobFinishSlot(UtilityNamespace::ItemStatus,QString)));

    // statsInfoBuilder emit signal when a disk space is insufficient :
    connect(this->mParent->getClientsObserver()->getStatsInfoBuilder(),
            SIGNAL(insufficientDiskSpaceSignal(QString)),
            this,
            SLOT(insufficientDiskSpaceSlot(QString)));

}

void NotificationManager::jobFinishSlot(const UtilityNamespace::ItemStatus status, const QString &message)
{
    this->sendEvent("jobFinished", QStringLiteral("%1 - %2").arg(this->mFinishSatusTextMap.value(status)).arg(message));
}

void NotificationManager::insufficientDiskSpaceSlot(const QString &message)
{
    this->sendEvent("insufficientDiskSpace", message, KNotification::Persistent);
}

void NotificationManager::sendEvent(const QString &eventId, const QString &message, KNotification::NotificationFlags notificationFlag)
{

    // if notifications have to be displayed :
    if (Settings::notification()) {

        // send event :
        KNotification::event(eventId, message, QPixmap(), (QWidget *)this->mParent->parent(), notificationFlag);

    }

}
