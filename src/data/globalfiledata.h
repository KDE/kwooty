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

#ifndef GLOBALFILEDATA_H
#define GLOBALFILEDATA_H


#include "nzbfiledata.h"
#include "itemstatusdata.h"
#include "utility.h"
using namespace UtilityNamespace;


class GlobalFileData
{

public:

    GlobalFileData(const NzbFileData&, const ItemStatusData& itemStatusData = ItemStatusData(), const int& progressValue = 0);
    GlobalFileData();
    ~GlobalFileData();

    NzbFileData getNzbFileData() const;
    void setNzbFileData(const NzbFileData&);

    ItemStatusData getItemStatusData() const;
    void setItemStatusData(const ItemStatusData&);

    int getProgressValue() const;
    void setProgressValue(const int&);


private:
    NzbFileData nzbFileData;
    ItemStatusData itemStatusData;
    int progressValue;


};


QDataStream& operator<<(QDataStream&, const GlobalFileData&);
QDataStream& operator>>(QDataStream&, GlobalFileData&);

Q_DECLARE_METATYPE(GlobalFileData);

#endif // GLOBALFILEDATA_H
