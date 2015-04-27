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

#include "kwooty_debug.h"
#include <KIconLoader>
#include <QIcon>
#include <KSeparator>
#include <KMultiTabBarTab>

#include <QVBoxLayout>
#include <QSplitter>

#include "widgets/mystatusbar.h"
#include "mainwindow.h"
#include "utilities/utilityiconpainting.h"


SideBarWidget::SideBarWidget(QWidget* parent) : QWidget(parent) {

    multiTabBar = new KMultiTabBar(KMultiTabBar::Top, parent);
    multiTabBar->setStyle(KMultiTabBar::KDEV3ICON);

    stackedWidget = new QStackedWidget(this);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setMargin(0);
    vBoxLayout->setSpacing(0);

    vBoxLayout->addWidget(multiTabBar);
    vBoxLayout->addWidget(stackedWidget);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

}


void SideBarWidget::addTab(QWidget* currentWidget, const ServerConnectionIcon& serverConnectionIcon, const QString& label) {

    int tabIndex = stackedWidget->count();

    multiTabBar->appendTab(UtilityServerStatus::getConnectionPixmap(serverConnectionIcon), tabIndex, label);
    stackedWidget->addWidget(currentWidget);

    // display associated server info widget when tab has been clicked :
    connect(multiTabBar->tab(tabIndex), SIGNAL(clicked(int)), this, SLOT(tabClickedSlot(int)));

}



void SideBarWidget::updateIconByIndex(const int& currentIndex, const ServerConnectionIcon& serverConnectionIcon) {

    // avoid useless icon drawing :
    if (indexServerIconMap.value(currentIndex) !=  serverConnectionIcon) {

        multiTabBar->tab(currentIndex)->setIcon(UtilityServerStatus::getConnectionPixmap(serverConnectionIcon));

        indexServerIconMap.insert(currentIndex, serverConnectionIcon);

    }
}


void SideBarWidget::updateTextByIndex(const int& currentIndex, const QString& tabName) {
    multiTabBar->tab(currentIndex)->setText(tabName);
}

void SideBarWidget::updateToolTipByIndex(const int& currentIndex, const QString& tabName) {
    multiTabBar->tab(currentIndex)->setToolTip(tabName);
}


void SideBarWidget::activeDefaultTab(const int& index) {

    tabClickedSlot(index);
    stackedWidget->setVisible(true);
    multiTabBar->tab(index)->setState(true);

}

bool SideBarWidget::isOnlyTabDisplayed() const {
    return stackedWidget->isHidden();
}

void SideBarWidget::displayTabOnly() {
    return stackedWidget->hide();
}


int SideBarWidget::count() const {
    return stackedWidget->count();
}


QWidget* SideBarWidget::widget(const int& index) {
    return stackedWidget->widget(index);
}

int SideBarWidget::currentIndex() const {
    return stackedWidget->currentIndex();
}


int SideBarWidget::indexOf(QWidget* currentWidget) const {
    return stackedWidget->indexOf(currentWidget);
}


void SideBarWidget::setDisplay(bool _display) {
    // keep display state there as it seems that isVisible() returns false when kwooty is iconified.
    // This is mandatary to save and restore the correct state of this widget :
    display = _display;

    setVisible(display);
}

bool SideBarWidget::isDisplayed() const {
    return display;
}



void SideBarWidget::removeLast() {

    if (count() > 0) {
        removeTabAndWidgetByIndex(count() - 1);
    }

}



void SideBarWidget::removeTabAndWidgetByIndex(int currentIndex) {

    // detelete and remove tab :
    multiTabBar->removeTab(currentIndex);

    // delete the associated widget :
    delete stackedWidget->widget(currentIndex);

}

void SideBarWidget::removeTabByWidget(QWidget* currentWidget) {
    return stackedWidget->removeWidget(currentWidget);
}


void SideBarWidget::tabClickedSlot(const int& tabIndex) {

    QWidget* currentWidget = stackedWidget->currentWidget();
    QWidget* targetWidget = stackedWidget->widget(tabIndex);

    if (currentWidget != targetWidget) {

        // disable previous button and enable the clicked one :
        multiTabBar->setTab(indexOf(currentWidget), false);
        multiTabBar->setTab(tabIndex, true);

        // display widget corresponding to selected tab :
        if (stackedWidget->isHidden()) {
            stackedWidget->show();
        }

        stackedWidget->setCurrentIndex(tabIndex);
        stackedWidget->currentWidget()->show();

    }
    // show / hide the current widget :
    else {
        stackedWidget->setVisible(!stackedWidget->currentWidget()->isVisible());
    }


}

