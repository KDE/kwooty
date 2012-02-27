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


#include "systraylegacy.h"

#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KIconLoader>
#include <KActionCollection>
#include <KIconEffect>

#include <QMenu>
#include <QToolTip>
#include <QPainter>

#include "mainwindow.h"
#include "core.h"
#include "statsinfobuilder.h"
#include "observers/clientsobserver.h"
#include "observers/queuefileobserver.h"
#include "kwootysettings.h"


SysTray::SysTray(MainWindow* parent) : KSystemTrayIcon(parent) {

    this->parent = parent;

    Core* core = parent->getCore();
    this->queueFileObserver = core->getQueueFileObserver();
    this->clientsObserver = core->getClientsObserver();
    this->statsInfoBuilder = core->getClientsObserver()->getStatsInfoBuilder();

    this->oldMergePos = PROGRESS_UNKNOWN;

    this->setupActions();
    this->initPixmaps();
    this->initShow();


}

SysTray::~SysTray() {

}


void SysTray::setupActions() {   

    this->contextMenu()->addAction(this->parent->actionCollection()->action("startAll"));
    this->contextMenu()->addAction(this->parent->actionCollection()->action("pauseAll"));
    this->contextMenu()->addSeparator();
    this->contextMenu()->addAction(this->parent->actionCollection()->action("downloadFolder"));

    KActionCollection* sysTrayActionCollection = this->actionCollection();
    KStandardAction::quit(this->parent, SLOT(quit()), sysTrayActionCollection);

}



void SysTray::setupConnections() {

    // update systray icon when download progress has changed :
    connect (this->queueFileObserver,
             SIGNAL(progressUpdateSignal(const int)),
             this,
             SLOT(progressUpdateSlot(const int)));

    // update status icon overlay when status has changed :
    connect (this->queueFileObserver,
             SIGNAL(statusUpdateSignal(const UtilityNamespace::ItemStatus)),
             this,
             SLOT(statusUpdateSlot(const UtilityNamespace::ItemStatus)));

    // update tooltip when download speed has been updated :
    connect (this->statsInfoBuilder,
             SIGNAL(updateDownloadSpeedInfoSignal(const QString)),
             this,
             SLOT(updateDownloadSpeedInfoSlot()));

    // update tooltip if connection server has changed :
    connect (this->clientsObserver,
             SIGNAL(updateConnectionStatusSignal()),
             this,
             SLOT(updateConnectionStatusSlot()));

    // allow toolTip display if context menu is hidden :
    connect (this->contextMenu(),
             SIGNAL(aboutToHide()),
             this,
             SLOT(menuAboutToHideSlot()));

    // avoid toolTip display if context menu is shown :
    connect (this->contextMenu(),
             SIGNAL(aboutToShow()),
             this,
             SLOT(menuAboutToShowSlot()));


}


void SysTray::initShow() {

    this->displayToolTip = true;

    this->show();
    // setup signals/slots connections :
    this->setupConnections();
    this->initPixmaps();

    // update systray icon
    this->updateIconProgress(PROGRESS_UNKNOWN);

}


void SysTray::progressUpdateSlot(const int progress) {
    this->updateIconProgress(progress);
    this->createToolTip();
}


void SysTray::statusUpdateSlot(const UtilityNamespace::ItemStatus itemStatus) {
    this->updateIconStatus(itemStatus);
    this->createToolTip();
}


void SysTray::updateDownloadSpeedInfoSlot() {
    this->createToolTip();
}

void SysTray::updateConnectionStatusSlot() {
    this->createToolTip();
}


void SysTray::menuAboutToHideSlot() {
    this->displayToolTip = true;
}

void SysTray::menuAboutToShowSlot() {
    this->displayToolTip = false;
}


void SysTray::initPixmaps() {

    QIcon icon = this->loadIcon("kwooty");
    this->setIcon(icon);

    // store normal icon :
    this->normalBaseIcon = icon.pixmap(geometry().size());

    // store grayed icon :
    this->grayedBaseIcon = this->normalBaseIcon;
    QImage normalImage = this->normalBaseIcon.toImage();
    KIconEffect::toMonochrome(normalImage, QColor("black"), QColor("black"), 0.30);
    this->grayedBaseIcon = QPixmap::fromImage(normalImage);



}


bool SysTray::updateIconStatus(const UtilityNamespace::ItemStatus& itemStatus) {

    KIcon icon;

    // get the proper icon according to item status :
    switch(itemStatus) {

    case UtilityNamespace::DownloadStatus:{

            icon = KIcon("mail-receive");
            break;
        }

    case UtilityNamespace::PauseStatus: {
            icon = KIcon("media-playback-pause");
            break;
        }

    default: {
            break;
        }

    }


    bool iconSet = false;
    if (!icon.isNull()) {

        QPixmap statusPixmap = icon.pixmap(10, 10);
        iconSet = this->blendOverlay(statusPixmap);

    }

    return iconSet;

}



void SysTray::updateIconProgress(const int& progress) {
    // - Icon painting has been adapted from amarok -

    // if  progress is unknown, reset icon :
    if (progress == PROGRESS_UNKNOWN) {

        this->oldMergePos = PROGRESS_UNKNOWN;
        this->setIcon(this->normalBaseIcon);

    }
    else {
        int mergePos = qRound((qreal)(progress * (geometry().height() - 1)) / 100);

        // update icon only if merge positions are different :
        if (mergePos != this->oldMergePos) {

            // draw the icon from the gray one :
            this->renderedIcon = this->grayedBaseIcon;

            QPainter p(&this->renderedIcon);
            p.drawPixmap( 0, 0, this->normalBaseIcon, 0, 0, 0, geometry().height() - mergePos );
            p.end();

            // draw small status icon at bottom right over kwooty icon :
            bool iconStatusSet = this->updateIconStatus(this->queueFileObserver->getFocusedItemStatus());

            // if status icon has not been set, just set the rendered icon without status overlay :
            if (!iconStatusSet) {
                this->setIcon(this->renderedIcon);
            }

            this->oldMergePos = mergePos;

        }

    }

}



bool SysTray::blendOverlay(const QPixmap& overlay) {

    bool iconSet = false;

    if (!overlay.isNull()) {

        // draw icon status overlay at bottom right
        const int x = geometry().size().width() - overlay.size().width();
        const int y = geometry().size().height() - overlay.size().width();

        QPixmap finalIcon = this->renderedIcon;
        QPainter p(&finalIcon);
        p.drawPixmap(x, y, overlay);
        p.end();
        this->setIcon(finalIcon);

        iconSet = true;
    }

    return iconSet;
}



void SysTray::createToolTip() {

    if (this->displayToolTip && this->geometry().contains(QCursor::pos()) ) {

        QString currentTip;
        currentTip.append("<table style='white-space: nowrap'>");

        // 1. display current status :
        QString globalStatusValue = i18n("Idle");

        UtilityNamespace::ItemStatus focusedItem = this->queueFileObserver->getFocusedItemStatus();

        // current status is either download or pause :
        if (focusedItem == UtilityNamespace::DownloadStatus) {
            globalStatusValue = i18n("Downloading");
        }
        else if (focusedItem == UtilityNamespace::PauseStatus)  {
            globalStatusValue = i18n("Pause");
        }
        // check if disconnected from server :
        else if (!this->clientsObserver->isConnected()){
                globalStatusValue = i18n("Disconnected");
        }

        currentTip.append(Utility::buildToolTipRow(i18n("Status: "), globalStatusValue));


        // 2. add additional info if status is either download or pause :
        if ( (focusedItem == UtilityNamespace::DownloadStatus) ||
             (focusedItem == UtilityNamespace::PauseStatus) ) {

            // add download speed :
            QString downloadSpeed = this->statsInfoBuilder->getDownloadSpeedReadableStr();
            currentTip.append(Utility::buildToolTipRow(i18n("Download speed: "), downloadSpeed));

            // add nzb file name :
            QString nzbFileNameValue = this->statsInfoBuilder->getNzbNameDownloading();
            currentTip.append(Utility::buildToolTipRow(i18n("File: "), nzbFileNameValue));

            // add download progress :
            QString progressValue = i18nc("download progress percent",
                                          "<numid>%1</numid> %", this->queueFileObserver->getFocusedProgressValue());
            currentTip.append(Utility::buildToolTipRow(i18n("Progress: "), progressValue));

        }

        // 3. add ETA or remainig time only if file is currently being downloading :
        if (focusedItem == UtilityNamespace::DownloadStatus) {

            QString timeLabel = this->statsInfoBuilder->getTimeLabel();
            QString currentTimeValue = this->statsInfoBuilder->getCurrentTimeValue();

            if (!currentTimeValue.isEmpty()) {
                currentTip.append(Utility::buildToolTipRow(timeLabel, currentTimeValue));
            }

        }

        currentTip.append("</table>");       

        QToolTip::showText(QCursor::pos(), currentTip);
    }

}


