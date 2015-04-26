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

#include "preferencesgeneral.h"
#include "kwootysettings.h"

PreferencesGeneral::PreferencesGeneral()
{
    setupUi(this);

    //set mode to folder mode :
    kcfg_completedFolder->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
    kcfg_temporaryFolder->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    // setup connections :
    this->setupConnections();


    // init combobox save/restore downloads methods list :
    QStringList downloadsSessionList;
    downloadsSessionList.append(i18n("With confirmation"));
    downloadsSessionList.append(i18n("Automatically"));

    kcfg_saveDownloadsMethods->addItems(downloadsSessionList);
    kcfg_restoreDownloadsMethods->addItems(downloadsSessionList);

    // enable or disable checkbox in confirmation dialog tab :
    this->stateChangedSlot();
}



void PreferencesGeneral::setupConnections() {

    // check/uncheck ssl checkbox according to port value:
    connect (kcfg_restoreDownloads, SIGNAL(stateChanged(int)), this, SLOT(stateChangedSlot()));

}



void PreferencesGeneral::stateChangedSlot() {

    // if "save pending downloads" is checked, enable confirmSaveSilently and confirmRestoreSilently checkboxes :
    if (kcfg_restoreDownloads->checkState() == Qt::Checked) {
        this->enableSaveRestoreItems(true);
    }
    // disable them :
    else {
       this->enableSaveRestoreItems(false);
    }


}


void PreferencesGeneral::enableSaveRestoreItems(const bool& enable) {

    restoreDownloadsLabel->setEnabled(enable);
    saveDownloadsLabel->setEnabled(enable);
    kcfg_saveDownloadsMethods->setEnabled(enable);
    kcfg_restoreDownloadsMethods->setEnabled(enable);

}


