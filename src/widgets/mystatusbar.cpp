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

    mClientsObserver = parent->getCore()->getClientsObserver();

    // create connection widget at bottom left of status bar :
    setConnectionWidget();

    // create remaining time widget at bottom left of status bar :
    setTimeInfoWidget();

    // create shutdown widget at bottom left of status bar :
    setShutdownWidget();

    // add capacity bar widget :
    mIconCapacityWidget = new IconCapacityWidget(this, IconCapacityWidgetIdentity);
    addPermanentWidget(mIconCapacityWidget);

    // add remaining download size item :
    mSizeLabel = new IconTextWidget(this, SizeWidgetIdentity);
    addPermanentWidget(mSizeLabel);

    // add download speed item :
    mSpeedLabel = new IconTextWidget(this, SpeedWidgetIdentity);
    addPermanentWidget(mSpeedLabel);

    // add info bar widget :
    setInfoBarWidget();

    setupConnections();

    // get current connection status, numbre of remainig files, etc... from clients observer :
    mClientsObserver->sendFullUpdate();

}

MyStatusBar::MyStatusBar() {}
MyStatusBar::~MyStatusBar() {}

void MyStatusBar::setupConnections()
{

    connect(mClientsObserver,
            &ClientsObserver::updateConnectionStatusSignal,
            this,
            &MyStatusBar::updateConnectionStatusSlot);

    // send remaining size to status bar :
    connect(mClientsObserver,
            &ClientsObserver::updateFileSizeInfoSignal,
            this,
            &MyStatusBar::updateFileSizeInfoSlot);

    // send download speed to status bar :
    connect(mClientsObserver->getStatsInfoBuilder(),
            &StatsInfoBuilder::updateDownloadSpeedInfoSignal,
            this,
            &MyStatusBar::updateDownloadSpeedInfoSlot);

    // send ETA to status bar :
    connect(mClientsObserver->getStatsInfoBuilder(),
            &StatsInfoBuilder::updateTimeInfoSignal,
            this,
            &MyStatusBar::updateTimeInfoSlot);

    // send free space to status bar :
    connect(mClientsObserver->getStatsInfoBuilder(),
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
    connect(mInfoBarWidget,
            &IconTextWidget::activeSignal,
            (static_cast<MainWindow *>(parentWidget()))->getSideBar(),
            &SideBar::activeSlot);

}

void MyStatusBar::setConnectionWidget()
{

    mConnectionWidget = new IconTextWidget(this, ConnectionWidgetIdentity);

    // add aggregated widget to the status bar :
    addWidget(mConnectionWidget);

    // set connection not active by default :
    updateConnectionStatusSlot();

}

void MyStatusBar::setTimeInfoWidget()
{

    mTimeInfoWidget = new IconTextWidget(this, TimeInfoWidgetIdentity);
    mTimeInfoWidget->setIcon(QStringLiteral("user-away"));

    // add aggregated widget to the status bar :
    addWidget(mTimeInfoWidget);

}

void MyStatusBar::setShutdownWidget()
{

    mShutdownWidget = new IconTextWidget(this, ShutdownWidgetIdentity);

    // add aggregated widget to the status bar :
    addWidget(mShutdownWidget);

}

void MyStatusBar::setInfoBarWidget()
{

    // add widget that will toggle info bar :
    mInfoBarWidget = new IconTextWidget(this, InfoBarWidgetIdentity);

    mInfoBarWidget->setIconMode(IconTextWidget::SwitchIcon);
    mInfoBarWidget->setIconOnly(QStringLiteral("arrow-up-double"), QStringLiteral("arrow-down-double"));

    bool sideBarDisplay = KConfigGroupHandler::getInstance()->readSideBarDisplay();
    mInfoBarWidget->setActive(sideBarDisplay);

    // add aggregated widget to the right of status bar :
    addPermanentWidget(mInfoBarWidget);

}

IconTextWidget *MyStatusBar::getInfoBarWidget()
{
    return mInfoBarWidget;
}

void MyStatusBar::statusBarShutdownInfoSlot(const QString &iconStr, const QString &text)
{

    mShutdownWidget->setIcon(iconStr);
    mShutdownWidget->setText(text);

}

void MyStatusBar::updateConnectionStatusSlot()
{

    QString connection;
    ServerConnectionIcon serverConnectionIcon = UtilityServerStatus::buildConnectionStringFromStatus(mClientsObserver, connection);

    // set icon :
    mConnectionWidget->setIcon(serverConnectionIcon);
    // set text :
    mConnectionWidget->setText(connection);

    // set tooltip to connection widget :
    buildConnWidgetToolTip(connection);

}

void MyStatusBar::buildConnWidgetToolTip(const QString &connection)
{

    QString toolTipStr;

    // display tooltip connection only for a single server connection :
    QString hostName;
    if (mClientsObserver->isSingleServer(hostName)) {
        toolTipStr = UtilityServerStatus::buildConnectionToolTip(mClientsObserver, connection, hostName);
    }

    // set tooltip :
    mConnectionWidget->setToolTip(toolTipStr);

}

void MyStatusBar::updateFileSizeInfoSlot(const quint64 totalFiles, const quint64 totalSize)
{

    // status bar update, display number of files and remianing size :
    const QString remainingFiles = i18n("Files: %1 (%2)", totalFiles, Utility::convertByteHumanReadable(totalSize));

    mSizeLabel->setTextOnly(remainingFiles);
}

void MyStatusBar::updateDownloadSpeedInfoSlot(const QString &speedInKBStr)
{

    mSpeedLabel->setTextOnly(i18n("Speed: %1", speedInKBStr));
}

void MyStatusBar::updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace diskSpaceStatus, const QString &availableVal, const int usedDiskPercentage)
{

    // diskspace is unknown, hide widgets :
    if (diskSpaceStatus == UnknownDiskSpace) {

        mIconCapacityWidget->hide();
    }

    else {

        // if capacity bar was hidden because diskSpaceStatus was previously UnknownDiskSpace :
        if (mIconCapacityWidget->isHidden()) {
            mIconCapacityWidget->show();
        }

        // if free disk space is not sufficient display warning icon :
        if (diskSpaceStatus == InsufficientDiskSpace) {

            mIconCapacityWidget->setIcon(QStringLiteral("dialog-warning"));
            mIconCapacityWidget->setToolTip(i18n("Insufficient disk space"));

        }

        // if free disk space is not sufficient do not display icon :
        if (diskSpaceStatus == SufficientDiskSpace) {

            mIconCapacityWidget->setIcon(QString());
            mIconCapacityWidget->setToolTip(QString());

        }

        // set text and repaint widget :
        mIconCapacityWidget->updateCapacity(availableVal, usedDiskPercentage);

    }

}

void MyStatusBar::updateTimeInfoSlot(const bool parentDownloadingFound)
{

    QString timeInfoStr;
    QString timeInfoToolTip;

    QString currentTimeValue = mClientsObserver->getStatsInfoBuilder()->getCurrentTimeValue();
    QString totalTimeValue = mClientsObserver->getStatsInfoBuilder()->getTotalTimeValue();

    // build text and toolTip :
    if (!currentTimeValue.isEmpty()) {

        timeInfoStr.append(currentTimeValue);

        QString timeLabel = mClientsObserver->getStatsInfoBuilder()->getTimeLabel();
        timeInfoToolTip.append(QStringLiteral("<b>%1</b>").arg(timeLabel));

        timeInfoToolTip.append(QStringLiteral("<table style='white-space: nowrap'>"));
        QString nzbNameDownloading = mClientsObserver->getStatsInfoBuilder()->getNzbNameDownloading();
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

    mTimeInfoWidget->setText(timeInfoStr);
    mTimeInfoWidget->setToolTip(timeInfoToolTip);

    // if download is not active, hide the widget :
    if (!parentDownloadingFound) {
        mTimeInfoWidget->hide();
    }
    // else display it :
    else if (mTimeInfoWidget->isHidden()) {
        mTimeInfoWidget->show();
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
