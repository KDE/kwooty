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


#include "sidebar.h"

#include "kwooty_debug.h"
#include <KLocalizedString>
#include "mainwindow.h"
#include "core.h"
#include "servermanager.h"
#include "servergroup.h"
#include "observers/clientsperserverobserver.h"
#include "widgets/sidebarwidget.h"
#include "widgets/serverstatuswidget.h"
#include "data/serverdata.h"
#include "data/segmentinfodata.h"
#include "utilities/utilityserverstatus.h"
#include "preferences/kconfiggrouphandler.h"


SideBar::SideBar(MainWindow* parent) : QObject(parent) {

    this->mCore = parent->getCore();
    this->mSideBarWidget = new SideBarWidget(parent);
    this->mServerManager = 0;
    this->mStateRestored = false;

    this->serverManagerSettingsChangedSlot();
    this->setupConnections();

}


void SideBar::setupConnections() {

    // notify sidebar that settings have been changed :
    connect (this->mCore->getServerManager(),
             SIGNAL(serverManagerSettingsChangedSignal()),
             this,
             SLOT(serverManagerSettingsChangedSlot()), Qt::QueuedConnection);

}


SideBarWidget* SideBar::getSideBarWidget() {
    return this->mSideBarWidget;
}


void SideBar::saveState() {

    // save info anly if there is at least one current widget :
    if (this->mSideBarWidget->currentIndex() != -1) {

        // save current states to restore them in next session :
        KConfigGroupHandler::getInstance()->writeSideBarDisplay(this->mSideBarWidget->isDisplayed());
        KConfigGroupHandler::getInstance()->writeSideBarTabOnlyDisplay(this->mSideBarWidget->isOnlyTabDisplayed());
        KConfigGroupHandler::getInstance()->writeSideBarServerIndex(this->mSideBarWidget->currentIndex());

    }

}


void SideBar::loadState() {

    // show / hide side bar :
    bool sideBarDisplay = KConfigGroupHandler::getInstance()->readSideBarDisplay();
    this->activeSlot(sideBarDisplay);

    // show / hide server status widget :
    bool tabOnlyDisplay = KConfigGroupHandler::getInstance()->readSideBarTabOnlyDisplay();

    if (tabOnlyDisplay) {
        this->mSideBarWidget->displayTabOnly();
    }

    else {
        // set focus on previous session tab :
        int tabIndex = KConfigGroupHandler::getInstance()->readSideBarServerIndex();

        if (tabIndex < this->mSideBarWidget->count()) {
            this->mSideBarWidget->activeDefaultTab(tabIndex);
        }
    }

    this->mStateRestored = true;

}



void SideBar::createSideBarWidgets() {

    // add a widget for a servergroup :
    while (this->mServerManager->getServerNumber() > this->mSideBarWidget->count()) {

        int serverWidgetIndex = this->mSideBarWidget->count();
        QString serverName = this->mServerManager->getServerGroupById(serverWidgetIndex)->getServerData().getServerName();

        this->mSideBarWidget->addTab(new ServerStatusWidget(this->mSideBarWidget), DisconnectedIcon, serverName);

    }

}



void SideBar::serverStatisticsUpdate(const int serverId) {

    if (this->mServerManager) {

        ServerGroup* serverGroup = this->mServerManager->getServerGroupById(serverId);

        ClientsPerServerObserver* clientsPerServerObserver = serverGroup->getClientsPerServerObserver();

        QString connection;
        ServerConnectionIcon serverConnectionIcon = UtilityServerStatus::buildConnectionStringFromStatus(clientsPerServerObserver, connection, UtilityServerStatus::DoNotDisplayEncryptionMethod);

        // if current server is downloading, display proper icon on to tab widget :
        ServerConnectionIcon serverConnectionTabIcon = serverConnectionIcon;

        if (clientsPerServerObserver->isDownloading()) {
            serverConnectionTabIcon = ConnectedDownloadingIcon;
        }

        // update tab icon :
        this->mSideBarWidget->updateIconByIndex(serverId, serverConnectionTabIcon);


        // retrieve server mode according to server :
        QString hostName = serverGroup->getServerData().getHostName();
        QString serverMode = i18n("Master");

        if (serverId != MasterServer) {
            serverMode = UtilityServerStatus::getServerModeString((UtilityNamespace::BackupServerMode)serverGroup->getServerData().getServerModeIndex());
        }


        // retrieve connection related information :
        QString sslConnectionInfo = UtilityServerStatus::buildSslHandshakeStatus(clientsPerServerObserver);


        // if server is currently being downloading, get the downloaded nzb name :
        QString downloadFileName = i18n("n/a");
        if (clientsPerServerObserver->isDownloading()) {
            downloadFileName = clientsPerServerObserver->getSegmentInfoData().getNzbFileName();
        }


        // retrieve ssl encryption method :
        bool displayIcon = false;
        QString sslStr = i18n("None");

        if (clientsPerServerObserver->isConnected() && clientsPerServerObserver->isSslActive()) {
            sslStr = clientsPerServerObserver->getEncryptionMethod();
            displayIcon = true;
        }


        // build speed text label :
        quint64 downloadSpeed = clientsPerServerObserver->getDownloadSpeed();
        quint64 effectiveMeanDownloadSpeed = clientsPerServerObserver->getEffectiveMeanDownloadSpeed();

        QString speedTextLabel = QString("%1 - %2").arg(Utility::convertDownloadSpeedHumanReadable(downloadSpeed)).arg(Utility::convertDownloadSpeedHumanReadable(effectiveMeanDownloadSpeed));
        QString speedToolTipLabel = i18n("Current download speed - Mean download speed");

        // update fields :
        ServerStatusWidget* serverStatusWidget = static_cast<ServerStatusWidget*>(this->mSideBarWidget->widget(serverId));

        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::StatusItem, connection);
        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::SpeedItem, speedTextLabel, speedToolTipLabel);
        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::VolumeItem, Utility::convertByteHumanReadable(clientsPerServerObserver->getBytesDownloadedForCurrentSession()));
        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::FileItem, downloadFileName);
        serverStatusWidget->updateRightLabelField(ServerStatusWidget::NameItem, hostName);
        serverStatusWidget->updateRightLabelField(ServerStatusWidget::ModeItem, serverMode);
        serverStatusWidget->updateTextPushButtonField(ServerStatusWidget::SslItem, sslStr, displayIcon, serverConnectionIcon, sslConnectionInfo);

    }
}





//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void SideBar::activeSlot(bool active) {

    this->mSideBarWidget->setDisplay(active);
}


void SideBar::serverManagerSettingsChangedSlot() {

    // if servermanager instance is not yet initialized :
    if (!this->mServerManager) {
        this->mServerManager = static_cast<MainWindow*>(this->parent())->getCore()->getServerManager();
    }

    if (this->mServerManager) {

        // remove widgets from side bar if number of servers has been reduced :
        if (this->mServerManager->getServerNumber() < this->mSideBarWidget->count()) {

            while (this->mServerManager->getServerNumber() < this->mSideBarWidget->count()) {

                this->mSideBarWidget->removeLast();
            }
        }
        // add widgets from side bar if number of servers has been increased :
        else {
            this->createSideBarWidgets();
        }


        // synchronize info for each server with newly settings :
        for (int i = 0; i < this->mSideBarWidget->count(); ++i) {

            ServerData serverData = this->mServerManager->getServerGroupById(i)->getServerData();

            // update text and tooltip accordingly :
            this->mSideBarWidget->updateTextByIndex(i, serverData.getServerName());
            this->mSideBarWidget->updateToolTipByIndex(i, serverData.getServerName());

            this->serverStatisticsUpdate(i);
        }

        // for first call, restore previous session settings :
        if (!this->mStateRestored) {
            this->loadState();
        }

    }

}






