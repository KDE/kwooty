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
#include "clientsobserver.h"
#include "statsinfobuilder.h"
#include "shutdownmanager.h"
#include "widgets/icontextwidget.h"
#include "widgets/iconcapacitywidget.h"

#include "kwootysettings.h"


MyStatusBar::MyStatusBar(MainWindow* parent) : KStatusBar(parent)
{
    
    this->clientsObserver = parent->getCentralWidget()->getClientsObserver();
    
    this->setupConnections();
    
    // create connection widget at bottom left of status bar :
    this->setConnectionWidget();
    
    // create remaining time widget at bottom left of status bar :
    this->setTimeInfoWidget();
    
    // create shutdown widget at bottom left of status bar :
    this->setShutdownWidget();
    
    // add capacity bar widget :
    this->iconCapacityWidget = new IconCapacityWidget(this);
    this->addPermanentWidget(this->iconCapacityWidget);
    
    // add remaining download size item :
    this->sizeLabel = new QLabel(this);
    this->addPermanentWidget(this->sizeLabel);
    
    // add download speed item :
    this->speedLabel = new QLabel(this);
    this->addPermanentWidget(this->speedLabel);
    
    // this map allows access to configuration preferences page when double-clicking on widgets in the statusbar :
    this->widgetPreferencesPageMap.insert(this->connectionWidget, ServerPage);
    this->widgetPreferencesPageMap.insert(this->iconCapacityWidget, GeneralPage);
    this->widgetPreferencesPageMap.insert(this->shutdownWidget, ShutdownPage);
    

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

    
}



void MyStatusBar::setConnectionWidget(){
    
    this->connectionWidget = new IconTextWidget(this);
    
    // add aggregated widget to the status bar :
    this->addWidget(this->connectionWidget);
    
    // set connection not active by default :
    this->updateConnectionStatusSlot();
    
    
}


void MyStatusBar::setTimeInfoWidget(){
    
    this->timeInfoWidget = new IconTextWidget(this);
    this->timeInfoWidget->setIcon("user-away");
    
    // add aggregated widget to the status bar :
    this->addWidget(this->timeInfoWidget);
    
}


void MyStatusBar::setShutdownWidget(){
    
    this->shutdownWidget = new IconTextWidget(this);
    
    // add aggregated widget to the status bar :
    this->addWidget(this->shutdownWidget);
    
}


void MyStatusBar::statusBarShutdownInfoSlot(QString iconStr, QString text) {
    
    this->shutdownWidget->setIcon(iconStr);
    this->shutdownWidget->setText(text);
    
}


void MyStatusBar::updateConnectionStatusSlot(){
    
    
    QString connectionIconStr;
    QString connection;
    bool displayOverlay = false;
    
    int totalConnections = this->clientsObserver->getTotalConnections();
    
    if (totalConnections == 0) {
        
        connectionIconStr ="weather-clear-night";
        connection = i18n("Disconnected");
        
        int nttpErrorStatus = this->clientsObserver->getNttpErrorStatus();
        // detail disconnection issues to user :
        if (nttpErrorStatus == HostNotFound){
            connection = i18n("Disconnected (Host not found)");
        }
        
        if (nttpErrorStatus == ConnectionRefused) {
            connection = i18n("Disconnected (Connection refused)");
        }
        
        if (nttpErrorStatus == RemoteHostClosed) {
            connection = i18n("Disconnected (Closed by remote host)");
        }
        
        if (nttpErrorStatus == SslHandshakeFailed) {
            connection = i18n("Disconnected (SSL handshake failed)");
        }
        
        //kDebug() << "nttpErrorStatus = " << nttpErrorStatus;
        if (nttpErrorStatus == AuthenticationNeeded) {
            connection = i18n("Disconnected (Authentication required)");
        }
        
        
        if (nttpErrorStatus == AuthenticationFailed) {
            connection = i18n("Disconnected (Authentication Denied)");
        }
        
    }
    else{
        // set connection icon :
        connectionIconStr = "applications-internet";
        connection = i18n("Connected: <numid>%1</numid>", totalConnections);
        
        if (this->clientsObserver->isSslActive()) {
            
            // if SSL active use another connection icon :
            connectionIconStr = "document-encrypt";
            
            // display type of encryption method used by server :
            QString encryptionMethod = this->clientsObserver->getEncryptionMethod();
            if (!encryptionMethod.isEmpty()) {
                connection = connection + " :: " + encryptionMethod;
            }
            
            // display overlay only if connected to server with ssl connection and with certificate not verified :
            if (!this->clientsObserver->isCertificateVerified()) {
                displayOverlay = true;
            }
            
        }
        
    }
    
    
    connectionWidget->setIcon(connectionIconStr);
    connectionWidget->setText(connection);
    
    // if certificate is not verified display warning icon over the secure connected one :
    if (displayOverlay) {
        connectionWidget->blendOverLay("emblem-important");
    }
    
    // set tooltip to connection widget :
    this->buildConnWidgetToolTip(connection);
    
    
    
}


void MyStatusBar::buildConnWidgetToolTip(const QString& connection) {
    
    QString toolTipStr;
    
    // if totalConnections == 0, client is disconnected :
    if (this->clientsObserver->getTotalConnections() == 0) {
        toolTipStr.append(connection);
    }
    
    else {
        // set host name info :
        // TODO
        //toolTipStr.append(i18n("Connected to %1<br>", Settings::hostName()));
        
        // set SSL connection info :
        if (this->clientsObserver->isSslActive()) {
            
            toolTipStr.append(i18n("Connection is SSL encrypted"));
            
            QString encryptionMethod = this->clientsObserver->getEncryptionMethod();
            if (!encryptionMethod.isEmpty()) {
                toolTipStr.append(i18nc("type of ssl encryption method", ": %1", encryptionMethod));
            }
            
            toolTipStr.append("<br>");
            
            if (this->clientsObserver->isCertificateVerified()) {
                toolTipStr.append(i18n("Certificate <b>verified</b> by %1", this->clientsObserver->getIssuerOrgranisation()));
            }
            else {
                toolTipStr.append(i18n("Certificate <b>can not be verified</b> "));
                
                // add ssl errors encountered :
                QStringList sslErrorList = this->clientsObserver->getSslErrors();
                
                if (!sslErrorList.isEmpty()) {
                    
                    QString errorListSeparator = "<li>";
                    toolTipStr.append(i18np("(%1 error during SSL handshake): %2",
                                            "(%1 errors during SSL handshake): %2",
                                            sslErrorList.size(),
                                            "<ul style=\"margin-top:0px; margin-bottom:0px;\">" +
                                            errorListSeparator + sslErrorList.join(errorListSeparator)) +
                                      "</ul>");
                }
                
            }
            
        }
        else {
            
            toolTipStr.append(i18n("Connection is not encrypted"));
        }
        
    }
    
    // set tooltip :
    this->connectionWidget->setToolTip(toolTipStr);
}





void MyStatusBar::updateFileSizeInfoSlot(const quint64 totalFiles, const quint64 totalSize) {
    
    // status bar update, display number of files and remianing size :
    QString remainingFiles = i18n("Files: <numid>%1</numid> (%2)", totalFiles, Utility::convertByteHumanReadable(totalSize));
    
    this->sizeLabel->setText(remainingFiles);
}



void MyStatusBar::updateDownloadSpeedInfoSlot(const QString speedInKBStr){
    
    this->speedLabel->setText(i18n("Speed: %1", speedInKBStr));
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



bool MyStatusBar::eventFilter(QObject* object, QEvent* event) {

    bool returnValue = true;

    if (event->type() == QEvent::MouseButtonDblClick) {

        // search object in map :
        if (widgetPreferencesPageMap.contains(object)) {

            // if found display proper settings page :
            const UtilityNamespace::PreferencesPage page = widgetPreferencesPageMap.value(object);
            emit showSettingsSignal(page);

        }
    }
    else {
        // pass the event on to the parent class :
        returnValue = KStatusBar::eventFilter(object, event);
    }

    return returnValue;

}
