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
#include <KDebug>

IconTextWidget::IconTextWidget(QWidget* parent) : QWidget(parent) {

    iconLoader = KIconLoader::global();

    hBoxLayout = new QHBoxLayout(this);

    iconLabel = new QLabel(this);
    textLabel = new QLabel(this);

    hBoxLayout->addWidget(iconLabel);
    hBoxLayout->addWidget(textLabel);
    hBoxLayout->setSpacing(5);
    hBoxLayout->setMargin(0);
}



void IconTextWidget::setIcon(const QString& iconStr) {

    if (!iconStr.isEmpty()) {

        this->iconLabel->setPixmap(iconLoader->loadIcon(iconStr, KIconLoader::Small));
    }
    else {
           this->iconLabel->setPixmap(QPixmap());
    }

}


void IconTextWidget::setText(const QString& text) {

   // kDebug() << text;
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


