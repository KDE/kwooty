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


#ifndef SYSTRAYLEGACY_H
#define SYSTRAYLEGACY_H

#include <QObject>

#include <KSystemTrayIcon>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class QueueFileObserver;
class ClientsObserver;
class StatsInfoBuilder;
class MainWindow;


class SysTray : public KSystemTrayIcon
{

    Q_OBJECT

public:
    SysTray(MainWindow*);
    ~SysTray();

private:

    MainWindow* parent;
    QueueFileObserver* queueFileObserver;
    ClientsObserver* clientsObserver;
    StatsInfoBuilder* statsInfoBuilder;
    QPixmap normalBaseIcon;
    QPixmap grayedBaseIcon;
    QPixmap renderedIcon;
    int oldMergePos;
    bool displayToolTip;

    bool blendOverlay(const QPixmap&);
    bool updateIconStatus(const UtilityNamespace::ItemStatus&);
    void setupConnections();
    void setupActions();
    void initPixmaps();
    void initShow();
    void updateIconProgress(const int&);
    void createToolTip();

signals:


public slots:

    void progressUpdateSlot(const int);
    void statusUpdateSlot(const UtilityNamespace::ItemStatus);
    void updateDownloadSpeedInfoSlot();
    void updateConnectionStatusSlot();

private slots:

    void menuAboutToHideSlot();
    void menuAboutToShowSlot();




};

#endif // SYSTRAYLEGACY_H
