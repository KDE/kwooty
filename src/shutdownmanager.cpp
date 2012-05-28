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

#include <KDebug>
#include <KJob>
#include <KMessageBox>
#include <KProcess>
#include <KApplication>
#include <KStandardDirs>
#include <kworkspace/kworkspace.h>

#include <QDateTime>

#include "core.h"
#include "widgets/centralwidget.h"
#include "mainwindow.h"
#include "standarditemmodel.h"
#include "standarditemmodelquery.h"
#include "widgets/mystatusbar.h"
#include "kwootysettings.h"


ShutdownManager::ShutdownManager(Core* parent) : QObject (parent) {
    
    // variable initialisation :
    this->parent = parent;
    this->noActivityCounter = 0;
    this->enableSystemShutdown = false;

    // system shutdown command for gnome desktop :
    this->gnomeShutdownApplication = "/usr/bin/gnome-session-save";

    // check if system has to shudown every 10 seconds :
    this->shutdownTimerInterval = 10000;

    // store settings values :
    this->storeSettings();

    // create activity timer :
    this->activityMonitorTimer = new QTimer(this);

    // create system shutdown timer :
    this->launchShutdownTimer = new QTimer(this);

    this->setupConnections();

}


void ShutdownManager::setupConnections() {

    // check application activity (download, extract, etc...) at each timeout :
    connect(this->activityMonitorTimer, SIGNAL(timeout()), this, SLOT(retrieveCurrentJobsInfoSlot()));

    // launch system shutdown after launchShutdownTimer timeout :
    connect(this->launchShutdownTimer, SIGNAL(timeout()), this, SLOT(launchSystemShutdownSlot()));

    // parent notify that settings have changed :
    connect (parent, SIGNAL(settingsChangedSignal()), this, SLOT(settingsChangedSlot()));

    // enable or disable shutdown button according to nzb parent status:
    connect(parent->getDownloadModel(), SIGNAL(parentStatusItemChangedSignal(QStandardItem*, ItemStatusData)), this, SLOT(statusItemUpdatedSlot()));

}



void ShutdownManager::systemAboutToShutdown() {
    
    // stop timer and reset noActivityCounter :
    this->enableSystemShutdownSlot(false);

    // shutdown system automatically in 10 seconds :
    this->launchShutdownTimer->start(10000);

    // get shutdown method text :
    UtilityNamespace::SystemShutdownType systemShutdownType = this->getChosenShutdownType();
    QString shutdownMethodText = this->getShutdownMethodText(systemShutdownType);

    // finally display shutdown confirmation dialog :
    int answer = this->parent->getCentralWidget()->displayAboutToShutdownMessageBox(shutdownMethodText);

    // user has confirmed shutdown, launch shutdown right now :
    if (answer == KDialog::Yes) {

        this->launchSystemShutdownSlot();
    }
    // shutdown cancelled :
    else {
        // stop shutdown timer if cancelled bu the user :
        this->launchShutdownTimer->stop();

        // update shutdown button (not enabled/checked) :
        this->shutdownCancelledSlot();
    }
    
}



void ShutdownManager::storeSettings() {

    //settings have changed, store new values :
    this->jobsRadioButton = Settings::jobsRadioButton();
    this->timerRadioButton = Settings::timerRadioButton();
    this->pausedShutdown = Settings::pausedShutdown();
    this->scheduleDateTimeStr = Settings::scheduleDateTime().time().toString("hh:mm");
}


void ShutdownManager::updateStatusBar() {

    QString shutdownMethodText;
    QString shutdownMethodIcon;

    // if shutdown scheduler is active :
    if (this->enableSystemShutdown) {

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
        shutdownMethodIcon = iconAvailableShutdownMap.key(this->getShutdownMethodText(this->getChosenShutdownType()));

    }

    // send info to status bar :
    emit statusBarShutdownInfoSignal(shutdownMethodIcon ,shutdownMethodText);

}




void ShutdownManager::requestShutdown() {

    ShutdownManager::SessionType sessionType = this->retrieveSessionType();

    // check type of session and call proper shutdown method
    // if KDE session :
    if (sessionType == ShutdownManager::Kde) {

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
            this->handleShutdownError(i18n("Shutdown has failed (session manager can not be contacted)."));
        }

    }
    // if GNOME session :
    else if ( sessionType == ShutdownManager::Gnome2 ||
              sessionType == ShutdownManager::Gnome3 ) {

        // halt the system now :
        KProcess* shutdowProcess = new KProcess(this);
        shutdowProcess->setProgram(this->retrieveGnomeSessionProgram(sessionType), this->retrieveGnomeSessionArgs(sessionType));
        shutdowProcess->start();
        shutdowProcess->closeWriteChannel();

    }

}



void ShutdownManager::requestSuspend(Solid::PowerManagement::SleepState suspendMethod) {
    // requests a suspend of the system :
#if KDE_IS_VERSION(4, 5, 82)
    Solid::PowerManagement::requestSleep(suspendMethod, 0, 0);
#else
    Solid::Control::PowerManager::suspend(static_cast<Solid::Control::PowerManager::SuspendMethod>(suspendMethod))->start();
#endif
}



ShutdownManager::SessionType ShutdownManager::retrieveSessionType() {

    ShutdownManager::SessionType sessionType = ShutdownManager::Unknown;

    QString desktopSession;

    // check if session is a KDE session :
    desktopSession = ::getenv("KDE_FULL_SESSION");
    if (desktopSession.contains("true", Qt::CaseInsensitive)) {

        sessionType = ShutdownManager::Kde;
    }

    // check if session is a GNOME session :
    else {
        desktopSession = ::getenv("GNOME_DESKTOP_SESSION_ID");

        if (desktopSession.isEmpty()) {
            desktopSession = ::getenv("GNOME_KEYRING_CONTROL");
        }


        if (!desktopSession.isEmpty()) {

            QString program = KStandardDirs::findExe("gnome-session");

            KProcess gnomeSessionProcess;
            gnomeSessionProcess.setProgram(program, QStringList() << "--version");
            gnomeSessionProcess.start();

            if (gnomeSessionProcess.waitForFinished()) {

                bool conversionOk = false;
                int mainVersion = gnomeSessionProcess.readAll().mid(program.size() + 1).left(1).toInt(&conversionOk);

                if (conversionOk) {

                    if (mainVersion == 3) {
                        sessionType = ShutdownManager::Gnome3;
                    }
                    else if (mainVersion == 2) {
                        sessionType = ShutdownManager::Gnome2;
                    }

                }

                gnomeSessionProcess.closeWriteChannel();

            }

        }
    }

    //kDebug() << "desktopSession : " << desktopSession;

    // return kde or gnome desktop session :
    return sessionType;

}


UtilityNamespace::SystemShutdownType ShutdownManager::getChosenShutdownType() {

    UtilityNamespace::SystemShutdownType systemShutdownType = UtilityNamespace::ShutdownMethodUnknown;

    QList<UtilityNamespace::SystemShutdownType> indexShutdownTypeList = this->retrieveAvailableShutdownMethods();

    // ensure that list contains element :
    if (indexShutdownTypeList.size() > Settings::shutdownMethods()) {
        systemShutdownType = indexShutdownTypeList.at(Settings::shutdownMethods());
    }

    return systemShutdownType;

}




QList<UtilityNamespace::SystemShutdownType> ShutdownManager::retrieveAvailableShutdownMethods() {

    QList<UtilityNamespace::SystemShutdownType> indexShutdownTypeList;

    // at first add system shutdown if session has been identified as kde or gnome :
    if (this->retrieveSessionType() != ShutdownManager::Unknown) {
        indexShutdownTypeList.append(UtilityNamespace::Shutdown);
    }

    // then add supported sleep types by system :
    foreach (SleepState sleepState, Solid::PowerManagement::supportedSleepStates()) {

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



QMap<QString, QString> ShutdownManager::retrieveIconAvailableShutdownMap() {

    QMap<QString, QString>iconAvailableShutdownMap;

    foreach (UtilityNamespace::SystemShutdownType shutdownType, this->retrieveAvailableShutdownMethods()) {

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

    return iconAvailableShutdownMap;

}



QString ShutdownManager::getShutdownMethodText(UtilityNamespace::SystemShutdownType systemShutdownType) const {

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



void ShutdownManager::handleShutdownError(const QString& message) {

    this->parent->getCentralWidget()->displayShutdownErrorMessageBox(message);

    // uncheck shutdown button :
    this->shutdownCancelledSlot();

}


QString ShutdownManager::retrieveGnomeSessionProgram(const ShutdownManager::SessionType gnomeSessionType) {

    QString programName;

    if (gnomeSessionType == ShutdownManager::Gnome2) {
        programName = "gnome-session-save";
    }
    else if (gnomeSessionType == ShutdownManager::Gnome3) {
        programName = "gnome-session-quit";
    }

   return KStandardDirs::findExe(programName);
}


QStringList ShutdownManager::retrieveGnomeSessionArgs(const ShutdownManager::SessionType gnomeSessionType) {

    QStringList args;

    //TODO : maybe remove shutdown dialog for gnome sessions ... ?

    // list of arguments for gnome-session-save command line :
    if (gnomeSessionType == ShutdownManager::Gnome2) {
        args.append("--shutdown-dialog");
    }
    // list of arguments for gnome-session-quit command line :
    else if (gnomeSessionType == ShutdownManager::Gnome3) {
        args.append("--logout");
        args.append("--power-off");
    }

   return args;
}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ShutdownManager::settingsChangedSlot() {

    if ( this->jobsRadioButton != Settings::jobsRadioButton()   ||
         this->timerRadioButton != Settings::timerRadioButton() ||
         this->pausedShutdown != Settings::pausedShutdown()     ||
         this->scheduleDateTimeStr != Settings::scheduleDateTime().time().toString("hh:mm") ) {

        // update settings :
        this->storeSettings();

        // relaunch shutdown schedule according to new settings :
        this->enableSystemShutdownSlot(this->enableSystemShutdown);

    }

    this->updateStatusBar();

}


void ShutdownManager::shutdownCancelledSlot() {

    // shutdown cancelled, set system button as "not checked" :
    emit setShutdownButtonCheckedSignal(false);

    // if no more jobs are available, set system button as "not enabled" :
    if (this->parent->getModelQuery()->areJobsFinished()) {
        emit setShutdownButtonEnabledSignal(false);
    }

    // reset variables :
    this->enableSystemShutdownSlot(false);

}

void ShutdownManager::statusItemUpdatedSlot() {

    // if activity detected, set shutdown button as enabled :
    if (!this->parent->getModelQuery()->areJobsFinished()) {

        emit setShutdownButtonEnabledSignal(true);
    }
    // else if shutdown scheduler is not active, shutdown button should be disabled :
    else if (!this->enableSystemShutdown){
        emit setShutdownButtonEnabledSignal(false);
    }


}




void ShutdownManager::enableSystemShutdownSlot(bool enable) {

    this->enableSystemShutdown = enable;

    if (this->enableSystemShutdown) {

        // if shutdown when jobs are finished :
        if (Settings::jobsRadioButton()) {

            // start activity polling timer :
            if (!this->parent->getModelQuery()->areJobsFinished()) {

                this->activityMonitorTimer->start(this->shutdownTimerInterval);

            }

        }

        // if shutdown at a given time :
        if (Settings::timerRadioButton()) {

            // set Timeout to time set in settings :
            this->activityMonitorTimer->stop();
            this->activityMonitorTimer->start(Settings::scheduleDateTime().time().hour() * UtilityNamespace::HOURS_TO_MILLISECONDS +
                                              Settings::scheduleDateTime().time().minute() * UtilityNamespace::MINUTES_TO_MILLISECONDS);


        }

    }
    // if scheduling is disabled, stop monitoring :
    else {

        this->activityMonitorTimer->stop();
        this->statusItemUpdatedSlot();
        this->noActivityCounter = 0;
    }


    this->updateStatusBar();


}



void ShutdownManager::retrieveCurrentJobsInfoSlot(){


    if (Settings::jobsRadioButton()) {

        bool jobsFinished = this->parent->getModelQuery()->areJobsFinished();

        // set timer interval to lower value if jobs finished has been confirmed :
        if (jobsFinished) {

            if (this->activityMonitorTimer->interval() != 1000) {
                this->activityMonitorTimer->setInterval(1000);
            }

            this->noActivityCounter++;

            // if no more downloads / repairing / extracting processes are active 3 consecutive times, shutdown system :
            if (this->noActivityCounter == 3) {

                //display warning to user before shutdown :
                this->systemAboutToShutdown();

            }
        }
        // activity has started again, reset variables :
        else {
            this->activityMonitorTimer->setInterval(this->shutdownTimerInterval);
            this->noActivityCounter = 0;
        }
    }

    else if (Settings::timerRadioButton()) {

        // if it's time to shutdown :
        this->systemAboutToShutdown();
    }

}



void ShutdownManager::launchSystemShutdownSlot() {

    this->launchShutdownTimer->stop();

    // close dialog if shutdown has been automatically launched :
    this->parent->getCentralWidget()->closeAboutToShutdownMessageBox();

    // save potential pending data for future session restoring without asking any questions :
    parent->savePendingDownloads(this->getChosenShutdownType(), SaveSilently);

    // shutdown is launched, set system button as "not checked" :
    emit setShutdownButtonCheckedSignal(false);

    // get type of system shutdown :
    switch (this->getChosenShutdownType()) {

    case UtilityNamespace::Shutdown: {
            this->requestShutdown();
            break;
        }

    case UtilityNamespace::Standby: {
            this->requestSuspend(Solid::PowerManagement::StandbyState);
            break;
        }

    case UtilityNamespace::Suspend: {
            this->requestSuspend(Solid::PowerManagement::SuspendState);
            break;
        }

    case UtilityNamespace::Hibernate: {
            this->requestSuspend(Solid::PowerManagement::HibernateState);
            break;
        }

    default: {
            this->handleShutdownError(i18n("System shutdown type unknown, shutdown is not possible!"));
            break;
        }

    }

}


