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
#include <QMap>

class PreferencesServer;

class ServerTabWidget : public KTabWidget {


    Q_OBJECT


public:

    // query name of server only when created by user :
    enum ServerNameQuery { AskServerName,
                           DoNotAskServerName,
                           DefaultSettingsName
                       };

    // new tab name or rename tab :
    enum ServerTabNaming { CreateTab,
                           RenameTab,
                       };

    ServerTabWidget(PreferencesServer*);
    void addNewTab();
    void addDefaultTab();
    void deleteAndRemoveTab(const int&);
    void setServerTabIcon(const int&, const int&);
    QMap<int, QString> getComboBoxIconTextMap();


private:
    QToolButton* newTab;
    QToolButton* closeTab;
    PreferencesServer* preferencesServer;
    QMap<int, QString> comboBoxIconTextMap;

    void setServerTabText(const ServerTabNaming& = CreateTab);
    void setupConnections();
    void enableDisableTabButtons();
    void syncGroupBoxTitle();
    QString displayEditDialogBox();
    QString displayRenameTabDialogBox();



signals:


public slots:
    void saveDataSlot();


private slots:

    void closeTabClickedSlot();
    void newTabClickedSlot(const ServerTabWidget::ServerNameQuery = ServerTabWidget::AskServerName);
    void tabMovedSlot(int, int);
    void currentChangedSlot(int);
    void valueChangedSlot();
    void renameTabSlot(QWidget*);

};

#endif // SERVERTABWIDGET_H
