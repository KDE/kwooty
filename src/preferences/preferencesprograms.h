/***************************************************************************
 *   Copyright (C) 2009 by Xavier Lefage                                   *
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

#ifndef PREFERENCESPROGRAMS_H
#define PREFERENCESPROGRAMS_H

#include <QWidget>
#include <KIconLoader>
#include "ui_preferencesprograms.h"


class PreferencesPrograms : public QWidget, public Ui::PreferencesPrograms
{

    Q_OBJECT


public:
    PreferencesPrograms();
    ~PreferencesPrograms();

private:
    KIconLoader* iconLoader;

    void displayProgramInfo(const bool, const QString&, QLabel*, QLabel*, const QString&);
    void enableGroupBox(bool, const QString&);

 public slots:
    void aboutToShowSettingsSlot();

private slots:

};

#endif // PREFERENCESPROGRAMS_H
