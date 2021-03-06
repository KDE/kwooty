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

#ifndef PREFERENCESSCHEDULER_H
#define PREFERENCESSCHEDULER_H

#include <KCModule>
#include <KPluginFactory>
#include <KPluginLoader>

#include <QWidget>
#include <QHash>
#include <QStandardItemModel>

#include "ui_preferencesscheduler.h"
#include "schedulerfilehandler.h"
using namespace SchedulerNamespace;


class PreferencesScheduler : public KCModule {

    Q_OBJECT
    Q_ENUMS(DownloadLimitStatus)

public:

    PreferencesScheduler(QWidget* = 0, const QVariantList& = QVariantList());
    ~PreferencesScheduler();

    virtual void save();
    virtual void load();

private:  

    QStandardItemModel* schedulerModel;
    Ui_PreferencesScheduler preferencesSchedulerUi;
    int mousePressedRow;
    int mousePressedColumn;
    bool schedulerPressed;
    bool permanentSpeedLimitPressed;

    void assignDownloadRateToCell(int, int);
    void setupConnections();


signals:

public slots:

private slots:
    void cellEnteredSlot(const QModelIndex&);
    void cellPressedSlot(const QModelIndex&);
    void downloadLimitValueChangedSlot(int);
    void schedulerToggledSlot(bool);
    void schedulerPressedSlot();
    void permanentSpeedLimitPressedSlot();
    void radioButtonReleasedSlot();
    void checkBoxStateChangedSlot(int);

};

#endif // PREFERENCESSCHEDULER_H
