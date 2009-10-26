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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <KIconLoader>
#include <KStatusBar>
#include <QLabel>
#include <QTimer>
#include "utility.h"
using namespace UtilityNamespace;

class MyStatusBar : public KStatusBar
{

    Q_OBJECT

public:
    MyStatusBar(QWidget* parent);
    MyStatusBar();
    ~MyStatusBar();
    void addSize(const quint64);
    void addFiles(const quint64);
    void fullFileSizeUpdate(const quint64, const quint64);


private:
    static const int SPEED_AVERAGE_SECONDS = 2;
    static const int FILES_NUMBER_ID = 1;
    static const int SIZE_ID = 2;
    static const int SPEED_ID = 3;

    KIconLoader* iconLoader;
    QLabel* connIconLabel;
    QLabel* connTextLabel;
    QLabel* filesLabel;
    QLabel* sizeLabel;
    QTimer* downloadSpeedTimer;
    QString encryptionMethod;
    quint64 totalFiles;
    quint64 totalSize;
    quint64 totalBytesDownloaded;
    int totalConnections;
    int nttpErrorStatus;
    bool sslActive;

    void updateSizeText();
    void updateFileText();
    void resetVariables();
    void setConnectionWidget();
    void setConnectionActive();



signals:

public slots:
    void decrementSlot(const quint64, const int);
    void connectionStatusSlot(const int);
    void encryptionStatusSlot(const bool, const QString);
    void nntpErrorSlot(const int);
    void speedSlot(const int);
    void updateDownloadSpeedSlot();

private slots:

};



#endif // STATUSBAR_H
