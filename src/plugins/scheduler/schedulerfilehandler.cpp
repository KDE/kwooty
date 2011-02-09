/***************************************************************************
 *   Copyright (C) 2011 by Xavier Lefage                                   *
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


#include "schedulerfilehandler.h"

#include "preferencesscheduler.h"

#include <KStandardDirs>
#include <KDebug>

#include <QFile>
#include <QXmlStreamWriter>

using namespace SchedulerNamespace;

SchedulerFileHandler::SchedulerFileHandler(QObject* parent) : QObject(parent) {

}

SchedulerFileHandler::SchedulerFileHandler() {

}



QStandardItemModel* SchedulerFileHandler::loadModelFromFile(QObject* parent) {


    QStandardItemModel* schedulerModel = new QStandardItemModel(parent);
    schedulerModel->setColumnCount(COLUMN_NUMBER_SCHEDULER);
    schedulerModel->setRowCount(ROW_NUMBER_SCHEDULER);

    QFile schedulerFile(this->retrieveSchedulerFilePath());
    schedulerFile.open(QIODevice::ReadOnly);
    QXmlStreamReader stream (&schedulerFile);

    int dayNumer = 1;
    int halfHourNumer = 0;
    int downloadLimitStatus = 0;

    while (!stream.atEnd()) {

        // define the data that holds data for a single file to be downloaded :
        QXmlStreamReader::TokenType tokenType = stream.readNext();

        if (tokenType == QXmlStreamReader::StartElement) {

            QXmlStreamAttributes attributes = stream.attributes();

            // get scheduler file version :
            if (stream.name().toString() == "scheduler") {

                if (attributes.value("version").toString() != "1") {

                    kDebug() << "this scheduler.xml version is not compatible with current kwooty version";
                    break;
                }
            }

            // get day number :
            if (stream.name().toString() == "day") {

                // retrieve the day number :
                dayNumer = attributes.value("number").toString().toInt();
                dayNumer = qBound(1, dayNumer, ROW_NUMBER_SCHEDULER - 1);

            }

            // get halfhour number :
            if (stream.name().toString() == "halfhour") {

                // retrieve the halfhour number :
                halfHourNumer = attributes.value("number").toString().toInt();
                halfHourNumer = qBound(0, halfHourNumer, COLUMN_NUMBER_SCHEDULER - 1);

                // retrieve the download limit status :
                downloadLimitStatus = stream.readElementText().toInt();

                // set data to model :
                QStandardItem* item = schedulerModel->itemFromIndex(schedulerModel->index(dayNumer, halfHourNumer));
                item->setData(downloadLimitStatus, DownloadLimitRole);

            }

        }

    }


    // parsing is done, close the file :
    schedulerFile.close();


    // check that all items are correctly set :
    for (int i = HEADER_ROW_SCHEDULER + 1; i < ROW_NUMBER_SCHEDULER; i++) {
        for (int j = 0; j < COLUMN_NUMBER_SCHEDULER; j++) {

            QStandardItem* item = schedulerModel->itemFromIndex(schedulerModel->index(i, j));

            bool conversionOk;
            item->data(DownloadLimitRole).toInt(&conversionOk);

            if (!conversionOk) {
                item->setData(static_cast<int>(NoLimitDownload), DownloadLimitRole);
            }
        }
    }


    return schedulerModel;

}


void SchedulerFileHandler::saveModelToFile(QStandardItemModel* schedulerModel) {

    QFile schedulerFile(this->retrieveSchedulerFilePath());
    kDebug() << "SAVE FILE" << schedulerFile.fileName();

    schedulerFile.open(QIODevice::WriteOnly);


    QXmlStreamWriter stream(&schedulerFile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    // <scheduler> :
    stream.writeStartElement("scheduler");
    stream.writeAttribute("application", "kwooty");
    stream.writeAttribute("version", "1");



    for (int i = HEADER_ROW_SCHEDULER + 1; i < ROW_NUMBER_SCHEDULER; i++) {

        // <day> :
        stream.writeStartElement("day");
        stream.writeAttribute("number", QString::number(i));


        for (int j = 0; j < COLUMN_NUMBER_SCHEDULER; j++) {


            QStandardItem* item = schedulerModel->itemFromIndex(schedulerModel->index(i, j));
            int downloadLimitStatus = item->data(DownloadLimitRole).toInt();

            // <halfour> :
            stream.writeStartElement("halfhour");

            stream.writeAttribute("number", QString::number(j));
            stream.writeCharacters(QString::number(downloadLimitStatus));

            // </halfour> :
            stream.writeEndElement();


        }

        // </day> :
        stream.writeEndElement();
    }

    // </scheduler> :
    stream.writeEndElement();

    stream.writeEndDocument();
    schedulerFile.close();

}


QString SchedulerFileHandler::retrieveSchedulerFilePath() {
    return KStandardDirs::locateLocal("appdata", QString::fromLatin1("scheduler.xml"));
}




