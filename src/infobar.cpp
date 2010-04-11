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

#include <KDebug>

#include "infobar.h"

#include "settings.h"
#include "centralwidget.h"



InfoBar::InfoBar(CentralWidget* parent) : QFrame(parent) {



    timeInfoWidget = new IconTextWidget(this);
    timeInfoWidget->setIcon("user-away");

    this->capacityBar = new KCapacityBar(KCapacityBar::DrawTextInline, this);

    hBoxLayout = new QHBoxLayout(this);
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);

    hBoxLayout->addWidget(this->timeInfoWidget);
    hBoxLayout->addSpacing(10);

    hBoxLayout->addStretch();
    iconTextFreeSpace = new IconTextWidget(this);
    iconTextFreeSpace->setText(i18n("Disk space: "));

    hBoxLayout->addWidget(iconTextFreeSpace);
    hBoxLayout->addWidget(this->capacityBar);

}



void InfoBar::updateTimeInfoSlot(const QString etaTimeStr) {

    QString timeText = i18n("Time of arrival: ");

    if (Settings::rtRadioButton()) {
        timeText = i18n("Remaining time: ");
    }

    this->timeInfoWidget->setText(timeText + etaTimeStr);
}



void InfoBar::updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace diskSpaceStatus, const QString availableVal, const int usedDiskPercentage) {

    // diskspace is unknown, hide widgets :
    if (diskSpaceStatus == UnknownDiskSpace) {

        this->capacityBar->hide();
        this->iconTextFreeSpace->hide();
    }

    else {

        // if capacity bar was hidden because diskSpaceStatus was previously UnknownDiskSpace :
        if (this->capacityBar->isHidden()) {

            this->capacityBar->show();
            this->iconTextFreeSpace->show();

        }

        // if free disk space is not sufficient display warning icon :
        if (diskSpaceStatus == InsufficientDiskSpace) {

            iconTextFreeSpace->setIcon("dialog-warning");
            //iconTextFreeSpace->showIcon();

        }

        // if free disk space is not sufficient do not display icon :
        if (diskSpaceStatus == SufficientDiskSpace) {

            iconTextFreeSpace->setIcon("");
            //iconTextFreeSpace->hideIcon();
        }


        // adjust capacityBar size according to text :
        int widthInPixel = this->capacityBar->fontMetrics().width(availableVal) + 30;

        // if capacitybar width is lower than current width, ajust it :
        if (this->capacityBar->minimumWidth() < widthInPixel) {

            this->capacityBar->setMinimumWidth(widthInPixel);

        }

        // set text and repaint widget :
        this->capacityBar->setValue(usedDiskPercentage);
        this->capacityBar->setText(availableVal);
        this->capacityBar->update();

    }

}

