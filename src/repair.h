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

#ifndef REPAIR_H
#define REPAIR_H

#include <KProcess>
#include <QObject>
#include <QProcess>
#include <QUuid>

#include "data/nzbcollectiondata.h"

#include "utilities/utility.h"
using namespace UtilityNamespace;

class NzbFileData;
class RepairDecompressThread;

class Repair : public QObject
{

    Q_OBJECT
    Q_ENUMS(InternalRepairStatus)

public:

    enum InternalRepairStatus { IdleRepair,
                                Verifying,
                                Repairing,
                                RepairingNotPossible,
                                RepairComplete
                              };

    Repair(RepairDecompressThread *);
    ~Repair();
    void launchProcess(const NzbCollectionData &);
    bool isProcessing();

private:
    QString par2ProgramPath;
    KProcess *repairProcess;
    QList<NzbFileData> nzbFileDataList;
    NzbCollectionData nzbCollectionData;
    QMap<QString, UtilityNamespace::ItemStatus> statusEnumMap;
    QStringList par2FilesOrderedList;
    QString stdOutputLines;
    RepairDecompressThread *parent;
    int repairStatus;
    int repairProgressValueOld;
    bool isPar2ProgramFound;
    bool isProcessingStatus;

    void setupConnections();
    void verifyUpdate(const QString &);
    void repairUpdate(const QString &);
    void sendVerifyNotification(const QString &, const QString &, const UtilityNamespace::ItemStatus);
    void sendMissingFilesNotification();
    void sendVerifyingFilesNotification();
    void sendPar2ProgramNotFoundNotification();
    void resetVariables();
    void updateNzbFileDataInList(NzbFileData &, const UtilityNamespace::ItemStatus, const int);
    void removePar2Files();
    void emitProcessUpdate(const QVariant &, const int &, const UtilityNamespace::ItemStatus &, const UtilityNamespace::ItemTarget &);
    QString sortPar2FilesBySize();
    UtilityNamespace::ItemTarget getItemTarget(const NzbFileData &);

Q_SIGNALS:
    void repairProcessEndedSignal(const NzbCollectionData &);

public Q_SLOTS:
    void repairReadyReadSlot();
    void repairFinishedSlot(int, QProcess::ExitStatus);
};

#endif // REPAIR_H
