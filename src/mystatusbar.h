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

#include <KIconLoader>
#include <KStatusBar>


#include <QLabel>

#include "utility.h"
using namespace UtilityNamespace;

class IconTextWidget;
class IconCapacityWidget;

class MyStatusBar : public KStatusBar
{

    Q_OBJECT

public:
    MyStatusBar(QWidget* parent);
    MyStatusBar();
    ~MyStatusBar();


private:
    KIconLoader* iconLoader;
    QLabel* sizeLabel;
    QLabel* speedLabel;
    IconTextWidget* connectionWidget;
    IconTextWidget* shutdownWidget;
    IconTextWidget* timeInfoWidget;
    IconCapacityWidget* iconCapacityWidget;
    QString encryptionMethod;
    QString issuerOrgranisation;
    int totalConnections;
    int nttpErrorStatus;
    bool sslActive;
    bool certificateVerified;

    void resetVariables();
    void setConnectionWidget();
    void setShutdownWidget();
    void setTimeInfoWidget();
    void setConnectionActive();
    void buildConnWidgetToolTip(const QString&);


signals:

public slots:

    void connectionStatusSlot(const int);
    void encryptionStatusSlot(const bool, const QString, const bool, const QString);
    void nntpErrorSlot(const int);
    void statusBarShutdownInfoSlot(QString, QString);

    void updateDownloadSpeedInfoSlot(const QString);
    void updateSizeInfoSlot(const QString);
    void updateTimeInfoSlot(const QString, const QString, const bool);
    void updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace, const QString, const int);


private slots:

};



#endif // STATUSBAR_H
