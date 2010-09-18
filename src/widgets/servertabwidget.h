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


#ifndef SERVERTABWIDGET_H
#define SERVERTABWIDGET_H

#include <KTabWidget>
#include <QWidget>
#include <QToolButton>

class PreferencesServer;

class ServerTabWidget : public KTabWidget {


    Q_OBJECT


public:

    enum ServerNameQuery { AskServerName,
                           DoNotAskServerName,
                       };

    ServerTabWidget(PreferencesServer*);
    void addNewTab();
    void deleteAndRemoveTab(const int&);


private:
    QToolButton* newTab;
    QToolButton* closeTab;
    PreferencesServer* preferencesServer;

    void setupConnections();
    void enableDisableTabButtons();
    void syncGroupBoxTitle();



signals:


public slots:
    void saveDataSlot();


private slots:

    void closeTabClickedSlot();
    void newTabClickedSlot(const ServerTabWidget::ServerNameQuery = ServerTabWidget::AskServerName);
    void tabMovedSlot(int, int);
    void currentChangedSlot(int);
    void valueChangedSlot();

};

#endif // SERVERTABWIDGET_H
