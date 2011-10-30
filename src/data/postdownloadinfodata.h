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


#ifndef POSTDOWNLOADINFODATA_H
#define POSTDOWNLOADINFODATA_H

#include <QString>
#include <QVariant>
#include <QModelIndex>

#include "utility.h"
using namespace UtilityNamespace;

class PostDownloadInfoData {

public:
    PostDownloadInfoData();
    void initRepairDecompress(const QVariant&, const int&, const UtilityNamespace::ItemStatus&, const UtilityNamespace::ItemTarget&);
    void initDecode(const QVariant&, const int&, const UtilityNamespace::ItemStatus&, const QString& = QString());

    QModelIndex getModelIndex() const;
    void setModelIndex(const QModelIndex&);

    void setParentIdentifer(const QVariant&);
    QVariant getParentIdentifer() const;

    void setProgression(const int&);
    int getProgression() const;

    void setStatus(const UtilityNamespace::ItemStatus&);
    UtilityNamespace::ItemStatus getStatus() const;

    void setItemTarget(const UtilityNamespace::ItemTarget&);
    UtilityNamespace::ItemTarget getItemTarget() const;

    void setDecodedFileName(const QString&);
    QString getDecodedFileName() const;

    void setCrc32Match(const bool&);
    bool isCrc32Match() const;

    void setAllPostProcessingCorrect(const bool&);
    bool areAllPostProcessingCorrect() const;

    void setPostProcessFinish(const bool&);
    bool isPostProcessFinish() const;

    void setArticleEncodingType(const UtilityNamespace::ArticleEncodingType&);
    UtilityNamespace::ArticleEncodingType getArticleEncodingType() const;

private:
    void init();

    UtilityNamespace::ArticleEncodingType articleEncodingType;
    UtilityNamespace::ItemStatus status;
    UtilityNamespace::ItemTarget itemTarget;
    QModelIndex modelIndex;
    QVariant parentIdentifer;
    QString decodedFileName;
    int progression;
    bool crc32Match;
    bool allPostProcessingCorrect;
    bool postProcessFinish;



signals:

public slots:



};

#endif // POSTDOWNLOADINFODATA_H
