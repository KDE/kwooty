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

#include "uniqueapp.h"

#include <KIcon>
#include <KCmdLineArgs>
#include <KDebug>
#include <KUrl>

#include "mainwindow.h"


UniqueApp::UniqueApp() : KUniqueApplication()
{
    this->kwootyInstance = false;
}

UniqueApp::~UniqueApp()
{
}




int UniqueApp::newInstance()
{

    // create a new instance :
    if (!this->kwootyInstance) {

        this->kwootyInstance = true;

        this->setWindowIcon(KIcon("kwooty"));

        mainWindow = new MainWindow();
        mainWindow->show();
    }

    // instance already exists :
    if (this->kwootyInstance) {

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        // open nzb files set as argument :
        for (int i = 0; i < args->count(); i++) {

            this->mainWindow->openFileByExternalApp(args->url(i));

        }

        args->clear();
    }

    return 0;
}
