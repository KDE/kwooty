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

#include "servertabwidget.h"

#include "kwooty_debug.h"
#include <QIcon>
#include <QInputDialog>
#include <KMessageBox>

#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QUuid>

#include "preferences/preferencesserver.h"
#include "serverpreferenceswidget.h"
#include "preferences/kconfiggrouphandler.h"

#include "utilities/utilityserverstatus.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

ServerTabWidget::ServerTabWidget(PreferencesServer *parent) : KTabWidget(parent)
{

    this->mPreferencesServer = parent;

    // add a button to add backup server :
    this->mNewTab = new QToolButton(this);
    this->mNewTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    this->mNewTab->setIcon(QIcon::fromTheme("list-add"));
    this->mNewTab->setToolTip("Add a backup server");

    // add a button to remove backup server :
    this->mCloseTab = new QToolButton(this);
    this->mCloseTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    this->mCloseTab->setIcon(QIcon::fromTheme("list-remove"));
    this->mCloseTab->setToolTip("Remove current backup server");

    // create icons and associated texts for servers mode :
    this->mComboBoxIconTextMap.insert(PassiveServer,     QString("system-reboot"));
    this->mComboBoxIconTextMap.insert(ActiveServer,      QString("system-log-out"));
    this->mComboBoxIconTextMap.insert(FailoverServer,    QString("system-switch-user"));
    this->mComboBoxIconTextMap.insert(DisabledServer,    QString("system-shutdown"));

    // set buttons to right and left corners :
    this->setCornerWidget(this->mNewTab, Qt::TopRightCorner);
    this->setCornerWidget(this->mCloseTab, Qt::TopLeftCorner);

    parent->mainLayout->addWidget(this);

    this->setFocusPolicy(Qt::NoFocus);
    this->setupConnections();

}

void ServerTabWidget::setupConnections()
{

    // tab buttons have been clicked :
    connect(this->mNewTab, SIGNAL(clicked(bool)), this, SLOT(newTabClickedSlot()));
    connect(this->mCloseTab, SIGNAL(clicked(bool)), this, SLOT(closeTabClickedSlot()));

    // current tab has been moved by user :
    connect(this->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(tabMovedSlot(int,int)));
    connect(this->tabBar(), SIGNAL(currentChanged(int)), this, SLOT(currentChangedSlot(int)));

    // save data asked from kConfigDialog :
    connect(this->mPreferencesServer, SIGNAL(saveDataSignal()), this, SLOT(saveDataSlot()));

    // notify changes in settings if tab has moved :
    connect(this->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(valueChangedSlot()));

    // double click has been performed on tab to rename server :
    connect(this, SIGNAL(mouseDoubleClick(QWidget*)), this, SLOT(renameTabSlot(QWidget*)));

}

QMap<int, QString> ServerTabWidget::getComboBoxIconTextMap() const
{
    return this->mComboBoxIconTextMap;
}

void ServerTabWidget::addNewTab()
{
    this->newTabClickedSlot(ServerTabWidget::DoNotAskServerName);
}

void ServerTabWidget::addDefaultTab()
{
    this->newTabClickedSlot(ServerTabWidget::DefaultSettingsName);
}

QString ServerTabWidget::displayEditDialogBox()
{
    return QInputDialog::getText(this, i18n("Backup server name"), i18n("Please enter backup server name:"));
}

QString ServerTabWidget::displayRenameTabDialogBox()
{
    return QInputDialog::getText(this, i18n("Rename server"), i18n("Rename server:"), QLineEdit::Normal, this->tabText(this->currentIndex()).remove("&"));
}

void ServerTabWidget::setServerTabText(const ServerTabNaming &serverTabNaming)
{

    QString input;

    // if tab is being created, ask for new tab name :
    if (serverTabNaming == CreateTab) {
        input = this->displayEditDialogBox();
    }
    // tab already exists, ask for renaming :
    else {
        input = this->displayRenameTabDialogBox();
    }

    if (!input.isEmpty()) {
        this->setTabText(this->currentIndex(), input);

        // enable apply button to notify changes :
        this->valueChangedSlot();
    }

}

void ServerTabWidget::deleteAndRemoveTab(const int &index)
{

    // get the widget to delete :
    QWidget *currentWidget = this->widget(index);

    // remove the tab :
    this->removeTab(index);

    // delete the widget :
    delete currentWidget;

    // update groupbox title according to its corresponding tab position :
    this->syncGroupBoxTitle();

    // enable apply button to notify changes :
    this->valueChangedSlot();
}

void ServerTabWidget::enableDisableTabButtons()
{

    this->mNewTab->setEnabled(true);
    this->mCloseTab->setEnabled(true);

    // do not allow to add more than 5 servers :
    if (this->count() == UtilityNamespace::MAX_SERVERS) {
        this->mNewTab->setEnabled(false);
    }
    // do not allow to remove the master server :
    else if (this->count() == 1) {
        this->mCloseTab->setEnabled(false);
    }

    if (this->currentIndex() == MasterServer) {
        this->mCloseTab->setEnabled(false);
    }

}

void ServerTabWidget::setServerTabIcon(const int &tabIndex, const int &serverModeIndex)
{

    QString iconStr;

    // set a dedicated icon for master server :
    if (tabIndex == MasterServer) {
        iconStr = "applications-internet";
    }
    // adjust icon according to server mode :
    else {
        iconStr = this->mComboBoxIconTextMap.value(serverModeIndex);
    }

    this->setTabIcon(tabIndex, QIcon::fromTheme(iconStr));

}

void ServerTabWidget::syncGroupBoxTitle()
{

    int tabNumber = this->count();

    for (int i = 1; i < tabNumber; ++i) {
        static_cast<ServerPreferencesWidget *>(this->widget(i))->setGroupBoxTitle(i);
    }

}

//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void ServerTabWidget::newTabClickedSlot(const ServerTabWidget::ServerNameQuery askServerName)
{

    int tabNumbers = this->count();

    // if number of current tabs is < 5 allow to add a new backup server :
    if (tabNumbers < UtilityNamespace::MAX_SERVERS) {

        QString tabText;

        // if server is masterServer, set its default name automatically :
        if (tabNumbers == MasterServer) {
            tabText = i18n("Master");
        }

        // if a backup server has been required by user :
        if (askServerName == ServerTabWidget::AskServerName) {

            // retrieve server name entered by the user :
            QString input = this->displayEditDialogBox();
            if (!input.isEmpty()) {
                tabText = input;
            }
        }
        // if the backup server has been previously created, search its name in settings :
        else {
            tabText = KConfigGroupHandler::getInstance()->tabName(tabNumbers, tabText);
        }

        // if default settings are requested :
        if (askServerName == DefaultSettingsName) {
            tabText = i18n("Master");
        }

        // if tabText is empty, do not create the new tab :
        if (!tabText.isEmpty()) {

            // add tab with new window widget :
            ServerPreferencesWidget *serverPreferencesWidget = new ServerPreferencesWidget(this, this->mPreferencesServer, tabNumbers, askServerName);

            this->addTab(serverPreferencesWidget, tabText);

            // set tab icon according to server mode :
            this->setServerTabIcon(tabNumbers, serverPreferencesWidget->getData().getServerModeIndex());

            this->setCurrentIndex(tabNumbers);
            this->enableDisableTabButtons();

            // update groupbox title according to its corresponding tab position :
            this->syncGroupBoxTitle();

            // enable apply button to notify changes :
            this->valueChangedSlot();

        }

    }

}

void ServerTabWidget::closeTabClickedSlot()
{

    int currentIndex = this->currentIndex();

    if (currentIndex != 0) {

        // ask for current server removing :
        int answer = KMessageBox::messageBox(this,
                                             KMessageBox::QuestionYesNo,
                                             i18n("Remove <b>%1</b> backup server ?", this->tabText(this->currentIndex()).remove("&")));

        // if selected rows has not been canceled :
        if (answer == KMessageBox::Yes) {

            // delete the widget and the corresponding tab :
            this->deleteAndRemoveTab(currentIndex);
            this->enableDisableTabButtons();

        }

    }

}

void ServerTabWidget::tabMovedSlot(int from, int to)
{

    // the master server tab must stay at the first position :
    if (from == MasterServer && to == 1) {
        this->tabBar()->moveTab(to, from);
    }

    this->syncGroupBoxTitle();

}

void ServerTabWidget::currentChangedSlot(int index)
{

    // the master server tab must stay at the first position :
    if (index == MasterServer) {
        this->setMovable(false);
    } else {
        this->setMovable(true);
    }

    // enable / disable buttons tab accordingly :
    this->enableDisableTabButtons();

}

void ServerTabWidget::saveDataSlot()
{

    int tabNumber = this->count();

    // delete all previous server settings entries :
    for (int i = 0; i < UtilityNamespace::MAX_SERVERS; ++i) {
        KConfigGroupHandler::getInstance()->removeServerSettings(i);
    }

    // save the new ones :
    for (int i = 0; i < tabNumber; ++i) {

        ServerData serverData = static_cast<ServerPreferencesWidget *>(this->widget(i))->getData();
        serverData.setServerId(i);

        serverData.setServerName(this->tabText(i));

        KConfigGroupHandler::getInstance()->writeServerSettings(i, serverData);

    }

    // save the current number of servers :
    KConfigGroupHandler::getInstance()->writeServerNumberSettings(tabNumber);

}

void ServerTabWidget::valueChangedSlot()
{
    mPreferencesServer->kcfg_serverChangesNotify->setText(QUuid::createUuid().toString());
}

void ServerTabWidget::renameTabSlot(QWidget *widget)
{

    // if widget has been found, rename tab :
    if (this->indexOf(widget) > -1) {
        this->setServerTabText(RenameTab);

    }

}

