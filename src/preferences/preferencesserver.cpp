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

#include "preferencesserver.h"
#include <KDebug>

PreferencesServer::PreferencesServer()
{
    setupUi(this);

    this->setupConnections();

}


void PreferencesServer::setupConnections() {

    // check/uncheck ssl checkbox according to port value:
    connect (kcfg_port, SIGNAL(valueChanged (int)), this, SLOT(portValueChangedSlot(int)));

}



void PreferencesServer::portValueChangedSlot(int portValue) {

    // if ports usually used for SSL are met :
    if (portValue == 563 || portValue == 443) {
        kcfg_enableSSL->setCheckState(Qt::Checked);
    }
    // else is ports usually used for normal connections are met :
    else {
        kcfg_enableSSL->setCheckState(Qt::Unchecked);
    }

}
