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


#ifndef CONCATSPLITFILESJOB_H
#define CONCATSPLITFILESJOB_H

#include <QObject>
#include <QThread>

#include "data/nzbfiledata.h"
#include "utility.h"
using namespace UtilityNamespace;

class ExtractSplit;


class ConcatSplitFilesJob : public QObject {

    Q_OBJECT

public:
    ConcatSplitFilesJob(ExtractSplit*);
    ~ConcatSplitFilesJob();


private:
    QList<NzbFileData> nzbFileDataList;
    QString fileSavePath;
    QString joinFileName;
    QThread* dedicatedThread;

    bool joinSplittedFiles();


signals:
    void progressPercentSignal(int, QString);
    void resultSignal(int);


public slots:
    void joinFilesSlot(QList<NzbFileData>, const QString, const QString);


private slots:


};

#endif // CONCATSPLITFILESJOB_H
