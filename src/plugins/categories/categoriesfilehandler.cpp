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


#include "categoriesfilehandler.h"

#include <KStandardDirs>
#include <KDebug>

#include <QXmlStreamWriter>

#include "utilities/utility.h"
using namespace UtilityNamespace;
using namespace CategoriesNamespace;


CategoriesFileHandler::CategoriesFileHandler(QObject* parent) : QObject(parent) {

}

CategoriesFileHandler::CategoriesFileHandler() {

}



QStandardItemModel* CategoriesFileHandler::loadModelFromFile(QObject* parent) {

//    // build categories model :
//    QStandardItemModel* categoriesModel = new QStandardItemModel(parent);
//    schedulerModel->setColumnCount(COLUMN_NUMBER_CATEGORIES);
//    //schedulerModel->setRowCount(ROW_NUMBER_SCHEDULER);

//    QFile categoriesFile(this->retrieveCategoriesFilePath());
//    categoriesFile.open(QIODevice::ReadOnly);
//    QXmlStreamReader stream (&categoriesFile);

//    int dayNumber = 1;
//    int halfHourNumber = 0;
//    int downloadLimitStatus = 0;

//    while (!stream.atEnd()) {

//        // define the data that holds data for a single file to be downloaded :
//        QXmlStreamReader::TokenType tokenType = stream.readNext();

//        if (tokenType == QXmlStreamReader::StartElement) {

//            QXmlStreamAttributes attributes = stream.attributes();

//            // get categories file version :
//            if (stream.name().toString() == "categories") {

//                if (attributes.value("categories").toString() != "1") {

//                    kDebug() << "this categories.xml version is not compatible with current kwooty version";
//                    break;
//                }
//            }

//            QString groupName;
//            QString groupMoveFolder;
//            QString mimeTypeName;
//            QString mimeMoveFolder;

//            // get mime group :
//            if (stream.name().toString() == "group") {

//                // retrieve the name of the group :
//                groupName = attributes.value("groupName").toString();
//                groupMoveFolder = attributes.value("groupMoveFolder").toString();

//            }


//            // get mime type from group :
//            if (stream.name().toString() == "mimeType") {

//                mimeTypeName = attributes.value("mimeTypeName").toString();
//                mimeMoveFolder = attributes.value("mimeMoveFolder").toString();

//                // set data to model :
//                QStandardItem* item = schedulerModel->itemFromIndex(schedulerModel->index(dayNumber, halfHourNumber));
//                item->setData(downloadLimitStatus, DownloadLimitRole);



//            }


//            if (!groupName.isEmpty()) {
//                QStandardItem* groupItem = new QStandardItem(groupName);
//                categoriesModel->appendRow(groupItem);
//            }

//        }

//    }


//    // parsing is done, close the file :
//    schedulerFile.close();


//    // check that all items are correctly set :
//    for (int i = HEADER_ROW_SCHEDULER + 1; i < ROW_NUMBER_SCHEDULER; i++) {
//        for (int j = 0; j < COLUMN_NUMBER_SCHEDULER; j++) {

//            QStandardItem* item = schedulerModel->itemFromIndex(schedulerModel->index(i, j));

//            bool conversionOk;
//            item->data(DownloadLimitRole).toInt(&conversionOk);

//            if (!conversionOk) {
//                item->setData(static_cast<int>(NoLimitDownload), DownloadLimitRole);
//            }
//        }
//    }


    //return schedulerModel;

    return new QStandardItemModel(parent);

}



QString CategoriesFileHandler::retrieveCategoriesFilePath() {
    return KStandardDirs::locateLocal("appdata", QString::fromLatin1("categories.xml"));
}



