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

#include <QApplication>
#include <QPainter>

#include "preferences/preferencesserver.h"


UtilityIconPainting::UtilityIconPainting() : QObject(qApp) {

    // build map in order to display status icon near to each child file name item :
    statusIconStrMap.insert(DownloadStatus,            "mail-receive");
    statusIconStrMap.insert(DownloadFinishStatus,      "mail-mark-unread");
    statusIconStrMap.insert(IdleStatus,                "mail-mark-unread");
    statusIconStrMap.insert(PauseStatus,               "mail-mark-unread");
    statusIconStrMap.insert(PausingStatus,             "mail-mark-unread");
    statusIconStrMap.insert(WaitForPar2IdleStatus,     "mail-mark-unread-new");
    statusIconStrMap.insert(ScanStatus,                "mail-mark-unread");
    statusIconStrMap.insert(DecodeStatus,              "mail-mark-unread");
    statusIconStrMap.insert(DecodeFinishStatus,        "mail-mark-read");
    statusIconStrMap.insert(DecodeErrorStatus,         "edit-delete");
    statusIconStrMap.insert(VerifyStatus,              "mail-mark-read");
    statusIconStrMap.insert(VerifyFoundStatus,         "dialog-ok-apply");
    statusIconStrMap.insert(VerifyMatchStatus,         "dialog-ok-apply");
    statusIconStrMap.insert(VerifyMissingStatus,       "edit-delete");
    statusIconStrMap.insert(VerifyDamagedStatus,       "edit-delete");
    statusIconStrMap.insert(RepairStatus,              "mail-mark-read");
    statusIconStrMap.insert(RepairNotPossibleStatus,   "edit-delete");
    statusIconStrMap.insert(RepairFailedStatus,        "edit-delete");
    statusIconStrMap.insert(ExtractStatus,             "dialog-ok-apply");
    statusIconStrMap.insert(ExtractBadCrcStatus,       "edit-delete");
    statusIconStrMap.insert(ExtractSuccessStatus,      "dialog-ok-apply");
    statusIconStrMap.insert(ExtractFailedStatus,       "edit-delete");

    // build map in order to display status icon near to each parent nzb file name item :
    parentStatusIconStrMap.insert(IdleStatus,           "go-next-view");
    parentStatusIconStrMap.insert(PauseStatus,          "go-next-view");
    parentStatusIconStrMap.insert(PausingStatus,        "go-next-view");
    parentStatusIconStrMap.insert(DownloadStatus,       "go-next-view");
    parentStatusIconStrMap.insert(VerifyFinishedStatus, "dialog-ok");
    parentStatusIconStrMap.insert(RepairFinishedStatus, "dialog-ok");
    parentStatusIconStrMap.insert(ExtractFinishedStatus,"dialog-ok");
    parentStatusIconStrMap.insert(DownloadFinishStatus, "go-next-view");



    // build a map to store every used icons in memory according to their names :
    foreach (QString iconName, statusIconStrMap.values()) {
        iconStrIconImageMap.insert(iconName, KIcon(iconName));
    }

    foreach (QString iconName, parentStatusIconStrMap.values()) {
        iconStrIconImageMap.insert(iconName, KIcon(iconName));
    }

    iconStrIconImageMap.insert("mail-reply-list",       KIcon("mail-reply-list"));
    iconStrIconImageMap.insert("mail-mark-important",   KIcon("mail-mark-important"));
    iconStrIconImageMap.insert("mail-mark-important",   KIcon("mail-mark-important"));
    iconStrIconImageMap.insert("dialog-warning",        KIcon("dialog-warning"));
    iconStrIconImageMap.insert("dialog-cancel",         KIcon("dialog-cancel"));

    // add semi-transparent effect to icon for Idle status :
    iconStrIconImageMap.insert("go-next-view-transparent", KIcon(this->instance->buildSemiTransparentIcon("go-next-view")));

}


UtilityIconPainting::~UtilityIconPainting() {}


UtilityIconPainting* UtilityIconPainting::instance = 0;
UtilityIconPainting* UtilityIconPainting::getInstance() {

    if (!instance) {
        instance = new UtilityIconPainting();
    }

    return instance;

}




bool UtilityIconPainting::retrieveParentIconFromStatus(const UtilityNamespace::ItemStatus& status, KIcon& icon) {

    icon = this->iconStrIconImageMap.value(this->parentStatusIconStrMap.value(status));
    return this->parentStatusIconStrMap.contains(status);

}

bool UtilityIconPainting::retrieveChildIconFromStatus(const UtilityNamespace::ItemStatus& status, KIcon& icon) {

    icon = this->iconStrIconImageMap.value(this->statusIconStrMap.value(status));
    return this->statusIconStrMap.contains(status);

}


bool UtilityIconPainting::retrieveIconFromString(const QString& iconName, KIcon& icon) {

    icon = this->iconStrIconImageMap.value(iconName);
    return this->iconStrIconImageMap.contains(iconName);

}




QPixmap UtilityIconPainting::blendOverLayEmblem(const QString& overlayIconStr, const QPixmap* pixmap) {

    QPixmap finalIcon;

    KIcon overlayIcon = KIcon(overlayIconStr);

    if (!overlayIcon.isNull() && pixmap) {

        QPixmap warningPixmap = overlayIcon.pixmap(8, 8);

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


QPixmap UtilityIconPainting::buildGrayIcon(const QString& sourceIconStr) {

    QPixmap pixmap = KIconLoader::global()->loadIcon(sourceIconStr, KIconLoader::Small);
    return this->instance->buildGrayIcon(pixmap);

}


QPixmap UtilityIconPainting::buildGrayIcon(const QPixmap& sourceIcon) {

    QImage clearImage = sourceIcon.toImage();
    KIconEffect::toGray(clearImage, 0.60);
    KIconEffect::deSaturate(clearImage, 1);
    return QPixmap::fromImage(clearImage);

}

QPixmap UtilityIconPainting::buildClearIcon(const QPixmap& sourceIcon) {

    QImage clearImage = sourceIcon.toImage();
    KIconEffect::toGamma(clearImage, 0.80);
    return QPixmap::fromImage(clearImage);

}


QPixmap UtilityIconPainting::buildSemiTransparentIcon(const QString& sourceIconStr) {

     QPixmap pixmap = KIconLoader::global()->loadIcon(sourceIconStr, KIconLoader::Small);
     return this->instance->buildSemiTransparentIcon(pixmap);

}

QPixmap UtilityIconPainting::buildSemiTransparentIcon(const QPixmap& sourceIcon) {

    QImage clearImage = sourceIcon.toImage();
    KIconEffect::semiTransparent(clearImage);
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


