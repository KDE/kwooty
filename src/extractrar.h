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


#ifndef EXTRACTRAR_H
#define EXTRACTRAR_H


#include "extractbase.h"


class ExtractRar : public ExtractBase
{
public:
        Q_OBJECT

public:
    ExtractRar(RepairDecompressThread*);
    ~ExtractRar();

private:

    QStringList createProcessArguments(const QString&, const QString&, const bool&, const QString&) ;
    void extractUpdate(const QString&);
    void checkIfArchivePassworded(const QString&, bool&);
    void sendExtractProgramNotFoundNotification();
    QString searchExtractProgram();
    void emitStatusToAllArchives(const int&, const UtilityNamespace::ItemStatus, const UtilityNamespace::ItemTarget);

signals:

public slots:


private slots:

};

#endif // EXTRACTRAR_H
