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


#include "serverpreferenceswidget.h"

#include <KMessageBox>
#include <QUuid>

#include "preferences/preferencesserver.h"
#include "preferences/kconfiggrouphandler.h"
#include "widgets/servertabwidget.h"
#include "kwootysettings.h"

#include "utility.h"
using namespace UtilityNamespace;

ServerPreferencesWidget::ServerPreferencesWidget(ServerTabWidget* parent,
                                                 PreferencesServer* preferencesServer,
                                                 int tabIndex,
                                                 const ServerTabWidget::ServerNameQuery serverNameQuery) : QWidget(parent) {


    this->serverTabWidget = parent;
    this->preferencesServer = preferencesServer;
    this->tabIndex = tabIndex;

    this->serverSettingsUi = new Ui_ServerSettings();
    this->serverSettingsUi->setupUi(this);


    this->hideWidgets(tabIndex);
    this->setupButtons();

    this->setupConnections();

    // ensure that default values are diplayed if a new tab has been created by user :
    if (serverNameQuery == ServerTabWidget::AskServerName) {
        // set a tab index that will no be found by kconfig in order to return default values for new tab creation :
        tabIndex = -1;
    }


    this->setData(tabIndex);



}


ServerPreferencesWidget::~ServerPreferencesWidget() {

    kDebug();
    delete this->serverSettingsUi;
}


int ServerPreferencesWidget::getTabIndex() {
    return this->tabIndex;
}


void ServerPreferencesWidget::setupButtons() {

    this->serverSettingsUi->pushButtonInfo->setIcon(KIcon("system-help"));

    QMap<int, QString> comboBoxIconTextMap = this->serverTabWidget->getComboBoxIconTextMap();

    foreach(QString pattern, comboBoxIconTextMap.values()) {

        QStringList patternList = pattern.split(";");
        this->serverSettingsUi->comboBoxServerMode->addItem(KIcon(patternList.value(0)), patternList.value(1));
    }

}


void ServerPreferencesWidget::setupConnections() {

    // check/uncheck ssl checkbox according to port value:
    connect (this->serverSettingsUi->port, SIGNAL(valueChanged (int)), this, SLOT(portValueChangedSlot(int)));

    connect (this->serverSettingsUi->comboBoxServerMode, SIGNAL(currentIndexChanged(int)), this, SLOT(serverModeValueChangedSlot(int)));


    // enable apply button when a setting has been changed :
    connect (this->serverSettingsUi->port, SIGNAL(valueChanged (int)), this, SLOT(valueChangedSlot()));
    connect (this->serverSettingsUi->connectionNumber, SIGNAL(valueChanged (int)), this, SLOT(valueChangedSlot()));
    connect (this->serverSettingsUi->disconnectTimeout, SIGNAL(valueChanged (int)), this, SLOT(valueChangedSlot()));

    connect (this->serverSettingsUi->hostName, SIGNAL(textChanged (const QString&)), this, SLOT(valueChangedSlot()));
    connect (this->serverSettingsUi->login, SIGNAL(textChanged (const QString&)), this, SLOT(valueChangedSlot()));
    connect (this->serverSettingsUi->password, SIGNAL(textChanged (const QString&)), this, SLOT(valueChangedSlot()));

    connect (this->serverSettingsUi->groupBoxAuthentication, SIGNAL(clicked (bool)), this, SLOT(valueChangedSlot()));
    connect (this->serverSettingsUi->enableSSL, SIGNAL(stateChanged(int)), this, SLOT(valueChangedSlot()));

    connect (this->serverSettingsUi->comboBoxServerMode, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChangedSlot()));

    // display information box when pushButtonInfo is clicked :
    connect (this->serverSettingsUi->pushButtonInfo, SIGNAL(clicked (bool)), this, SLOT(pushButtonInfoClickedSlot()));

}


void ServerPreferencesWidget::setGroupBoxTitle(const int& index) {

    QString serverStr;
    switch (index) {

    case 0: {
            serverStr = i18n("Master");
            break;
        }
    case 1: {
            serverStr = i18n("First backup");
            break;
        }
    case 2: {
            serverStr = i18n("Second backup");
            break;
        }
    case 3: {
            serverStr = i18n("Third backup");
            break;
        }
    case 4: {
            serverStr = i18n("Fourth backup");
            break;
        }
    default: {
            break;
        }

    }

    this->serverSettingsUi->groupBoxConfig->setTitle(i18n("%1 server configuration", serverStr));

}



void ServerPreferencesWidget::setData(const int& tabIndex) {

    // set all previously stored settings to widgets :
    ServerData serverData = KConfigGroupHandler::getInstance()->readServerSettings(tabIndex);

    this->serverSettingsUi->hostName->setText(serverData.getHostName());
    this->serverSettingsUi->login->setText(serverData.getLogin());
    this->serverSettingsUi->password->setText(serverData.getPassword());

    this->serverSettingsUi->port->setValue(serverData.getPort());
    this->serverSettingsUi->connectionNumber->setValue(serverData.getConnectionNumber());
    this->serverSettingsUi->disconnectTimeout->setValue(serverData.getDisconnectTimeout());

    this->serverSettingsUi->groupBoxAuthentication->setChecked(serverData.isAuthentication());
    this->serverSettingsUi->enableSSL->setChecked(serverData.isEnableSSL());

    this->serverSettingsUi->comboBoxServerMode->setCurrentIndex(serverData.getServerModeIndex());
}


ServerData ServerPreferencesWidget::getData() {

    // get all settings from widgets :
    ServerData serverData;

    serverData.setHostName(this->serverSettingsUi->hostName->text());
    serverData.setLogin(this->serverSettingsUi->login->text());
    serverData.setPassword(this->serverSettingsUi->password->text());

    serverData.setPort(this->serverSettingsUi->port->value());
    serverData.setConnectionNumber(this->serverSettingsUi->connectionNumber->value());
    serverData.setDisconnectTimeout(this->serverSettingsUi->disconnectTimeout->value());

    serverData.setAuthentication(this->serverSettingsUi->groupBoxAuthentication->isChecked());
    serverData.setEnableSSL(this->serverSettingsUi->enableSSL->isChecked());

    serverData.setServerModeIndex(this->serverSettingsUi->comboBoxServerMode->currentIndex());

    return serverData;

}



void ServerPreferencesWidget::valueChangedSlot() {
    preferencesServer->kcfg_serverChangesNotify->setText(QUuid::createUuid().toString());
}


void ServerPreferencesWidget::portValueChangedSlot(int portValue) {

    // if ports usually used for SSL are met :
    if (portValue == 563 || portValue == 443) {
        this->serverSettingsUi->enableSSL->setCheckState(Qt::Checked);
    }
    // else is ports usually used for normal connections are met :
    else {
        this->serverSettingsUi->enableSSL->setCheckState(Qt::Unchecked);
    }

}


void ServerPreferencesWidget::serverModeValueChangedSlot(int serverModeIndex) {

    this->serverTabWidget->setServerTabIcon(this->tabIndex, serverModeIndex);

    if (serverModeIndex == UtilityNamespace::DisabledServer) {
        this->enableWidgets(false);
    }
    else {
        this->enableWidgets(true);
    }


}



void ServerPreferencesWidget::enableWidgets(const bool& enable) {

    this->serverSettingsUi->hostName->setEnabled(enable);
    this->serverSettingsUi->login->setEnabled(enable);
    this->serverSettingsUi->password->setEnabled(enable);

    this->serverSettingsUi->port->setEnabled(enable);
    this->serverSettingsUi->connectionNumber->setEnabled(enable);
    this->serverSettingsUi->disconnectTimeout->setEnabled(enable);

    this->serverSettingsUi->groupBoxAuthentication->setEnabled(enable);
    this->serverSettingsUi->enableSSL->setEnabled(enable);

    this->serverSettingsUi->labelHost->setEnabled(enable);
    this->serverSettingsUi->labelConnections->setEnabled(enable);
    this->serverSettingsUi->labelPort->setEnabled(enable);
    this->serverSettingsUi->labelLogin->setEnabled(enable);
    this->serverSettingsUi->labelPassword->setEnabled(enable);
    this->serverSettingsUi->labelDisconnect->setEnabled(enable);
}



void ServerPreferencesWidget::hideWidgets(const int& tabIndex) {

    if (tabIndex == 0) {

        this->serverSettingsUi->labelServerMode->hide();
        this->serverSettingsUi->comboBoxServerMode->hide();
        this->serverSettingsUi->pushButtonInfo->hide();

    }

}




void ServerPreferencesWidget::pushButtonInfoClickedSlot() {

    QString divStyle = "<div style=\"margin-left: 20px; margin-top: 3px; margin-bottom: 10px\">";
    QString text;

    text.append(i18n("<b>%1</b>","Backup Server Mode:"));
    text.append(divStyle);
    text.append(i18n("<i>%1</i> %2<br>", "Passive:", "download only files not found on Master server"));
    text.append(i18n("<i>%1</i> %2<br>", "Active:", "download files on par with Master server"));
    text.append(i18n("<i>%1</i> %2", "Server Disabled:", "server not used"));
    text.append("</div>");

    text.append(i18n("<b>%1</b>", "Server priority:"));
    text.append(divStyle);
    text.append(i18n("Drag and drop tabs to manage backup server priority in <i>Passive</i> mode"));
    text.append("</div>");

    text.append(i18n("<b>%1</b>", "Tab renaming:"));
    text.append(divStyle);
    text.append(i18n("Double click on current tab for tab renaming"));
    text.append("</div>");


    KMessageBox::information(this, text, i18n("Backup Server hints"));

}








