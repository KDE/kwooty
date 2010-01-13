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

#include "mystatusbar.h"

#include <KAboutData>
#include <KIcon>
#include <KDebug>

#include <QHBoxLayout>


MyStatusBar::MyStatusBar(QWidget* parent) : KStatusBar(parent)
{

    iconLoader = KIconLoader::global() ;

    this->resetVariables();

    // create connection widget at bottom left of status bar :
    this->setConnectionWidget();

    // create shutdown widget at bottom left of status bar :
    this->setShutdownWidget();

    // add file counter item :
    this->insertPermanentItem("", FILES_NUMBER_ID);
    // add remaining download size item :
    this->insertPermanentItem("", SIZE_ID);
    // add download speed item :
    this->insertPermanentItem("", SPEED_ID);
    // init display to default at startup :
    this->fullFileSizeUpdate(0, 0);
    this->updateDownloadSpeedSlot();

    // set timer to compute download speed every each SPEED_AVERAGE_SECONDS :
    downloadSpeedTimer = new QTimer(this);
    downloadSpeedTimer->start(SPEED_AVERAGE_SECONDS * 1000);
    connect(downloadSpeedTimer, SIGNAL(timeout()), this, SLOT(updateDownloadSpeedSlot()));

}

MyStatusBar::MyStatusBar()
{
}

MyStatusBar::~MyStatusBar()
{
}



void MyStatusBar::resetVariables(){
    this->totalFiles = 0;
    this->totalSize = 0;
    this->totalConnections = 0;
    this->totalBytesDownloaded = 0;
    this->sslActive = false;
    this->nttpErrorStatus = NoError;
}


void MyStatusBar::setConnectionWidget(){

    this->connIconLabel = new QLabel(this);
    this->connTextLabel = new QLabel(this);

    // set connection not active by default :
    this->setConnectionActive();

    this->addWidgetToLayout(this->connIconLabel, this->connTextLabel);


}


void MyStatusBar::setShutdownWidget(){

    this->shutdownIconLabel = new QLabel(this);
    this->shutdownTextLabel = new QLabel(this);

    this->addWidgetToLayout(this->shutdownIconLabel, this->shutdownTextLabel);

}


void MyStatusBar::addWidgetToLayout(QLabel* iconLabel, QLabel* textLabel){

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);
    layout->setMargin(0);
    layout->setSpacing(2);

    QWidget* globalWidget = new QWidget(this);
    globalWidget->setLayout(layout);

    // add aggregated widget to the status bar :
    this->addWidget(globalWidget);

}


void MyStatusBar::setConnectionActive(){

    QPixmap connectionPixmap;
    QString connection;

    // iinitialize icon loader
    iconLoader->newIconLoader();

    if (totalConnections == 0) {
        connectionPixmap = iconLoader->loadIcon("weather-clear-night", KIconLoader::Small);
        connection = i18n(" Disconnected");

        // detail disconnection issues to user :
        if (nttpErrorStatus == HostNotFound){
            connection = i18n(" Disconnected (Host not found)");
        }

        if (nttpErrorStatus == ConnectionRefused) {
            connection = i18n(" Disconnected (Connection refused)");
        }

        if (nttpErrorStatus == RemoteHostClosed) {
            connection = i18n(" Disconnected (Closed by remote host)");
        }

        if (nttpErrorStatus == SslHandshakeFailed) {
            connection = i18n(" Disconnected (SSL handshake failed)");
        }

        //kDebug() << "nttpErrorStatus = " << nttpErrorStatus;
        if (nttpErrorStatus == AuthenticationNeeded) {
            connection = i18n(" Disconnected (Authentication required)");
        }


        if (nttpErrorStatus == AuthenticationFailed) {
            connection = i18n(" Disconnected (Authentication Denied)");
        }

    }
    else{
        // set connection icon :
        connectionPixmap = iconLoader->loadIcon("applications-internet", KIconLoader::Small);
        connection = i18n(" Connected: ") + QString::number(totalConnections);

        if (this->sslActive) {

            // if SSL active use another connection icon :
            connectionPixmap = iconLoader->loadIcon("document-encrypt", KIconLoader::Small);

            // display type of encryption method used by server :
            if (!encryptionMethod.isEmpty()) {
                connection = connection + " :: " + this->encryptionMethod;
            }

        }

    }


    connIconLabel->setPixmap(connectionPixmap);
    connTextLabel->setText(connection);

}




void MyStatusBar::addSize(const quint64 size){
    totalSize += size;
    updateSizeText();
}


void MyStatusBar::addFiles(const quint64 fileNumber){
    totalFiles += fileNumber;
    updateFileText();
}


void MyStatusBar::updateSizeText() {
    QString sizeStr = Utility::convertByteHumanReadable(totalSize);
    this->changeItem(i18n("Size: ") + sizeStr, SIZE_ID);
}


void MyStatusBar::updateFileText() {
    this->changeItem(i18n("Files: ") + QString::number(totalFiles), FILES_NUMBER_ID);
}




void MyStatusBar::fullFileSizeUpdate(const quint64 size, const quint64 files) {

    totalSize = size;
    totalFiles = files;

    // status bar updates :
    updateFileText();
    updateSizeText();

}


void MyStatusBar::decrementSlot(const quint64 size, const int fileNumber = 1) {

    totalFiles -= fileNumber;
    totalSize -= size;

    // status bar updates :
    updateFileText();
    updateSizeText();

}


void MyStatusBar::connectionStatusSlot(const int connectionStatus){

    if (connectionStatus == Connected){
        totalConnections++;
    }

    if (connectionStatus == Disconnected){
        totalConnections--;
    }

    this->setConnectionActive();

}


void MyStatusBar::nntpErrorSlot(const int nttpErrorStatus){

    this->nttpErrorStatus = nttpErrorStatus;
    this->setConnectionActive();

}

void MyStatusBar::encryptionStatusSlot(const bool sslActive, const QString encryptionMethod){

    //kDebug() << "sslActive : " << sslActive << "encryptionMethod" << encryptionMethod;

    this->encryptionMethod = encryptionMethod;
    this->sslActive = sslActive;
    this->setConnectionActive();

}




void MyStatusBar::speedSlot(const int bytesDownloaded){

    totalBytesDownloaded += bytesDownloaded;

}


void MyStatusBar::updateDownloadSpeedSlot(){

    int downloadSizeInKB = totalBytesDownloaded / 1024 / SPEED_AVERAGE_SECONDS;
    QString sizeUnit = i18n(" KiB/s");
    this->changeItem(i18n("Speed: ") + QString::number(downloadSizeInKB) + sizeUnit, SPEED_ID);

    // reset number of bytes downloaded after text update :
    totalBytesDownloaded = 0;
}



void MyStatusBar::statusBarShutdownInfoSlot(QString iconStr, QString text) {

    QPixmap connectionPixmap;

    if (!iconStr.isEmpty()) {
        // initialize icon loader
        iconLoader->newIconLoader();
        connectionPixmap = iconLoader->loadIcon(iconStr, KIconLoader::Small);
    }

    this->shutdownIconLabel->setPixmap(connectionPixmap);
    this->shutdownTextLabel->setText(text);

}



