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


#include "schedulerplugin.h"

#include "kwooty_debug.h"
#include <KPluginLoader>

#include "plugin.h"
#include "mainwindow.h"
#include "core.h"


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<SchedulerPlugin>();)
K_EXPORT_PLUGIN(PluginFactory("kwooty_schedulerplugin"))


SchedulerPlugin::SchedulerPlugin(QObject* parent, const QList<QVariant>&) : Plugin(parent)
{

}


SchedulerPlugin::~SchedulerPlugin()
{

}



void SchedulerPlugin::load() {

    this->scheduler = new Scheduler(this);
}

void SchedulerPlugin::unload() {

    // disable speed limit when plugin is unloaded :
    this->scheduler->disableSpeedLimit();

    delete this->scheduler;

}


void SchedulerPlugin::configUpdated() {
    this->scheduler->settingsChanged();
}
#include "schedulerplugin.moc"

