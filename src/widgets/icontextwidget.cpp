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

    mIconPressed = false;
    mIconLabel = new QLabel(this);
    mTextLabel = new QLabel(this);

    mIconMode = NormalModeIcon;
    mServerConnectionIcon = InitIcon;

    hBoxLayout = new QHBoxLayout(this);
    hBoxLayout->addWidget(mIconLabel);
    hBoxLayout->addWidget(mTextLabel);

    hBoxLayout->setSpacing(5);
    hBoxLayout->setMargin(0);

}

void IconTextWidget::enterEvent(QEvent *event)
{

    Q_UNUSED(event);

    if (mIconMode == GammaIcon) {
        mIconLabel->setPixmap(mClearNormalIcon);
    }

    if (mIconMode == SwitchIcon && !mIconPressed) {
        mIconLabel->setPixmap(mClearNormalIcon);
    }

    if (mIconMode == SwitchIcon && mIconPressed) {
        mIconLabel->setPixmap(mClearActiveIcon);
    }

}

void IconTextWidget::leaveEvent(QEvent *event)
{

    Q_UNUSED(event);

    if (mIconMode == GammaIcon && !mIconPressed) {
        mIconLabel->setPixmap(mNormalIcon);
    }

    if (mIconMode == SwitchIcon && !mIconPressed) {
        mIconLabel->setPixmap(mNormalIcon);
    }

    if (mIconMode == SwitchIcon && mIconPressed) {
        mIconLabel->setPixmap(mActiveIcon);
    }

}

void IconTextWidget::setIconMode(const IconMode _iconMode)
{
    mIconMode = _iconMode;
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
    mIconPressed = !mIconPressed;
    enterEvent(event);
    emit activeSignal(mIconPressed);

}

void IconTextWidget::setIcon(const ServerConnectionIcon &_serverConnectionIcon)
{

    // avoid useless icon drawing :
    if (mServerConnectionIcon != _serverConnectionIcon) {

        mIconLabel->setPixmap(UtilityServerStatus::getConnectionPixmap(_serverConnectionIcon));
        mServerConnectionIcon = _serverConnectionIcon;

    }

}

void IconTextWidget::setIcon(const QString &normalIconStr)
{

    if (!normalIconStr.isEmpty()) {

        mNormalIcon = iconLoader->loadIcon(normalIconStr, KIconLoader::Small);
        mIconLabel->setPixmap(mNormalIcon);
        mClearNormalIcon = UtilityIconPainting::getInstance()->buildClearIcon(mNormalIcon);

    } else {
        mIconLabel->setPixmap(QPixmap());
    }

}

void IconTextWidget::setIcon(const QString &normalIconStr, const QString &enabledIconStr)
{

    setIcon(normalIconStr);

    if (!enabledIconStr.isEmpty()) {

        mActiveIcon = iconLoader->loadIcon(enabledIconStr, KIconLoader::Small);
        mClearActiveIcon = UtilityIconPainting::getInstance()->buildClearIcon(mActiveIcon);

    }

}

void IconTextWidget::setActive(const bool &active)
{

    mIconPressed = active;

    if (mIconMode == GammaIcon) {
        mIconLabel->setPixmap(mClearNormalIcon);
    }

    else if (active && mIconMode == SwitchIcon) {
        mIconLabel->setPixmap(mActiveIcon);
    }

}

void IconTextWidget::setText(const QString &text)
{

    mTextLabel->setText(text);
}

void IconTextWidget::setTextOnly(const QString &text)
{

    if (!mIconLabel->isHidden()) {
        hBoxLayout->setSpacing(0);
        mIconLabel->hide();
    }

    mTextLabel->setText(text);
}

QString IconTextWidget::getText() const
{
    return mTextLabel->text();
}

void IconTextWidget::showIcon()
{
    mIconLabel->show();
}

void IconTextWidget::hideIcon()
{
    mIconLabel->hide();
}

