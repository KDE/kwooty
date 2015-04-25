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


#ifndef JOBNOTIFYDATA_H
#define JOBNOTIFYDATA_H

#include <QDateTime>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class JobNotifyData
{


public:
    JobNotifyData();

    QString getParentUniqueIdentifier() const;
    void setParentUniqueIdentifier(const QString&);
    UtilityNamespace::ItemStatus getStatus() const;
    void setStatus(const UtilityNamespace::ItemStatus&);
    QDateTime getDateTime() const;
    void setDateTime(const QDateTime&);
    QString getNzbFileName() const;
    void setNzbFileName(const QString&);
    bool operator==(const JobNotifyData&);


private:

    QString parentUniqueIdentifier;
    UtilityNamespace::ItemStatus status;
    QDateTime dateTime;
    QString nzbFileName;


Q_SIGNALS:

public Q_SLOTS:



};

#endif // JOBNOTIFYDATA_H
