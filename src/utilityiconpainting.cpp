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


#include "utilityiconpainting.h"

#include <KDebug>
#include <KIconEffect>
#include <KIconLoader>

#include <QApplication>
#include <QPainter>

#include "preferences/preferencesserver.h"
#include "utility.h"
using namespace UtilityNamespace;



UtilityIconPainting::UtilityIconPainting() : QObject(qApp){}

UtilityIconPainting::~UtilityIconPainting() {}


UtilityIconPainting* UtilityIconPainting::instance = 0;
UtilityIconPainting* UtilityIconPainting::getInstance() {

    if (!instance) {
        instance = new UtilityIconPainting();
    }

    return instance;

}


QPixmap UtilityIconPainting::blendOverLayEmblem(const QString& overlayIconStr, const QPixmap* pixmap) {

    QPixmap finalIcon;

    KIcon overlayIcon = KIcon(overlayIconStr);

    if (!overlayIcon.isNull() && pixmap) {

        QPixmap warningPixmap = overlayIcon.pixmap(10, 10);

        finalIcon = pixmap->copy();
        QPainter p(&finalIcon);
        p.drawPixmap(KIconLoader::SizeSmall / 2, KIconLoader::SizeSmall  / 2, warningPixmap);
        p.end();

    }

    return finalIcon;

}


QPixmap UtilityIconPainting::blendOverLayEmblem(const QString& overlayIconStr, const QIcon& icon) {

    QPixmap pixmap = icon.pixmap(KIconLoader::SizeSmall);

    return UtilityIconPainting::getInstance()->blendOverLayEmblem(overlayIconStr, &pixmap);


}


QPixmap UtilityIconPainting::buildClearIcon(const QPixmap& sourceIcon) {

    QImage clearImage = sourceIcon.toImage();
    KIconEffect::toGamma(clearImage, 0.80);
    return QPixmap::fromImage(clearImage);

}







QPixmap UtilityIconPainting::blendOverLayTopRight(const QString& mainIconStr, const QString& overlayIconStr) {

    QPixmap pixmap;
    QString keyStr = mainIconStr + overlayIconStr;

    if (UtilityIconPainting::getInstance()->textIconMap.contains(keyStr)) {

        pixmap = UtilityIconPainting::getInstance()->textIconMap.value(keyStr);

    }
    else {

        KIcon overlayIcon = KIcon(overlayIconStr);

        if (!overlayIcon.isNull()) {

            pixmap = KIconLoader::global()->loadIcon(mainIconStr, KIconLoader::Small, KIconLoader::SizeSmall);

            QPixmap overlayPixmap = KIconLoader::global()->loadIcon(overlayIconStr, KIconLoader::Small, KIconLoader::SizeSmall);
            overlayPixmap = overlayPixmap.scaled(10, 10, Qt::IgnoreAspectRatio, Qt::FastTransformation);

            QPainter p(&pixmap);
            p.drawPixmap(6, 0, overlayPixmap);
            p.end();

             UtilityIconPainting::getInstance()->textIconMap.insert(keyStr, pixmap);
        }

    }


    return pixmap;
}


