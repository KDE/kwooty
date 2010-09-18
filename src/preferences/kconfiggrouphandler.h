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


#ifndef KCONFIGGROUPHANDLER_H
#define KCONFIGGROUPHANDLER_H

#include <QObject>

//TODO :
#include "ksharedconfig.h"

#include <KConfigGroup>


#include "data/serverdata.h"


class KConfigGroupHandler : public QObject
{

    Q_OBJECT

public:
    KConfigGroupHandler();
    ~KConfigGroupHandler();

    static KConfigGroupHandler* getInstance();

    ServerData readServerSettings(const int&);
    void writeServerSettings(const int&, ServerData);
    void removeServerSettings(const int&);

    int readServerNumberSettings();
    void writeServerNumberSettings(const int&);
    int serverConnectionNumber(const int&);
    QString tabName(const int&, const QString&);
private:

    static KConfigGroupHandler* instance;

 //   KConfigGroup* configGroup;


};

#endif // KCONFIGGROUPHANDLER_H
