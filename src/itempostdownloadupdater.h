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

#ifndef ITEMPOSTDOWNLOADUPDATER_H
#define ITEMPOSTDOWNLOADUPDATER_H

#include "itemabstractupdater.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class StandardItemModel;
class NzbFileData;
class PostDownloadInfoData;


class ItemPostDownloadUpdater : public ItemAbstractUpdater {

public:

    ItemPostDownloadUpdater(ItemParentUpdater*);
    void updateItems(const PostDownloadInfoData&);
    void addFileTypeInfo(const PostDownloadInfoData&);

private:

    void updateNzbChildrenItems(const QModelIndex&, const int, const int);
    void updateDecodeItems(const PostDownloadInfoData&);
    void updateRepairExtractItems(const PostDownloadInfoData&);
    void updateRepairExtractParentItems(const PostDownloadInfoData&);


};

#endif // ITEMPOSTDOWNLOADUPDATER_H
