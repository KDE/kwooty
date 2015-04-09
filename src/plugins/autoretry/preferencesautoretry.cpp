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

#include "preferencesautoretry.h"


#include "kwooty_debug.h"
#include <KGlobal>

#include <QHBoxLayout>

#include "kwooty_autoretrysettings.h"
#include <kgenericfactory.h>


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<PreferencesAutoRetry>();)
K_EXPORT_PLUGIN(PluginFactory("kwooty_autoretrysettings"))


PreferencesAutoRetry::PreferencesAutoRetry(QWidget* parent, const QVariantList& args) :
KCModule(parent, args) {

    // set layout config layout :
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);

    // setup ui file :
    QWidget* widget = new QWidget(this);
    this->preferencesAutoRetryUi.setupUi(widget);
    layout->addWidget(widget);

    // add main kconfigskeleton :
    this->addConfig(AutoRetrySettings::self(), widget);

}


PreferencesAutoRetry::~PreferencesAutoRetry() {

}


void PreferencesAutoRetry::load(){
    KCModule::load();
}


void PreferencesAutoRetry::save(){
    KCModule::save();

}

#include "preferencesautoretry.moc"
