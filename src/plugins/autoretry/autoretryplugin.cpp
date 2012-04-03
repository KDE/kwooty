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


#include "autoretryplugin.h"

#include <KDebug>
#include <KPluginLoader>
#include <kgenericfactory.h>

#include "plugin.h"
#include "mainwindow.h"
#include "core.h"


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<AutoRetryPlugin>();)
K_EXPORT_PLUGIN(PluginFactory("kwooty_autoretryplugin"))


AutoRetryPlugin::AutoRetryPlugin(QObject* parent, const QList<QVariant>&) : Plugin(PluginFactory::componentData(), parent)
{

}


AutoRetryPlugin::~AutoRetryPlugin()
{

}



void AutoRetryPlugin::load() {
    this->autoRetry = new AutoRetry(this);
}

void AutoRetryPlugin::unload() {
    delete this->autoRetry;
}


void AutoRetryPlugin::configUpdated() {
    this->autoRetry->settingsChanged();
}

