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

#ifndef EXTRACT_H
#define EXTRACT_H

#include <KProcess>
#include <QObject>

#include "utility.h"
using namespace UtilityNamespace;

class NzbFileData;

class Extract : public QObject
{

    Q_OBJECT
    Q_ENUMS(InternalExtractStatus)

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


    Extract();
    ~Extract();
    void launchProcess(const QList<NzbFileData>&, Extract::ArchivePasswordStatus = ArchiveCheckIfPassworded,
                       bool passwordEnteredByUSer = false, const QString passwordStr = QString());

private:

    KProcess* extractProcess;
    QList<NzbFileData> nzbFileDataList;
    QTimer* processWaitAnswerTimer;
    Extract::ArchivePasswordStatus  archivePasswordStatus;
    QString unrarProgramPath;
    QString fileSavePath;
    QString fileNameToExtract;
    QString stdOutputLines;
    int extractProgressValue;
    bool isUnrarProgramFound;

    void setupConnections();
    void resetVariables();
    void updateNzbFileDataInList(NzbFileData&, const UtilityNamespace::ItemStatus, const int);
    void sendUnrarProgramNotFoundNotification();
    void extractUpdate(const QString&);
    void emitProgressToArchivesWithCurrentStatus(const UtilityNamespace::ItemStatus, const UtilityNamespace::ItemTarget, const int);
    void emitFinishToArchivesWithoutErrors(const UtilityNamespace::ItemStatus, const int);
    void emitStatusToAllArchives(const UtilityNamespace::ItemStatus);
    void checkIfArchivePassworded(const QString&, bool&);
    void findItemAndNotifyUser(const QString& fileNameStr, const UtilityNamespace::ItemStatus, const UtilityNamespace::ItemTarget);
    void removeRarFiles();
    NzbFileData getFirstRarFileFromList() const;


signals:
    void extractProcessEndedSignal();
    void updateExtractSignal(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget);
    void extractPasswordRequiredSignal(QString);

public slots:
    void passwordEnteredByUserSlot(bool, QString password = QString());

private slots:
    void extractReadyReadSlot();
    void extractFinishedSlot(const int, const QProcess::ExitStatus);

};

#endif // EXTRACT_H
