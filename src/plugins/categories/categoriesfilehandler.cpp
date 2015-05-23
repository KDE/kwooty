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
#include "kwooty_debug.h"
#include <kmimetype.h>
#include <KSharedPtr>
#include <QXmlStreamWriter>
#include <QFile>

#include "mimedata.h"
#include "categoriesmodel.h"

#include "utilitycategories.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

CategoriesFileHandler::CategoriesFileHandler(QObject *parent) : QObject(parent)
{

}

CategoriesFileHandler::CategoriesFileHandler()
{

}

QString CategoriesFileHandler::retrieveCategoriesFilePath()
{
    return KStandardDirs::locateLocal("appdata", QStringLiteral("categories.xml"));
}

bool CategoriesFileHandler::isStartElement(QXmlStreamReader &stream, const QString &element)
{

    return (stream.tokenType() == QXmlStreamReader::StartElement &&
            stream.name() == element);
}

bool CategoriesFileHandler::isEndElement(QXmlStreamReader &stream, const QString &element)
{

    return (stream.tokenType() == QXmlStreamReader::EndElement &&
            stream.name() == element);
}

QString CategoriesFileHandler::readNextCharacters(QXmlStreamReader &stream)
{
    stream.readNext();
    return stream.text().toString();
}

void CategoriesFileHandler::reloadModel(CategoriesModel *categoriesModel)
{

    categoriesModel->init();

    //reload model from file :
    this->fillModel(categoriesModel);
}

CategoriesModel *CategoriesFileHandler::loadModelFromFile(QObject *parent)
{

    // build categories model :
    CategoriesModel *categoriesModel = new CategoriesModel(parent);
    this->reloadModel(categoriesModel);

    return categoriesModel;

}

void CategoriesFileHandler::fillModel(CategoriesModel *categoriesModel)
{

    QFile categoriesFile(this->retrieveCategoriesFilePath());

    bool fileOpen = categoriesFile.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&categoriesFile);

    QStringList mainCategoryList = UtilityCategories::retrieveMainCategoryList();

    while (!stream.atEnd() &&
            !stream.hasError()) {

        // define the data that holds data for a single file to be downloaded :
        QXmlStreamReader::TokenType tokenType = stream.readNext();

        if (tokenType == QXmlStreamReader::StartElement) {

            QXmlStreamAttributes attributes = stream.attributes();

            // get categories file version :
            if (stream.name().toString() == "categories") {

                if (attributes.value("version").toString() != "1") {

                    qDebug() << "this categories.xml version is not compatible with current category plugin version";
                    break;
                }
            }

            // get group element :
            if (stream.name().toString() == "group") {

                MimeData groupMimeData(MimeData::MainCategory);

                // set main category :
                QString mainCategory = attributes.value("name").toString();

                if (mainCategoryList.contains(mainCategory)) {

                    groupMimeData.setMainCategory(mainCategory);

                    QStandardItem *groupItem = new QStandardItem(groupMimeData.getDisplayedText());
                    categoriesModel->storeMimeData(groupItem, groupMimeData);
                    categoriesModel->appendRow(groupItem);

                    // look for next <mime> :
                    if (stream.readNextStartElement() &&
                            stream.name() == "mime") {

                        // check that we are inside <group> </group> :
                        MimeData mimeData(MimeData::SubCategory, groupMimeData.getMainCategory());

                        while (!this->isEndElement(stream, "group") &&
                                !stream.atEnd()) {

                            if (this->isStartElement(stream, "mimeType")) {

                                mimeData.setSubCategory(this->readNextCharacters(stream));
                            }

                            if (this->isStartElement(stream, "moveFolderPath")) {
                                mimeData.setMoveFolderPath(this->readNextCharacters(stream));
                            }

                            stream.readNext();

                            // if we reach </mime> :
                            if (this->isEndElement(stream, "mime")) {

                                UtilityCategories::builPartialMimeData(mimeData);

                                if (!mimeData.getSubCategory().isEmpty()) {

                                    QStandardItem *childCategoryItem = new QStandardItem(mimeData.getDisplayedText());
                                    QStandardItem *childTargetItem = new QStandardItem(mimeData.getMoveFolderPath());

                                    categoriesModel->storeMimeData(childCategoryItem, mimeData);

                                    int row = groupItem->rowCount();
                                    groupItem->setChild(row, CategoriesModel::ColumnCategory, childCategoryItem);
                                    groupItem->setChild(row, CategoriesModel::ColumnTarget, childTargetItem);

                                    // clear mimeData subcategory  for filling with next mime element :
                                    mimeData.setSubCategory(QString());;
                                }

                            }

                        }

                    } // end of mime element

                }
            } // group element not found

        } // not a StartElement

    }

    // parsing is done, close the file :
    categoriesFile.close();

    bool error = false;

    if (!fileOpen) {
        error = true;
        qDebug() << "categories.xml can not be open !";
    } else {
        if (stream.hasError()) {
            error = true;
            qDebug() << "categories.xml can not been parsed correctly !";
        }
    }

    // if any error, fill model with init values :
    if (error) {
        categoriesModel->init();
    }

}

void CategoriesFileHandler::saveModelToFile(CategoriesModel *categoriesModel)
{

    QFile categoriesFile(this->retrieveCategoriesFilePath());
    categoriesFile.open(QIODevice::WriteOnly);

    QXmlStreamWriter stream(&categoriesFile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    // <categories> :
    stream.writeStartElement("categories");
    stream.writeAttribute("application", "kwooty");
    stream.writeAttribute("version", "1");

    for (int i = 0; i < categoriesModel->rowCount(); ++i) {

        QStandardItem *groupItem = categoriesModel->item(i);

        // <group> :
        stream.writeStartElement("group");
        stream.writeAttribute("name", categoriesModel->getMainCategory(groupItem));

        if (groupItem->hasChildren()) {

            for (int j = 0; j < groupItem->rowCount(); ++j) {

                QStandardItem *mimeItem = groupItem->child(j);
                MimeData currentMimeData = categoriesModel->loadMimeData(mimeItem);

                // <mime> :
                stream.writeStartElement("mime");

                stream.writeTextElement("mimeType", currentMimeData.getSubCategory());
                stream.writeTextElement("moveFolderPath", currentMimeData.getMoveFolderPath());
                stream.writeTextElement("patterns", currentMimeData.getPatterns());

                // </mime> :
                stream.writeEndElement();

            }

        }

        // </group> :
        stream.writeEndElement();

    }

    stream.writeEndDocument();
    categoriesFile.close();

}

