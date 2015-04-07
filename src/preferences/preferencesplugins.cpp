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


#include "preferencesplugins.h"

#include <KPluginInfo>
#include <KDebug>
#include <KSharedConfig>
#include <kpluginfactory.h>
#include <KLocalizedString>
#include <QVBoxLayout>

#include "plugins/pluginmanager.h"


PreferencesPlugins::PreferencesPlugins(KConfigDialog* kConfigDialog, PluginManager* pluginManager) {


    this->kConfigDialog = kConfigDialog;
    this->pluginManager = pluginManager;

    this->kPluginSelector = new KPluginSelector(this);
    this->kPluginSelector->addPlugins(this->pluginManager->getPluginInfoList(), KPluginSelector::ReadConfigFile, i18n("General Plugins"));

    // setup layout :
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(this->kPluginSelector);

    // update buttons when plugins settings have changed :
    connect(this->kPluginSelector, SIGNAL(changed(bool)), this->kConfigDialog, SLOT(updateButtons()));
    connect(this->kPluginSelector, SIGNAL(changed(bool)), this->kConfigDialog, SLOT(enableButtonApply(bool)));

    // config plugin has been committed, inform plugin manager :
    connect(this->kPluginSelector, SIGNAL(configCommitted(const QByteArray&)), this->pluginManager, SLOT(configCommittedSlot(const QByteArray&)));

    // feedback from kConfigDialog, choose action according to button clicked by user :
    connect(this->kConfigDialog, SIGNAL(buttonClicked(KDialog::ButtonCode)), this, SLOT(configButtonClickedSlot(KDialog::ButtonCode)));

}



void PreferencesPlugins::configButtonClickedSlot(KDialog::ButtonCode button) {

    switch (button) {

    case KDialog::Cancel: case KDialog::Close: {

            // plugins settings may have been changed but user cancel changes,
            // reload plugins to previous states :
            this->kPluginSelector->load();
            break;
        }

    case KDialog::Ok: case KDialog::Apply: {

            // plugins settings have been changed, save them and update plugins loading :
            this->kPluginSelector->save();

            // update plugins to load/unload :
            this->pluginManager->loadPlugins();
            break;
        }

    default: {
            break;

        }

    }

}


