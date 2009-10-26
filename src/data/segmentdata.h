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

#ifndef SEGMENTDATA_H
#define SEGMENTDATA_H

#include <QStandardItem>

class SegmentData
{

public:
    SegmentData(const QString&, const QString&, const QString&, const int, const QString&);
    SegmentData();


    void setBytes(const QString&);
    QString getBytes() const;

    void setNumber(const QString&);
    QString getNumber() const;

    void setPart(const QString&);
    QString getPart() const;

    void setStatus(const int);
    int getStatus() const;

    int getProgress() const;
    void setProgress(const int);

    void setFileSavePath(const QString&);
    QString getFileSavePath() const;

    void setElementInList(const quint32);
    quint32 getElementInList() const;

    void setParentUniqueIdentifier(const QVariant&);
    QVariant getParentUniqueIdentifier() const;

    void setArticlePresenceOnServer(const int);
    int getArticlePresenceOnServer() const;


private:
    QString bytes;
    QString number;
    QString part;
    QString fileSavePath;
    QVariant parentUniqueIdentifier;
    quint32 elementInList;
    int status;
    int progress;
    int articlePresence;

};


Q_DECLARE_METATYPE(SegmentData);

#endif // SEGMENTDATA_H
