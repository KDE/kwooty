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

#include "jobnotifydata.h"

JobNotifyData::JobNotifyData()
{

}

QString JobNotifyData::getParentUniqueIdentifier() const
{
    return this->mParentUniqueIdentifier;
}

void JobNotifyData::setParentUniqueIdentifier(const QString &parentUniqueIdentifier)
{
    this->mParentUniqueIdentifier = parentUniqueIdentifier;
}

UtilityNamespace::ItemStatus JobNotifyData::getStatus() const
{
    return this->mStatus;
}

void JobNotifyData::setStatus(const UtilityNamespace::ItemStatus &status)
{
    this->mStatus = status;
}

QDateTime JobNotifyData::getDateTime() const
{
    return this->mDateTime;
}

void JobNotifyData::setDateTime(const QDateTime &dateTime)
{
    this->mDateTime = dateTime;
}

QString JobNotifyData::getNzbFileName() const
{
    return this->mNzbFileName;
}

void JobNotifyData::setNzbFileName(const QString &nzbFileName)
{
    this->mNzbFileName = nzbFileName;
}

bool JobNotifyData::operator==(const JobNotifyData &jobNotifyDataToCompare)
{
    return (this->getParentUniqueIdentifier() == jobNotifyDataToCompare.getParentUniqueIdentifier());
}
