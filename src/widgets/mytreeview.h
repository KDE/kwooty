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

#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <KUrl>

#include <QTreeView>
#include <QModelIndex>
#include <QStandardItem>

#include <QDragEnterEvent>
#include <QDragMoveEvent>

#include "kwooty_export.h"

#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class MainWindow;
class StandardItemModel;

class KWOOTY_EXPORT MyTreeView : public QTreeView {

    Q_OBJECT

public:
    MyTreeView(MainWindow*);
    void achieveInit();
    void setHeaderLabels();

private:

    enum MoveRowType {
        MoveRowsUp,
        MoveRowsDown,
        MoveRowsTop,
        MoveRowsBottom
    };


    MainWindow* mainWindow;

    void setupConnections();
    StandardItemModel* getDownloadModel();
    Core* getCore();


protected:
    void contextMenuEvent(QContextMenuEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);

signals:


public slots:
    void settingsChangedSlot();
    void expandedSlot(const QModelIndex&);


    // #QTBUG-5201
#if QT_VERSION == 0x040503
    void dataChangedSlot(QStandardItem*);
#endif

private slots:


};

#endif // MYTREEVIEW_H
