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

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <KMultiTabBar>
#include <QStackedWidget>
#include <QHash>

#include "utilities/utilityserverstatus.h"

class MainWindow;


class SideBarWidget : public QWidget {

    Q_OBJECT

public:
    SideBarWidget(QWidget*);

    void addTab(QWidget*, const ServerConnectionIcon&, const QString&);
    void removeTabAndWidgetByIndex(int);
    void removeTabByWidget(QWidget*);
    void removeLast();
    int count() const;
    QWidget* widget(const int& index);
    int indexOf(QWidget*) const;
    int currentIndex() const;
    void setDisplay(const bool&);
    bool isDisplayed() const;
    void updateIconByIndex(const int&, const ServerConnectionIcon&);
    void updateTextByIndex(const int&, const QString&);
    void updateToolTipByIndex(const int&, const QString&);
    void activeDefaultTab(const int&);
    bool isOnlyTabDisplayed() const;
    void displayTabOnly();

private:
    QStackedWidget* stackedWidget;
    KMultiTabBar* multiTabBar;
    QHash<int, ServerConnectionIcon> indexServerIconMap;
    bool display;


public slots:


private slots:
    void tabClickedSlot(const int&);

};

#endif // SIDEBARWIDGET_H
