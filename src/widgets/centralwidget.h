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

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>

#include <KDialog>
#include <QPointer>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class MainWindow;

class CentralWidget : public QWidget
{

    Q_OBJECT

public:
    explicit CentralWidget(MainWindow *);

    QString extractPasswordRequired(const QString &, bool &);
    int displayRestoreMessageBox();
    int displaySaveMessageBox(SaveFileBehavior);
    int displayAboutToShutdownMessageBox(const QString &);
    int displayRemoveAllFilesMessageBox();
    int displayRemoveSelectedFilesMessageBox();
    int displayMergeItemsMessageBox(const QString &, const QString &);
    bool isDialogExisting();
    void saveFileError(const int);
    void displaySorryMessageBox(const QString &);
    void closeAboutToShutdownMessageBox();
    void displayNzbHandleErrorMessageBox(const QString &);

private:
    int saveErrorButtonCode;
    QPointer<KDialog> aboutToShutdownDialog;

};

#endif // CENTRALWIDGET_H
