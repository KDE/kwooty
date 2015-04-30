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

#ifndef CATEGORIESMANUAL_H
#define CATEGORIESMANUAL_H

#include <QObject>
#include <QMenu>
#include <QHash>
#include "categories.h"

class Core;
class StandardItemModel;
class MyTreeView;

class CategoriesManual : public QObject
{

    Q_OBJECT

public:
    explicit CategoriesManual(Categories *parent);
    bool isManualFolderSelected(const QString &);
    QString getMoveFolderPath(const QString &);
    void unload();

private:

    void setupConnections();
    void updateNzbFileNameToolTip(QStandardItem *, const QString & = QString());
    bool isActionAllowed(QStandardItem *) const;

    Core *core;
    MyTreeView *treeView;
    StandardItemModel *downloadModel;
    QHash<QString, QString> uuidFolderMap;
public Q_SLOTS:
    void addExternalActionSlot(QMenu *, QStandardItem *);
    void manualTransferFolderSlot();

};

#endif // CATEGORIESMANUAL_H
