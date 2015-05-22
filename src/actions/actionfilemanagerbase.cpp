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

    this->mActionsManager = actionsManager;
    this->mCore = actionsManager->getCore();
    this->mTreeView = this->mCore->getTreeView();
    this->mDownloadModel = this->mCore->getDownloadModel();
    this->mSegmentBuffer = this->mCore->getServerManager()->getSegmentBuffer();

    this->mActionFileStep = ActionFileIdle;

    this->setupConnections();

}

void ActionFileManagerBase::setupConnections()
{

    // process to renaming when segment buffer notifies that lock is enabled :
    connect(this->mSegmentBuffer,
            SIGNAL(finalizeDecoderLockedSignal()),
            this,
            SLOT(processFileSlot()));

}

void ActionFileManagerBase::processFileSlot()
{

    if (this->mActionFileStep == ActionFileRequested) {

        // stop file data decoding from thread :
        this->mSegmentBuffer->lockFinalizeDecode();

        // wait until decoder is idle :
        if (this->mSegmentBuffer->isfinalizeDecodeIdle()) {

            this->launchProcess();

            // notify decoder to process is done :
            this->mSegmentBuffer->unlockFinalizeDecode();

        }

    }

}

void ActionFileManagerBase::displayMessage(const QString &message)
{

    this->mActionFileStep = ActionFileIdle;
    this->mCore->getCentralWidget()->displaySorryMessageBox(message);

}

