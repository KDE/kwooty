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


#ifndef PLUGIN_H
#define PLUGIN_H


#include <QObject>
#include <kxmlguiclient.h>
#include "kwooty_export.h"
class MainWindow;


class KWOOTY_EXPORT Plugin : public QObject, public KXMLGUIClient {

    Q_OBJECT

public:

    Plugin(QObject*);
    void setCore(MainWindow*);
    MainWindow* getMainWindow();

    virtual void load() = 0;
    virtual void unload() = 0;
    virtual void configUpdated() = 0;


private:

    MainWindow* core;

signals:

public slots:

private slots:


};

#endif // PLUGIN_H
