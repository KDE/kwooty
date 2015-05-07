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
#include <QIcon>

#include "preferences/preferencesserver.h"
#include "preferences/kconfiggrouphandler.h"
#include "widgets/servertabwidget.h"
#include "kwootysettings.h"

#include "utilities/utilityserverstatus.h"
#include "utilities/utilityiconpainting.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

ServerPreferencesWidget::ServerPreferencesWidget(ServerTabWidget *parent,
        PreferencesServer *preferencesServer,
        int tabIndex,
        const ServerTabWidget::ServerNameQuery serverNameQuery) : QWidget(parent)
{

    mServerTabWidget = parent;
    mPreferencesServer = preferencesServer;
    mTabIndex = tabIndex;

    mServerSettingsUi = new Ui_ServerSettings();
    mServerSettingsUi->setupUi(this);

    // hide server mode settings for master server :
    hideWidgets(tabIndex);

    // setup help push button and server modes combo box:
    setupButtons();

    setupConnections();

    // ensure that default values are displayed if a new tab has been created by user
    // or default settings have been requested :
    if ((serverNameQuery == ServerTabWidget::AskServerName) ||
            (serverNameQuery == ServerTabWidget::DefaultSettingsName)) {
        // set a tab index that will no be found by kconfig in order to return default values for new tab creation :
        tabIndex = -1;
    }

    // fill previously stored data in forms :
    setData(tabIndex);

    // finally check if fields contents contain whitespace :
    formEditingFinishedSlot();

}

ServerPreferencesWidget::~ServerPreferencesWidget()
{
    delete mServerSettingsUi;
}

int ServerPreferencesWidget::getTabIndex()
{
    return mTabIndex;
}

void ServerPreferencesWidget::setupButtons()
{

    // set icon for info button :
    mServerSettingsUi->pushButtonInfo->setIcon(QIcon::fromTheme("system-help"));

    // set icon and text for server mode combo box :
    QMap<int, QString> comboBoxIconTextMap = mServerTabWidget->getComboBoxIconTextMap();

    foreach (const QString &iconStr, comboBoxIconTextMap.values()) {

        QString serverModeStr = UtilityServerStatus::getServerModeString((UtilityNamespace::BackupServerMode)comboBoxIconTextMap.key(iconStr));
        mServerSettingsUi->comboBoxServerMode->addItem(QIcon::fromTheme(iconStr), serverModeStr);
    }

}

void ServerPreferencesWidget::setupConnections()
{

    // check/uncheck ssl checkbox according to port value:
    connect(mServerSettingsUi->port, SIGNAL(valueChanged(int)), this, SLOT(portValueChangedSlot(int)));

    connect(mServerSettingsUi->comboBoxServerMode, SIGNAL(currentIndexChanged(int)), this, SLOT(serverModeValueChangedSlot(int)));

    // enable apply button when a setting has been changed :
    connect(mServerSettingsUi->port, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot()));
    connect(mServerSettingsUi->connectionNumber, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot()));
    connect(mServerSettingsUi->disconnectTimeout, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot()));

    connect(mServerSettingsUi->hostName, SIGNAL(textChanged(QString)), this, SLOT(valueChangedSlot()));
    connect(mServerSettingsUi->login, SIGNAL(textChanged(QString)), this, SLOT(valueChangedSlot()));
    connect(mServerSettingsUi->password, SIGNAL(textChanged(QString)), this, SLOT(valueChangedSlot()));

    connect(mServerSettingsUi->groupBoxAuthentication, &QGroupBox::clicked, this, &ServerPreferencesWidget::valueChangedSlot);
    connect(mServerSettingsUi->enableSSL, &QCheckBox::stateChanged, this, &ServerPreferencesWidget::valueChangedSlot);

    connect(mServerSettingsUi->comboBoxServerMode, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChangedSlot()));

    // display information box when pushButtonInfo is clicked :
    connect(mServerSettingsUi->pushButtonInfo, &QAbstractButton::clicked, this, &ServerPreferencesWidget::pushButtonInfoClickedSlot);

    // check if server name, login and password contain trailling white spaces when editing finished :
    connect(mServerSettingsUi->hostName, &QLineEdit::editingFinished, this, &ServerPreferencesWidget::formEditingFinishedSlot);
    connect(mServerSettingsUi->login, &QLineEdit::editingFinished, this, &ServerPreferencesWidget::formEditingFinishedSlot);
    connect(mServerSettingsUi->password, &QLineEdit::editingFinished, this, &ServerPreferencesWidget::formEditingFinishedSlot);

}

void ServerPreferencesWidget::setGroupBoxTitle(const int &index)
{

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

    mServerSettingsUi->groupBoxConfig->setTitle(i18n("%1 server configuration", serverStr));

}

void ServerPreferencesWidget::setData(const int &tabIndex)
{

    // set all previously stored settings to widgets :
    ServerData serverData = KConfigGroupHandler::getInstance()->readServerSettings(tabIndex);

    mServerSettingsUi->hostName->setText(serverData.getHostName());
    mServerSettingsUi->login->setText(serverData.getLogin());
    mServerSettingsUi->password->setText(serverData.getPassword());

    mServerSettingsUi->port->setValue(serverData.getPort());
    mServerSettingsUi->connectionNumber->setValue(serverData.getConnectionNumber());
    mServerSettingsUi->disconnectTimeout->setValue(serverData.getDisconnectTimeout());

    mServerSettingsUi->groupBoxAuthentication->setChecked(serverData.isAuthentication());
    mServerSettingsUi->enableSSL->setChecked(serverData.isEnableSSL());

    mServerSettingsUi->comboBoxServerMode->setCurrentIndex(serverData.getServerModeIndex());
}

ServerData ServerPreferencesWidget::getData()
{

    // get all settings from widgets :
    ServerData serverData;

    mServerSettingsUi->hostName->setText(mServerSettingsUi->hostName->text());
    serverData.setHostName(mServerSettingsUi->hostName->text());

    serverData.setLogin(mServerSettingsUi->login->text());
    serverData.setPassword(mServerSettingsUi->password->text());

    serverData.setPort(mServerSettingsUi->port->value());
    serverData.setConnectionNumber(mServerSettingsUi->connectionNumber->value());
    serverData.setDisconnectTimeout(mServerSettingsUi->disconnectTimeout->value());

    serverData.setAuthentication(mServerSettingsUi->groupBoxAuthentication->isChecked());
    serverData.setEnableSSL(mServerSettingsUi->enableSSL->isChecked());

    serverData.setServerModeIndex(mServerSettingsUi->comboBoxServerMode->currentIndex());

    return serverData;

}

void ServerPreferencesWidget::enableWidgets(bool enable)
{

    mServerSettingsUi->hostName->setEnabled(enable);
    mServerSettingsUi->login->setEnabled(enable);
    mServerSettingsUi->password->setEnabled(enable);

    mServerSettingsUi->port->setEnabled(enable);
    mServerSettingsUi->connectionNumber->setEnabled(enable);
    mServerSettingsUi->disconnectTimeout->setEnabled(enable);

    mServerSettingsUi->groupBoxAuthentication->setEnabled(enable);
    mServerSettingsUi->enableSSL->setEnabled(enable);

    mServerSettingsUi->labelHost->setEnabled(enable);
    mServerSettingsUi->labelConnections->setEnabled(enable);
    mServerSettingsUi->labelPort->setEnabled(enable);
    mServerSettingsUi->labelLogin->setEnabled(enable);
    mServerSettingsUi->labelPassword->setEnabled(enable);
    mServerSettingsUi->labelDisconnect->setEnabled(enable);
}

void ServerPreferencesWidget::hideWidgets(const int &tabIndex)
{

    if (tabIndex == MasterServer) {

        mServerSettingsUi->labelServerMode->hide();
        mServerSettingsUi->comboBoxServerMode->hide();
        mServerSettingsUi->pushButtonInfo->hide();

    }

}

bool ServerPreferencesWidget::checkFormText(const QString &text)
{
    return (text != text.trimmed());
}

void ServerPreferencesWidget::fillWarningLabel(QLabel *label, const QString &toolTip)
{

    label->setPixmap(UtilityIconPainting::getInstance()->buildNormalIcon("dialog-warning"));
    label->setToolTip(toolTip);

}

void ServerPreferencesWidget::clearWarningLabel(QLabel *label)
{

    label->setToolTip(QString());
    label->setPixmap(QPixmap());

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ServerPreferencesWidget::valueChangedSlot()
{
    mPreferencesServer->kcfg_serverChangesNotify->setText(QUuid::createUuid().toString());
}

void ServerPreferencesWidget::portValueChangedSlot(int portValue)
{

    // if ports usually used for SSL are met :
    if (portValue == 563 || portValue == 443) {
        mServerSettingsUi->enableSSL->setCheckState(Qt::Checked);
    }
    // else is ports usually used for normal connections are met :
    else {
        mServerSettingsUi->enableSSL->setCheckState(Qt::Unchecked);
    }

}

void ServerPreferencesWidget::serverModeValueChangedSlot(int serverModeIndex)
{

    // update tab icon :
    mServerTabWidget->setServerTabIcon(mServerTabWidget->indexOf(this), serverModeIndex);

    // disable all settings if current server has been disabled :
    if (serverModeIndex == UtilityNamespace::DisabledServer) {
        enableWidgets(false);
    } else {
        enableWidgets(true);
    }

}

void ServerPreferencesWidget::formEditingFinishedSlot()
{

    // check if host field contains white-space :
    if (checkFormText(mServerSettingsUi->hostName->text())) {
        fillWarningLabel(mServerSettingsUi->hostCheckLabel, i18n("Host field contains white-space"));
    } else {
        clearWarningLabel(mServerSettingsUi->hostCheckLabel);
    }

    // check if host login contains white-space :
    if (checkFormText(mServerSettingsUi->login->text())) {
        fillWarningLabel(mServerSettingsUi->loginCheckLabel, i18n("Login field contains white-space"));
    } else {
        clearWarningLabel(mServerSettingsUi->loginCheckLabel);
    }

    // check if host password contains white-space :
    if (checkFormText(mServerSettingsUi->password->text())) {
        fillWarningLabel(mServerSettingsUi->passwordCheckLabel, i18n("Password field contains white-space"));
    } else {
        clearWarningLabel(mServerSettingsUi->passwordCheckLabel);
    }

}

void ServerPreferencesWidget::pushButtonInfoClickedSlot()
{

    QString divStyle = "<div style=\"margin-left: 20px; margin-top: 3px; margin-bottom: 10px\">";
    QString text;

    text.append(i18n("<b>Backup Server Mode:</b>"));
    text.append(divStyle);
    text.append(i18n("<i>Passive:</i> downloads only files not found on Master server<br>"));
    text.append(i18n("<i>Active:</i> downloads files simultaneously with Master server<br>"));
    text.append(i18n("<i>Failover:</i> supersedes former unavailable <i>Master</i> or <i>Failover</i> server, otherwise works as <i>Passive</i><br>"));
    text.append(i18n("<i>Server Disabled:</i> server not used"));
    text.append("</div>");

    text.append(i18n("<b>Server priority:</b>"));
    text.append(divStyle);
    text.append(i18n("Drag and drop tabs to manage backup server priority in <i>Passive</i> and <i>Failover</i> modes"));
    text.append("</div>");

    text.append(i18n("<b>Tab renaming:</b>"));
    text.append(divStyle);
    text.append(i18n("Double click on current tab for server renaming"));
    text.append("</div>");

    KMessageBox::information(this, text, i18n("Backup Server hints"));

}

