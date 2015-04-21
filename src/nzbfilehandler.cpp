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

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QUuid>

#include "data/nzbfiledata.h"
#include "data/globalfiledata.h"
#include "core.h"
#include "widgets/centralwidget.h"
#include "kwootysettings.h"
#include "utilities/utility.h"




NzbFileHandler::NzbFileHandler(Core* parent) : QObject (parent) {
    this->parent = parent;
}



bool sortingOrderLessThan(const GlobalFileData& globalFileData1, const GlobalFileData& globalFileData2) {

  return (globalFileData1.getNzbFileData().getFileName() < globalFileData2.getNzbFileData().getFileName());

}



QList<GlobalFileData> NzbFileHandler::processNzbFile(QFile& file, const QString& nzbName) {

    
    // variables definition :
    QMap<int, SegmentData> segmentMap;
    QList<GlobalFileData> globalFileDataList;
    QList<GlobalFileData> globalPar2DataList;

    NzbFileData nzbFileData;
    quint32 elementInList = 0;
    quint64 fileSize = 0;
    bool fileSucessfulyProcessed = true;

    // workaround for xml files with encoding not supported by Qt like 'us-ascii'
    // skip xml encoding line if it exists :
    QString firstLine = file.readLine();
    if (!firstLine.contains(QRegExp("<\\?xml.*us-ascii.*\\?>"))) {
        file.reset();
    }

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

                // try to reduce file name (remove all useless stuff to keep only the concise file name) :
                QRegExp rx("(.*\"?)?.*\"(.*)\".*");

                QString reducedFileName;
                // if concise file name has been found :
                if (rx.indexIn(fileName) != -1) {
                    reducedFileName = rx.cap(2).trimmed();
                }

                // if reduced file name seems to be valid :
                if ( !reducedFileName.isEmpty() &&
                     !QFileInfo(reducedFileName).suffix().isEmpty() ) {

                    nzbFileData.setReducedFileName(reducedFileName);

                }
                // else keep the whole file name :
                else {

                    nzbFileData.setReducedFileName(fileName);

                }

                // identify right now par2 files from others :
                if (fileName.contains(QRegExp("\\.par2.*(yenc)?.*", Qt::CaseInsensitive))) {
                    nzbFileData.setPar2File(true);
                }

                // set name of the nzb :
                nzbFileData.setNzbName(nzbName);

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

                QString uniqueIdStr = QUuid::createUuid().toString();
                // set unique identifier :
                nzbFileData.setUniqueIdentifier(QVariant(uniqueIdStr));

                // create a temporary file name built upon uniqueId value :
                nzbFileData.setTemporaryFileName(uniqueIdStr);

                // set download folder :
                nzbFileData.setDownloadFolderPath(Settings::completedFolder().path());

                // add the nzbFileData to the data map excepted par2 files :
                if (!nzbFileData.isPar2File()) {
                    globalFileDataList.append(GlobalFileData(nzbFileData));
                }
                // add par2 files only in another map :
                else {
                    globalPar2DataList.append(GlobalFileData(nzbFileData));
                }

                // clear variables :    
                NzbFileData nzbFileDataTemp;
                nzbFileData = nzbFileDataTemp;
                segmentMap.clear();
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

    QList<GlobalFileData> globalFileDataOrderedList;

    // if error occurred display error message box :
    if(!fileSucessfulyProcessed) {
        this->parent->getCentralWidget()->displayNzbHandleErrorMessageBox(file.fileName());

    }
    else {

        qSort(globalFileDataList.begin(), globalFileDataList.end(), sortingOrderLessThan);
        qSort(globalPar2DataList.begin(), globalPar2DataList.end(), sortingOrderLessThan);

        globalFileDataOrderedList.append(globalFileDataList);
        globalFileDataOrderedList.append(globalPar2DataList);

    }


    return globalFileDataOrderedList;

}


