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

#include "globalfiledata.h"

GlobalFileData::GlobalFileData(const NzbFileData &nzbFileData, const ItemStatusData &itemStatusData, int progressValue)
{
    mNzbFileData = nzbFileData;
    mItemStatusData = itemStatusData;
    mProgressValue = progressValue;
}

GlobalFileData::GlobalFileData()
{
}

GlobalFileData::~GlobalFileData()
{
}

NzbFileData GlobalFileData::getNzbFileData() const
{
    return mNzbFileData;
}

void GlobalFileData::setNzbFileData(const NzbFileData &nzbFileData)
{
    mNzbFileData = nzbFileData;
}

ItemStatusData GlobalFileData::getItemStatusData() const
{
    return mItemStatusData;
}

void GlobalFileData::setItemStatusData(const ItemStatusData &itemStatusData)
{
    mItemStatusData = itemStatusData;
}

int GlobalFileData::getProgressValue() const
{
    return mProgressValue;
}

void GlobalFileData::setProgressValue(int progressValue)
{
    mProgressValue = progressValue;
}

QDataStream &operator<<(QDataStream &out, const GlobalFileData &globalFileData)
{

    out << globalFileData.getNzbFileData()
        << globalFileData.getItemStatusData()
        << globalFileData.getProgressValue();

    return out;
}

QDataStream &operator>>(QDataStream &in, GlobalFileData &globalFileData)
{
    NzbFileData nzbFileData;
    ItemStatusData itemStatusData;
    int progressValue;

    in >> nzbFileData
       >> itemStatusData
       >> progressValue;

    globalFileData.setNzbFileData(nzbFileData);
    globalFileData.setItemStatusData(itemStatusData);
    globalFileData.setProgressValue(progressValue);

    return in;
}

