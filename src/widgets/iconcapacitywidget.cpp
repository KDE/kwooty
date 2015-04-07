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


#include "iconcapacitywidget.h"

#include <KIconLoader>
#include "kwooty_debug.h"

#include <QPixmap>

#include "statusbarwidgetbase.h"


IconCapacityWidget::IconCapacityWidget(QWidget* parent, MyStatusBar::WidgetIdentity widgetIdentity) : StatusBarWidgetBase(parent, widgetIdentity) {

    this->iconLoader = KIconLoader::global();

    this->iconLabel = new QLabel(this);
    this->capacityBar = new KCapacityBar(KCapacityBar::DrawTextInline, this);

    this->hBoxLayout = new QHBoxLayout(this);
    this->hBoxLayout->addWidget(this->iconLabel);
    this->hBoxLayout->addWidget(this->capacityBar);

    this->hBoxLayout->setSpacing(5);
    this->hBoxLayout->setMargin(0);

}



void IconCapacityWidget::setIcon(const QString& iconStr) {

    if (!iconStr.isEmpty()) {

        this->iconLabel->setPixmap(this->iconLoader->loadIcon(iconStr, KIconLoader::Small));
    }
    else {
        this->iconLabel->setPixmap(QPixmap());
    }

}



void IconCapacityWidget::showIcon() {
    this->iconLabel->show();
}


void IconCapacityWidget::hideIcon() {
    this->iconLabel->hide();
}



void IconCapacityWidget::updateCapacity(const QString& availableVal, const int& usedDiskPercentage) {

    // adjust capacityBar size according to text :
    int widthInPixel = this->capacityBar->fontMetrics().width(availableVal) + 30;

    // if capacitybar width is lower than current width, ajust it :
    if (this->capacityBar->minimumWidth() < widthInPixel) {

        this->capacityBar->setMinimumWidth(widthInPixel);

    }

    this->capacityBar->setValue(usedDiskPercentage);
    this->capacityBar->setText(availableVal);
    this->capacityBar->update();

}
