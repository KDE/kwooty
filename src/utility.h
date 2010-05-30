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

#ifndef UTILITY_H
#define UTILITY_H

#include <QObject>
#include <QStandardItem>


namespace UtilityNamespace
{
    Q_ENUMS(MyRoles)
    Q_ENUMS(ItemStatus)
    Q_ENUMS(ConnectionStatus)
    Q_ENUMS(Article)
    Q_ENUMS(Data)
    Q_ENUMS(SaveError)
    Q_ENUMS(ItemTarget)
            
   // treeView columns :
    static const int FILE_NAME_COLUMN = 0;
    static const int STATE_COLUMN = 1;
    static const int PROGRESS_COLUMN = 2;
    static const int SIZE_COLUMN = 3;
    
    // percentage progression values :
    static const int PROGRESS_COMPLETE = 100;
    static const int PROGRESS_INIT = 0;
    static const int PROGRESS_UNKNOWN = -1;
    
    // file patterns :
    static const QString rarFileMagicNumber = "Rar";
    static const QString zipFileMagicNumber = "PK";
    static const QString sevenZipFileMagicNumber = "7z";
    static const QString par2FileMagicNumber = "PAR2";

    static const QString par2FileExt = ".par2";
    
    static const QString repairProgram = "par2";
    static const QString rarExtractProgram = "unrar";
    static const QString sevenZipExtractProgram = "7z;7za";
    
    // segment file identifier :
    static const QString applicationFileOwner = "Kwooty_87b022df-17b9-409f-a423-3cc626831adc\r\n";
    static const QString remainingDownloadsFile = "kwootyInQueueDownloads.dat";
    
    // units :
    static const int MINUTES_TO_MILLISECONDS = 60000;
    static const int HOURS_TO_MILLISECONDS = 3600000;

    static const int SECONDS_IN_DAY = 86400;
    static const int SECONDS_IN_HOUR = 3600;
    static const int SECONDS_IN_MINUTE = 60;

    static const double NBR_BYTES_IN_GB = 1073741824;
    static const double NBR_BYTES_IN_MB = 1048576;
    static const double NBR_BYTES_IN_KB = 1024;
    static const double ONE_UNIT = 1;


    // custom roles used for storing data in items :
    enum MyRoles{
        StatusRole  = Qt::UserRole + 1,
        SegmentRole = Qt::UserRole + 2,
        NzbFileDataRole = Qt::UserRole + 3,
        IdentifierRole = Qt::UserRole + 4,
        ProgressRole = Qt::UserRole + 5,
        SizeRole = Qt::UserRole + 6
    };
    
    // item status list :
    enum ItemStatus{
        IdleStatus,
        // -- from here download related enums :
        DownloadStatus,
        DownloadFinishStatus,
        PauseStatus,
        PausingStatus,
        WaitForPar2IdleStatus,
        // -- from here decoding related enums :
        DecodeStatus,
        DecodeErrorStatus,
        DecodeFinishStatus,
        ScanStatus,
        // -- from here verifying/repairing (par2) related enums :
        VerifyStatus,
        VerifyFoundStatus,
        VerifyMatchStatus,
        VerifyMissingStatus,
        VerifyDamagedStatus,
        VerifyFinishedStatus,
        RepairStatus,
        RepairFinishedStatus,
        RepairNotPossibleStatus,
        RepairFailedStatus,
        Par2ProgramMissing,
        // -- from here extracting (unrar) related enums :
        ExtractStatus,
        ExtractBadCrcStatus,
        ExtractSuccessStatus,
        ExtractFinishedStatus,
        ExtractFailedStatus,
        UnrarProgramMissing,
        SevenZipProgramMissing
    };
    
    // connection status list used by statusbar :
    enum ConnectionStatus {
        Disconnected,
        HostFound,
        HostNotFound,
        ConnectionRefused,
        RemoteHostClosed,
        SslHandshakeFailed,
        AuthenticationFailed,
        AuthenticationAccepted,
        AuthenticationNeeded,
        ConnectedSslActive,
        Connected,
        NoError
    };
    
    // indicate presence of segment on server :
    enum Article {
        Present,
        NotPresent,
        Unknown,
    };
    
    // indicate if downloaded data are missing :
    enum Data {
        NoData,
        DataIncomplete,
        DataComplete,
    };
    
    
    // add details about message box error :
    enum SaveError {
        DuringDecode,
        DuringDownload
    };
    
    
    // choose target item to be updated :
    enum ItemTarget {
        ChildItemTarget,
        ParentItemTarget,
        BothItemsTarget
    };
    

    // indicate if file is open by another app or directly by this app :
    enum OpenFileMode {
        OpenNormal,
        OpenWith
    };


    // indicate the crc status of each downloaded file :
    enum CrcNotify {
        CrcOk,
        CrcKo,
        CrcKoNotified
    };

    // type of status bar file size update :
    enum StatusBarUpdateType {
        Incremental,
        Reset
    };

    // type of system shudown :
    enum SystemShutdownType {
        Shutdown,
        Standby,
        Suspend,
        Hibernate,
        ShutdownMethodUnknown
    };


    // type of archive :
    enum ArchiveFormat {
        RarFormat,
        ZipFormat,
        SevenZipFormat,
        UnknownArchiveFormat
    };


    // info about free disk space :
    enum FreeDiskSpace {
        SufficientDiskSpace,
        InsufficientDiskSpace,
        UnknownDiskSpace
    };



}

class Utility
{
    
public:
    
    Utility();
    
    
    static QString convertByteHumanReadable(const quint64);
    static QString convertDownloadSpeedHumanReadable(const quint64);
    static bool isInDownloadProcess(const UtilityNamespace::ItemStatus);
    static bool isReadyToDownload(const UtilityNamespace::ItemStatus);
    static bool isInQueue(const UtilityNamespace::ItemStatus);
    static bool isPaused(const UtilityNamespace::ItemStatus);
    static bool isPausing(const UtilityNamespace::ItemStatus);
    static bool isDownloadOrPausing(const UtilityNamespace::ItemStatus);
    static bool isDownloadFinish(const UtilityNamespace::ItemStatus);
    static bool isDecoding(const UtilityNamespace::ItemStatus);
    static bool isWaitingForDecode(const UtilityNamespace::ItemStatus, const UtilityNamespace::Data);
    static bool isPostDownloadProcessing(const UtilityNamespace::ItemStatus);
    static bool isJobFinish(const UtilityNamespace::ItemStatus);
    static bool saveData(const QString&, const QString&, const QByteArray&);
    static bool createFolder(const QString&);
    static bool isFolderExists(const QString&);
    static bool removeData(const QString&);
    static QString searchExternalPrograms(const QString&, bool&);
    static QString getSystemTimeFormat(const QString&);
    
    
private:

    
};



#endif // UTILITY_H
