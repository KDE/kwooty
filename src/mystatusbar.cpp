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

#include "mystatusbar.h"

#include <KAboutData>
#include <KIcon>
#include <KDebug>

#include <QHBoxLayout>

#include "mainwindow.h"
#include "centralwidget.h"
#include "statsinfobuilder.h"
#include "sidebar.h"
#include "shutdown/shutdownmanager.h"
#include "observers/clientsobserver.h"
#include "widgets/icontextwidget.h"
#include "widgets/iconcapacitywidget.h"
#include "preferences/kconfiggrouphandler.h"
#include "utilityserverstatus.h"
#include "kwootysettings.h"


MyStatusBar::MyStatusBar(MainWindow* parent) : KStatusBar(parent) {
    
    this->clientsObserver = parent->getCentralWidget()->getClientsObserver();

    // create connection widget at bottom left of status bar :
    this->setConnectionWidget();
    
    // create remaining time widget at bottom left of status bar :
    this->setTimeInfoWidget();
    
    // create shutdown widget at bottom left of status bar :
    this->setShutdownWidget();
    
    // add capacity bar widget :
    this->iconCapacityWidget = new IconCapacityWidget(this, IconCapacityWidgetIdentity);
    this->addPermanentWidget(this->iconCapacityWidget);
    
    // add remaining download size item :
    this->sizeLabel = new IconTextWidget(this, SizeWidgetIdentity);
    this->addPermanentWidget(this->sizeLabel);
    
    // add download speed item :
    this->speedLabel = new IconTextWidget(this, SpeedWidgetIdentity);
    this->addPermanentWidget(this->speedLabel);
    
    // add info bar widget :
    this->setInfoBarWidget();

    this->setupConnections();

    // get current connection status, numbre of remainig files, etc... from clients observer :
    this->clientsObserver->sendFullUpdate();
    
}

MyStatusBar::MyStatusBar() {}
MyStatusBar::~MyStatusBar() {}



void MyStatusBar::setupConnections() {
    
    connect (this->clientsObserver,
             SIGNAL(updateConnectionStatusSignal()),
             this,
             SLOT(updateConnectionStatusSlot()));
    
    // send remaining size to status bar :
    connect (this->clientsObserver,
             SIGNAL(updateFileSizeInfoSignal(const quint64, const quint64)),
             this,
             SLOT(updateFileSizeInfoSlot(const quint64, const quint64)));
    
    // send download speed to status bar :
    connect (this->clientsObserver->getStatsInfoBuilder(),
             SIGNAL(updateDownloadSpeedInfoSignal(const QString)),
             this,
             SLOT(updateDownloadSpeedInfoSlot(const QString)));
    
    // send ETA to status bar :
    connect (this->clientsObserver->getStatsInfoBuilder(),
             SIGNAL(updateTimeInfoSignal(const bool)),
             this,
             SLOT(updateTimeInfoSlot(const bool)));
    
    // send free space to status bar :
    connect (this->clientsObserver->getStatsInfoBuilder(),
             SIGNAL(updateFreeSpaceSignal(const UtilityNamespace::FreeDiskSpace, const QString, const int)),
             this,
             SLOT(updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace, const QString, const int)));
    
    
    // send shutdown info in status bar :
    connect(((MainWindow*)this->parentWidget())->getCentralWidget()->getShutdownManager(),
            SIGNAL(statusBarShutdownInfoSignal(QString, QString)),
            this,
            SLOT(statusBarShutdownInfoSlot(QString, QString)));

    // display the proper settings page when double clicking on widgets from statusBar :
    connect(this,
            SIGNAL(showSettingsSignal(UtilityNamespace::PreferencesPage)),
            (MainWindow*)this->parentWidget(),
            SLOT(showSettings(UtilityNamespace::PreferencesPage)));

    // show / hide sideBarWidget when button is toggled :
    connect(this->infoBarWidget,
            SIGNAL(activeSignal(bool)),
            ((MainWindow*)this->parentWidget())->getSideBar(),
            SLOT(activeSlot(bool)));

}



void MyStatusBar::setConnectionWidget(){
    
    this->connectionWidget = new IconTextWidget(this, ConnectionWidgetIdentity);
    
    // add aggregated widget to the status bar :
    this->addWidget(this->connectionWidget);
    
    // set connection not active by default :
    this->updateConnectionStatusSlot();
    
    
}


void MyStatusBar::setTimeInfoWidget(){
    
    this->timeInfoWidget = new IconTextWidget(this, TimeInfoWidgetIdentity);
    this->timeInfoWidget->setIcon("user-away");
    
    // add aggregated widget to the status bar :
    this->addWidget(this->timeInfoWidget);
    
}


void MyStatusBar::setShutdownWidget(){
    
    this->shutdownWidget = new IconTextWidget(this, ShutdownWidgetIdentity);
    
    // add aggregated widget to the status bar :
    this->addWidget(this->shutdownWidget);
    
}

void MyStatusBar::setInfoBarWidget(){

    // add widget that will toggle info bar :
    this->infoBarWidget = new IconTextWidget(this, InfoBarWidgetIdentity);

    this->infoBarWidget->setIconMode(IconTextWidget::SwitchIcon);
    this->infoBarWidget->setIconOnly("arrow-up-double", "arrow-down-double");

    bool sideBarDisplay = KConfigGroupHandler::getInstance()->readSideBarDisplay();
    this->infoBarWidget->setActive(sideBarDisplay);

    // add aggregated widget to the right of status bar :
    this->addPermanentWidget(this->infoBarWidget);

}


IconTextWidget* MyStatusBar::getInfoBarWidget(){
    return this->infoBarWidget;
}


void MyStatusBar::statusBarShutdownInfoSlot(QString iconStr, QString text) {
    
    this->shutdownWidget->setIcon(iconStr);
    this->shutdownWidget->setText(text);
    
}


void MyStatusBar::updateConnectionStatusSlot(){
    
    
    QString connectionIconStr;
    QString connection;

    ServerConnectionIcon serverConnectionIcon = UtilityServerStatus::buildConnectionStringFromStatus(this->clientsObserver, connection);

    // set icon :
    this->connectionWidget->setIcon(serverConnectionIcon);
    // set text :
    this->connectionWidget->setText(connection);
    
    // set tooltip to connection widget :
    this->buildConnWidgetToolTip(connection);
    
    
    
}


void MyStatusBar::buildConnWidgetToolTip(const QString& connection) {

    QString toolTipStr;

    // display tooltip connection only for a single server connection :
    QString hostName;
    if (this->clientsObserver->isSingleServer(hostName)) {
        toolTipStr = UtilityServerStatus::buildConnectionToolTip(this->clientsObserver, connection, hostName);
    }

    // set tooltip :
    this->connectionWidget->setToolTip(toolTipStr);

}





void MyStatusBar::updateFileSizeInfoSlot(const quint64 totalFiles, const quint64 totalSize) {
    
    // status bar update, display number of files and remianing size :
    QString remainingFiles = i18n("Files: <numid>%1</numid> (%2)", totalFiles, Utility::convertByteHumanReadable(totalSize));
    
    this->sizeLabel->setTextOnly(remainingFiles);
}



void MyStatusBar::updateDownloadSpeedInfoSlot(const QString speedInKBStr){
    
    this->speedLabel->setTextOnly(i18n("Speed: %1", speedInKBStr));
}


void MyStatusBar::updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace diskSpaceStatus, const QString availableVal, const int usedDiskPercentage) {
    
    // diskspace is unknown, hide widgets :
    if (diskSpaceStatus == UnknownDiskSpace) {
        
        this->iconCapacityWidget->hide();
    }
    
    else {
        
        // if capacity bar was hidden because diskSpaceStatus was previously UnknownDiskSpace :
        if (this->iconCapacityWidget->isHidden()) {
            this->iconCapacityWidget->show();
        }
        
        // if free disk space is not sufficient display warning icon :
        if (diskSpaceStatus == InsufficientDiskSpace) {
            
            this->iconCapacityWidget->setIcon("dialog-warning");
            this->iconCapacityWidget->setToolTip(i18n("Insufficient disk space"));
            
        }
        
        // if free disk space is not sufficient do not display icon :
        if (diskSpaceStatus == SufficientDiskSpace) {
            
            this->iconCapacityWidget->setIcon(QString());
            this->iconCapacityWidget->setToolTip(QString());
            
        }
        
        // set text and repaint widget :
        this->iconCapacityWidget->updateCapacity(availableVal, usedDiskPercentage);
        
    }
    
    
    
}


void MyStatusBar::updateTimeInfoSlot(const bool parentDownloadingFound) {
    
    QString timeInfoStr;
    QString timeInfoToolTip;
    
    QString currentTimeValue = this->clientsObserver->getStatsInfoBuilder()->getCurrentTimeValue();
    QString totalTimeValue = this->clientsObserver->getStatsInfoBuilder()->getTotalTimeValue();
    
    // build text and toolTip :
    if (!currentTimeValue.isEmpty()) {
        
        timeInfoStr.append(currentTimeValue);
        
        QString timeLabel = this->clientsObserver->getStatsInfoBuilder()->getTimeLabel();
        timeInfoToolTip.append((QString("<b>%1</b>").arg(timeLabel)));
        
        timeInfoToolTip.append("<table style='white-space: nowrap'>");
        QString nzbNameDownloading = this->clientsObserver->getStatsInfoBuilder()->getNzbNameDownloading();
        timeInfoToolTip.append(Utility::buildToolTipRow(QString("%1:").arg(currentTimeValue), nzbNameDownloading));
        
    }
    
    if (!totalTimeValue.isEmpty()) {
        
        timeInfoStr.append("  -  ");
        timeInfoStr.append(totalTimeValue);
        
        timeInfoToolTip.append(Utility::buildToolTipRow(QString("%1:").arg(totalTimeValue), i18n("Total")));
    }
    
    timeInfoToolTip.append("</table>");
    
    
    if (currentTimeValue.isEmpty()) {
        timeInfoStr = i18n("n/a");
        timeInfoToolTip.clear();
    }
    
    this->timeInfoWidget->setText(timeInfoStr);
    this->timeInfoWidget->setToolTip(timeInfoToolTip);
    
    // if download is not active, hide the widget :
    if (!parentDownloadingFound) {
        this->timeInfoWidget->hide();
    }
    // else display it :
    else if (this->timeInfoWidget->isHidden()) {
        this->timeInfoWidget->show();
    }
    
    
}



void MyStatusBar::statusBarWidgetDblClickSlot(MyStatusBar::WidgetIdentity widgetIdentity) {

    // access to configuration preferences page when double-clicking on widgets in the statusbar :
    switch (widgetIdentity) {

    case ConnectionWidgetIdentity: {
        emit showSettingsSignal(ServerPage);
        break;
    }
    case ShutdownWidgetIdentity: {
        emit showSettingsSignal(ShutdownPage);
        break;
    }
    case IconCapacityWidgetIdentity: {
        emit showSettingsSignal(GeneralPage);
        break;
    }
    default: {
        // notify observers that widget has been double clicked :
        emit statusBarWidgetDblClickSignal(widgetIdentity);
        break;
    }

    }

}
