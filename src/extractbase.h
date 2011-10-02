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

#ifndef EXTRACTBASE_H
#define EXTRACTBASE_H

#include <KProcess>
#include <QObject>

#include "data/nzbcollectiondata.h"

#include "utility.h"
using namespace UtilityNamespace;

class NzbFileData;
class RepairDecompressThread;

class ExtractBase : public QObject
{

    Q_OBJECT
    Q_ENUMS(InternalExtractStatus)
    Q_ENUMS(ArchivePasswordStatus)

  public:

    enum InternalExtractStatus{ IdleExtract,
                                ExtractQuestion,
                                Extracting,
                                ExtractingNotPossible,
                                ExtractComplete

                            };
    enum ArchivePasswordStatus { ArchiveCheckIfPassworded,
                                 ArchiveIsPassworded,
                                 ArchiveIsNotPassworded,
                                 ArchivePasswordCheckEnded,
                             };


    ExtractBase(RepairDecompressThread*);
    virtual ~ExtractBase();
    virtual void launchProcess(const NzbCollectionData&, ExtractBase::ArchivePasswordStatus = ArchiveCheckIfPassworded,
                               bool passwordEnteredByUSer = false, const QString passwordStr = QString());
    virtual void preRepairProcessing(const NzbCollectionData&);
    bool canHandleFormat(UtilityNamespace::ArchiveFormat);



protected:

    KProcess* extractProcess;
    NzbCollectionData nzbCollectionData;
    QList<NzbFileData> nzbFileDataList;
    QTimer* processWaitAnswerTimer;
    ExtractBase::ArchivePasswordStatus  archivePasswordStatus;
    QString extractProgramPath;
    QString stdOutputLines;
    int extractProgressValue;
    bool isExtractProgramFound;
    RepairDecompressThread* parent;
    UtilityNamespace::ArchiveFormat archiveFormat;

    void setupConnections();
    void resetVariables();
    void updateNzbFileDataInList(NzbFileData&, const UtilityNamespace::ItemStatus, const int);
    void emitProgressToArchivesWithCurrentStatus(const UtilityNamespace::ItemStatus, const UtilityNamespace::ItemTarget, const int);
    void emitFinishToArchivesWithoutErrors(const UtilityNamespace::ItemStatus, const int);
    void emitStatusToAllArchives(const int&, const UtilityNamespace::ItemStatus, const UtilityNamespace::ItemTarget);
    void findItemAndNotifyUser(const QString& fileNameStr, const UtilityNamespace::ItemStatus, const UtilityNamespace::ItemTarget);
    void removeArchiveFiles();
    NzbFileData getFirstArchiveFileFromList() const;
    NzbFileData getFirstArchiveFileFromList(const QList<NzbFileData>&) const;
    QString getOriginalFileName(const NzbFileData&) const ;

    // virtual methods :
    virtual void removeRenamedArchiveFile(const NzbFileData&);

    // pure virtual methods implemented by extractrar and extractzip :
    virtual QStringList createProcessArguments(const QString&, const QString&, const bool&, const QString&)  = 0;
    virtual void extractUpdate(const QString&) = 0;
    virtual void checkIfArchivePassworded(const QString&, bool&) = 0;
    virtual void sendExtractProgramNotFoundNotification() = 0;
    virtual QString searchExtractProgram() = 0;


signals:
    void extractProcessEndedSignal(NzbCollectionData = NzbCollectionData());
    void extractPasswordRequiredSignal(QString);


public slots:
    void passwordEnteredByUserSlot(bool, QString password = QString());

protected slots:
    void extractReadyReadSlot();
    void extractFinishedSlot(const int, const QProcess::ExitStatus);

};

#endif // EXTRACTBASE_H
