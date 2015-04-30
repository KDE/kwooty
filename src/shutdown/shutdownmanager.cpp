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

#include "shutdownmanager.h"

#include "kwooty_debug.h"
#include <KJob>
#include <KMessageBox>
#include <KApplication>
#include <KStandardDirs>

#include <QDateTime>
#include <QProcess>

#include "core.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "kwootysettings.h"
#include "widgets/mystatusbar.h"
#include "widgets/centralwidget.h"
#include "shutdown/sessionbase.h"
#include "shutdown/sessionkde.h"
#include "shutdown/sessiongnome3.h"
#include "shutdown/sessiongnome2.h"

ShutdownManager::ShutdownManager(Core *parent) : QObject(parent)
{

    // variable initialisation :
    this->mParent = parent;
    this->mNoActivityCounter = 0;
    this->mEnableSystemShutdown = false;

    // check if system has to shudown every 10 seconds :
    this->mShutdownTimerInterval = 10000;

    // store settings values :
    this->storeSettings();

    // create activity timer :
    this->mActivityMonitorTimer = new QTimer(this);

    // create system shutdown timer :
    this->mLaunchShutdownTimer = new QTimer(this);

    this->setupConnections();

    // retrieve type of session right now :
    this->retrieveSession();

}

void ShutdownManager::setupConnections()
{

    // check application activity (download, extract, etc...) at each timeout :
    connect(this->mActivityMonitorTimer, SIGNAL(timeout()), this, SLOT(retrieveCurrentJobsInfoSlot()));

    // launch system shutdown after launchShutdownTimer timeout :
    connect(this->mLaunchShutdownTimer, SIGNAL(timeout()), this, SLOT(launchSystemShutdownSlot()));

    // parent notify that settings have changed :
    connect(mParent, SIGNAL(settingsChangedSignal()), this, SLOT(settingsChangedSlot()));

    // enable or disable shutdown button according to nzb parent status:
    connect(mParent->getDownloadModel(), SIGNAL(parentStatusItemChangedSignal(QStandardItem*,ItemStatusData)), this, SLOT(statusItemUpdatedSlot()));

}

void ShutdownManager::systemAboutToShutdown()
{

    if (this->mSession) {

        // stop timer and reset noActivityCounter :
        this->enableSystemShutdownSlot(false);

        // shutdown system automatically in 10 seconds :
        this->mLaunchShutdownTimer->start(10000);

        // text by default :
        QString shutdownMethodText = this->getShutdownMethodText(UtilityNamespace::Shutdown);

        // get shutdown method text :
        UtilityNamespace::SystemShutdownType systemShutdownType = this->mSession->getChosenShutdownType();
        shutdownMethodText = this->getShutdownMethodText(systemShutdownType);

        // finally display shutdown confirmation dialog :
        int answer = this->mParent->getCentralWidget()->displayAboutToShutdownMessageBox(shutdownMethodText);

        // user has confirmed shutdown, launch shutdown right now :
        if (answer == KDialog::Yes) {

            this->launchSystemShutdownSlot();
        }
        // shutdown cancelled :
        else {
            // stop shutdown timer if cancelled bu the user :
            this->mLaunchShutdownTimer->stop();

            // update shutdown button (not enabled/checked) :
            this->shutdownCancelledSlot();
        }

    }

}

void ShutdownManager::storeSettings()
{

    //settings have changed, store new values :
    this->mJobsRadioButton = Settings::jobsRadioButton();
    this->mTimerRadioButton = Settings::timerRadioButton();
    this->mPausedShutdown = Settings::pausedShutdown();
    this->mScheduleDateTimeStr = Settings::scheduleDateTime().time().toString("hh:mm");
}

void ShutdownManager::updateStatusBar()
{

    QString shutdownMethodText;
    QString shutdownMethodIcon;

    if (this->mSession) {

        // if shutdown scheduler is active :
        if (this->mEnableSystemShutdown) {

            // define shutown method text :
            if (Settings::jobsRadioButton()) {
                shutdownMethodText = i18n("when jobs complete");
            }

            if (Settings::timerRadioButton()) {
                // get scheduled shutdown time :
                QDateTime dateTime = QDateTime::currentDateTime();
                dateTime = dateTime.addSecs(Settings::scheduleDateTime().time().hour() * 3600 +
                                            Settings::scheduleDateTime().time().minute() * 60);

                shutdownMethodText = i18nc("shutdown time notifier in status bar, example : 'shutdown icon' at 12:56",
                                           "at %1", dateTime.toString(Utility::getSystemTimeFormat("hh:mm")));
            }

            // define shutdown method icon :
            QMap<QString, QString>iconAvailableShutdownMap = this->retrieveIconAvailableShutdownMap();
            shutdownMethodIcon = iconAvailableShutdownMap.key(this->getShutdownMethodText(this->mSession->getChosenShutdownType()));

        }

        // send info to status bar :
        emit statusBarShutdownInfoSignal(shutdownMethodIcon , shutdownMethodText);

    }

}

void ShutdownManager::retrieveSession()
{

    this->mSession = 0;

    QString desktopSession;

    // check if session is a KDE session :
    desktopSession = ::getenv("KDE_FULL_SESSION");
    if (desktopSession.contains("true", Qt::CaseInsensitive)) {

        this->mSession = new SessionKde(this);
    }

    // check if session is a GNOME session :
    else {
        desktopSession = ::getenv("GNOME_DESKTOP_SESSION_ID");

        if (desktopSession.isEmpty()) {
            desktopSession = ::getenv("GNOME_KEYRING_CONTROL");
        }
        if (desktopSession.isEmpty()) {
            desktopSession = ::getenv("MATE_DESKTOP_SESSION_ID");
        }

        if (!desktopSession.isEmpty()) {

            QString program = KStandardDirs::findExe("gnome-session");

            QProcess gnomeSessionProcess;
            gnomeSessionProcess.start(program, QStringList() << "--version");

            if (gnomeSessionProcess.waitForFinished()) {

                bool conversionOk = false;
                int mainVersion = gnomeSessionProcess.readAll().mid(program.size() + 1).left(1).toInt(&conversionOk);

                if (conversionOk) {

                    if (mainVersion == 3) {
                        this->mSession = new SessionGnome3(this);

                    } else if (mainVersion == 2) {
                        this->mSession = new SessionGnome2(this);

                    }

                }

                gnomeSessionProcess.closeWriteChannel();

            }

        }
    }

}

QMap<QString, QString> ShutdownManager::retrieveIconAvailableShutdownMap()
{

    QMap<QString, QString>iconAvailableShutdownMap;

    if (this->mSession) {

        foreach (const UtilityNamespace::SystemShutdownType &shutdownType, this->mSession->retrieveAvailableShutdownMethods()) {

            // add shutdown :
            if (shutdownType == UtilityNamespace::Shutdown) {
                iconAvailableShutdownMap.insertMulti("system-shutdown", this->getShutdownMethodText(shutdownType));

            }
            // add standby :
            if (shutdownType == UtilityNamespace::Standby) {
                iconAvailableShutdownMap.insertMulti("system-suspend", this->getShutdownMethodText(shutdownType));

            }
            // add suspend :
            if (shutdownType == UtilityNamespace::Suspend) {
                iconAvailableShutdownMap.insertMulti("system-suspend", this->getShutdownMethodText(shutdownType));

            }
            // add hibernate :
            if (shutdownType == UtilityNamespace::Hibernate) {
                iconAvailableShutdownMap.insertMulti("system-suspend-hibernate", this->getShutdownMethodText(shutdownType));

            }
        }

    }

    return iconAvailableShutdownMap;

}

QString ShutdownManager::getShutdownMethodText(UtilityNamespace::SystemShutdownType systemShutdownType) const
{

    QString shutdownText;

    // shutdown :
    if (systemShutdownType == UtilityNamespace::Shutdown) {
        shutdownText = i18n("Shutdown");
    }

    // standby :
    else if (systemShutdownType == UtilityNamespace::Standby) {
        shutdownText = i18n("Standby");
    }

    // suspend :
    else if (systemShutdownType == UtilityNamespace::Suspend) {
        shutdownText = i18n("Suspend to RAM");
    }

    // hibernate :
    else if (systemShutdownType == UtilityNamespace::Hibernate) {
        shutdownText = i18n("Suspend to disk");
    }

    return shutdownText;
}

void ShutdownManager::handleShutdownError(const QString &message)
{

    this->mParent->getCentralWidget()->displaySorryMessageBox(message);

    // uncheck shutdown button :
    this->shutdownCancelledSlot();

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ShutdownManager::settingsChangedSlot()
{

    if (this->mJobsRadioButton != Settings::jobsRadioButton()   ||
            this->mTimerRadioButton != Settings::timerRadioButton() ||
            this->mPausedShutdown != Settings::pausedShutdown()     ||
            this->mScheduleDateTimeStr != Settings::scheduleDateTime().time().toString("hh:mm")) {

        // update settings :
        this->storeSettings();

        // relaunch shutdown schedule according to new settings :
        this->enableSystemShutdownSlot(this->mEnableSystemShutdown);

    }

    this->updateStatusBar();

}

void ShutdownManager::shutdownCancelledSlot()
{

    // shutdown cancelled, set system button as "not checked" :
    emit setShutdownButtonCheckedSignal(false);

    // if no more jobs are available, set system button as "not enabled" :
    if (this->mParent->getModelQuery()->areJobsFinished()) {
        emit setShutdownButtonEnabledSignal(false);
    }

    // reset variables :
    this->enableSystemShutdownSlot(false);

}

void ShutdownManager::statusItemUpdatedSlot()
{

    // if activity detected, set shutdown button as enabled :
    if (!this->mParent->getModelQuery()->areJobsFinished()) {

        emit setShutdownButtonEnabledSignal(true);
    }
    // else if shutdown scheduler is not active, shutdown button should be disabled :
    else if (!this->mEnableSystemShutdown) {
        emit setShutdownButtonEnabledSignal(false);
    }

}

void ShutdownManager::enableSystemShutdownSlot(bool enable)
{

    this->mEnableSystemShutdown = enable;

    if (this->mEnableSystemShutdown) {

        // if shutdown when jobs are finished :
        if (Settings::jobsRadioButton()) {

            // start activity polling timer :
            if (!this->mParent->getModelQuery()->areJobsFinished()) {

                this->mActivityMonitorTimer->start(this->mShutdownTimerInterval);

            }

        }

        // if shutdown at a given time :
        if (Settings::timerRadioButton()) {

            // set Timeout to time set in settings :
            this->mActivityMonitorTimer->stop();
            this->mActivityMonitorTimer->start(Settings::scheduleDateTime().time().hour() * UtilityNamespace::HOURS_TO_MILLISECONDS +
                                               Settings::scheduleDateTime().time().minute() * UtilityNamespace::MINUTES_TO_MILLISECONDS);

        }

    }
    // if scheduling is disabled, stop monitoring :
    else {

        this->mActivityMonitorTimer->stop();
        this->statusItemUpdatedSlot();
        this->mNoActivityCounter = 0;
    }

    this->updateStatusBar();

}

void ShutdownManager::retrieveCurrentJobsInfoSlot()
{

    if (Settings::jobsRadioButton()) {

        bool jobsFinished = this->mParent->getModelQuery()->areJobsFinished();

        // set timer interval to lower value if jobs finished has been confirmed :
        if (jobsFinished) {

            if (this->mActivityMonitorTimer->interval() != 1000) {
                this->mActivityMonitorTimer->setInterval(1000);
            }

            this->mNoActivityCounter++;

            // if no more downloads / repairing / extracting processes are active 3 consecutive times, shutdown system :
            if (this->mNoActivityCounter == 3) {

                //display warning to user before shutdown :
                this->systemAboutToShutdown();

            }
        }
        // activity has started again, reset variables :
        else {
            this->mActivityMonitorTimer->setInterval(this->mShutdownTimerInterval);
            this->mNoActivityCounter = 0;
        }
    }

    else if (Settings::timerRadioButton()) {

        // if it's time to shutdown :
        this->systemAboutToShutdown();
    }

}

void ShutdownManager::launchSystemShutdownSlot()
{

    this->mLaunchShutdownTimer->stop();

    // close dialog if shutdown has been automatically launched :
    this->mParent->getCentralWidget()->closeAboutToShutdownMessageBox();

    if (this->mSession) {

        // save potential pending data for future session restoring without asking any questions :
        mParent->savePendingDownloads(this->mSession->getChosenShutdownType(), SaveSilently);

        // shutdown is launched, set system button as "not checked" :
        emit setShutdownButtonCheckedSignal(false);

        // launch system shutdown according to session used :
        this->mSession->launchSystemShutdown();

    } else {
        qCDebug(KWOOTY_LOG) << "session has not been identified, shutdown not available !";
    }

}

