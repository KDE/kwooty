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
#include <QIcon>
#include "kwooty_debug.h"
#include <KLocalizedString>
#include <QHBoxLayout>

#include "mainwindow.h"
#include "core.h"
#include "statsinfobuilder.h"
#include "sidebar.h"
#include "shutdown/shutdownmanager.h"
#include "observers/clientsobserver.h"
#include "widgets/icontextwidget.h"
#include "widgets/iconcapacitywidget.h"
#include "preferences/kconfiggrouphandler.h"
#include "utilities/utilityserverstatus.h"
#include "kwootysettings.h"

MyStatusBar::MyStatusBar(MainWindow *parent) : QStatusBar(parent)
{

    clientsObserver = parent->getCore()->getClientsObserver();

    // create connection widget at bottom left of status bar :
    setConnectionWidget();

    // create remaining time widget at bottom left of status bar :
    setTimeInfoWidget();

    // create shutdown widget at bottom left of status bar :
    setShutdownWidget();

    // add capacity bar widget :
    iconCapacityWidget = new IconCapacityWidget(this, IconCapacityWidgetIdentity);
    addPermanentWidget(iconCapacityWidget);

    // add remaining download size item :
    sizeLabel = new IconTextWidget(this, SizeWidgetIdentity);
    addPermanentWidget(sizeLabel);

    // add download speed item :
    speedLabel = new IconTextWidget(this, SpeedWidgetIdentity);
    addPermanentWidget(speedLabel);

    // add info bar widget :
    setInfoBarWidget();

    setupConnections();

    // get current connection status, numbre of remainig files, etc... from clients observer :
    clientsObserver->sendFullUpdate();

}

MyStatusBar::MyStatusBar() {}
MyStatusBar::~MyStatusBar() {}

void MyStatusBar::setupConnections()
{

    connect(clientsObserver,
            &ClientsObserver::updateConnectionStatusSignal,
            this,
            &MyStatusBar::updateConnectionStatusSlot);

    // send remaining size to status bar :
    connect(clientsObserver,
            &ClientsObserver::updateFileSizeInfoSignal,
            this,
            &MyStatusBar::updateFileSizeInfoSlot);

    // send download speed to status bar :
    connect(clientsObserver->getStatsInfoBuilder(),
            &StatsInfoBuilder::updateDownloadSpeedInfoSignal,
            this,
            &MyStatusBar::updateDownloadSpeedInfoSlot);

    // send ETA to status bar :
    connect(clientsObserver->getStatsInfoBuilder(),
            &StatsInfoBuilder::updateTimeInfoSignal,
            this,
            &MyStatusBar::updateTimeInfoSlot);

    // send free space to status bar :
    connect(clientsObserver->getStatsInfoBuilder(),
            &StatsInfoBuilder::updateFreeSpaceSignal,
            this,
            &MyStatusBar::updateFreeSpaceSlot);

    // send shutdown info in status bar :
    connect((static_cast<MainWindow *>(parentWidget()))->getCore()->getShutdownManager(),
            &ShutdownManager::statusBarShutdownInfoSignal,
            this,
            &MyStatusBar::statusBarShutdownInfoSlot);

    // display the proper settings page when double clicking on widgets from statusBar :
    connect(this,
            &MyStatusBar::showSettingsSignal,
            static_cast<MainWindow *>(parentWidget()),
            &MainWindow::showSettings);

    // show / hide sideBarWidget when button is toggled :
    connect(infoBarWidget,
            &IconTextWidget::activeSignal,
            (static_cast<MainWindow *>(parentWidget()))->getSideBar(),
            &SideBar::activeSlot);

}

void MyStatusBar::setConnectionWidget()
{

    connectionWidget = new IconTextWidget(this, ConnectionWidgetIdentity);

    // add aggregated widget to the status bar :
    addWidget(connectionWidget);

    // set connection not active by default :
    updateConnectionStatusSlot();

}

void MyStatusBar::setTimeInfoWidget()
{

    timeInfoWidget = new IconTextWidget(this, TimeInfoWidgetIdentity);
    timeInfoWidget->setIcon(QStringLiteral("user-away"));

    // add aggregated widget to the status bar :
    addWidget(timeInfoWidget);

}

void MyStatusBar::setShutdownWidget()
{

    shutdownWidget = new IconTextWidget(this, ShutdownWidgetIdentity);

    // add aggregated widget to the status bar :
    addWidget(shutdownWidget);

}

void MyStatusBar::setInfoBarWidget()
{

    // add widget that will toggle info bar :
    infoBarWidget = new IconTextWidget(this, InfoBarWidgetIdentity);

    infoBarWidget->setIconMode(IconTextWidget::SwitchIcon);
    infoBarWidget->setIconOnly(QStringLiteral("arrow-up-double"), QStringLiteral("arrow-down-double"));

    bool sideBarDisplay = KConfigGroupHandler::getInstance()->readSideBarDisplay();
    infoBarWidget->setActive(sideBarDisplay);

    // add aggregated widget to the right of status bar :
    addPermanentWidget(infoBarWidget);

}

IconTextWidget *MyStatusBar::getInfoBarWidget()
{
    return infoBarWidget;
}

void MyStatusBar::statusBarShutdownInfoSlot(const QString &iconStr, const QString &text)
{

    shutdownWidget->setIcon(iconStr);
    shutdownWidget->setText(text);

}

void MyStatusBar::updateConnectionStatusSlot()
{

    QString connection;
    ServerConnectionIcon serverConnectionIcon = UtilityServerStatus::buildConnectionStringFromStatus(clientsObserver, connection);

    // set icon :
    connectionWidget->setIcon(serverConnectionIcon);
    // set text :
    connectionWidget->setText(connection);

    // set tooltip to connection widget :
    buildConnWidgetToolTip(connection);

}

void MyStatusBar::buildConnWidgetToolTip(const QString &connection)
{

    QString toolTipStr;

    // display tooltip connection only for a single server connection :
    QString hostName;
    if (clientsObserver->isSingleServer(hostName)) {
        toolTipStr = UtilityServerStatus::buildConnectionToolTip(clientsObserver, connection, hostName);
    }

    // set tooltip :
    connectionWidget->setToolTip(toolTipStr);

}

void MyStatusBar::updateFileSizeInfoSlot(const quint64 totalFiles, const quint64 totalSize)
{

    // status bar update, display number of files and remianing size :
    const QString remainingFiles = i18n("Files: %1 (%2)", totalFiles, Utility::convertByteHumanReadable(totalSize));

    sizeLabel->setTextOnly(remainingFiles);
}

void MyStatusBar::updateDownloadSpeedInfoSlot(const QString &speedInKBStr)
{

    speedLabel->setTextOnly(i18n("Speed: %1", speedInKBStr));
}

void MyStatusBar::updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace diskSpaceStatus, const QString &availableVal, const int usedDiskPercentage)
{

    // diskspace is unknown, hide widgets :
    if (diskSpaceStatus == UnknownDiskSpace) {

        iconCapacityWidget->hide();
    }

    else {

        // if capacity bar was hidden because diskSpaceStatus was previously UnknownDiskSpace :
        if (iconCapacityWidget->isHidden()) {
            iconCapacityWidget->show();
        }

        // if free disk space is not sufficient display warning icon :
        if (diskSpaceStatus == InsufficientDiskSpace) {

            iconCapacityWidget->setIcon(QStringLiteral("dialog-warning"));
            iconCapacityWidget->setToolTip(i18n("Insufficient disk space"));

        }

        // if free disk space is not sufficient do not display icon :
        if (diskSpaceStatus == SufficientDiskSpace) {

            iconCapacityWidget->setIcon(QString());
            iconCapacityWidget->setToolTip(QString());

        }

        // set text and repaint widget :
        iconCapacityWidget->updateCapacity(availableVal, usedDiskPercentage);

    }

}

void MyStatusBar::updateTimeInfoSlot(const bool parentDownloadingFound)
{

    QString timeInfoStr;
    QString timeInfoToolTip;

    QString currentTimeValue = clientsObserver->getStatsInfoBuilder()->getCurrentTimeValue();
    QString totalTimeValue = clientsObserver->getStatsInfoBuilder()->getTotalTimeValue();

    // build text and toolTip :
    if (!currentTimeValue.isEmpty()) {

        timeInfoStr.append(currentTimeValue);

        QString timeLabel = clientsObserver->getStatsInfoBuilder()->getTimeLabel();
        timeInfoToolTip.append(QStringLiteral("<b>%1</b>").arg(timeLabel));

        timeInfoToolTip.append(QStringLiteral("<table style='white-space: nowrap'>"));
        QString nzbNameDownloading = clientsObserver->getStatsInfoBuilder()->getNzbNameDownloading();
        timeInfoToolTip.append(Utility::buildToolTipRow(QStringLiteral("%1:").arg(currentTimeValue), nzbNameDownloading));

    }

    if (!totalTimeValue.isEmpty()) {

        timeInfoStr.append(QStringLiteral("  -  "));
        timeInfoStr.append(totalTimeValue);

        timeInfoToolTip.append(Utility::buildToolTipRow(QStringLiteral("%1:").arg(totalTimeValue), i18n("Total")));
    }

    timeInfoToolTip.append(QStringLiteral("</table>"));

    if (currentTimeValue.isEmpty()) {
        timeInfoStr = i18n("n/a");
        timeInfoToolTip.clear();
    }

    timeInfoWidget->setText(timeInfoStr);
    timeInfoWidget->setToolTip(timeInfoToolTip);

    // if download is not active, hide the widget :
    if (!parentDownloadingFound) {
        timeInfoWidget->hide();
    }
    // else display it :
    else if (timeInfoWidget->isHidden()) {
        timeInfoWidget->show();
    }

}

void MyStatusBar::statusBarWidgetDblClickSlot(MyStatusBar::WidgetIdentity widgetIdentity)
{

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
