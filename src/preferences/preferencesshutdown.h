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

#ifndef PREFERENCESSHUTDOWN_H
#define PREFERENCESSHUTDOWN_H

#include <QWidget>
#include "ui_preferencesshutdown.h"

class Core;

class PreferencesShutdown : public QWidget, public Ui::PreferencesShutdown
{

    Q_OBJECT
public:
    PreferencesShutdown(Core*);

private:
    void setupConnections();


public slots:
    void currentIndexChangedSlot();
    void radioButtonToggledSlot();
};

#endif // PREFERENCESSHUTDOWN_H
