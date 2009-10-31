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

#include <KUniqueApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KIcon>
#include <KDebug>

#include "mainwindow.h"


int main (int argc, char *argv[])
{
    KAboutData aboutData("kwooty",
                         "kwooty",
                         ki18n("Kwooty"), "0.1.2",
                         ki18n("Kwooty is a friendly nzb usenet binary client.\n It also supports automatic file repairing and archive extraction."),
                         KAboutData::License_GPL,
                         ki18n("Copyright (c) 2009 Xavier Lefage"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    KUniqueApplication app;
    app.setWindowIcon(KIcon("kwooty"));
    MainWindow* window = new MainWindow();
    window->show();

    return app.exec();
}
