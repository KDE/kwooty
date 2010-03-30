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

#include <QFileInfo>

#include "centralwidget.h"
#include "settings.h"

FileOperations::FileOperations(CentralWidget* centralWidget) : QObject(centralWidget) {

    this->centralWidget = centralWidget;


}




void FileOperations::openFile() {

    bool isWrongUrl = false;

    QStringList fileNameFromDialogList = KFileDialog::getOpenFileNames(KUrl(), "*.nzb|" + i18n("nzb files"), this->centralWidget);

    // process selected file(s) :
    for (int i = 0; i < fileNameFromDialogList.size(); i++) {

        QString fileNameFromDialog = fileNameFromDialogList.at(i);

        if (!fileNameFromDialog.isNull() || !fileNameFromDialog.isEmpty()) {

            this->openUrl(KUrl(fileNameFromDialog), isWrongUrl, UtilityNamespace::OpenNormal);

        } // end of iteration loop


        // If url cannot be reached open an error message box
        if (isWrongUrl){
            KMessageBox::error(this->centralWidget , KIO::NetAccess::lastErrorString());
        }


    }

}




void FileOperations::openFileWithFileMode(KUrl nzbUrl, UtilityNamespace::OpenFileMode openFileMode) {

    bool isWrongUrl = false;

    // if file is opened by file or internet browser :
    this->openUrl(nzbUrl, isWrongUrl, openFileMode);

    // If url cannot be reached open an error message box
    if (isWrongUrl){
        KMessageBox::error(this->centralWidget , KIO::NetAccess::lastErrorString());
    }

}



void FileOperations::openUrl(KUrl url, bool& isWrongUrl, UtilityNamespace::OpenFileMode openFileMode) {

    QString downloadFile;

    if(KIO::NetAccess::download(url, downloadFile, this->centralWidget)){

        QFile file(downloadFile);

        // Open the nzb file :
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            KMessageBox::error(this->centralWidget , KIO::NetAccess::lastErrorString());
        }

        // add nzbFile data to the view :
        this->centralWidget->handleNzbFile(file);

        file.close();

        // copy nzb file in its associated download folder if file has been open has been triggered by another app  :
        if (Settings::openWith() &&
            (openFileMode == UtilityNamespace::OpenWith)) {

            //remove .nzb extension to file name :
            QFileInfo fileInfo(file.fileName());
            QString nzbBaseName = fileInfo.completeBaseName();

            // create download folder :
            QString downloadFolderPath = Settings::completedFolder().path() + '/' + nzbBaseName;
            Utility::createFolder(downloadFolderPath);

            // copy nzb file in created download folder :
            file.copy(downloadFolderPath + '/' + url.fileName());
            QFile::setPermissions(downloadFolderPath + '/' + url.fileName(), QFile::ReadOwner | QFile::WriteOwner);

        }

        // remove temporary downloaded file :
        KIO::NetAccess::removeTempFile(downloadFile);

    }
    // the url can not be opened
    else {
        isWrongUrl = true;
    }

}
