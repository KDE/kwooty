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

#include "preferencesserver.h"

#include <KDebug>

#include <QHBoxLayout>
#include <QUuid>

#include "widgets/servertabwidget.h"
#include "data/serverdata.h"
#include "kwootysettings.h"
#include "kconfiggrouphandler.h"
#include "utility.h"
using namespace UtilityNamespace;


PreferencesServer::PreferencesServer(KConfigDialog* dialog)
{

    this->dialog = dialog;

    setupUi(this);

    // dirty way to notify kCondfigDialog that a setting have changed in any server tabs widgets :
    kcfg_serverChangesNotify->hide();
    connect (kcfg_serverChangesNotify, SIGNAL(textChanged (const QString&)), this, SLOT(textChangedSlot (const QString&)));  

    tabWidget = new ServerTabWidget(this);

    // feedback from kConfigDialog, choose action according to button clicked by user :
    connect(this->dialog,
            SIGNAL(buttonClicked(KDialog::ButtonCode)),
            this,
            SLOT(configButtonClickedSlot(KDialog::ButtonCode)));


    this->loadSettings();

}



void PreferencesServer::configButtonClickedSlot(KDialog::ButtonCode button) {


    switch (button) {

    case KDialog::Cancel: case KDialog::Close: {

            // restore previous settings :
            this->restorePreviousSettings();
            break;
        }

    case KDialog::Ok: case KDialog::Apply: {

            // save them :
            this->saveSettings();
            break;
        }

    case KDialog::Default:    {
            //TODO :
            kDebug() << "Default button";
            break;
        }

    default: {
            break;

        }

    }

}



void PreferencesServer::textChangedSlot(const QString& textChangedSlot) {

    if (textChangedSlot.isEmpty()) {
        // TODO :
        kDebug() << "resetToDefaults !!";
    }

}



void PreferencesServer::restorePreviousSettings() {

    // restore settings previously saved in file :
    while(this->tabWidget->count() != 0) {

        this->tabWidget->deleteAndRemoveTab(0);
    }

    this->loadSettings();

}


void PreferencesServer::loadSettings() {

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    for (int i = 0; i < serverNumber; i++) {
        this->tabWidget->addNewTab();
    }


}


void PreferencesServer::saveSettings() {
   emit saveDataSignal();
}

