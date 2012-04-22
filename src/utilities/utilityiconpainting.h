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


#ifndef UTILITYICONPAINTING_H
#define UTILITYICONPAINTING_H


#include <KIcon>
#include <KIconLoader>

#include <QMap>
#include <QPixmap>
#include <QLabel>
#include <QStandardItem>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class UtilityIconPainting : public QObject {

    Q_OBJECT

public:
    UtilityIconPainting();
    ~UtilityIconPainting();

    static UtilityIconPainting* getInstance();
    QPixmap blendOverLayTopRight(const QString&, const QString&);
    QPixmap blendOverLayEmblem(const QString&, const QPixmap*);
    QPixmap blendOverLayEmblem(const QString&, const QIcon&);
    QPixmap buildGrayIcon(const QString&);
    QPixmap buildGrayIcon(const QPixmap&);
    QPixmap buildClearIcon(const QPixmap&);
    QPixmap buildSemiTransparentIcon(const QString&);
    QPixmap buildSemiTransparentIcon(const QPixmap&);
    QLabel* buildLighterTextLabel(const QString&, QWidget*);
    QColor lighterColor(const qreal&);
    void displayLighterText(QStandardItem*);

    bool retrieveParentIconFromStatus(const UtilityNamespace::ItemStatus&, KIcon& icon);
    bool retrieveChildIconFromStatus(const UtilityNamespace::ItemStatus&, KIcon& icon);
    bool retrieveIconFromString(const QString&, KIcon& icon);

private:
    static UtilityIconPainting* instance;
    QMap<QString, QPixmap> textIconMap;

    QHash<int, QString> statusIconStrMap;
    QHash<int, QString> parentStatusIconStrMap;
    QHash<QString, KIcon> iconStrIconImageMap;





};

#endif // UTILITYICONPAINTING_H
