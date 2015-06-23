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

#ifndef EXTRACTSPLIT_H
#define EXTRACTSPLIT_H

#include <QObject>

#include "jobs/concatsplitfilesjob.h"
#include "data/nzbcollectiondata.h"
#include "extractbase.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class RepairDecompressThread;

class ExtractSplit : public ExtractBase
{
    Q_OBJECT

public:
    explicit ExtractSplit(RepairDecompressThread *);
    ~ExtractSplit();
    void launchProcess(const NzbCollectionData &, ExtractBase::ArchivePasswordStatus = ArchiveCheckIfPassworded,
                       bool passwordEnteredByUSer = false, const QString &passwordStr = QString()) Q_DECL_OVERRIDE;

private:
    QStringList createProcessArguments(const QString &, const QString &, const bool &, const QString &) Q_DECL_OVERRIDE;
    void extractUpdate(const QString &) Q_DECL_OVERRIDE;
    void checkIfArchivePassworded(const QString &, bool &) Q_DECL_OVERRIDE;
    void sendExtractProgramNotFoundNotification() Q_DECL_OVERRIDE;
    QString searchExtractProgram() Q_DECL_OVERRIDE;
    void retrieveFullPathJoinFileName(const NzbCollectionData &, QString &, QString &) const;
    QList<NzbFileData> retrieveSplitFilesOnly(const QString &) const;
    void removeRenamedArchiveFile(const NzbFileData &) Q_DECL_OVERRIDE;
    void preRepairProcessing(const NzbCollectionData &) Q_DECL_OVERRIDE;
    ConcatSplitFilesJob *mConcatSplitFilesJob;

Q_SIGNALS:
    void joinFilesSignal(const QList <NzbFileData> &, const QString &, const QString &);
private Q_SLOTS:
    void jobPercentSlot(int, const QString &);
    void jobFinishSlot(int);

};

#endif // EXTRACTSPLIT_H
