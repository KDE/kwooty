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

#include "kwooty_debug.h"
#include <KMessageBox>
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QPushButton>

#include "sidebar.h"
#include "widgets/textpushbuttonwidget.h"
#include "utilities/utilityiconpainting.h"

ServerStatusWidget::ServerStatusWidget(QWidget *parent) : QDockWidget(parent)
{

    this->setAllowedAreas(Qt::AllDockWidgetAreas);
    this->setFeatures(QDockWidget::NoDockWidgetFeatures);

    this->setTitleBarWidget(new QWidget(this));

    // build left form :
    QWidget *leftWidget = new QWidget(this);
    this->formLayoutLeft = new QFormLayout(leftWidget);
    this->formatLayout(this->formLayoutLeft);

    this->insertLeftRowFormLayout(StatusItem, this->formLayoutLeft, i18n("Availability:"), new QLabel(this));
    this->insertLeftRowFormLayout(SpeedItem, this->formLayoutLeft, i18n("Speed:"), new QLabel(this));
    this->insertLeftRowFormLayout(VolumeItem, this->formLayoutLeft, i18n("Downloaded:"), new QLabel(this));

    // allow label size reduction in case nzb file name would be too long :
    QLabel *fileLabel = new QLabel(this);
    fileLabel->setMinimumWidth(10);
    this->insertLeftRowFormLayout(FileItem, this->formLayoutLeft, i18n("File:"), fileLabel);

    // build right form :
    QWidget *rightWidget = new QWidget(this);
    this->formLayoutRight = new QFormLayout(rightWidget);
    this->formatLayout(this->formLayoutRight);

    this->insertRightRowFormLayout(NameItem, this->formLayoutRight, i18n("Server:"), new QLabel(this));
    this->insertRightRowFormLayout(ModeItem, this->formLayoutRight, i18n("Mode:"), new QLabel(this));
    this->insertRightRowFormLayout(SslItem, this->formLayoutRight, i18n("Encryption:"), new TextPushButtonWidget(this));

    QWidget *mainWidget = new QWidget(this);
    QHBoxLayout *hBoxLayout = new QHBoxLayout(mainWidget);

    hBoxLayout->addWidget(leftWidget);
    hBoxLayout->addSpacerItem(new QSpacerItem(100, 1, QSizePolicy::Expanding));
    hBoxLayout->addWidget(rightWidget);

    this->setWidget(mainWidget);

}

void ServerStatusWidget::insertLeftRowFormLayout(ServerStatusWidget::RowItemsLeft rowItemsLeft, QFormLayout *formLayout, const QString &text, QWidget *widget)
{

    formLayout->insertRow(rowItemsLeft, UtilityIconPainting::getInstance()->buildLighterTextLabel(text, this), widget);

}

void ServerStatusWidget::insertRightRowFormLayout(ServerStatusWidget::RowItemsRight rowItemsRight, QFormLayout *formLayout, const QString &text, QWidget *widget)
{

    formLayout->insertRow(rowItemsRight, UtilityIconPainting::getInstance()->buildLighterTextLabel(text, this), widget);

}

void ServerStatusWidget::formatLayout(QFormLayout *formLayout)
{

    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setSpacing(0);
    formLayout->setContentsMargins(10, 5, 10, 0);
    formLayout->setHorizontalSpacing(20);
}

void ServerStatusWidget::updateLeftLabelField(const int &itemIndex, const QString &text, const QString &toolTip)
{

    QLabel *currentLabel = static_cast<QLabel *>(this->formLayoutLeft->itemAt(itemIndex, QFormLayout::FieldRole)->widget());
    this->updateLabelField(currentLabel, text, toolTip);

}

void ServerStatusWidget::updateRightLabelField(const int &itemIndex, const QString &text, const QString &toolTip)
{

    QLabel *currentLabel = static_cast<QLabel *>(this->formLayoutRight->itemAt(itemIndex, QFormLayout::FieldRole)->widget());
    this->updateLabelField(currentLabel, text, toolTip);

}

void ServerStatusWidget::updateLabelField(QLabel *currentLabel, const QString &text, const QString &toolTip)
{

    if (currentLabel) {
        currentLabel->setText(text);

        if (!toolTip.isEmpty()) {
            currentLabel->setToolTip(toolTip);
        }

    }
}

void ServerStatusWidget::updateTextPushButtonField(const int &itemIndex, const QString &text, const bool &displayIcon, const ServerConnectionIcon &serverConnectionIcon, const QString &sslConnectionInfo)
{

    TextPushButtonWidget *textPushButtonWidget = static_cast<TextPushButtonWidget *>(this->formLayoutRight->itemAt(itemIndex, QFormLayout::FieldRole)->widget());

    textPushButtonWidget->setText(text);

    if (displayIcon) {
        textPushButtonWidget->showIcon();
        textPushButtonWidget->setIcon(serverConnectionIcon);
    } else {
        textPushButtonWidget->hideIcon();
    }

    // store connectionInfo for KMessageBox display information :
    this->sslConnectionInfo = sslConnectionInfo;

}

void ServerStatusWidget::buttonPressedSlot()
{
    KMessageBox::information(this, this->sslConnectionInfo, i18n("Encryption Information"));
}

