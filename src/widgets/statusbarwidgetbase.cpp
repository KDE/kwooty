/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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


#include "statusbarwidgetbase.h"



StatusBarWidgetBase::StatusBarWidgetBase(QWidget* parent, MyStatusBar::WidgetIdentity widgetIdentity) : QWidget(parent) {

    this->widgetIdentity = widgetIdentity;
    this->setupConnections();

}


void StatusBarWidgetBase::setupConnections() {

    // notify status bar that double mouse click on widget has been catched :
    connect (this,
             SIGNAL(statusBarWidgetDblClickSignal(MyStatusBar::WidgetIdentity)),
             this->parent(),
             SLOT(statusBarWidgetDblClickSlot(MyStatusBar::WidgetIdentity)));


}


MyStatusBar::WidgetIdentity StatusBarWidgetBase::getWidgetIdentity() {
    return this->widgetIdentity;
}


void StatusBarWidgetBase::mouseDoubleClickEvent(QMouseEvent* event) {

    emit statusBarWidgetDblClickSignal(this->widgetIdentity);
    QWidget::mouseDoubleClickEvent(event);

}

