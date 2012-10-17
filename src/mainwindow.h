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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>
#include <KUrl>
#include <KPageWidgetItem>

#include <QPointer>
#include "kwooty_export.h"
#include "plugins/pluginmanager.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class MyStatusBar;
class CentralWidget;
class MyTreeView;
class Core;
class SysTray;
class PluginManager;
class SideBar;
class KConfigGroupHandler;


class KWOOTY_EXPORT MainWindow : public KXmlGuiWindow

{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);
    ~MainWindow();

    void openFileWithFileMode(KUrl, UtilityNamespace::OpenFileMode);
    QAction* getActionFromName(const QString&);
    MyStatusBar* getStatusBar() const;
    SideBar* getSideBar() const;
    Core* getCore() const;
    CentralWidget* getCentralWidget() const;
    MyTreeView* getTreeView() const;
    QSize sizeHint() const;


private:
    Core* core;
    CentralWidget* centralWidget;
    MyTreeView* treeView;
    QPointer<SysTray> sysTray;
    MyStatusBar* statusBar;
    SideBar* sideBar;
    PluginManager* pluginManager;
    KConfigGroupHandler* kConfigGroupHandler;
    QHash<PreferencesPage, KPageWidgetItem*> preferencesPagesMap;
    bool quitSelected;

    void initVariables();
    void buildLayout(QWidget*);
    void setupActions();
    bool queryClose();
    bool queryExit();
    void askForSavingDownloads(bool&);


signals:
    void aboutToShowSettingsSignal();
    void savePendingDownloadsSignal();

public slots:
    void systraySlot();


private slots:
    void openFile();
    void showSettings(UtilityNamespace::PreferencesPage = UtilityNamespace::GeneralPage);
    void quit();

};

#endif
