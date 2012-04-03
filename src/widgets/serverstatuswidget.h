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


#ifndef SERVERSTATUSWIDGET_H
#define SERVERSTATUSWIDGET_H

#include <QDockWidget>
#include <QFormLayout>
#include <QLabel>

#include "utilities/utilityserverstatus.h"

class ServerStatusWidget : public QDockWidget {

    Q_OBJECT

public:

    enum RowItemsLeft {
        StatusItem,
        VolumeItem,
        SpeedItem,
        FileItem
    };

    enum RowItemsRight {
        NameItem,
        ModeItem,
        SslItem
    };

    enum RowPosition {
        RowLeft,
        RowRight
    };

    ServerStatusWidget(QWidget*);

    void updateLeftLabelField(const int&, const QString&);
    void updateRightLabelField(const int&, const QString&);
    void updateTextPushButtonField(const int&, const QString&,  const bool&, const ServerConnectionIcon&, const QString&);


private:
    QFormLayout* formLayoutLeft;
    QFormLayout* formLayoutRight;
    QString sslConnectionInfo;

    void updateLabelField(QLabel*, const QString&);
    void formatLayout(QFormLayout*);
    void insertLeftRowFormLayout(ServerStatusWidget::RowItemsLeft, QFormLayout*, const QString&, QWidget*);
    void insertRightRowFormLayout(ServerStatusWidget::RowItemsRight, QFormLayout*, const QString&, QWidget*);


public slots:
    void buttonPressedSlot();


};

#endif // SERVERSTATUSWIDGET_H
