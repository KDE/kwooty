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

#include "kwooty_debug.h"

#include <QHBoxLayout>
#include <QUuid>

#include "widgets/servertabwidget.h"
#include "data/serverdata.h"
#include "kwootysettings.h"
#include "kconfiggrouphandler.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

PreferencesServer::PreferencesServer(KConfigDialog *dialog)
{

    this->dialog = dialog;

    setupUi(this);

    // dirty way to notify kCondfigDialog that a setting have changed in any server tabs widgets :
    kcfg_serverChangesNotify->hide();

    // create one tab for each server :
    this->tabWidget = new ServerTabWidget(this);

    // feedback from kConfigDialog, choose action according to button clicked by user :
    connect(this->dialog->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)),
            this,
            SLOT(slotOkClicked()));
    connect(this->dialog->button(QDialogButtonBox::Apply), SIGNAL(clicked(bool)),
            this,
            SLOT(slotApplyClicked()));
    connect(this->dialog->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)),
            this,
            SLOT(slotCancelClicked()));

    connect(this->dialog->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked(bool)),
            this,
            SLOT(slotDefaultClicked()));

    this->loadSettings();

}

void PreferencesServer::slotDefaultClicked()
{
    defaultSettings();
}

void PreferencesServer::slotCancelClicked()
{
    restorePreviousSettings();
    this->dialog->reject();
}

void PreferencesServer::slotApplyClicked()
{
    saveSettings();
}

void PreferencesServer::slotOkClicked()
{
    saveSettings();
    this->dialog->accept();
}

void PreferencesServer::restorePreviousSettings()
{

    // restore settings previously saved in file :
    while (this->tabWidget->count() != 0) {

        this->tabWidget->deleteAndRemoveTab(0);
    }

    this->loadSettings();
}

void PreferencesServer::loadSettings()
{

    int serverNumber = KConfigGroupHandler::getInstance()->readServerNumberSettings();

    for (int i = 0; i < serverNumber; ++i) {
        this->tabWidget->addNewTab();
    }

}

void PreferencesServer::saveSettings()
{
    emit saveDataSignal();
}

void PreferencesServer::defaultSettings()
{

    // set default settings :
    while (this->tabWidget->count() != 0) {

        this->tabWidget->deleteAndRemoveTab(0);
    }

    this->tabWidget->addDefaultTab();
}

