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

#include "pluginmanager.h"

#include "kwooty_debug.h"
#include <KServiceTypeTrader>
#include <KSharedConfig>
#include "mainwindow.h"

PluginManager::PluginManager(MainWindow *core) : QObject(core)
{

    mCore = core;

    // retrieve all plugins associated to kwooty :
    QString pluginVersionQuery = QString("[X-Kwooty-Plugin-Version] == %1").arg(KWOOTY_PLUGIN_VERSION);
    mPluginInfoList = KPluginInfo::fromServices(KServiceTypeTrader::self()->query("Kwooty/Plugin", pluginVersionQuery));

}

PluginManager::~PluginManager()
{

}

void PluginManager::loadPlugins()
{

    KConfigGroup kConfigGroup = KSharedConfig::openConfig()->group("Plugins");

    foreach (const KPluginInfo &currentPluginInfo, mPluginInfoList) {

        // look in config file if the plugin have to be loaded :
        QString entryReadStr = kConfigGroup.readEntry(QString("%1Enabled").arg(currentPluginInfo.pluginName()), "");

        bool pluginEnable;

        // if no config previously stored found, check if plugin has to be loaded by default :
        if (entryReadStr == "") {

            pluginEnable = currentPluginInfo.isPluginEnabledByDefault();
        }
        // config has been found, enable or disable plugin accordingly :
        else {

            pluginEnable = kConfigGroup.readEntry<bool>(QString("%1Enabled").arg(currentPluginInfo.pluginName()), true);

        }

        //qCDebug(KWOOTY_LOG) << "plugin enabled ? :" << pluginEnable;

        // load plugin :
        if (pluginEnable && !mLoadedInfoPluginMap.contains(currentPluginInfo)) {
            loadCurrentPlugin(currentPluginInfo);
        }
        // unload plugin :
        else if (!pluginEnable && mLoadedInfoPluginMap.contains(currentPluginInfo)) {
            unloadCurrentPlugin(currentPluginInfo);
        }

    }

}

void PluginManager::loadCurrentPlugin(const KPluginInfo &currentPluginInfo)
{

    KPluginFactory *factory = KPluginLoader(currentPluginInfo.service()->library()).factory();
    if (factory) {

        // instantiate plugin :
        Plugin* plugin = factory->create<Plugin>(this);

        if (plugin) {

            //qCDebug(KWOOTY_LOG) << "Load plugin:" << currentPluginInfo.service()->name();

            // give full access to plugin :
            plugin->setCore(mCore);

            // load current plugin :
            plugin->load();

            mLoadedInfoPluginMap.insert(currentPluginInfo, plugin);

        } else {
            qCDebug(KWOOTY_LOG) << "Plugin can not be created:" << currentPluginInfo.service()->library();
        }

    }
    // factory is null :
    else {
        qCDebug(KWOOTY_LOG) << "KPluginFactory could not load the plugin:" << currentPluginInfo.service()->library();

    }

}

void PluginManager::unloadCurrentPlugin(const KPluginInfo &currentPluginInfo)
{

    // remove plugin from map :
    Plugin *pluginToUnload = mLoadedInfoPluginMap.take(currentPluginInfo);

    // check plugin validity before unload :
    if (pluginToUnload) {

        pluginToUnload->unload();
        delete pluginToUnload;

    }

}

void PluginManager::configCommittedSlot(const QByteArray &componentName)
{

    // look for plugin whose config has been updated :
    foreach (Plugin *currentPlugin, mLoadedInfoPluginMap.values()) {
#if 0 //PORT KF5
        // corresponding plugin has been found :
        if (componentName == currentPlugin->componentData().componentName()) {

            // update settings to current plugin :
            currentPlugin->configUpdated();
            break;
        }
#endif
    }

}

QList<KPluginInfo> PluginManager::getPluginInfoList()
{

    return mPluginInfoList;
}

