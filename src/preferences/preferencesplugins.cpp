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
#include "kwooty_debug.h"
#include <KSharedConfig>
#include <kpluginfactory.h>
#include <KLocalizedString>
#include <QPushButton>
#include <QVBoxLayout>

#include "plugins/pluginmanager.h"

PreferencesPlugins::PreferencesPlugins(KConfigDialog *kConfigDialog, PluginManager *pluginManager)
{

    this->kConfigDialog = kConfigDialog;
    this->pluginManager = pluginManager;

    this->kPluginSelector = new KPluginSelector(this);
    this->kPluginSelector->addPlugins(this->pluginManager->getPluginInfoList(), KPluginSelector::ReadConfigFile, i18n("General Plugins"));

    // setup layout :
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(this->kPluginSelector);

    // update buttons when plugins settings have changed :
    connect(this->kPluginSelector, SIGNAL(changed(bool)), this->kConfigDialog, SLOT(updateButtons()));
    connect(this->kPluginSelector, SIGNAL(changed(bool)), this, SLOT(slotEnableButtonApply(bool)));

    // config plugin has been committed, inform plugin manager :
    connect(this->kPluginSelector, SIGNAL(configCommitted(QByteArray)), this->pluginManager, SLOT(configCommittedSlot(QByteArray)));

    connect(this->kConfigDialog->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)),
            this,
            SLOT(slotOkClicked()));
    connect(this->kConfigDialog->button(QDialogButtonBox::Apply), SIGNAL(clicked(bool)),
            this,
            SLOT(slotApplyClicked()));
    connect(this->kConfigDialog->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)),
            this,
            SLOT(slotCancelClicked()));

    connect(this->kConfigDialog->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked(bool)),
            this,
            SLOT(slotDefaultClicked()));

}

void PreferencesPlugins::slotEnableButtonApply(bool enable)
{
    this->kConfigDialog->button(QDialogButtonBox::Apply)->setEnabled(enable);
}

void PreferencesPlugins::slotDefaultClicked()
{
    //TODO ?
}

void PreferencesPlugins::slotCancelClicked()
{
    // plugins settings may have been changed but user cancel changes,
    // reload plugins to previous states :
    this->kPluginSelector->load();
    this->kConfigDialog->reject();
}

void PreferencesPlugins::slotApplyClicked()
{
    // plugins settings have been changed, save them and update plugins loading :
    this->kPluginSelector->save();

    // update plugins to load/unload :
    this->pluginManager->loadPlugins();
}

void PreferencesPlugins::slotOkClicked()
{
    // plugins settings have been changed, save them and update plugins loading :
    this->kPluginSelector->save();

    // update plugins to load/unload :
    this->pluginManager->loadPlugins();
    this->kConfigDialog->accept();
}
