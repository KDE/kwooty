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


#include "fileoperations.h"

#include <KFileDialog>
#include <KMessageBox>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <QFileInfo>
#include <QRegExp>

#include "core.h"
#include "standarditemmodelquery.h"
#include "widgets/centralwidget.h"
#include "kwootysettings.h"

FileOperations::FileOperations(Core* core) : QObject(core) {

    this->core = core;

}


void FileOperations::openFile() {

    bool isWrongUrl = false;

    QStringList fileNameFromDialogList = KFileDialog::getOpenFileNames(KUrl(), i18n("*.nzb|nzb files"), this->core->getCentralWidget());

    // process selected file(s) :
    foreach (const QString& fileNameFromDialog, fileNameFromDialogList) {

        if (!fileNameFromDialog.isNull() || !fileNameFromDialog.isEmpty()) {

            this->openUrl(KUrl(fileNameFromDialog), isWrongUrl, UtilityNamespace::OpenNormal);

        }


        // If url cannot be reached open an error message box
        if (isWrongUrl){
            KMessageBox::error(this->core->getCentralWidget() , KIO::NetAccess::lastErrorString());
        }


    }

}


void FileOperations::openFileWithFileMode(const KUrl &nzbUrl, UtilityNamespace::OpenFileMode openFileMode) {

    bool isWrongUrl = false;

    // if file is opened by file or internet browser :
    this->openUrl(nzbUrl, isWrongUrl, openFileMode);

    // If url cannot be reached open an error message box
    if (isWrongUrl){
        KMessageBox::error(this->core->getCentralWidget(), KIO::NetAccess::lastErrorString());
    }

}



void FileOperations::openUrl(const KUrl &url, bool& isWrongUrl, UtilityNamespace::OpenFileMode openFileMode) {

    QString downloadFile;

    if(KIO::NetAccess::download(url, downloadFile, this->core->getCentralWidget())){

        QFile file(downloadFile);

        // Open the nzb file :
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            KMessageBox::error(this->core->getCentralWidget(), KIO::NetAccess::lastErrorString());
        }

        // if the current nzb file has the same name as a previous nzb file :
        QString nzbRealBaseName = QFileInfo(file.fileName()).completeBaseName();
        QString nzbBaseName = nzbRealBaseName;

        int counter = 1;
        while ( counter < 100 &&
                this->core->getModelQuery()->isParentFileNameExists(nzbBaseName) ) {

            // add a suffix to distinguish files between each others :
            nzbBaseName = nzbRealBaseName + "-" + QString::number(counter++);

        }

        // add nzbFile data to the view :
        this->core->handleNzbFile(file, nzbBaseName);

        file.close();

        // copy nzb file in its associated download folder if file has been open has been triggered by another app  :
        if (Settings::openWith() &&
            (openFileMode == UtilityNamespace::OpenWith)) {

            // create download folder :
            QString downloadFolderPath = Utility::buildFullPath(Settings::completedFolder().path(), nzbBaseName);
            Utility::createFolder(downloadFolderPath);

            // copy nzb file in created download folder :
            file.copy( Utility::buildFullPath(downloadFolderPath, url.fileName()) );
            QFile::setPermissions( Utility::buildFullPath(downloadFolderPath, url.fileName() ), QFile::ReadOwner | QFile::WriteOwner);

        }

        // remove temporary downloaded file :
        KIO::NetAccess::removeTempFile(downloadFile);

    }
    // the url can not be opened
    else {
        isWrongUrl = true;
    }

}


bool FileOperations::isSplitFileFormat(const QFile& decodedFile) {

    bool splitFile = false;

    QRegExp regExp("\\d+");
    regExp.setCaseSensitivity(Qt::CaseInsensitive);

    if (regExp.exactMatch(QFileInfo(decodedFile).suffix())) {
        splitFile = true;
    }

    return splitFile;

}

