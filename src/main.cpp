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
#include <KUrl>
#include <KIcon>
#include <KDebug>

#include "uniqueapp.h"
#include "mainwindow.h"


int main (int argc, char** argv)
{
    KAboutData aboutData("kwooty",
                         "",
                         ki18n("Kwooty"), "0.4.0",
                         ki18n("Kwooty is a friendly nzb usenet binary downloader.\n It also supports automatic file repairing and archive extraction."),
                         KAboutData::License_GPL,
                         ki18n("Copyright (c) 2010 Xavier Lefage"),
                         KLocalizedString(),
                         "http://kwooty.sourceforge.net/");
    aboutData.addAuthor(ki18n("Xavier Lefage"),ki18n("Maintainer, Lead Developer"), "xavier.kwooty@gmail.com");
    aboutData.setBugAddress("http://sourceforge.net/projects/kwooty");
    aboutData.setCustomAuthorText(ki18n("Please report bugs to http://sourceforge.net/projects/kwooty"),
                                  ki18n("Please report bugs to <a href='http://sourceforge.net/tracker/?group_id=285032&atid=1208199'>Kwooty bug tracker</a>"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[Url]", ki18n("Document to open"));
    KCmdLineArgs::addCmdLineOptions(options);


    UniqueApp app;
    return app.exec();
}
