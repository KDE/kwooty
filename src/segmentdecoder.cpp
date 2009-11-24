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

#include "segmentdecoder.h"

#define PROTOTYPES //required for uudeview.h to prototype function args.

#include <KDebug>
#include <QDir>

#include "uudeview.h"
#include "errno.h"
#include "settings.h"
#include "data/segmentdata.h"
#include "segmentsdecoderthread.h"


SegmentDecoder::SegmentDecoder()
{
    isDecodingStatus = false;
}

SegmentDecoder::~SegmentDecoder() { }


int decodingProgressionNotification (void* opaque, uuprogress* progressionStructure) {

    if (progressionStructure->action == UUACT_DECODING){
        int currentPartNumber = progressionStructure->partno;
        int processedCurrentPartPercent = progressionStructure->percent;
        int maximumPartNumber = progressionStructure->numparts;

        int progression = (100 * currentPartNumber - processedCurrentPartPercent) / maximumPartNumber;

        SegmentDecoder* instance = (SegmentDecoder*) opaque;
        instance->decodeProgression(progression, DecodeStatus);

    }


    if (progressionStructure->action == UUACT_SCANNING){
        // kDebug() << "scanning";
    }

    return 0;
}





void SegmentDecoder::decodeProgression(const int progression, const UtilityNamespace::ItemStatus status, const QString& decodedFileName){

    // update segmentData :
    emit updateDecodeSignal(this->parentIdentifer, progression, status, decodedFileName);

}



void SegmentDecoder::decodeSegments(NzbFileData currentNzbFileData){

    isDecodingStatus = true;

    QString fileSavePath = currentNzbFileData.getFileSavePath();
    this->parentIdentifer = currentNzbFileData.getUniqueIdentifier();

    if (Utility::createFolder(fileSavePath)) {

        // initialisation :
        UUInitialize();
        this->segmentDataList = currentNzbFileData.getSegmentList();

        // setup options :
        this->setupOptions(fileSavePath);

        // setup hook functions for process notification :
        UUSetBusyCallback (this, decodingProgressionNotification, 100);

        // scan segments :
        this->scanSegmentFiles();

        // decode segments :
        this->decodeSegmentFiles();

        // clean memory :
        UUCleanUp ();
        this->segmentDataList.clear();
    }
    // notify user of the issue :
    else {
        emit saveFileErrorSignal(DuringDecode);

    }

    isDecodingStatus = false;

}




bool SegmentDecoder::isDecoding(){

    return isDecodingStatus;
}




void SegmentDecoder::setupOptions(const QString& fileSavePath){

    UUSetOption(UUOPT_FAST, 1, NULL);
    UUSetOption(UUOPT_DUMBNESS, 1, NULL);
    UUSetOption(UUOPT_DESPERATE, 1, NULL);

    UUSetOption(UUOPT_SAVEPATH, 0, fileSavePath.toAscii().data());
    UUSetOption(UUOPT_REMOVE, 1, NULL);
    // UUSetOption(UUOPT_DEBUG, 1, NULL);


}



void SegmentDecoder::scanSegmentFiles(){

    //notify item that list of files is being scanned :
    if (!this->segmentDataList.isEmpty()) {
        this->decodeProgression(0, ScanStatus);
    }

    // it seems there is a bug if a file is composed of only one segment => file is not decoded
    // duplicating the segment seems to solve the issue :
    if (this->segmentDataList.size() == 1) {
        this->segmentDataList.append(this->segmentDataList.at(0));
    }



    // scan every files to be decoded :
    foreach (SegmentData currentSegment, this->segmentDataList) {

        // set current segment to Deocode status :
        currentSegment.setStatus(DecodeStatus);

        // add item to the current list only if it has been downloaded :
        if (currentSegment.getArticlePresenceOnServer() == Present) {

            QString temporaryFolder = Settings::temporaryFolder().path() + '/';
            QString pathNameFile = temporaryFolder + currentSegment.getPart();

            //scan current segment and add it to item list :
            int returnVal = UULoadFile(pathNameFile.toAscii().data(), NULL, 0);


            // manage errors :
            if (returnVal != UURET_OK) {

                QString errorMessage = this->getErrorMessage(returnVal);
            }
        }
    }

}




void SegmentDecoder::decodeSegmentFiles(){

    // decoding will begin :
    this->decodeProgression(PROGRESS_INIT, DecodeStatus);

    // decode scanned files :
    int elementInList = 0;

    // decode each segment from list :
    uulist* item;

    while ((item = UUGetFileListItem(elementInList)) != NULL) {

        QString fileName;

        // decode segment :
        int returnVal = UUDecodeFile (item, NULL);

        // manage errors :
        if (returnVal == UURET_OK) {

            // decoding is over :
            this->decodeProgression(PROGRESS_COMPLETE, DecodeFinishStatus, item->filename);

        }
        else {
            if (item->filename != NULL) {
                fileName = item->filename;
            }

            QString errorMessage = this->getErrorMessage(returnVal);

            this->decodeProgression(PROGRESS_COMPLETE, DecodeErrorStatus);
            //  kDebug() << "error decoding " << fileName << " : " << errorMessage;

        }

        elementInList++;

    }

    // no files has been decoded, notify it :
    if (elementInList == 0) {
        kDebug() << "No file to decode !";
        this->decodeProgression(PROGRESS_INIT, DecodeErrorStatus);
    }

}



QString SegmentDecoder::getErrorMessage(const int returnVal){

    QString errorMessage;
    if (returnVal == UURET_IOERR) {

        int errnoVal = UUGetOption (UUOPT_ERRNO, NULL, NULL, 0);


        // some segment files may have not been found on server, notify IO errors
        // to user except for segments not found :
        if (errnoVal != ENOENT) {
            errorMessage = strerror (errnoVal);
            kDebug() << "Decoding error message :" << errorMessage << " errno :" << errnoVal;
            // send save error signal to open a message box :
            emit saveFileErrorSignal(DuringDecode);
        }

    }
    else {
        errorMessage = UUstrerror(returnVal);
    }

    return errorMessage;

}

