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

#ifndef SHUTDOWNMANAGER_H
#define SHUTDOWNMANAGER_H

#include <solid/control/powermanager.h>
#include <KDialog>

#include <QObject>
#include <QTimer>
#include <QPointer>

#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;

class ShutdownManager : public QObject {

    Q_OBJECT

public:

    // distinguish type of desktop session in order to call proper shutdown command :
    enum SessionType {
      Kde,
      Gnome,
      Unknown
    };

    ShutdownManager(CentralWidget* parent = 0);
    QMap<QString, QString> retrieveIconAvailableShutdownMap();


private:

    CentralWidget* parent;
    QPointer<KDialog> aboutToShutdownDialog;
    QTimer* activityMonitorTimer;
    QTimer* launchShutdownTimer;
    QString scheduleDateTimeStr;
    QString gnomeShutdownApplication;
    int noActivityCounter;
    int shutdownTimerInterval;
    bool enableSystemShutdown;
    bool jobsRadioButton;
    bool timerRadioButton;
    bool pausedShutdown;

    UtilityNamespace::SystemShutdownType getChosenShutdownType();
    ShutdownManager::SessionType retrieveSessionType();
    QList<UtilityNamespace::SystemShutdownType> retrieveAvailableShutdownMethods();
    QString getShutdownMethodText(UtilityNamespace::SystemShutdownType) const;
    int displayAboutToShutdownMessageBox(const QString&);
    void systemAboutToShutdown();
    void requestSuspend(Solid::Control::PowerManager::SuspendMethod);
    void requestShutdown();
    void storeSettings();
    void displayShutdownErrorMessageBox(const QString&);
    void updateStatusBar();
    void setupConnections();


signals:
    void setShutdownButtonCheckedSignal(bool);
    void setShutdownButtonEnabledSignal(bool);
    void statusBarShutdownInfoSignal(QString, QString);

public slots:
  void enableSystemShutdownSlot(bool);
  void shutdownCancelledSlot();
  void statusItemUpdatedSlot();

private slots:
    void retrieveCurrentJobsInfoSlot();
    void settingsChangedSlot();
    void launchSystemShutdownSlot();

};

#endif // SHUTDOWNMANAGER_H
