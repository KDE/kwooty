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


#ifndef ICONCAPACITYWIDGET_H
#define ICONCAPACITYWIDGET_H

#include <KIconLoader>
#include <kcapacitybar.h>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class IconCapacityWidget : public QWidget {


Q_OBJECT

public:
    IconCapacityWidget(QWidget *parent = 0);
    void showIcon();
    void hideIcon();
    void setIcon(const QString&);
    void updateCapacity(const QString&, const int&);

private:

    KIconLoader* iconLoader;
    KCapacityBar* capacityBar;
    QLabel* iconLabel;
    QHBoxLayout* hBoxLayout;

signals:


public slots:


};

#endif // ICONCAPACITYWIDGET_H