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

#ifndef NZBFILEDATA_H
#define NZBFILEDATA_H

#include "segmentdata.h"
#include "utility.h"
using namespace UtilityNamespace;

class NzbFileData
{

public:
    NzbFileData();
    NzbFileData(const QString&, const QStringList&, const QList<SegmentData>&);
    ~NzbFileData();
    QString getFileName() const;
    void setFileName(const QString& fileName);

    QStringList getGroupList() const;
    void setGroupList(const QStringList& groupList);

    QList<SegmentData> getSegmentList() const;
    void setSegmentList(const QList<SegmentData>&);

    quint64 getSize() const;
    void setSize(const quint64);

    QString getNzbName() const;
    void setNzbName(const QString&);

    void setBaseName(const QString&);
    QString getBaseName() const;

    bool match(const QString&, const QString& = QString()) ;

    QString getDecodedFileName() const;
    void setDecodedFileName(const QString&);

    QString getTemporaryFileName() const;
    void setTemporaryFileName(const QString&);

    QString getRenamedFileName() const;
    void setRenamedFileName(const QString&, const QString&);

    void setUniqueIdentifier(const QVariant&);
    QVariant getUniqueIdentifier() const;

    void setFileSavePath(const QString&);
    QString getFileSavePath() const;

    bool isPar2File() const;
    void setPar2File(const bool);

    bool isArchiveFile() const;
    void setArchiveFile(const bool);

    void setArchiveFormat(const UtilityNamespace::ArchiveFormat);
    UtilityNamespace::ArchiveFormat getArchiveFormat() const;

    void setVerifyProgressionStep(const UtilityNamespace::ItemStatus);
    UtilityNamespace::ItemStatus getVerifyProgressionStep() const;

    void setExtractProgressionStep(const UtilityNamespace::ItemStatus);
    UtilityNamespace::ItemStatus getExtractProgressionStep() const;


    bool operator==(const NzbFileData&);
    bool operator<(const NzbFileData&) const;

private:
    QString fileName;
    QString decodedFileName;
    QString temporaryFileName;
    QString renamedFileName;
    QStringList possibleFileNameList;
    QString baseName;
    QString nzbName;
    QString fileSavePath;
    QStringList groupList;
    QList<SegmentData> segmentList;
    QVariant uniqueIdentifier;
    quint64 size;
    UtilityNamespace::ItemStatus verifyProgressionStep;
    UtilityNamespace::ItemStatus extractProgressionStep;
    UtilityNamespace::ArchiveFormat archiveFormat;
    bool par2File;
    bool archiveFile;

};

QDataStream& operator<<(QDataStream&, const NzbFileData&);
QDataStream& operator>>(QDataStream&, NzbFileData&);

Q_DECLARE_METATYPE(NzbFileData);

#endif // NZBFILEDATA_H
