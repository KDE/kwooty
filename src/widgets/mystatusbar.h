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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>

#include <QLabel>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class MainWindow;
class ClientsObserver;
class IconTextWidget;
class IconCapacityWidget;

class MyStatusBar : public QStatusBar
{
    Q_OBJECT
public:
    enum WidgetIdentity { SizeWidgetIdentity,
                          SpeedWidgetIdentity,
                          ConnectionWidgetIdentity,
                          ShutdownWidgetIdentity,
                          TimeInfoWidgetIdentity,
                          InfoBarWidgetIdentity,
                          IconCapacityWidgetIdentity
                        };

    explicit MyStatusBar(MainWindow*);
    MyStatusBar();
    IconTextWidget* getInfoBarWidget();
    ~MyStatusBar();


protected:


private:

    ClientsObserver* clientsObserver;
    IconTextWidget* sizeLabel;
    IconTextWidget* speedLabel;
    IconTextWidget* connectionWidget;
    IconTextWidget* shutdownWidget;
    IconTextWidget* timeInfoWidget;
    IconTextWidget* infoBarWidget;
    IconCapacityWidget* iconCapacityWidget;

    void setupConnections();
    void setConnectionWidget();
    void setShutdownWidget();
    void setTimeInfoWidget();
    void setInfoBarWidget();

    void buildConnWidgetToolTip(const QString&);


Q_SIGNALS:
    void showSettingsSignal(UtilityNamespace::PreferencesPage);
    void statusBarWidgetDblClickSignal(MyStatusBar::WidgetIdentity);

public Q_SLOTS:

    void updateConnectionStatusSlot();
    void updateDownloadSpeedInfoSlot(const QString &);
    void updateFileSizeInfoSlot(const quint64, const quint64);
    void updateTimeInfoSlot(const bool);
    void updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace, const QString &, const int);
    void statusBarShutdownInfoSlot(const QString &, const QString &);
    void statusBarWidgetDblClickSlot(MyStatusBar::WidgetIdentity);
};



#endif // STATUSBAR_H
