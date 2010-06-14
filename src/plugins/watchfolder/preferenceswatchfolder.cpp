/***************************************************************************
 *   Copyright (C) 2009 by Xavier Lefage                                   *
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

#include "preferenceswatchfolder.h"


#include <KDebug>
#include <KGlobal>
#include <QHBoxLayout>

#include "watchfoldersettings.h"
#include <kgenericfactory.h>


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<PreferencesWatchFolder>();)
K_EXPORT_PLUGIN(PluginFactory("kwooty_watchfoldersettings"))

PreferencesWatchFolder::PreferencesWatchFolder( QWidget* parent, const QVariantList& args ) :
KCModule( PluginFactory::componentData(), parent, args )
{

    kDebug() << parent;

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);

    QWidget* widget = new QWidget(this);
    this->preferencesWatchFolderUi.setupUi(widget);
    layout->addWidget(widget);

    this->addConfig(WatchFolderSettings::self(), widget);


    this->preferencesWatchFolderUi.kcfg_watchFolder->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);


}


PreferencesWatchFolder::~PreferencesWatchFolder() {

    kDebug();
    //delete this->preferencesWatchFolderUi;
}

void PreferencesWatchFolder::load(){
    kDebug();
    KCModule::load();
}

void PreferencesWatchFolder::save(){

    kDebug();
    KCModule::save();

}
