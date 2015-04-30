/***************************************************************************
 *   Copyright (C) 2013 by Xavier Lefage                                   *
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

#ifndef ACTIONRENAMEMANAGER_H
#define ACTIONRENAMEMANAGER_H

#include "actionfilemanagerbase.h"

class ActionRenameManager : public ActionFileManagerBase
{

    Q_OBJECT

public:
    explicit ActionRenameManager(ActionsManager *);
    void checkRenameCandidates(bool &);

private:

    QString input;
    QString selectedItemUuid;

    bool validateNewFolderName(QStandardItem *) const;
    bool isRenameAllowed(QStandardItem *) const;
    void processRename(QStandardItem *);
    void launchProcess() Q_DECL_OVERRIDE;
public Q_SLOTS:
    void actionTriggeredSlot() Q_DECL_OVERRIDE;
    void handleResultSlot(KJob *) Q_DECL_OVERRIDE;

};

#endif // ACTIONRENAMEMANAGER_H
