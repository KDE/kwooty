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


#include "sidebarwidget.h"

#include <KDebug>
#include <KIconLoader>
#include <KIcon>
#include <KSeparator>
#include <KMultiTabBarTab>

#include <QVBoxLayout>
#include <QSplitter>

#include "mystatusbar.h"
#include "mainwindow.h"
#include "utilityiconpainting.h"


SideBarWidget::SideBarWidget(QWidget* parent) : QWidget(parent) {

    this->multiTabBar = new KMultiTabBar(KMultiTabBar::Top, parent);
    this->multiTabBar->setStyle(KMultiTabBar::KDEV3ICON);


    this->stackedWidget = new QStackedWidget(this);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setMargin(0);
    vBoxLayout->setSpacing(0);

    vBoxLayout->addWidget(this->multiTabBar);
    vBoxLayout->addWidget(this->stackedWidget);

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

}


void SideBarWidget::addTab(QWidget* currentWidget, const ServerConnectionIcon& serverConnectionIcon, const QString& label) {

    int tabIndex = this->stackedWidget->count();

    this->multiTabBar->appendTab(UtilityServerStatus::getConnectionPixmap(serverConnectionIcon), tabIndex, label);
    this->stackedWidget->addWidget(currentWidget);

    // display associated server info widget when tab has been clicked :
    connect(this->multiTabBar->tab(tabIndex), SIGNAL(clicked(int)), this, SLOT(tabClickedSlot(int)));

}



void SideBarWidget::updateIconByIndex(const int& currentIndex, const ServerConnectionIcon& serverConnectionIcon) {

    // avoid useless icon drawing :
    if (this->indexServerIconMap.value(currentIndex) !=  serverConnectionIcon) {

        this->multiTabBar->tab(currentIndex)->setIcon(UtilityServerStatus::getConnectionPixmap(serverConnectionIcon));

        this->indexServerIconMap.insert(currentIndex, serverConnectionIcon);

    }
}


void SideBarWidget::updateTextByIndex(const int& currentIndex, const QString& tabName) {
    this->multiTabBar->tab(currentIndex)->setText(tabName);
}

void SideBarWidget::updateToolTipByIndex(const int& currentIndex, const QString& tabName) {
    this->multiTabBar->tab(currentIndex)->setToolTip(tabName);
}


void SideBarWidget::activeDefaultTab(const int& index) {

    this->tabClickedSlot(index);
    this->stackedWidget->setVisible(true);
    this->multiTabBar->tab(index)->setState(true);

}

bool SideBarWidget::isOnlyTabDisplayed() const {
    return this->stackedWidget->isHidden();
}

void SideBarWidget::displayTabOnly() {
    return this->stackedWidget->hide();
}


int SideBarWidget::count() const {
    return this->stackedWidget->count();
}


QWidget* SideBarWidget::widget(const int& index) {
    return this->stackedWidget->widget(index);
}

int SideBarWidget::currentIndex() const {
    return this->stackedWidget->currentIndex();
}


int SideBarWidget::indexOf(QWidget* currentWidget) const {
    return this->stackedWidget->indexOf(currentWidget);
}


void SideBarWidget::removeLast() {

    if (this->count() > 0) {
        this->removeTabAndWidgetByIndex(this->count() - 1);
    }

}



void SideBarWidget::removeTabAndWidgetByIndex(int currentIndex) {

    // detelete and remove tab :
    this->multiTabBar->removeTab(currentIndex);

    // delete the associated widget :
    delete this->stackedWidget->widget(currentIndex);

}

void SideBarWidget::removeTabByWidget(QWidget* currentWidget) {
    return this->stackedWidget->removeWidget(currentWidget);
}


void SideBarWidget::tabClickedSlot(const int& tabIndex) {

    QWidget* currentWidget = this->stackedWidget->currentWidget();
    QWidget* targetWidget = this->stackedWidget->widget(tabIndex);

    if (currentWidget != targetWidget) {

        // disable previous button and enable the clicked one :
        this->multiTabBar->setTab(this->indexOf(currentWidget), false);
        this->multiTabBar->setTab(tabIndex, true);

        // display widget corresponding to selected tab :
        if (this->stackedWidget->isHidden()) {
            this->stackedWidget->show();
        }

        this->stackedWidget->setCurrentIndex(tabIndex);
        this->stackedWidget->currentWidget()->show();

    }
    // show / hide the current widget :
    else {
        this->stackedWidget->setVisible(!this->stackedWidget->currentWidget()->isVisible());
    }


}

