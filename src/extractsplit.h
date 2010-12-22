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


#ifndef EXTRACTSPLIT_H
#define EXTRACTSPLIT_H

#include <KJob>

#include <QObject>

#include "data/nzbcollectiondata.h"
#include "extractbase.h"
#include "utility.h"
using namespace UtilityNamespace;

class RepairDecompressThread;


class ExtractSplit : public ExtractBase {

public:

    Q_OBJECT

public:
    ExtractSplit(RepairDecompressThread*);
    void launchProcess(const NzbCollectionData&);
    void removeEventualPreviouslyJoinedFile(const NzbCollectionData&);

private:
    QStringList createProcessArguments(const QString&, const QString&, const bool&, const QString&);
    void extractUpdate(const QString&);
    void checkIfArchivePassworded(const QString&, bool&);
    void sendExtractProgramNotFoundNotification();
    QString searchExtractProgram();
    void retrieveFullPathJoinFileName(const NzbCollectionData&, QString&, QString&) const;
    QList<NzbFileData> retrieveSplitFilesOnly(const QString&) const;

public slots:

private slots:
    //void jobPercentSlot(KJob*, unsigned long);
    void jobPercentSlot(int, QString);
    void jobFinishSlot(KJob*);


};

#endif // EXTRACTSPLIT_H
