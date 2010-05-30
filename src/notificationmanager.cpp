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

#include "uniqueapp.h"
#include "settings.h"


QHash<UtilityNamespace::ItemStatus, QString> NotificationManager::finishSatusTextMap;


NotificationManager::NotificationManager() {

}



void NotificationManager::init() {

    finishSatusTextMap.insert(DownloadFinishStatus,      i18n("Download finished"));
    finishSatusTextMap.insert(DecodeFinishStatus,        i18n("Decoding finished"));
    finishSatusTextMap.insert(VerifyFinishedStatus,      i18n("Verify finished"));
    finishSatusTextMap.insert(RepairFinishedStatus,      i18n("Repair complete"));
    finishSatusTextMap.insert(ExtractFinishedStatus,     i18n("Extract finished"));

}


void NotificationManager::sendJobFinishedEvent(const UtilityNamespace::ItemStatus& status, const QString& message) {
    sendEvent("jobFinished", i18n("%1 - %2", finishSatusTextMap.value(status), message));
}


void NotificationManager::sendInsufficientDiskSpaceEvent(const QString& message) {
    sendEvent("insufficientDiskSpace", message, KNotification::Persistent);
}



void NotificationManager::sendEvent(const QString& eventId, const QString& message, KNotification::NotificationFlags notificationFlag) {

    // if notifications have to be displayed :
    if (Settings::notification()) {

        // send event :
        KNotification::event(eventId, message, QPixmap(), (QWidget*)UniqueApp::mainwindow(), notificationFlag);

    }

}
