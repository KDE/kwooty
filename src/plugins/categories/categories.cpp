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


#include "categories.h"

#include <KDebug>
#include <klocale.h>


#include <QDir>

#include "mainwindow.h"
#include "core.h"
#include "categoriesfilehandler.h"
#include "categoriesplugin.h"
#include "standarditemmodel.h"
#include "data/itemstatusdata.h"
#include "data/nzbfiledata.h"

#include "kwootysettings.h"
#include "kwooty_categoriessettings.h"



Categories::Categories(CategoriesPlugin* parent) :  QObject(parent) {

    this->core = parent->getMainWindow()->getCore();

    // get model :
    this->categoriesModel = CategoriesFileHandler().loadModelFromFile(this);

    // init categories behavior :
    this->settingsChanged();

    // setup signals/slots connections :
    this->setupConnections();

}


Categories::~Categories() {

}


void Categories::setupConnections() {

    connect(this->core->getDownloadModel(),
            SIGNAL(parentStatusItemChangedSignal(QStandardItem*, ItemStatusData)),
            this,
            SLOT(parentStatusItemChangedSlot(QStandardItem*)));

}



QStringList Categories::retrieveMimeGroups() {

    QStringList mimeGroups;

    foreach (KSharedPtr<KMimeType>  mimeType,  KMimeType::allMimeTypes()) {

        QStringList tempList = mimeType->name().split("/");

        if ( (tempList.size() > 1 )&&
             !mimeGroups.contains(tempList.at(0)) ) {

            mimeGroups.append(tempList.at(0));
        }

    }

    kDebug() << mimeGroups;
    return mimeGroups;

}




//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void Categories::parentStatusItemChangedSlot(QStandardItem* stateItem) {

    StandardItemModel* downloadModel = this->core->getDownloadModel();
    ItemStatusData itemStatusData = downloadModel->getStatusDataFromIndex(stateItem->index());

    if ( itemStatusData.isPostProcessFinish() &&
         itemStatusData.areAllPostProcessingCorrect() ) {

        kDebug() << "post processing correct";

        // avoid double parentStatusItemChangedSlot call by checking
        // if status has just not be set to be ready for downloading again :
        NzbFileData nzbFileData = downloadModel->getNzbFileDataFromIndex(downloadModel->getNzbItem(stateItem)->child(0)->index());
        kDebug() << nzbFileData.getFileSavePath() << nzbFileData.getNzbName() << nzbFileData.getFileName();

        QString nzbFileSavepath = nzbFileData.getFileSavePath();

        QDir nzbDir(nzbFileSavepath);
        QStringList nzbDirFileList = nzbDir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Size);

        foreach (QString currentFileStr, nzbDirFileList) {

            QFile currentFile(currentFileStr);

            currentFile.open(QIODevice::ReadOnly);

            kDebug() << "File : " << currentFileStr;

            int accuracy;
            KSharedPtr< KMimeType> mimeType = KMimeType::findByUrl(KUrl(nzbFileSavepath + '/' + currentFileStr), 0, true, false, &accuracy);

            //KSharedPtr<KMimeType> mimeType = KMimeType::findByNameAndContent(currentFileStr, &currentFile);
            kDebug() << "findByUrl :" << mimeType->name() << mimeType->sharedMimeInfoVersion() << mimeType->patterns() << mimeType->entryPath() << mimeType->comment(KUrl(currentFileStr)) << accuracy;

            if (this->isDefaultMimeType(mimeType)) {
                mimeType = KMimeType::findByContent(&currentFile);
            }

            if (this->isDefaultMimeType(mimeType)) {
                kDebug() << "=> No mime type found for :"  << currentFileStr << " : " << mimeType->name();
            }
            else {
                kDebug() << "=> Mime type for :"  << currentFileStr << "is :" << mimeType->name();
            }


            //KSharedPtr<KMimeType> ptr = KMimeType::findByNameAndContent(currentFileStr, &currentFile);
            //kDebug() << "findByNameAndContent :" << ptr->name() << ptr->sharedMimeInfoVersion() << ptr->patterns() << ptr->entryPath() << ptr->comment(KUrl(currentFileStr));

            currentFile.close();

        }

        QString destinationFolder = this->core->getCompletedFolder() + "/Finished";

        kDebug() << "destination folder :" << destinationFolder;

        Utility::createFolder(destinationFolder);

        KIO::CopyJob* moveJob = KIO::move(KUrl(nzbFileSavepath), KUrl(destinationFolder), KIO::Overwrite);

        connect( moveJob, SIGNAL(result(KJob*)), this, SLOT(handleResultSlot(KJob*)) );
        connect( moveJob, SIGNAL(moving(KIO::Job*, const KUrl& , const KUrl&)), this, SLOT(jobProgressionSlot(KIO::Job*)) );

        connect( moveJob, SIGNAL(result(KJob*)), this, SLOT(jobFinishedSlot(KJob*)) );

        moveJob->start();

    }

}



void Categories::handleResultSlot(KJob* moveJob) {

    if (moveJob->error()) {

        kDebug() << "MOVE ERROR : " << moveJob->errorText();
    }

}

void Categories::jobProgressionSlot(KIO::Job* moveJob) {
        kDebug() << "progression : " << moveJob->percent();
}

void Categories::jobFinishedSlot(KJob* moveJob) {
    kDebug() << "Job finished !";


}



bool Categories::isDefaultMimeType(KSharedPtr<KMimeType> mimeType) {

    return mimeType->is(KMimeType::defaultMimeType());

}




void Categories::settingsChanged() {

    // reload settings from just saved config file :
    CategoriesSettings::self()->readConfig();

}


