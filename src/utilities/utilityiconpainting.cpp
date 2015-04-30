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

#include "kwooty_debug.h"
#include <KIconEffect>
#include "kcolorutils.h"

#include <QApplication>
#include <QPainter>

#include "preferences/preferencesserver.h"

UtilityIconPainting::UtilityIconPainting() : QObject(qApp)
{

    // build map in order to display status icon near to each child file name item :
    mStatusIconStrMap.insert(DownloadStatus,            "mail-receive");
    mStatusIconStrMap.insert(DownloadFinishStatus,      "mail-mark-unread");
    mStatusIconStrMap.insert(IdleStatus,                "mail-mark-unread");
    mStatusIconStrMap.insert(PauseStatus,               "mail-mark-unread");
    mStatusIconStrMap.insert(PausingStatus,             "mail-mark-unread");
    mStatusIconStrMap.insert(WaitForPar2IdleStatus,     "mail-mark-unread-new");
    mStatusIconStrMap.insert(ScanStatus,                "mail-mark-unread");
    mStatusIconStrMap.insert(DecodeStatus,              "mail-mark-unread");
    mStatusIconStrMap.insert(DecodeFinishStatus,        "mail-mark-read");
    mStatusIconStrMap.insert(DecodeErrorStatus,         "edit-delete");
    mStatusIconStrMap.insert(VerifyStatus,              "mail-mark-read");
    mStatusIconStrMap.insert(VerifyFoundStatus,         "dialog-ok-apply");
    mStatusIconStrMap.insert(VerifyMatchStatus,         "dialog-ok-apply");
    mStatusIconStrMap.insert(VerifyMissingStatus,       "edit-delete");
    mStatusIconStrMap.insert(VerifyDamagedStatus,       "edit-delete");
    mStatusIconStrMap.insert(RepairStatus,              "mail-mark-read");
    mStatusIconStrMap.insert(RepairNotPossibleStatus,   "edit-delete");
    mStatusIconStrMap.insert(RepairFailedStatus,        "edit-delete");
    mStatusIconStrMap.insert(ExtractStatus,             "dialog-ok-apply");
    mStatusIconStrMap.insert(ExtractBadCrcStatus,       "edit-delete");
    mStatusIconStrMap.insert(ExtractSuccessStatus,      "dialog-ok-apply");
    mStatusIconStrMap.insert(ExtractFailedStatus,       "edit-delete");

    // build map in order to display status icon near to each parent nzb file name item :
    mParentStatusIconStrMap.insert(IdleStatus,           "go-next-view");
    mParentStatusIconStrMap.insert(PauseStatus,          "go-next-view");
    mParentStatusIconStrMap.insert(PausingStatus,        "go-next-view");
    mParentStatusIconStrMap.insert(DownloadStatus,       "go-next-view");
    mParentStatusIconStrMap.insert(VerifyFinishedStatus, "dialog-ok");
    mParentStatusIconStrMap.insert(RepairFinishedStatus, "dialog-ok");
    mParentStatusIconStrMap.insert(ExtractFinishedStatus, "dialog-ok");
    mParentStatusIconStrMap.insert(DownloadFinishStatus, "go-next-view");

    // build a map to store every used icons in memory according to their names :
    foreach (const QString &iconName, mStatusIconStrMap.values()) {
        mIconStrIconImageMap.insert(iconName, QIcon::fromTheme(iconName));
    }

    foreach (const QString &iconName, mParentStatusIconStrMap.values()) {
        mIconStrIconImageMap.insert(iconName, QIcon::fromTheme(iconName));
    }
    mIconStrIconImageMap.insert("mail-reply-list",       QIcon::fromTheme("mail-reply-list"));
    mIconStrIconImageMap.insert("mail-mark-important",   QIcon::fromTheme("mail-mark-important"));
    mIconStrIconImageMap.insert("mail-mark-important",   QIcon::fromTheme("mail-mark-important"));
    mIconStrIconImageMap.insert("dialog-warning",        QIcon::fromTheme("dialog-warning"));
    mIconStrIconImageMap.insert("dialog-cancel",         QIcon::fromTheme("dialog-cancel"));

    // add semi-transparent effect to icon for Idle status :
    mIconStrIconImageMap.insert("go-next-view-transparent", QIcon(this->instance->buildSemiTransparentIcon("go-next-view")));
}

UtilityIconPainting::~UtilityIconPainting() {}

UtilityIconPainting *UtilityIconPainting::instance = 0;
UtilityIconPainting *UtilityIconPainting::getInstance()
{

    if (!instance) {
        instance = new UtilityIconPainting();
    }

    return instance;

}

bool UtilityIconPainting::retrieveParentIconFromStatus(const UtilityNamespace::ItemStatus &status, QIcon &icon)
{

    icon = this->mIconStrIconImageMap.value(this->mParentStatusIconStrMap.value(status));
    return this->mParentStatusIconStrMap.contains(status);

}

bool UtilityIconPainting::retrieveChildIconFromStatus(const UtilityNamespace::ItemStatus &status, QIcon &icon)
{

    icon = this->mIconStrIconImageMap.value(this->mStatusIconStrMap.value(status));
    return this->mStatusIconStrMap.contains(status);

}

bool UtilityIconPainting::retrieveIconFromString(const QString &iconName, QIcon &icon)
{

    icon = this->mIconStrIconImageMap.value(iconName);
    return this->mIconStrIconImageMap.contains(iconName);

}

QPixmap UtilityIconPainting::blendOverLayEmblem(const QString &overlayIconStr, const QPixmap *pixmap)
{

    QPixmap finalIcon;

    QIcon overlayIcon = QIcon::fromTheme(overlayIconStr);

    if (!overlayIcon.isNull() && pixmap) {

        QPixmap warningPixmap = overlayIcon.pixmap(8, 8);

        finalIcon = pixmap->copy();
        QPainter p(&finalIcon);
        p.drawPixmap(KIconLoader::SizeSmall / 2, KIconLoader::SizeSmall  / 2, warningPixmap);
        p.end();

    }

    return finalIcon;

}

QPixmap UtilityIconPainting::blendOverLayEmblem(const QString &overlayIconStr, const QIcon &icon)
{

    QPixmap pixmap = icon.pixmap(KIconLoader::SizeSmall);

    return UtilityIconPainting::getInstance()->blendOverLayEmblem(overlayIconStr, &pixmap);

}

QPixmap UtilityIconPainting::buildNormalIcon(const QString &sourceIconStr)
{

    return KIconLoader::global()->loadIcon(sourceIconStr, KIconLoader::Small);

}

QPixmap UtilityIconPainting::buildGrayIcon(const QString &sourceIconStr)
{

    QPixmap pixmap = KIconLoader::global()->loadIcon(sourceIconStr, KIconLoader::Small);
    return this->instance->buildGrayIcon(pixmap);

}

QPixmap UtilityIconPainting::buildGrayIcon(const QPixmap &sourceIcon)
{

    QImage clearImage = sourceIcon.toImage();
    KIconEffect::toGray(clearImage, 0.60);
    KIconEffect::deSaturate(clearImage, 1);
    return QPixmap::fromImage(clearImage);

}

QPixmap UtilityIconPainting::buildClearIcon(const QPixmap &sourceIcon)
{

    QImage clearImage = sourceIcon.toImage();
    KIconEffect::toGamma(clearImage, 0.80);
    return QPixmap::fromImage(clearImage);

}

QPixmap UtilityIconPainting::buildSemiTransparentIcon(const QString &sourceIconStr)
{

    QPixmap pixmap = KIconLoader::global()->loadIcon(sourceIconStr, KIconLoader::Small);
    return this->instance->buildSemiTransparentIcon(pixmap);

}

QPixmap UtilityIconPainting::buildSemiTransparentIcon(const QPixmap &sourceIcon)
{

    QImage clearImage = sourceIcon.toImage();
    KIconEffect::semiTransparent(clearImage);
    return QPixmap::fromImage(clearImage);

}

QPixmap UtilityIconPainting::blendOverLayTopRight(const QString &mainIconStr, const QString &overlayIconStr)
{

    QPixmap pixmap;
    QString keyStr = mainIconStr + overlayIconStr;

    if (UtilityIconPainting::getInstance()->mTextIconMap.contains(keyStr)) {

        pixmap = UtilityIconPainting::getInstance()->mTextIconMap.value(keyStr);

    } else {

        QIcon overlayIcon = QIcon::fromTheme(overlayIconStr);

        if (!overlayIcon.isNull()) {

            pixmap = KIconLoader::global()->loadIcon(mainIconStr, KIconLoader::Small, KIconLoader::SizeSmall);

            QPixmap overlayPixmap = KIconLoader::global()->loadIcon(overlayIconStr, KIconLoader::Small, KIconLoader::SizeSmall);
            overlayPixmap = overlayPixmap.scaled(10, 10, Qt::IgnoreAspectRatio, Qt::FastTransformation);

            QPainter p(&pixmap);
            p.drawPixmap(6, 0, overlayPixmap);
            p.end();

            UtilityIconPainting::getInstance()->mTextIconMap.insert(keyStr, pixmap);
        }

    }

    return pixmap;
}

QLabel *UtilityIconPainting::buildLighterTextLabel(const QString &text, QWidget *parentWidget)
{

    QLabel *textLabel = new QLabel(text, parentWidget);

    QPalette textLabelPalette = textLabel->palette();
    QColor fgcolor = KColorUtils::tint(textLabelPalette.color(QPalette::Disabled, QPalette::WindowText),
                                       textLabelPalette.color(QPalette::Active, QPalette::WindowText),
                                       0.6);

    textLabelPalette.setColor(QPalette::WindowText, fgcolor);

    textLabel->setPalette(textLabelPalette);

    return textLabel;
}

QColor UtilityIconPainting::lighterColor(const qreal &amount)
{

    QPalette textLabelPalette = qApp->palette();
    return KColorUtils::tint(textLabelPalette.color(QPalette::Disabled, QPalette::WindowText),
                             textLabelPalette.color(QPalette::Active, QPalette::WindowText),
                             amount);

}

void UtilityIconPainting::displayLighterText(QStandardItem *item)
{

    QBrush itemBrush = item->foreground();
    itemBrush.setColor(this->lighterColor(0.4));
    item->setForeground(itemBrush);

}

