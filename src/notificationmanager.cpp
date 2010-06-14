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

#include <KLocale>
#include <KDebug>

#include "centralwidget.h"
#include "clientsobserver.h"
#include "queuefileobserver.h"
#include "statsinfobuilder.h"
#include "settings.h"



NotificationManager::NotificationManager(CentralWidget* parent) : QObject(parent) {

    this->parent = parent;

    this->init();
    this->setupConnections();
}



void NotificationManager::init() {

    this->finishSatusTextMap.insert(DownloadFinishStatus,      i18n("Download finished"));
    this->finishSatusTextMap.insert(DecodeFinishStatus,        i18n("Decoding finished"));
    this->finishSatusTextMap.insert(VerifyFinishedStatus,      i18n("Verify finished"));
    this->finishSatusTextMap.insert(RepairFinishedStatus,      i18n("Repair complete"));
    this->finishSatusTextMap.insert(ExtractFinishedStatus,     i18n("Extract finished"));

}

void NotificationManager::setupConnections() {

    // queueFileObserver emit signal when a job is finished :
    connect(this->parent->getQueueFileObserver(),
            SIGNAL(jobFinishSignal(const UtilityNamespace::ItemStatus, const QString)),
            this,
            SLOT(jobFinishSlot(const UtilityNamespace::ItemStatus, const QString)));


    // statsInfoBuilder emit signal when a disk space is insufficient :
    connect(this->parent->getClientsObserver()->getStatsInfoBuilder(),
            SIGNAL(insufficientDiskSpaceSignal(const QString)),
            this,
            SLOT(insufficientDiskSpaceSlot(const QString)));


}



void NotificationManager::jobFinishSlot(const UtilityNamespace::ItemStatus status, const QString message) {
    this->sendEvent("jobFinished", i18n("%1 - %2", this->finishSatusTextMap.value(status), message));
}


void NotificationManager::insufficientDiskSpaceSlot(const QString message) {
    this->sendEvent("insufficientDiskSpace", message, KNotification::Persistent);
}



void NotificationManager::sendEvent(const QString& eventId, const QString& message, KNotification::NotificationFlags notificationFlag) {

    // if notifications have to be displayed :
    if (Settings::notification()) {

        // send event :
        KNotification::event(eventId, message, QPixmap(), (QWidget*)this->parent->parent(), notificationFlag);

    }

}
