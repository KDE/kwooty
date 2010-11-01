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


#include "textpushbuttonwidget.h"

#include <KIcon>
#include <KIconLoader>

#include "utilityiconpainting.h"
#include "serverstatuswidget.h"


TextPushButtonWidget::TextPushButtonWidget(QWidget* parent) : QWidget(parent) {


    this->textLabel = new QLabel(this);
    this->pushButton = new QPushButton(this);
    this->pushButton->setFlat(true);

    this->pushButton->setMaximumSize(KIconLoader::SizeSmallMedium , 18);
    this->pushButton->setFocusPolicy(Qt::NoFocus);

    this->hBoxLayout = new QHBoxLayout(this);
    this->hBoxLayout->addWidget(this->textLabel);
    this->hBoxLayout->addWidget(this->pushButton);

    this->hBoxLayout->setSpacing(5);
    this->hBoxLayout->setMargin(0);

    this->setupConnections();

}



void TextPushButtonWidget::setupConnections() {

    // forward pushButton pressed signal :
    connect (this->pushButton, SIGNAL(clicked(bool)), (ServerStatusWidget*)this->parent(), SLOT(buttonPressedSlot()));

}



void TextPushButtonWidget::showIcon() {
    this->pushButton->show();
}


void TextPushButtonWidget::hideIcon() {
    this->pushButton->hide();
}


void TextPushButtonWidget::setIcon(const QString& iconStr, const bool& displayOverlay) {

    if (!iconStr.isEmpty()
        ) {

        this->pushButton->setIcon(KIcon(iconStr));
        this->iconStr = iconStr;
    }

    // if certificate is not verified display warning icon over the secure connected one :
    if (displayOverlay) {

        QPixmap finalPixmap = UtilityIconPainting::getInstance()->blendOverLayEmblem("emblem-important", this->pushButton->icon());

        this->pushButton->setIcon(finalPixmap);

    }

}



void TextPushButtonWidget::setText(const QString& text) {
    this->textLabel->setText(text);
}


QString TextPushButtonWidget::getText() const {
    return this->textLabel->text();
}

