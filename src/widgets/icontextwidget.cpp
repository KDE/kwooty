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
#include <QIcon>
#include <KIconEffect>
#include "kwooty_debug.h"
#include <QMouseEvent>

#include <QPixmap>
#include <QPainter>

#include "utilities/utilityiconpainting.h"
#include "statusbarwidgetbase.h"

IconTextWidget::IconTextWidget(QWidget *parent, MyStatusBar::WidgetIdentity widgetIdentity) : StatusBarWidgetBase(parent, widgetIdentity)
{

    iconLoader = KIconLoader::global();

    iconPressed = false;
    iconLabel = new QLabel(this);
    textLabel = new QLabel(this);

    iconMode = NormalModeIcon;
    serverConnectionIcon = InitIcon;

    hBoxLayout = new QHBoxLayout(this);
    hBoxLayout->addWidget(iconLabel);
    hBoxLayout->addWidget(textLabel);

    hBoxLayout->setSpacing(5);
    hBoxLayout->setMargin(0);

}

void IconTextWidget::enterEvent(QEvent *event)
{

    Q_UNUSED(event);

    if (iconMode == GammaIcon) {
        iconLabel->setPixmap(clearNormalIcon);
    }

    if (iconMode == SwitchIcon && !iconPressed) {
        iconLabel->setPixmap(clearNormalIcon);
    }

    if (iconMode == SwitchIcon && iconPressed) {
        iconLabel->setPixmap(clearActiveIcon);
    }

}

void IconTextWidget::leaveEvent(QEvent *event)
{

    Q_UNUSED(event);

    if (iconMode == GammaIcon && !iconPressed) {
        iconLabel->setPixmap(normalIcon);
    }

    if (iconMode == SwitchIcon && !iconPressed) {
        iconLabel->setPixmap(normalIcon);
    }

    if (iconMode == SwitchIcon && iconPressed) {
        iconLabel->setPixmap(activeIcon);
    }

}

void IconTextWidget::setIconMode(const IconMode _iconMode)
{
    iconMode = _iconMode;
}

void IconTextWidget::setIconOnly(const QString &normalIconStr, const QString &enabledIconStr)
{

    hBoxLayout->setSpacing(0);
    setIcon(normalIconStr, enabledIconStr);

    // disconnect in order to not forward mouse double click events :
    disconnect(SIGNAL(statusBarWidgetDblClickSignal(MyStatusBar::WidgetIdentity)));

}

void IconTextWidget::mousePressEvent(QMouseEvent *event)
{

    Q_UNUSED(event);
    iconPressed = !iconPressed;
    enterEvent(event);
    emit activeSignal(iconPressed);

}

void IconTextWidget::setIcon(const ServerConnectionIcon &_serverConnectionIcon)
{

    // avoid useless icon drawing :
    if (serverConnectionIcon != _serverConnectionIcon) {

        iconLabel->setPixmap(UtilityServerStatus::getConnectionPixmap(_serverConnectionIcon));
        serverConnectionIcon = _serverConnectionIcon;

    }

}

void IconTextWidget::setIcon(const QString &normalIconStr)
{

    if (!normalIconStr.isEmpty()) {

        normalIcon = iconLoader->loadIcon(normalIconStr, KIconLoader::Small);
        iconLabel->setPixmap(normalIcon);
        clearNormalIcon = UtilityIconPainting::getInstance()->buildClearIcon(normalIcon);

    } else {
        iconLabel->setPixmap(QPixmap());
    }

}

void IconTextWidget::setIcon(const QString &normalIconStr, const QString &enabledIconStr)
{

    setIcon(normalIconStr);

    if (!enabledIconStr.isEmpty()) {

        activeIcon = iconLoader->loadIcon(enabledIconStr, KIconLoader::Small);
        clearActiveIcon = UtilityIconPainting::getInstance()->buildClearIcon(activeIcon);

    }

}

void IconTextWidget::setActive(const bool &active)
{

    iconPressed = active;

    if (iconMode == GammaIcon) {
        iconLabel->setPixmap(clearNormalIcon);
    }

    else if (active && iconMode == SwitchIcon) {
        iconLabel->setPixmap(activeIcon);
    }

}

void IconTextWidget::setText(const QString &text)
{

    textLabel->setText(text);
}

void IconTextWidget::setTextOnly(const QString &text)
{

    if (!iconLabel->isHidden()) {
        hBoxLayout->setSpacing(0);
        iconLabel->hide();
    }

    textLabel->setText(text);
}

QString IconTextWidget::getText() const
{
    return textLabel->text();
}

void IconTextWidget::showIcon()
{
    iconLabel->show();
}

void IconTextWidget::hideIcon()
{
    iconLabel->hide();
}

