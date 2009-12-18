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

#ifndef ITEMSTATUSDATA_H
#define ITEMSTATUSDATA_H

#include <QStandardItem>
#include "utility.h"
using namespace UtilityNamespace;

class ItemStatusData
{
public:
    ItemStatusData();
    ~ItemStatusData();
    void init();
    void setStatus(const UtilityNamespace::ItemStatus);
    UtilityNamespace::ItemStatus getStatus() const;
    void setDataStatus(const UtilityNamespace::Data);
    UtilityNamespace::Data getDataStatus() const;
    bool isDownloadFinish() const;
    void setDownloadFinish(const bool);
    bool isDecodeFinish() const;
    void setDecodeFinish(const bool);
    UtilityNamespace::CrcNotify getCrc32Match() const;
    void setCrc32Match(const UtilityNamespace::CrcNotify);

private:
    UtilityNamespace::ItemStatus status;
    UtilityNamespace::Data data;
    bool downloadFinish;
    bool decodeFinish;
    UtilityNamespace::CrcNotify crc32Match;

};

QDataStream& operator<<(QDataStream&, const ItemStatusData&);
QDataStream& operator>>(QDataStream&, ItemStatusData&);

Q_DECLARE_METATYPE(ItemStatusData);

#endif // ITEMSTATUSDATA_H

