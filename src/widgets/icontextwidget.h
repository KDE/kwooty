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


#ifndef ICONTEXTWIDGET_H
#define ICONTEXTWIDGET_H

#include <KIconLoader>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

#include "utilityserverstatus.h"

class IconTextWidget : public QWidget {

    Q_OBJECT

public:

    enum IconMode {
        NormalModeIcon,
        GammaIcon,
        SwitchIcon
    };

    IconTextWidget(QWidget* parent = 0);

    void setIconMode(const IconMode = NormalModeIcon);
    void setIcon(const ServerConnectionIcon&);
    void setIcon(const QString&);
    void setIcon(const QString&, const QString&);
    void setIconOnly(const QString&, const QString& = QString());
    void setText(const QString&);
    QString getText() const;
    void showIcon();
    void hideIcon();
    void setActive(const bool&);

protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent*);


private:

    KIconLoader* iconLoader;
    QLabel* iconLabel;
    QLabel* textLabel;
    QHBoxLayout* hBoxLayout;
    QPixmap normalIcon;
    QPixmap clearNormalIcon;
    QPixmap clearActiveIcon;
    QPixmap activeIcon;
    IconMode iconMode;
    ServerConnectionIcon serverConnectionIcon;
    bool iconPressed;

    void buildClearIcon(const QPixmap&, QPixmap&);

signals:
    void activeSignal(bool);




};

#endif // ICONTEXTWIDGET_H
