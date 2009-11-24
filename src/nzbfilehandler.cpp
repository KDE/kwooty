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

#include "nzbfilehandler.h"

#include <KDebug>
#include <KMessageBox>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QUuid>

#include "data/nzbfiledata.h"
#include "data/globalfiledata.h"
#include "centralwidget.h"
#include "settings.h"
#include "utility.h"


NzbFileHandler::NzbFileHandler()
{

}


QList<GlobalFileData> NzbFileHandler::processNzbFile(CentralWidget* parent, QFile& file, const QString& nzbName) {

    
    // variables definition :
    QMap<int, SegmentData> segmentMap;
    QMap<QString, GlobalFileData> globalFileDataNameMap;

    NzbFileData nzbFileData;
    QVariant parentVariantId;
    quint32 elementInList = 0;
    quint64 fileSize = 0;
    bool fileSucessfulyProcessed = true;

    
    QXmlStreamReader stream (&file);

    while (!stream.atEnd()) {

        // define the data that holds data for a single file to be downloaded :
        QXmlStreamReader::TokenType tokenType = stream.readNext();

        if (tokenType == QXmlStreamReader::StartElement) {

            QXmlStreamAttributes attributes = stream.attributes();

            // get the file name :
            if (stream.name().toString() == "file") {

                QString fileName = attributes.value("subject").toString();
                nzbFileData.setFileName(fileName);

                // set name of the nzb :
                nzbFileData.setNzbName(nzbName);
                // set unique ID for parent :
                parentVariantId.setValue(QUuid::createUuid().toString());
            }

            // get the group(s) :
            if (stream.name().toString() == "group") {
                QString group = stream.readElementText();
                nzbFileData.getGroupList().append(group);
            }

            // get segments, bytes, segment numbers :
            if (stream.name().toString() == "segment") {

                QString bytes = attributes.value("bytes").toString();
                QString number = attributes.value("number").toString();
                QString part = stream.readElementText();

                // add segments size to get whole file size :
                fileSize += bytes.toUInt();

                // retrieve temporary folder :
                QString fileSavePath = Settings::temporaryFolder().path() + "/";

                SegmentData segmentData(bytes, number, part, UtilityNamespace::IdleStatus);
                segmentData.setProgress(PROGRESS_INIT);
                segmentData.setElementInList(elementInList++);


                // segmentData is stored in a map in order to sort segments in ascending order :
                segmentMap.insert(number.toInt(), segmentData);

            }

        } else if (tokenType == QXmlStreamReader::EndElement) {

            if (stream.name().toString() == "file") {

                // append segments in ascending sorted order :

                //set corresponding element value for the current segmentData :
                QList<SegmentData> segmentDataOrderedList = segmentMap.values();
                for (int i = 0; i < segmentDataOrderedList.size(); i++) {

                    SegmentData segmentData = segmentDataOrderedList.at(i);
                    segmentData.setElementInList(i);
                    segmentDataOrderedList.replace(i, segmentData);

                }

                // set segments associated to the nzbFileData
                nzbFileData.setSegmentList(segmentDataOrderedList);
                // set size of the nzbFileData
                nzbFileData.setSize(fileSize);
                // set unique identifier :
                nzbFileData.setUniqueIdentifier(parentVariantId);
                // set download folder :
                nzbFileData.setFileSavePath(Settings::completedFolder().path() + "/" + nzbFileData.getNzbName() + "/");

                // add the nzbFileData to the data map :
                globalFileDataNameMap.insert(nzbFileData.getFileName(), GlobalFileData(nzbFileData));

                // clear variables :    
                NzbFileData nzbFileDataTemp;
                nzbFileData = nzbFileDataTemp;
                segmentMap.clear();
                parentVariantId.clear();
                elementInList = 0;
                fileSize = 0;

            }


            else if ( tokenType == QXmlStreamReader::Invalid ) {
                fileSucessfulyProcessed = false;
            }
        }


        if (stream.hasError()) {
            fileSucessfulyProcessed = false;
        }

    }

    // if error occured display error message box :
    if(!fileSucessfulyProcessed) {
        this->displayMessageBox(parent, file.fileName());
    }

    QList<GlobalFileData> globalFileDataOrderedList = globalFileDataNameMap.values();
    return globalFileDataOrderedList;

}




void NzbFileHandler::displayMessageBox(CentralWidget* parent, const QString& fileName) {

    KMessageBox::messageBox(parent,
                            KMessageBox::Sorry,
                            i18n("The file <b>%1</b> can not be processed",
                                 fileName),
                            i18n("File process error"));

}


