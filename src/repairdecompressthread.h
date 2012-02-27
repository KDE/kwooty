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
#include "data/postdownloadinfodata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class Core;
class Repair;
class ExtractBase;


class RepairDecompressThread : public QObject {

    Q_OBJECT

public:
    RepairDecompressThread(Core*);
    RepairDecompressThread();
    ~RepairDecompressThread();
    Core* getCore();
    void emitProcessUpdate(const PostDownloadInfoData&);

private:
    QThread* dedicatedThread;
    QTimer* repairDecompressTimer;
    Core* parent;
    Repair* repair;
    QList<ExtractBase*> extracterList;
    QList<NzbCollectionData> filesToRepairList;
    QList<NzbCollectionData> filesToExtractList;
    QList<NzbCollectionData> filesToProcessList;
    bool waitForNextProcess;

    void init();
    void setupConnections();
    void processPendingFiles();
    void processRarFilesFromDifferentGroups(const QStringList&, NzbCollectionData&);
    void processRarFilesFromSameGroup(NzbCollectionData&);
    void preRepairProcessing(const NzbCollectionData&);
    void notifyNzbProcessEnded(const NzbCollectionData& nzbCollectionData);
    void propagatePostProcessFailureToPendingCollection(QList<NzbCollectionData>&, const NzbCollectionData&);
    bool isListContainsdifferentGroups(const QList<NzbFileData>&);
    NzbFileData tryToGuessDecodedFileName(NzbFileData&, const QList<NzbFileData>&, const QString&);
    QStringList listDifferentFileBaseName(NzbCollectionData&);
    QString getBaseNameFromPar2(const NzbFileData&);
    QString getBaseNameFromRar(const NzbFileData&);
    UtilityNamespace::ArchiveFormat getArchiveFormatFromList(const QList<NzbFileData>&);
    ExtractBase* retrieveCorrespondingExtracter(const NzbCollectionData&);



signals:
    void updateRepairExtractSegmentSignal(PostDownloadInfoData);

public slots:
    void repairDecompressSlot(NzbCollectionData);
    void repairProcessEndedSlot(NzbCollectionData);
    void extractProcessEndedSlot(NzbCollectionData);

private slots:
    void startRepairSlot();
    void startExtractSlot();    
    void processJobSlot();



};

#endif // REPAIRDECOMPRESSTHREAD_H
