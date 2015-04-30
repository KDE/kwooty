/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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

#include "categoriesplugin.h"

#include "kwooty_debug.h"
#include <KPluginLoader>
#include <kgenericfactory.h>

#include "plugin.h"
#include "mainwindow.h"
#include "core.h"

K_PLUGIN_FACTORY(PluginFactory, registerPlugin<CategoriesPlugin>();)
K_EXPORT_PLUGIN(PluginFactory("kwooty_categoriesplugin"))

CategoriesPlugin::CategoriesPlugin(QObject *parent, const QList<QVariant> &) : Plugin(parent)
{

}

CategoriesPlugin::~CategoriesPlugin()
{

}

void CategoriesPlugin::load()
{
    this->categories = new Categories(this);
}

void CategoriesPlugin::unload()
{

    this->categories->unload();
    delete this->categories;

}

void CategoriesPlugin::configUpdated()
{
    this->categories->settingsChanged();
}
#include "categoriesplugin.moc"
