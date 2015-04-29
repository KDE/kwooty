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

#include <kcapacitybar.h>
#include <QLabel>

#include <KIconLoader>
#include "kwooty_debug.h"

#include <QPixmap>

#include "statusbarwidgetbase.h"


IconCapacityWidget::IconCapacityWidget(QWidget* parent, MyStatusBar::WidgetIdentity widgetIdentity) : StatusBarWidgetBase(parent, widgetIdentity) {

    iconLoader = KIconLoader::global();

    mIconLabel = new QLabel(this);
    mCapacityBar = new KCapacityBar(KCapacityBar::DrawTextInline, this);

    hBoxLayout = new QHBoxLayout(this);
    hBoxLayout->addWidget(mIconLabel);
    hBoxLayout->addWidget(mCapacityBar);

    hBoxLayout->setSpacing(5);
    hBoxLayout->setMargin(0);

}



void IconCapacityWidget::setIcon(const QString& iconStr) {

    if (!iconStr.isEmpty()) {

        mIconLabel->setPixmap(iconLoader->loadIcon(iconStr, KIconLoader::Small));
    }
    else {
        mIconLabel->setPixmap(QPixmap());
    }

}



void IconCapacityWidget::showIcon() {
    mIconLabel->show();
}


void IconCapacityWidget::hideIcon() {
    mIconLabel->hide();
}



void IconCapacityWidget::updateCapacity(const QString& availableVal, int usedDiskPercentage) {

    // adjust capacityBar size according to text :
    int widthInPixel = mCapacityBar->fontMetrics().width(availableVal) + 30;

    // if capacitybar width is lower than current width, ajust it :
    if (mCapacityBar->minimumWidth() < widthInPixel) {

        mCapacityBar->setMinimumWidth(widthInPixel);

    }

    mCapacityBar->setValue(usedDiskPercentage);
    mCapacityBar->setText(availableVal);
    mCapacityBar->update();

}
