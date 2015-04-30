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

#include "actionfilemanagerbase.h"

ActionFileManagerBase::ActionFileManagerBase(ActionsManager *actionsManager) : QObject(actionsManager)
{

    this->actionsManager = actionsManager;
    this->core = actionsManager->getCore();
    this->treeView = this->core->getTreeView();
    this->downloadModel = this->core->getDownloadModel();
    this->segmentBuffer = this->core->getServerManager()->getSegmentBuffer();

    this->actionFileStep = ActionFileIdle;

    this->setupConnections();

}

void ActionFileManagerBase::setupConnections()
{

    // process to renaming when segment buffer notifies that lock is enabled :
    connect(this->segmentBuffer,
            SIGNAL(finalizeDecoderLockedSignal()),
            this,
            SLOT(processFileSlot()));

}

void ActionFileManagerBase::processFileSlot()
{

    if (this->actionFileStep == ActionFileRequested) {

        // stop file data decoding from thread :
        this->segmentBuffer->lockFinalizeDecode();

        // wait until decoder is idle :
        if (this->segmentBuffer->isfinalizeDecodeIdle()) {

            this->launchProcess();

            // notify decoder to process is done :
            this->segmentBuffer->unlockFinalizeDecode();

        }

    }

}

void ActionFileManagerBase::displayMessage(const QString &message)
{

    this->actionFileStep = ActionFileIdle;
    this->core->getCentralWidget()->displaySorryMessageBox(message);

}

