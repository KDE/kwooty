/***************************************************************************
 *   Copyright (C) 2009 by Xavier Lefage                                   *
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

#include "itemabstractupdater.h"

#include <KDebug>
#include "itemparentupdater.h"
#include "standarditemmodel.h"
#include "data/nzbfiledata.h"
#include "utility.h"
using namespace UtilityNamespace;


ItemAbstractUpdater::ItemAbstractUpdater(QObject* parent) : QObject (parent)
{
}

ItemAbstractUpdater::ItemAbstractUpdater()
{
}


void ItemAbstractUpdater::clear() {

    this->downloadItemNumber = 0;
    this->pauseItemNumber = 0;
    this->progressNumber = 0;
    this->downloadFinishItemNumber = 0;
    this->inQueueItemNumber = 0;
    this->pausingItemNumber = 0;
    this->decodeFinishItemNumber = 0;
    this->decodeErrorItemNumber = 0;
    this->articleNotFoundNumber = 0;
    this->articleFoundNumber = 0;
    this->verifyItemNumber = 0;
    this->verifyFinishItemNumber = 0;
    this->repairItemNumber = 0;

}



void ItemAbstractUpdater::countItemStatus(const int status) {


    switch (status) {

    case DownloadStatus: {
            this->downloadItemNumber++;
            break;
        };
    case DownloadFinishStatus: {
            this->downloadFinishItemNumber++;
            break;
        };
    case IdleStatus: {
            this->inQueueItemNumber++;
            break;
        };
    case PauseStatus: {
            this->pauseItemNumber++;
            break;
        };
    case PausingStatus: {
            this->pausingItemNumber++;
            break;
        };
    case DecodeFinishStatus: {
            this->decodeFinishItemNumber++;
            break;
        };
    case DecodeErrorStatus: {
            this->decodeErrorItemNumber++;
            break;
        };
    case DecodeStatus: {
            this->decodeItemNumber++;
            break;
        };
    case ScanStatus: {
            this->scanItemNumber++;
            break;
        };

    }

}

