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


#include "icontextwidget.h"

#include <KIconLoader>
#include <KIcon>
#include <KIconEffect>
#include <KDebug>
#include <QMouseEvent>

#include <QPixmap>
#include <QPainter>


IconTextWidget::IconTextWidget(QWidget* parent) : QWidget(parent) {

    this->iconLoader = KIconLoader::global();

    this->iconGamma = false;
    this->iconPressed = false;
    this->iconLabel = new QLabel(this);
    this->textLabel = new QLabel(this);

    this->hBoxLayout = new QHBoxLayout(this);
    this->hBoxLayout->addWidget(this->iconLabel);
    this->hBoxLayout->addWidget(this->textLabel);

    this->hBoxLayout->setSpacing(5);
    this->hBoxLayout->setMargin(0);

    // installs event filter in order to display associated settings page :
    this->installEventFilter(parent);

}


void IconTextWidget::enterEvent(QEvent* event) {

    Q_UNUSED(event);
    if (this->iconGamma) {
        this->iconLabel->setPixmap(this->clearIcon);
    }
}

void IconTextWidget::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    if (this->iconGamma && !this->iconPressed) {
        this->iconLabel->setPixmap(this->normalIcon);
    }
}

void IconTextWidget::setIconGamma(const bool& iconGamma) {
    this->iconGamma = iconGamma;
}


void IconTextWidget::setIconOnly(const QString& iconStr) {
     this->hBoxLayout->setSpacing(0);
     this->setIcon(iconStr);
}

void IconTextWidget::mousePressEvent(QMouseEvent* event)  {

    Q_UNUSED(event);
    this->iconPressed = !this->iconPressed;
    this->enterEvent(event);

}



void IconTextWidget::setIcon(const QString& iconStr) {

    if (!iconStr.isEmpty()) {

        this->normalIcon = this->iconLoader->loadIcon(iconStr, KIconLoader::Small);
        this->iconLabel->setPixmap(this->normalIcon);

        QImage clearImage = this->normalIcon.toImage();
        KIconEffect::toGamma(clearImage, 0.80);
        this->clearIcon = QPixmap::fromImage(clearImage);        
    }
    else {
        this->iconLabel->setPixmap(QPixmap());
    }

}


void IconTextWidget::blendOverLay(const QString& overlayIconStr) {

    KIcon overlayIcon = KIcon(overlayIconStr);

    if (!overlayIcon.isNull() && this->iconLabel->pixmap()) {

        QPixmap warningPixmap = overlayIcon.pixmap(10, 10);

        QPixmap finalIcon = this->iconLabel->pixmap()->copy();
        QPainter p(&finalIcon);
        p.drawPixmap(KIconLoader::SizeSmall / 2, KIconLoader::SizeSmall  / 2, warningPixmap);
        p.end();

        this->iconLabel->setPixmap(finalIcon);
    }

}


void IconTextWidget::setText(const QString& text) {

    this->textLabel->setText(text);
}


QString IconTextWidget::getText() const {
    return this->textLabel->text();
}


void IconTextWidget::showIcon() {
    this->iconLabel->show();
}


void IconTextWidget::hideIcon() {
    this->iconLabel->hide();
}


