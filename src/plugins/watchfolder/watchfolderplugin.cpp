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


#include "watchfolderplugin.h"

#include <KDebug>
#include <KPluginLoader>
#include <kgenericfactory.h>

#include "plugin.h"
#include "mainwindow.h"
#include "centralwidget.h"


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<WatchFolderPlugin>();)
K_EXPORT_PLUGIN(PluginFactory("kwooty_watchfolderplugin"))

WatchFolderPlugin::WatchFolderPlugin(QObject* parent, const QList<QVariant>&) : Plugin(PluginFactory::componentData(), parent)
{

}


WatchFolderPlugin::~WatchFolderPlugin()
{
    kDebug() << "DELETE !!";

}



void WatchFolderPlugin::load() {

    kDebug() << "LOAD ... !!";
    this->watchFolder = new WatchFolder(this->getCore()->getCentralWidget());
    kDebug() << "LOAD OK!!";
}

void WatchFolderPlugin::unload() {

    kDebug() << "UNLOAD ... !!";
    delete this->watchFolder;
    kDebug() << "UNLOAD OK !!";
}


void WatchFolderPlugin::configUpdated() {

    kDebug() << "CONFIG  ... !!";
    this->watchFolder->settingsChanged();
    kDebug() << "CONFIG UPDATED !!";
}

