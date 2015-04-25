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


#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QObject>

#include "utilities/utility.h"
using namespace UtilityNamespace;


class Core;
class MainWindow;
class SideBarWidget;
class ServerManager;


class SideBar : public QObject {


    Q_OBJECT


public:
    SideBar(MainWindow*);
    SideBarWidget* getSideBarWidget();
    void saveState();
    void loadState();
    void serverStatisticsUpdate(const int);

private:
    Core* core;
    SideBarWidget* sideBarWidget;
    ServerManager* serverManager;
    bool stateRestored;

    void createSideBarWidgets();
    void setupConnections();


Q_SIGNALS:


public Q_SLOTS:
    void activeSlot(bool);
    void serverManagerSettingsChangedSlot();


private Q_SLOTS:



};

#endif // SIDEBAR_H
