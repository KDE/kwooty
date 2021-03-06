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
#include <kstartupinfo.h>

#include "mainwindow.h"

UniqueApp::UniqueApp() : KUniqueApplication()
{
    this->kwootyInstance = false;
}

UniqueApp::~UniqueApp()
{
    if (this->mainWindow) {
        delete this->mainWindow;
    }
}




int UniqueApp::newInstance() {

    // create a new instance :
    if (!this->kwootyInstance) {

        this->setWindowIcon(KIcon("kwooty"));
        this->kwootyInstance = true;
        this->mainWindow = new MainWindow();

    }

    // instance already exists :
    if ( this->kwootyInstance ){

        if (this->mainWindow ) {

            KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

            // open nzb files set as argument :
            for (int i = 0; i < args->count(); i++) {

                this->mainWindow->openFileWithFileMode(args->url(i), UtilityNamespace::OpenWith);

            }

            // if kwooty has been called without arguments :
            if (args->count() == 0) {

                // display main window if it only visible is systray :
                if ( !this->isSessionRestored() &&
                     !this->mainWindow->isVisible() ) {

                    this->mainWindow->show();

                }
            }

            // if nzb files are present in arguments :
            else {

                // shown only main window if it is not only present in systray :
                if (this->mainWindow->isVisible()) {
                    this->mainWindow->show();
                }

                args->clear();

                KStartupInfo::setNewStartupId(this->mainWindow, this->startupId());
            }

        }
        // main window may not exist yet if for instance kwallet promps the user to open the wallet :
        else {
            kDebug() << "mainWindow not ready yet !";
        }

    }

    return 0;
}


