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


#include "serverstatuswidget.h"

#include <KDebug>
#include <KMessageBox>
#include <KLocale>

#include <QHBoxLayout>
#include <QPushButton>

#include "sidebar.h"
#include "widgets/textpushbuttonwidget.h"


ServerStatusWidget::ServerStatusWidget(QWidget* parent) : QDockWidget(parent) {


    this->setAllowedAreas(Qt::AllDockWidgetAreas);
    this->setFeatures(QDockWidget::NoDockWidgetFeatures);

    this->setTitleBarWidget(new QWidget(this));


    // build left form :
    QWidget* leftWidget = new QWidget(this);
    this->formLayoutLeft = new QFormLayout(leftWidget);
    this->formatLayout(this->formLayoutLeft);

    this->formLayoutLeft->insertRow(StatusItem, i18n("<b>Availability:</b>"), new QLabel(this));
    this->formLayoutLeft->insertRow(SpeedItem, i18n("<b>Speed:</b>"), new QLabel(this));
    this->formLayoutLeft->insertRow(VolumeItem, i18n("<b>Downloaded:</b>"), new QLabel(this));

    // allow label size reduction in case nzb file name would be too long :
    QLabel* fileLabel = new QLabel(this);
    fileLabel->setMinimumWidth(10);
    this->formLayoutLeft->insertRow(FileItem, i18n("<b>File:</b>"), fileLabel);


    // build right form :
    QWidget* rightWidget = new QWidget(this);
    this->formLayoutRight = new QFormLayout(rightWidget);
    this->formatLayout(this->formLayoutRight);

    this->formLayoutRight->insertRow(NameItem, i18n("<b>Server:</b>"), new QLabel(this));
    this->formLayoutRight->insertRow(ModeItem, i18n("<b>Mode:</b>"), new QLabel(this));
    this->formLayoutRight->insertRow(SslItem, i18n("<b>Encryption:</b>"), new TextPushButtonWidget(this));


    QWidget* mainWidget = new QWidget(this);
    QHBoxLayout* hBoxLayout = new QHBoxLayout(mainWidget);

    hBoxLayout->addWidget(leftWidget);
    hBoxLayout->addSpacerItem(new QSpacerItem(100, 1, QSizePolicy::Expanding));
    hBoxLayout->addWidget(rightWidget);

    this->setWidget(mainWidget);


}

void ServerStatusWidget::formatLayout(QFormLayout* formLayout) {

    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setSpacing(0);
    formLayout->setContentsMargins(10, 5, 10, 0);
    formLayout->setHorizontalSpacing(20);
}



void ServerStatusWidget::updateLeftLabelField(const int& itemIndex, const QString& text) {

    QLabel* currentLabel = static_cast<QLabel*>(this->formLayoutLeft->itemAt(itemIndex, QFormLayout::FieldRole)->widget());
    this->updateLabelField(currentLabel, text);

}


void ServerStatusWidget::updateRightLabelField(const int& itemIndex, const QString& text) {

    QLabel* currentLabel = static_cast<QLabel*>(this->formLayoutRight->itemAt(itemIndex, QFormLayout::FieldRole)->widget());
    this->updateLabelField(currentLabel, text);

}


void ServerStatusWidget::updateLabelField(QLabel* currentLabel, const QString& text) {

    if (currentLabel) {
        currentLabel->setText(text);
    }
}


void ServerStatusWidget::updateTextPushButtonField(const int& itemIndex, const QString& text, const bool& displayIcon, const ServerConnectionIcon& serverConnectionIcon, const QString& sslConnectionInfo) {


    TextPushButtonWidget* textPushButtonWidget = static_cast<TextPushButtonWidget*>(this->formLayoutRight->itemAt(itemIndex, QFormLayout::FieldRole)->widget());

    textPushButtonWidget->setText(text);

    if (displayIcon) {
        textPushButtonWidget->showIcon();
        textPushButtonWidget->setIcon(serverConnectionIcon);
    }
    else{
        textPushButtonWidget->hideIcon();
    }

    // store connectionInfo for KMessageBox display information :
    this->sslConnectionInfo = sslConnectionInfo;

}




void ServerStatusWidget::buttonPressedSlot() {
    KMessageBox::information(this, this->sslConnectionInfo, i18n("Encryption Information"));
}



