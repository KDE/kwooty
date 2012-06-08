/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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


#ifndef CATEGORIES_H
#define CATEGORIES_H

#include <QObject>
#include <QStandardItem>
#include <QStandardItemModel>

#include <kio/copyjob.h>
#include <kmimetype.h>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class CategoriesPlugin;

class Categories : public QObject {

    Q_OBJECT

public:
    Categories(CategoriesPlugin*);
    ~Categories();
    void settingsChanged();


private:

    Core* core;
    QStandardItemModel* categoriesModel;

    void setupConnections();
    bool isDefaultMimeType(KSharedPtr<KMimeType>);
    QStringList retrieveMimeGroups();



signals:


public slots:
    void handleResultSlot(KJob*);
    void jobProgressionSlot(KIO::Job*);
    void jobFinishedSlot(KJob*);

private slots:
    void parentStatusItemChangedSlot(QStandardItem*);





};

#endif // CATEGORIES_H
