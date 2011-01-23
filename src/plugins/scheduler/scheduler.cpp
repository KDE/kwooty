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
#include <kdirwatch.h>
#include <klocale.h>

#include <QFileInfo>
#include <QDir>

#include "mainwindow.h"
#include "centralwidget.h"
#include "schedulerplugin.h"
#include "kwooty_schedulersettings.h"
#include "fileoperations.h"


Scheduler::Scheduler(SchedulerPlugin* parent) :  QObject(parent) {

    this->centralWidget = parent->getCore()->getCentralWidget();

    this->kDirWatch = new KDirWatch(this);

    // init folder to watch :
    this->settingsChanged();

    // start nzb file process timer
    this->fileCompleteTimer = new QTimer(this);
    this->fileCompleteTimer->start(500);

    // setup signals/slots connections :
    this->setupConnections();

}


Scheduler::~Scheduler() {

}


void Scheduler::setupConnections() {

    // kDirWatch notify that a file has been created :
    connect (this->kDirWatch,
             SIGNAL(created(const QString&)),
             this,
             SLOT(watchFileSlot(const QString& )));


    // kDirWatch notify that a file has changed :
    connect (this->kDirWatch,
             SIGNAL(dirty(const QString&)),
             this,
             SLOT(watchFileSlot(const QString& )));

    // when time-out occurs, check if nzb files from list can be processed
    connect(this->fileCompleteTimer,
            SIGNAL(timeout()),
            this,
            SLOT(fileCompleteTimerSlot()));


}





void Scheduler::appendFileToList(const QString& filePath) {

}



//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void Scheduler::watchFileSlot(const QString& filePath) {


}



void Scheduler::fileCompleteTimerSlot() {


}



void Scheduler::settingsChanged() {

    // reload settings from just saved config file :
    SchedulerSettings::self()->readConfig();


}


