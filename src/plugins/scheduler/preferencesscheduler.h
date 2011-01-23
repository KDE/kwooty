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

#include <QWidget>
#include "ui_preferencesscheduler.h"

#include <KCModule>

#include <KPluginFactory>
#include <KPluginLoader>



class PreferencesScheduler : public KCModule
{
    Q_OBJECT

public:
    PreferencesScheduler(QWidget* = 0, const QVariantList& = QVariantList());
    ~PreferencesScheduler();

    virtual void save();
    virtual void load();

private:

    enum DownloadLimit {
        NoLimitDownload,
        LimitDownload,
        DisabledDownload
    };

    Ui_PreferencesScheduler preferencesSchedulerUi;
    QColor getRateLimitColor(PreferencesScheduler::DownloadLimit);
    void assignDownloadRateToCell(int, int);
    int mousePressedRow;
    int mousePressedColumn;


signals:

public slots:

private slots:
    void cellEnteredSlot(int, int);
    void cellPressedSlot(int, int);
    void downloadLimitValueChangedSlot(int);

};

#endif // PREFERENCESSCHEDULER_H
