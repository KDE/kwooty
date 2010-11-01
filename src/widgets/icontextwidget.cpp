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

#include "utilityiconpainting.h"


IconTextWidget::IconTextWidget(QWidget* parent) : QWidget(parent) {

    this->iconLoader = KIconLoader::global();

    this->iconPressed = false;
    this->iconLabel = new QLabel(this);
    this->textLabel = new QLabel(this);

    this->iconMode = NormalModeIcon;

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

    if (this->iconMode == GammaIcon) {
        this->iconLabel->setPixmap(this->clearNormalIcon);
    }

    if (this->iconMode == SwitchIcon && !this->iconPressed) {
        this->iconLabel->setPixmap(this->clearNormalIcon);
    }

    if (this->iconMode == SwitchIcon && this->iconPressed) {
        this->iconLabel->setPixmap(this->clearActiveIcon);
    }

}


void IconTextWidget::leaveEvent(QEvent* event) {

    Q_UNUSED(event);

    if (this->iconMode == GammaIcon && !this->iconPressed) {
        this->iconLabel->setPixmap(this->normalIcon);
    }

    if (this->iconMode == SwitchIcon && !this->iconPressed) {
        this->iconLabel->setPixmap(this->normalIcon);
    }

    if (this->iconMode == SwitchIcon && this->iconPressed) {
        this->iconLabel->setPixmap(this->activeIcon);
    }

}


void IconTextWidget::setIconMode(const IconMode iconMode) {
    this->iconMode = iconMode;
}



void IconTextWidget::setIconOnly(const QString& normalIconStr, const QString& enabledIconStr) {

    this->hBoxLayout->setSpacing(0);
    this->setIcon(normalIconStr, enabledIconStr);

    // remove event filter in order to catch mouse double click events :
    this->removeEventFilter(this->parent());
}



void IconTextWidget::mousePressEvent(QMouseEvent* event)  {

    Q_UNUSED(event);
    this->iconPressed = !this->iconPressed;
    this->enterEvent(event);
    emit activeSignal(this->iconPressed);

}



void IconTextWidget::setIcon(const QString& normalIconStr, const bool& displayOverlay) {

    if (!normalIconStr.isEmpty()) {

        this->normalIcon = this->iconLoader->loadIcon(normalIconStr, KIconLoader::Small);

        this->iconLabel->setPixmap(this->normalIcon);

        this->clearNormalIcon = UtilityIconPainting::getInstance()->buildClearIcon(this->normalIcon);

    }
    else {
        this->iconLabel->setPixmap(QPixmap());
    }

    // if certificate is not verified display warning icon over the secure connected one :
    if (displayOverlay) {
        this->iconLabel->setPixmap(UtilityIconPainting::getInstance()->blendOverLayEmblem("emblem-important", this->iconLabel->pixmap()));
    }

}


void IconTextWidget::setIcon(const QString& normalIconStr, const QString& enabledIconStr, const bool& displayOverlay) {

    this->setIcon(normalIconStr, displayOverlay);

    if (!enabledIconStr.isEmpty()) {

        this->activeIcon = this->iconLoader->loadIcon(enabledIconStr, KIconLoader::Small);
        this->clearActiveIcon = UtilityIconPainting::getInstance()->buildClearIcon(this->activeIcon);

    }

}





void IconTextWidget::setActive(const bool& active) {

    this->iconPressed = active;

    if (this->iconMode == GammaIcon) {
        this->iconLabel->setPixmap(this->clearNormalIcon);
    }

    else if (active && this->iconMode == SwitchIcon) {
        this->iconLabel->setPixmap(this->activeIcon);
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


