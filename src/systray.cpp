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


#include "systray.h"

#include "kwooty_debug.h"
#include <KColorScheme>
#include <QIcon>
#include <KIconLoader>
#include <KActionCollection>
#include <KIconEffect>
#include <QMenu>
#include <KLocalizedString>
#include <QToolTip>
#include <QPainter>

#include "mainwindow.h"
#include "core.h"
#include "statsinfobuilder.h"
#include "observers/clientsobserver.h"
#include "observers/queuefileobserver.h"
#include "kwootysettings.h"


SysTray::SysTray(MainWindow* parent) : KStatusNotifierItem(parent) {

    this->mParent = parent;

    Core* core = parent->getCore();
    this->mQueueFileObserver = core->getQueueFileObserver();
    this->mClientsObserver = core->getClientsObserver();
    this->mStatsInfoBuilder = core->getClientsObserver()->getStatsInfoBuilder();

    this->mOldMergePos = PROGRESS_UNKNOWN;

    this->setupActions();
    this->initPixmaps();
    this->initShow();


}



void SysTray::setupActions() {   

    this->contextMenu()->addAction(this->mParent->actionCollection()->action("startAll"));
    this->contextMenu()->addAction(this->mParent->actionCollection()->action("pauseAll"));
    this->contextMenu()->addSeparator();
    this->contextMenu()->addAction(this->mParent->actionCollection()->action("downloadFolder"));
#if 0 //PORT KF5
    QList<QAction*> sysTrayActionCollection = this->actionCollection();
    KStandardAction::quit(this->parent, SLOT(quit()), sysTrayActionCollection);
#endif
}



void SysTray::setupConnections() {

    // update systray icon when download progress has changed :
    connect (this->mQueueFileObserver,
             SIGNAL(progressUpdateSignal(int)),
             this,
             SLOT(progressUpdateSlot(int)));

    // update status icon overlay when status has changed :
    connect (this->mQueueFileObserver,
             SIGNAL(statusUpdateSignal(UtilityNamespace::ItemStatus)),
             this,
             SLOT(statusUpdateSlot(UtilityNamespace::ItemStatus)));

    // update tooltip when download speed has been updated :
    connect (this->mStatsInfoBuilder,
             SIGNAL(updateDownloadSpeedInfoSignal(QString)),
             this,
             SLOT(updateDownloadSpeedInfoSlot()));

    // update tooltip if connection server has changed :
    connect (this->mClientsObserver,
             SIGNAL(updateConnectionStatusSignal()),
             this,
             SLOT(updateConnectionStatusSlot()));

}


void SysTray::initShow() {

    this->setStatus(KStatusNotifierItem::Active);
    this->setCategory(KStatusNotifierItem::ApplicationStatus);

    // setup signals/slots connections :
    this->setupConnections();
    this->initPixmaps();

    // build general tooltip info :
    this->setToolTipIconByName("kwooty");
    this->setToolTipTitle("Kwooty");

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



void SysTray::initPixmaps() {

    this->mNormalBaseIcon = KIconLoader::global()->loadIcon("kwooty", KIconLoader::Panel, KIconLoader::SizeSmallMedium);
    this->setIconByName("kwooty");
    this->setOverlayIconByName(QString());

    // store grayed icon :
    this->mGrayedBaseIcon = this->mNormalBaseIcon;
    QImage normalImage = this->mNormalBaseIcon.toImage();
    KIconEffect::toMonochrome(normalImage, QColor("black"), QColor("black"), 0.30);
    this->mGrayedBaseIcon = QPixmap::fromImage(normalImage);

}


bool SysTray::updateIconStatus(const UtilityNamespace::ItemStatus& itemStatus) {

    QIcon icon;

    // get the proper icon according to item status :
    switch(itemStatus) {

    case UtilityNamespace::DownloadStatus:{

            icon = QIcon::fromTheme("mail-receive");
            break;
        }

    case UtilityNamespace::PauseStatus: {
            icon = QIcon::fromTheme("media-playback-pause");
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

        this->mOldMergePos = PROGRESS_UNKNOWN;
        this->setIconByName("kwooty");
        this->setOverlayIconByName(QString());

    }
    else {
        int mergePos = qRound((qreal)(progress * (this->mNormalBaseIcon.height() - 1)) / 100);

        // update icon only if merge positions are different :
        if (mergePos != this->mOldMergePos) {

            // draw the icon from the gray one :
            this->mRenderedIcon = this->mGrayedBaseIcon;

            QPainter p(&this->mRenderedIcon);
            p.drawPixmap( 0, 0, this->mNormalBaseIcon, 0, 0, 0, this->mNormalBaseIcon.height() - mergePos );
            p.end();

            // draw kwooty icon with status overlay :
            bool iconStatusSet = this->updateIconStatus(this->mQueueFileObserver->getFocusedItemStatus());

            // if status icon has not been set, just set the rendered icon without status overlay :
            if (!iconStatusSet) {
                this->setIconByPixmap (this->mRenderedIcon);
            }

            this->mOldMergePos = mergePos;

        }

    }

}

bool SysTray::blendOverlay(const QPixmap& overlay) {

    bool iconSet = false;

    if (!overlay.isNull()) {

        // draw icon status overlay at bottom right
        const int x = this->mNormalBaseIcon.size().width() - overlay.size().width();
        const int y = this->mNormalBaseIcon.size().height() - overlay.size().width();

        QPixmap finalIcon = this->mRenderedIcon;
        QPainter p(&finalIcon);
        p.drawPixmap(x, y, overlay);
        p.end();
        this->setIconByPixmap(finalIcon);

        iconSet = true;
    }

    return iconSet;
}





void SysTray::createToolTip() {

    QString currentTip;
    currentTip.append("<table style='white-space: nowrap'>");

    // 1. display current status :
    QString globalStatusValue = i18n("Idle");

    UtilityNamespace::ItemStatus focusedItem = this->mQueueFileObserver->getFocusedItemStatus();

    // current status is either download or pause :
    if (focusedItem == UtilityNamespace::DownloadStatus) {
        globalStatusValue = i18n("Downloading");
    }
    else if (focusedItem == UtilityNamespace::PauseStatus)  {
        globalStatusValue = i18n("Pause");
    }
    // check if disconnected from server :
    else if (!this->mClientsObserver->isConnected()) {
            globalStatusValue = i18n("Disconnected");
    }

    currentTip.append(Utility::buildToolTipRow(i18n("Status: "), globalStatusValue));


    // 2. add additional info if status is either download or pause :
    if ( (focusedItem == UtilityNamespace::DownloadStatus) ||
         (focusedItem == UtilityNamespace::PauseStatus) ) {

        // add download speed :
        QString downloadSpeed = this->mStatsInfoBuilder->getDownloadSpeedReadableStr();
        currentTip.append(Utility::buildToolTipRow(i18n("Download speed: "), downloadSpeed));

        // add nzb file name :
        QString nzbFileNameValue = this->mStatsInfoBuilder->getNzbNameDownloading();
        currentTip.append(Utility::buildToolTipRow(i18n("File: "), nzbFileNameValue));

        // add download progress :
        QString progressValue = i18nc("download progress percent",
                                      "%1 %", this->mQueueFileObserver->getFocusedProgressValue());
        currentTip.append(Utility::buildToolTipRow(i18n("Progress: "), progressValue));

    }

    // 3. add ETA or remainig time only if file is currently being downloading :
    if (focusedItem == UtilityNamespace::DownloadStatus) {

        QString timeLabel = this->mStatsInfoBuilder->getTimeLabel();
        QString currentTimeValue = this->mStatsInfoBuilder->getCurrentTimeValue();

        if (!currentTimeValue.isEmpty()) {
            currentTip.append(Utility::buildToolTipRow(timeLabel, currentTimeValue));
        }

    }

    currentTip.append("</table>");

    this->setToolTipSubTitle(currentTip);

}


