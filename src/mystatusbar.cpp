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

#include "widgets/icontextwidget.h"
#include "widgets/iconcapacitywidget.h"

#include "settings.h"


MyStatusBar::MyStatusBar(QWidget* parent) : KStatusBar(parent)
{

    iconLoader = KIconLoader::global();

    this->resetVariables();

    // create connection widget at bottom left of status bar :
    this->setConnectionWidget();


    // create remaining time widget at bottom left of status bar :
    this->setTimeInfoWidget();

    // create shutdown widget at bottom left of status bar :
    this->setShutdownWidget();


    // add capacity bar widget :
    this->iconCapacityWidget = new IconCapacityWidget(this);
    this->addPermanentWidget(this->iconCapacityWidget);

    // add remaining download size item :
    this->sizeLabel = new QLabel(this);
    this->addPermanentWidget(this->sizeLabel);

    // add download speed item :
    this->speedLabel = new QLabel(this);
    this->addPermanentWidget(this->speedLabel);


}

MyStatusBar::MyStatusBar() {}
MyStatusBar::~MyStatusBar() {}





void MyStatusBar::resetVariables(){
    this->totalConnections = 0;
    this->sslActive = false;
    this->nttpErrorStatus = NoError;
}


void MyStatusBar::setConnectionWidget(){

    this->connectionWidget = new IconTextWidget(this);

    // add aggregated widget to the status bar :
    this->addWidget(this->connectionWidget);

    // set connection not active by default :
    this->setConnectionActive();


}


void MyStatusBar::setTimeInfoWidget(){

    this->timeInfoWidget = new IconTextWidget(this);
    this->timeInfoWidget->setIcon("user-away");

    // add aggregated widget to the status bar :
    this->addWidget(this->timeInfoWidget);

}


void MyStatusBar::setShutdownWidget(){

    this->shutdownWidget = new IconTextWidget(this);

    // add aggregated widget to the status bar :
    this->addWidget(this->shutdownWidget);

}




void MyStatusBar::setConnectionActive(){

    QString connectionIconStr;
    QString connection;

    // iinitialize icon loader
    iconLoader->newIconLoader();

    if (this->totalConnections == 0) {
        connectionIconStr ="weather-clear-night";
        connection = i18n("Disconnected");

        // detail disconnection issues to user :
        if (nttpErrorStatus == HostNotFound){
            connection = i18n("Disconnected (Host not found)");
        }

        if (nttpErrorStatus == ConnectionRefused) {
            connection = i18n("Disconnected (Connection refused)");
        }

        if (nttpErrorStatus == RemoteHostClosed) {
            connection = i18n("Disconnected (Closed by remote host)");
        }

        if (nttpErrorStatus == SslHandshakeFailed) {
            connection = i18n("Disconnected (SSL handshake failed)");
        }

        //kDebug() << "nttpErrorStatus = " << nttpErrorStatus;
        if (nttpErrorStatus == AuthenticationNeeded) {
            connection = i18n("Disconnected (Authentication required)");
        }


        if (nttpErrorStatus == AuthenticationFailed) {
            connection = i18n("Disconnected (Authentication Denied)");
        }

    }
    else{
        // set connection icon :
        connectionIconStr = "applications-internet";
        connection = i18n("Connected: ") + QString::number(this->totalConnections);

        if (this->sslActive) {

            // if SSL active use another connection icon :
            connectionIconStr = "document-encrypt";

            // display type of encryption method used by server :
            if (!encryptionMethod.isEmpty()) {
                connection = connection + " :: " + this->encryptionMethod;
            }

        }

    }


    connectionWidget->setIcon(connectionIconStr);
    connectionWidget->setText(connection);


    // set tooltip to connection widget :
    this->buildConnWidgetToolTip(connection);



}


void MyStatusBar::buildConnWidgetToolTip(const QString& connection) {

    QString toolTipStr;

    // if totalConnections == 0, client is disconnected :
    if (this->totalConnections == 0) {
        toolTipStr.append(connection);
    }

    else {
        // set host name info :
        toolTipStr.append(i18n("Connected to ") + Settings::hostName() + "<br>");

        // set SSL connection info :
        if (this->sslActive) {

            toolTipStr.append(i18n("Connection is SSL encrypted"));

            if (!encryptionMethod.isEmpty()) {
                toolTipStr.append(": " + this->encryptionMethod);
            }

            toolTipStr.append("<br>");

            if (this->certificateVerified) {
                toolTipStr.append(i18n("Certificate <b>verified</b> by ") + this->issuerOrgranisation);
            }
            else {
                toolTipStr.append(i18n("Certificate <b>can not be verified</b>"));
            }

        }
        else {

            toolTipStr.append("Connection is not encrypted");
        }

    }

    // set tooltip :
    this->connectionWidget->setToolTip(toolTipStr);
}





void MyStatusBar::updateSizeInfoSlot(const QString sizeStr) {

    this->sizeLabel->setText(sizeStr);
}



void MyStatusBar::updateDownloadSpeedInfoSlot(const QString speedInKBStr){

    this->speedLabel->setText(i18n("Speed: ") + speedInKBStr);

}

void MyStatusBar::updateFreeSpaceSlot(const UtilityNamespace::FreeDiskSpace diskSpaceStatus, const QString availableVal, const int usedDiskPercentage) {

    // diskspace is unknown, hide widgets :
    if (diskSpaceStatus == UnknownDiskSpace) {

        this->iconCapacityWidget->hide();
    }

    else {

        // if capacity bar was hidden because diskSpaceStatus was previously UnknownDiskSpace :
        if (this->iconCapacityWidget->isHidden()) {
            this->iconCapacityWidget->show();
        }

        // if free disk space is not sufficient display warning icon :
        if (diskSpaceStatus == InsufficientDiskSpace) {

            this->iconCapacityWidget->setIcon("dialog-warning");
            this->iconCapacityWidget->setToolTip(i18n("Insufficient disk space"));

        }

        // if free disk space is not sufficient do not display icon :
        if (diskSpaceStatus == SufficientDiskSpace) {

            this->iconCapacityWidget->setIcon(QString());
            this->iconCapacityWidget->setToolTip(QString());

        }

        // set text and repaint widget :
        this->iconCapacityWidget->updateCapacity(availableVal, usedDiskPercentage);

    }



}






void MyStatusBar::updateTimeInfoSlot(const QString timeStr, const QString timeToolTip, const bool parentDownloadingFound) {

    this->timeInfoWidget->setText(timeStr);
    this->timeInfoWidget->setToolTip(timeToolTip);

    // if download is not active, hide the widget :
    if (!parentDownloadingFound) {
        this->timeInfoWidget->hide();
    }
    // else display it :
    else if (this->timeInfoWidget->isHidden()) {
        this->timeInfoWidget->show();
    }


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

void MyStatusBar::encryptionStatusSlot(const bool sslActive, const QString encryptionMethod, const bool certificateVerified, const QString issuerOrgranisation){

    //kDebug() << "sslActive : " << sslActive << "encryptionMethod" << encryptionMethod;

    this->encryptionMethod = encryptionMethod;
    this->sslActive = sslActive;
    this->certificateVerified = certificateVerified;
    this->issuerOrgranisation = issuerOrgranisation;

    this->setConnectionActive();

}



void MyStatusBar::statusBarShutdownInfoSlot(QString iconStr, QString text) {

    this->shutdownWidget->setIcon(iconStr);
    this->shutdownWidget->setText(text);

}



