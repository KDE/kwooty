/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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


#include "scheduler.h"

#include <KDebug>

#include <QTime>
#include <QDate>
#include <QStandardItem>

#include "mainwindow.h"
#include "centralwidget.h"
#include "schedulerplugin.h"
#include "kwooty_schedulersettings.h"
#include "fileoperations.h"
#include "schedulerfilehandler.h"

#include "servermanager.h"
#include "servergroup.h"
#include "observers/clientsperserverobserver.h"


using namespace SchedulerNamespace;

Scheduler::Scheduler(SchedulerPlugin* parent) :  QObject(parent) {

    kDebug() << parent << this;

    this->centralWidget = parent->getCore()->getCentralWidget();

    kDebug() <<"LOAD";
    this->schedulerModel = SchedulerFileHandler().loadModelFromFile(this);

    kDebug() <<"LOAD DONE"; 

    this->serverManager = parent->getCore()->getCentralWidget()->getServerManager();

    // start nzb file process timer
    this->schedulerTimer = new QTimer(this);
    this->schedulerTimer->start(5000);

    // update scheduler behavior :
    this->settingsChanged();

    // setup signals/slots connections :
    this->setupConnections();

}


Scheduler::~Scheduler() {

}


void Scheduler::setupConnections() {

    // when time-out occurs, check if nzb files from list can be processed
    connect(this->schedulerTimer,
            SIGNAL(timeout()),
            this,
            SLOT(schedulerTimerSlot()));


}




//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void Scheduler::schedulerTimerSlot() {

    QTime currentTime = QTime::currentTime();
    int column = (currentTime.hour() * 60 + currentTime.minute()) / 30;
    int row = QDate().currentDate().dayOfWeek();

    QStandardItem* item = this->schedulerModel->item(row, column);

    int downloadLimitStatus = item->data(DownloadLimitRole).toInt();



    int serverNumber = this->serverManager->getServerNumber();

    int totalDownloadSpeed = 0;

    for (int i = 0; i < serverNumber; i++) {
        totalDownloadSpeed += this->serverManager->retrieveServerDownloadSpeed(i);
    }

    for (int i = 0; i < serverNumber; i++) {

        if (totalDownloadSpeed > SchedulerSettings::downloadLimitSpinBox()) {
            int ratio = static_cast<qint64>(this->serverManager->retrieveServerDownloadSpeed(i) * SchedulerSettings::downloadLimitSpinBox()) / totalDownloadSpeed;

            kDebug() << "serveur group : " << i << ", limited download speed : " << ratio << " KiB/s";
        }


    }


    kDebug() << "row, column, downloadStatus : " << row << column << downloadLimitStatus;

    //kDebug () << "download speed per server : " << this->serverManager->retrieveDownloadSpeedServersList() << SchedulerSettings::downloadLimitSpinBox();

}



void Scheduler::settingsChanged() {

    kDebug() << "settings changed";
    // reload settings from just saved config file :
    SchedulerSettings::self()->readConfig();

    this->schedulerModel = SchedulerFileHandler().loadModelFromFile(this);

}


