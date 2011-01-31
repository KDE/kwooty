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


#include <QHash>
#include <KCModule>

#include <KPluginFactory>
#include <KPluginLoader>




class PreferencesScheduler : public KCModule
{
    Q_OBJECT
    Q_ENUMS(DownloadLimitStatus)

public:

    enum DownloadLimitStatus {
        NoLimitDownload,
        LimitDownload,
        DisabledDownload
    };


    // custom roles used for storing data in items :
    enum SchedulerRoles {
        DownloadLimitRole  = Qt::UserRole + 1
                         };


    PreferencesScheduler(QWidget* = 0, const QVariantList& = QVariantList());
    ~PreferencesScheduler();

    virtual void save();
    virtual void load();

private:

    static const int HEADER_ROW = 0;
    static const int ROW_NUMBER = 8;
    static const int COLUMN_NUMBER = 48;

    Ui_PreferencesScheduler preferencesSchedulerUi;
    QColor getRateLimitColor(PreferencesScheduler::DownloadLimitStatus);
    QHash<DownloadLimitStatus, QColor> statusColorMap;
    int mousePressedRow;
    int mousePressedColumn;


    void assignDownloadRateToCell(int, int);
    void initItems();
    void setupConnections();



signals:

public slots:

private slots:
    void cellEnteredSlot(int, int);
    void cellPressedSlot(int, int);
    void downloadLimitValueChangedSlot(int);
    void schedulerToggledSlot(bool);

};

#endif // PREFERENCESSCHEDULER_H
