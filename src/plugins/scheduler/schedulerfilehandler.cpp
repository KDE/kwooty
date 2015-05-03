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

#include <KStandardDirs>
#include "kwooty_debug.h"
#include <KLocale>

#include <QFile>
#include <QDate>
#include <QTime>
#include <QXmlStreamWriter>

#include "utilities/utility.h"
using namespace UtilityNamespace;
using namespace SchedulerNamespace;

SchedulerFileHandler::SchedulerFileHandler(QObject *parent) : QObject(parent)
{

}

SchedulerFileHandler::SchedulerFileHandler()
{

}

void SchedulerFileHandler::reloadModel(QStandardItemModel *schedulerModel)
{

    schedulerModel->clear();

    //reload model from file :
    this->fillModel(schedulerModel);
}

QStandardItemModel *SchedulerFileHandler::loadModelFromFile(QObject *parent)
{

    // build scheduler model :
    QStandardItemModel *schedulerModel = new QStandardItemModel(parent);
    this->reloadModel(schedulerModel);

    return schedulerModel;
}

void SchedulerFileHandler::fillModel(QStandardItemModel *schedulerModel)
{

    schedulerModel->setColumnCount(COLUMN_NUMBER_SCHEDULER);
    schedulerModel->setRowCount(ROW_NUMBER_SCHEDULER);

    QFile schedulerFile(this->retrieveSchedulerFilePath());
    schedulerFile.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&schedulerFile);

    int dayNumber = 1;
    int halfHourNumber = 0;
    int downloadLimitStatus = 0;

    while (!stream.atEnd()) {

        // define the data that holds data for a single file to be downloaded :
        QXmlStreamReader::TokenType tokenType = stream.readNext();

        if (tokenType == QXmlStreamReader::StartElement) {

            QXmlStreamAttributes attributes = stream.attributes();

            // get scheduler file version :
            if (stream.name().toString() == "scheduler") {

                if (attributes.value("version").toString() != "1") {

                    qDebug() << "this scheduler.xml version is not compatible with current kwooty version";
                    break;
                }
            }

            // get day number :
            if (stream.name().toString() == "day") {

                // retrieve the day number :
                dayNumber = attributes.value("number").toString().toInt();
                dayNumber = qBound(1, dayNumber, ROW_NUMBER_SCHEDULER - 1);

            }

            // get halfhour number :
            if (stream.name().toString() == "halfhour") {

                // retrieve the halfhour number :
                halfHourNumber = attributes.value("number").toString().toInt();
                halfHourNumber = qBound(0, halfHourNumber, COLUMN_NUMBER_SCHEDULER - 1);

                // retrieve the download limit status :
                downloadLimitStatus = stream.readElementText().toInt();

                // set data to model :
                QStandardItem *item = schedulerModel->itemFromIndex(schedulerModel->index(dayNumber, halfHourNumber));
                item->setData(downloadLimitStatus, DownloadLimitRole);

                // set tooltip :
                int cellHour = halfHourNumber / 2;
                int cellMinute = halfHourNumber - (cellHour * 2);

                QString timeBegin = QTime(cellHour, cellMinute * 30).toString(Utility::getSystemTimeFormat("hh:mm"));
                QString timeEnd = QTime(cellHour, cellMinute * 30 + 29).toString(Utility::getSystemTimeFormat("hh:mm"));

                item->setToolTip(i18nc("day of week, begin time - end time",
                                       "%1, %2 - %3", QDate::longDayName(dayNumber), timeBegin, timeEnd));

            }

        }

    }

    // parsing is done, close the file :
    schedulerFile.close();

    // check that all items are correctly set :
    for (int i = HEADER_ROW_SCHEDULER + 1; i < ROW_NUMBER_SCHEDULER; ++i) {
        for (int j = 0; j < COLUMN_NUMBER_SCHEDULER; j++) {

            QStandardItem *item = schedulerModel->itemFromIndex(schedulerModel->index(i, j));

            bool conversionOk;
            item->data(DownloadLimitRole).toInt(&conversionOk);

            if (!conversionOk) {
                item->setData(static_cast<int>(NoLimitDownload), DownloadLimitRole);
            }
        }
    }

}

void SchedulerFileHandler::saveModelToFile(QStandardItemModel *schedulerModel)
{

    QFile schedulerFile(this->retrieveSchedulerFilePath());
    schedulerFile.open(QIODevice::WriteOnly);

    QXmlStreamWriter stream(&schedulerFile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    // <scheduler> :
    stream.writeStartElement("scheduler");
    stream.writeAttribute("application", "kwooty");
    stream.writeAttribute("version", "1");

    for (int i = HEADER_ROW_SCHEDULER + 1; i < ROW_NUMBER_SCHEDULER; ++i) {

        // <day> :
        stream.writeStartElement("day");
        stream.writeAttribute("number", QString::number(i));

        for (int j = 0; j < COLUMN_NUMBER_SCHEDULER; j++) {

            QStandardItem *item = schedulerModel->itemFromIndex(schedulerModel->index(i, j));
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

QString SchedulerFileHandler::retrieveSchedulerFilePath()
{
    return KStandardDirs::locateLocal("appdata", QStringLiteral("scheduler.xml"));
}

