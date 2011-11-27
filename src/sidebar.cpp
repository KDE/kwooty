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

#include <KDebug>

#include "mainwindow.h"
#include "centralwidget.h"
#include "servermanager.h"
#include "servergroup.h"
#include "observers/clientsperserverobserver.h"
#include "widgets/sidebarwidget.h"
#include "widgets/serverstatuswidget.h"
#include "data/serverdata.h"
#include "data/segmentinfodata.h"
#include "utilityserverstatus.h"
#include "preferences/kconfiggrouphandler.h"


SideBar::SideBar(MainWindow* parent) : QObject(parent) {

    this->sideBarWidget = new SideBarWidget(parent);
    this->serverManager = 0;
    this->stateRestored = false;

}


SideBarWidget* SideBar::getSideBarWidget() {
    return this->sideBarWidget;
}


void SideBar::saveState() {

    // save info anly if there is at least one current widget :
    if (this->sideBarWidget->currentIndex() != -1) {

        // save current states to restore them in next session :
        KConfigGroupHandler::getInstance()->writeSideBarDisplay(this->sideBarWidget->isVisible());
        KConfigGroupHandler::getInstance()->writeSideBarTabOnlyDisplay(this->sideBarWidget->isOnlyTabDisplayed());
        KConfigGroupHandler::getInstance()->writeSideBarServerIndex(this->sideBarWidget->currentIndex());

    }

}


void SideBar::loadState() {

    // show / hide side bar :
    bool sideBarDisplay = KConfigGroupHandler::getInstance()->readSideBarDisplay();
    this->activeSlot(sideBarDisplay);

    // show / hide server status widget :
    bool tabOnlyDisplay = KConfigGroupHandler::getInstance()->readSideBarTabOnlyDisplay();

    if (tabOnlyDisplay) {
        this->sideBarWidget->displayTabOnly();
    }

    else {
        // set focus on previous session tab :
        int tabIndex = KConfigGroupHandler::getInstance()->readSideBarServerIndex();

        if (tabIndex < this->sideBarWidget->count()) {
            this->sideBarWidget->activeDefaultTab(tabIndex);
        }
    }

    this->stateRestored = true;

}



void SideBar::createSideBarWidgets() {

    // add a widget for a servergroup :
    while (this->serverManager->getServerNumber() > this->sideBarWidget->count()) {

        int serverWidgetIndex = this->sideBarWidget->count();
        QString serverName = this->serverManager->getServerGroupById(serverWidgetIndex)->getServerData().getServerName();

        this->sideBarWidget->addTab(new ServerStatusWidget(this->sideBarWidget), "weather-clear-night", serverName);

    }

}




//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//


void SideBar::activeSlot(bool active) {

    this->sideBarWidget->setVisible(active);
}


void SideBar::serverManagerSettingsChangedSlot() {

    // if servermanager instance is not yet initialized :
    if (!this->serverManager) {
        this->serverManager = static_cast<MainWindow*>(this->parent())->getCentralWidget()->getServerManager();
    }

    if (this->serverManager) {

        // remove widgets from side bar if number of servers has been reduced :
        if (this->serverManager->getServerNumber() < this->sideBarWidget->count()) {

            while (this->serverManager->getServerNumber() < this->sideBarWidget->count()) {

                this->sideBarWidget->removeLast();
            }
        }
        // add widgets from side bar if number of servers has been increased :
        else {
            this->createSideBarWidgets();
        }


        // synchronize info for each server with newly settings :
        for (int i = 0; i < this->sideBarWidget->count(); i++) {

            ServerData serverData = this->serverManager->getServerGroupById(i)->getServerData();

            // update text and tooltip accordingly :
            this->sideBarWidget->updateTextByIndex(i, serverData.getServerName());
            this->sideBarWidget->updateToolTipByIndex(i, serverData.getServerName());

            this->serverStatisticsUpdateSlot(i);
        }

        // for first call, restore previous session settings :
        if (!this->stateRestored) {
            this->loadState();
        }

    }

}




void SideBar::serverStatisticsUpdateSlot(const int serverId) {

    if (this->serverManager) {

        ServerGroup* serverGroup = this->serverManager->getServerGroupById(serverId);

        ClientsPerServerObserver* clientsPerServerObserver = serverGroup->getClientsPerServerObserver();

        // display tab icon according to server status :
        QString tabConnectionIconStr = "weather-clear-night";

        if (clientsPerServerObserver->isConnected()) {
            // set connection icon :
            tabConnectionIconStr = "applications-internet";

            if (clientsPerServerObserver->isSslActive()) {

                // if SSL active use another connection icon :
                tabConnectionIconStr = "document-encrypt";
            }
        }


        quint64 downloadSpeed = clientsPerServerObserver->getDownloadSpeed();

        if (downloadSpeed > 0) {
            tabConnectionIconStr = "mail-receive";
        }



        QString connection;
        QString connectionIconStr;

        bool displayOverlay = UtilityServerStatus::buildConnectionStringFromStatus(clientsPerServerObserver, connectionIconStr, connection, UtilityServerStatus::DoNotDisplayEncryptionMethod);


        QString tabConnectionIconMapStr = tabConnectionIconStr;
        if (displayOverlay) {
            tabConnectionIconMapStr.append("_withOverlay");
        }

        // if previous icon is different from the current one, update display :
        if (serverIdConnectionIconMap.value(serverId) != tabConnectionIconMapStr) {

            bool displayOverlayLocal = displayOverlay;

            // if tab displays download icon, do not display overlay on it :
            if (downloadSpeed > 0) {
                displayOverlayLocal = false;
            }

            // update tab icon :
            this->sideBarWidget->updateIconByIndex(serverId, tabConnectionIconStr, displayOverlayLocal);

            // store icon currently displayed :
            this->serverIdConnectionIconMap.insert(serverId, tabConnectionIconMapStr);

        }



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
        if (downloadSpeed > 0) {
            downloadFileName = clientsPerServerObserver->getSegmentInfoData().getNzbFileName();
        }


        // retrieve ssl encryption method :
        bool displayIcon = false;
        QString sslStr = i18n("None");

        if (clientsPerServerObserver->isConnected() && clientsPerServerObserver->isSslActive()) {
            sslStr = clientsPerServerObserver->getEncryptionMethod();
            displayIcon = true;
        }


        // update fields :
        ServerStatusWidget* serverStatusWidget = static_cast<ServerStatusWidget*>(this->sideBarWidget->widget(serverId));

        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::StatusItem, connection);
        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::SpeedItem, Utility::convertDownloadSpeedHumanReadable(downloadSpeed));
        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::VolumeItem, Utility::convertByteHumanReadable(clientsPerServerObserver->getBytesDownloadedForCurrentSession()));
        serverStatusWidget->updateLeftLabelField(ServerStatusWidget::FileItem, downloadFileName);
        serverStatusWidget->updateRightLabelField(ServerStatusWidget::NameItem, hostName);
        serverStatusWidget->updateRightLabelField(ServerStatusWidget::ModeItem, serverMode);
        serverStatusWidget->updateTextPushButtonField(ServerStatusWidget::SslItem, sslStr, displayIcon, connectionIconStr, displayOverlay, sslConnectionInfo);

    }
}



