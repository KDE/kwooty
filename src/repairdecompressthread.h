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

#ifndef REPAIRDECOMPRESSTHREAD_H
#define REPAIRDECOMPRESSTHREAD_H

#include <QThread>
#include <QTimer>

#include "data/nzbfiledata.h"
#include "data/nzbcollectiondata.h"
#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class Repair;
class ExtractRar;
class ExtractZip;


class RepairDecompressThread : public QObject {

    Q_OBJECT

public:
    RepairDecompressThread(CentralWidget*);
    RepairDecompressThread();
    ~RepairDecompressThread();
    CentralWidget* getCentralWidget();

private:
    QThread* dedicatedThread;
    QTimer* repairDecompressTimer;
    CentralWidget* parent;
    Repair* repair;
    ExtractRar* extractRar;
    ExtractZip* extractZip;
    QList<NzbCollectionData> filesToRepairList;
    QList<NzbCollectionData> filesToExtractList;
    QList<NzbCollectionData> filesToProcessList;
    bool waitForNextProcess;

    void init();
    void setupConnections();
    void processRarFilesFromDifferentGroups(const QStringList&, NzbCollectionData&);
    void processRarFilesFromSameGroup(NzbCollectionData&);
    bool isListContainsdifferentGroups(const QList<NzbFileData>&);
    NzbFileData tryToGuessDecodedFileName(NzbFileData&, const QList<NzbFileData>&, const QString&);
    QStringList listDifferentFileBaseName(NzbCollectionData&);
    QString getBaseNameFromPar2(const NzbFileData&);
    QString getBaseNameFromRar(const NzbFileData&);
    UtilityNamespace::ArchiveFormat getArchiveFormatFromList(const QList<NzbFileData>&);


signals:
    void updateRepairSignal(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget);
    void updateExtractSignal(QVariant, int, UtilityNamespace::ItemStatus, UtilityNamespace::ItemTarget);

public slots:
    void repairDecompressSlot(NzbCollectionData);
    void repairProcessEndedSlot(NzbCollectionData, UtilityNamespace::ItemStatus);
    void extractProcessEndedSlot(NzbCollectionData);

private slots:
    void startRepairSlot();
    void startExtractSlot();
    void processPendingFilesSlot();
    void processJobSlot();



};

#endif // REPAIRDECOMPRESSTHREAD_H
