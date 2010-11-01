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


#include "kconfiggrouphandler.h"

#include <KGlobal>
#include <KDebug>
#include <KMessageBox>

#include <QApplication>

#include "mainwindow.h"
#include "centralwidget.h"
#include "kwootysettings.h"
#include "preferences/preferencesserver.h"
#include "utility.h"
using namespace UtilityNamespace;


KConfigGroupHandler::KConfigGroupHandler(MainWindow* mainwindow) : QObject(qApp) {

    this->mainWindow = mainwindow;
    this->wallet = 0;
    this->dialogButtonCode = 0;
    this->useKwallet = Settings::useKwallet();

    instance = this;

}

KConfigGroupHandler* KConfigGroupHandler::instance = 0;
KConfigGroupHandler* KConfigGroupHandler::getInstance() {
    return instance;
}




KConfigGroupHandler::~KConfigGroupHandler() {
    if (this->wallet) {
        delete this->wallet;
    }
}


void KConfigGroupHandler::settingsChangedSlot() {

    // switch passwords between config file and kwallet :
    if (this->useKwallet != Settings::useKwallet() && this->openWallet()) {       

        // avoid to display dialog box during synchronization :
        this->dialogButtonCode = KMessageBox::Ok;

        for (int serverId = 0; serverId < this->readServerNumberSettings(); serverId++) {

            // set old value right now :
            this->useKwallet = !Settings::useKwallet();

            KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));

            // read password from plain text or kwallet :
            QString password = this->readPassword(serverId, configGroup);

            // set new value right now :
            this->useKwallet = Settings::useKwallet();

            // write password kwallet or plain text :
            this->writePassword(serverId, configGroup, password);

        }

        // store new setting value :
        this->useKwallet = Settings::useKwallet();

        this->dialogButtonCode = 0;
    }

}





//======================================================================================//
//                                    KWallet :                                         //
//======================================================================================//


void KConfigGroupHandler::openWalletFails() {

    if (this->dialogButtonCode == 0) {


        this->dialogButtonCode = KMessageBox::Cancel;
        this->dialogButtonCode = KMessageBox::messageBox(0,
                                                         KMessageBox::Sorry,
                                                         i18n("No running KWallet found. Passwords will be saved in plaintext."),
                                                         i18n("KWallet is not running"));

        // disable kwallet usage :
        Settings::setUseKwallet(false);
        this->useKwallet = Settings::useKwallet();

        if (this->dialogButtonCode == KMessageBox::Ok) {
            this->dialogButtonCode = 0;
        }

    }

}


bool KConfigGroupHandler::openWallet() {

    bool walletOpen = false;

    if (this->mainWindow) {

        // open the wallet if not open :
        if (!this->wallet) {
            this->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), this->mainWindow->winId());
        }

        // if wallet has been successfully open :
        if (this->wallet) {

            // check if kwooty's entry exists :
            QString entryStr = "kwooty";

            // check if wallet is open and current folder properly set :
            if (this->wallet->isOpen() &&
                this->wallet->currentFolder() == entryStr) {
                walletOpen = true;
            }
            // create and set wallet in proper folder :
            else {

                // create the entry if it not already exists :
                if (!this->wallet->hasEntry(entryStr)) {
                    this->wallet->createFolder(entryStr);
                }

                // set the current working folder :
                if (!this->wallet->hasEntry(entryStr)) {
                    walletOpen = this->wallet->setFolder(entryStr);
                }
            }
        }
        // wallet can not be open :
        else {
            walletOpen = false;
        }

    }

    if (!walletOpen) {
        this->openWalletFails();

    }

    return walletOpen;

}


QString KConfigGroupHandler::readPassword(const int& serverId, KConfigGroup& configGroup) {

    QString password = "";

    // read password from kwallet if enabled in settings :
    if (this->useKwallet) {

        if (this->openWallet()) {

            QString passwordAlias = QString("PasswordServer_%1").arg(serverId);
            this->wallet->readPassword(passwordAlias, password);
        }

    }
    // read password from kconfig file :
    else {
        password = configGroup.readEntry("password", QString());
    }

    return password;
}


void KConfigGroupHandler::writePassword(const int& serverId, KConfigGroup& configGroup, const QString& password) {

    // write password to kwallet if enabled in settings :
    if (this->useKwallet) {

        if (this->openWallet()) {

            QString passwordAlias = QString("PasswordServer_%1").arg(serverId);
            int writeSuccessful = this->wallet->writePassword(passwordAlias, password);

            // password has been successfully written in wallet, remove eventual plain text password :
            if (writeSuccessful == 0) {
                this->removePasswordEntry(configGroup);
            }
        }
    }
    // write password to kconfig file :
    else {
        configGroup.writeEntry("password", password);
    }


}



//======================================================================================//
//                              Multi-server settings :                                 //
//======================================================================================//


ServerData KConfigGroupHandler::readServerSettings(const int& serverId) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));

    // if KConfigGroup is invalid, it may be the first launch from a previous kwooty version
    // try get previous settings :
    if ((serverId == MasterServer) && !configGroup.exists()) {
        configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("server"));
    }


    ServerData serverData;

    serverData.setServerId(configGroup.readEntry("serverId", serverId));
    serverData.setHostName(configGroup.readEntry("hostName", QString()));
    serverData.setServerName(configGroup.readEntry("serverName", QString()).remove("&"));
    serverData.setPort(configGroup.readEntry("port", 119));
    serverData.setConnectionNumber(configGroup.readEntry("connectionNumber", 4));
    serverData.setAuthentication(configGroup.readEntry("authentication", false));
    serverData.setLogin(configGroup.readEntry("login", QString()));  
    serverData.setDisconnectTimeout(configGroup.readEntry("disconnectTimeout", 5));
    serverData.setEnableSSL(configGroup.readEntry("enableSSL", false));
    serverData.setServerModeIndex(configGroup.readEntry("serverModeIndex", 0));

    // read password with Kwallet or kconfig file :
    serverData.setPassword(this->readPassword(serverId, configGroup));

    return serverData;

}



void KConfigGroupHandler::writeServerSettings(const int& serverId, ServerData serverData) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));

    configGroup.writeEntry("serverId", serverData.getServerId());
    configGroup.writeEntry("serverName", serverData.getServerName());
    configGroup.writeEntry("hostName", serverData.getHostName());
    configGroup.writeEntry("port", serverData.getPort());
    configGroup.writeEntry("connectionNumber", serverData.getConnectionNumber());
    configGroup.writeEntry("authentication", serverData.isAuthentication());
    configGroup.writeEntry("login", serverData.getLogin());
    configGroup.writeEntry("disconnectTimeout", serverData.getDisconnectTimeout());
    configGroup.writeEntry("enableSSL", serverData.isEnableSSL());
    configGroup.writeEntry("serverModeIndex", serverData.getServerModeIndex());

    // write password with Kwallet or kconfig file :
    this->writePassword(serverId, configGroup, serverData.getPassword());

    configGroup.sync();

}



void KConfigGroupHandler::writeServerNumberSettings(const int& serverNumber) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("NumberOfServers"));
    configGroup.writeEntry("serverNumber", serverNumber);
    configGroup.sync();
}


int KConfigGroupHandler::readServerNumberSettings() {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("NumberOfServers"));
    int serverNumber = configGroup.readEntry("serverNumber", 1);

    return qMin(PreferencesServer::MAX_SERVERS, serverNumber);

}


void KConfigGroupHandler::removeServerSettings(const int& serverId) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));

    if (configGroup.exists()) {
        configGroup.deleteGroup();
    }

}



void KConfigGroupHandler::removePasswordEntry(KConfigGroup& configGroup) {

    QString passwordStr = "password";
    // check if password entry exists :
    if (configGroup.hasKey(passwordStr)) {
        configGroup.deleteEntry(passwordStr);
    }

}





int KConfigGroupHandler::serverConnectionNumber(const int& serverId) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));
    return configGroup.readEntry("connectionNumber", 4);

}


QString KConfigGroupHandler::tabName(const int& serverId, const QString& tabText) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("Server_%1").arg(serverId));
    return configGroup.readEntry("serverName", tabText);
}




//======================================================================================//
//                              Side bar states :                                       //
//======================================================================================//

void KConfigGroupHandler::writeSideBarDisplay(const bool& display) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("SideBar"));
    configGroup.writeEntry("sideBarDisplay", display);
    configGroup.sync();
}


bool KConfigGroupHandler::readSideBarDisplay() {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("SideBar"));
    return configGroup.readEntry("sideBarDisplay", false);

}


void KConfigGroupHandler::writeSideBarTabOnlyDisplay(const bool& tabOnlyDisplay) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("SideBar"));
    configGroup.writeEntry("sideBarTabOnlyDisplay", tabOnlyDisplay);
    configGroup.sync();
}


bool KConfigGroupHandler::readSideBarTabOnlyDisplay() {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("SideBar"));
    return configGroup.readEntry("sideBarTabOnlyDisplay", false);

}





void KConfigGroupHandler::writeSideBarServerIndex(const int& index) {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("SideBar"));
    configGroup.writeEntry("sideBarServerIndex", index);
    configGroup.sync();
}


int KConfigGroupHandler::readSideBarServerIndex() {

    KConfigGroup configGroup = KConfigGroup(KGlobal::config(), QString::fromLatin1("SideBar"));
    return configGroup.readEntry("sideBarServerIndex", 0);

}





