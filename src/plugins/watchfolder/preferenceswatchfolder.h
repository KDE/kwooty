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

#ifndef PREFERENCESWATCHFOLDER_H
#define PREFERENCESWATCHFOLDER_H

#include <QWidget>
#include "ui_preferenceswatchfolder.h"

#include <KCModule>

#include <KPluginFactory>
#include <KPluginLoader>



class PreferencesWatchFolder : public KCModule
{
    Q_OBJECT

public:
    PreferencesWatchFolder(QWidget* = 0, const QVariantList& = QVariantList());
    ~PreferencesWatchFolder();

    virtual void save();
    virtual void load();

private:
    Ui_PreferencesWatchFolder preferencesWatchFolderUi;


Q_SIGNALS:

public Q_SLOTS:

private Q_SLOTS:

};

#endif // PREFERENCESWATCHFOLDER_H
