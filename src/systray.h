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


#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QObject>

#include <KStatusNotifierItem>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class QueueFileObserver;
class ClientsObserver;
class StatsInfoBuilder;
class MainWindow;


class SysTray : public KStatusNotifierItem
{

    Q_OBJECT

public:
    explicit SysTray(MainWindow*);


private:

    MainWindow* mParent;
    QueueFileObserver* mQueueFileObserver;
    ClientsObserver* mClientsObserver;
    StatsInfoBuilder* mStatsInfoBuilder;
    QPixmap mNormalBaseIcon;
    QPixmap mGrayedBaseIcon;
    QPixmap mRenderedIcon;
    int mOldMergePos;

    bool updateIconStatus(const UtilityNamespace::ItemStatus&);
    bool blendOverlay(const QPixmap&);
    void setupConnections();
    void setupActions();
    void initPixmaps();
    void initShow();
    void updateIconProgress(const int&);
    void createToolTip();
public Q_SLOTS:

    void progressUpdateSlot(const int);
    void statusUpdateSlot(const UtilityNamespace::ItemStatus);
    void updateDownloadSpeedInfoSlot();
    void updateConnectionStatusSlot();

};

#endif // SYSTRAY_H
