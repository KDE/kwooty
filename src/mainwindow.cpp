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

#include "mainwindow.h"

#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KConfigDialog>
#include <KPageDialog>
#include <KPageWidgetItem>
#include <KMessageBox>
#include <KActionCollection>
#include <KSaveFile>
#include <KDebug>
#include <KMenuBar>

#include <QTextStream>
#include <QtGui>

#include "kwootysettings.h"
#include "core.h"
#include "fileoperations.h"
#include "sidebar.h"
#include "actions/actionsmanager.h"
#include "actions/actionbuttonsmanager.h"
#include "actions/actionmergemanager.h"
#include "actions/actionrenamemanager.h"
#include "widgets/mystatusbar.h"
#include "widgets/mytreeview.h"
#include "widgets/centralwidget.h"
#include "widgets/sidebarwidget.h"
#include "shutdown/shutdownmanager.h"
#include "plugins/pluginmanager.h"
#include "preferences/kconfiggrouphandler.h"
#include "preferences/preferencesserver.h"
#include "preferences/preferencesgeneral.h"
#include "preferences/preferencesprograms.h"
#include "preferences/preferencesdisplay.h"
#include "preferences/preferencesshutdown.h"
#include "preferences/preferencesplugins.h"


#ifdef HAVE_KSTATUSNOTIFIERITEM
#include "systray.h"
#else
#include "systraylegacy.h"
#endif



MainWindow::MainWindow(QWidget* parent): KXmlGuiWindow(parent) {

    this->initVariables();

    // setup kconfig group handler :
    this->kConfigGroupHandler = new KConfigGroupHandler(this);

    // setup central widget :
    this->centralWidget = new CentralWidget(this);

    // setup treeview :
    this->treeView = new MyTreeView(this);

    // setup core :
    this->core = new Core(this);

    // finish treeView init :
    this->treeView->achieveInit();

    // setup side bar manager :
    this->sideBar = new SideBar(this);


    // create the user interface :
    QWidget* widget = new QWidget(this);
    this->buildLayout(widget);
    this->setCentralWidget(widget);

    this->setupActions();

    // setup statusBar :
    this->statusBar = new MyStatusBar(this);
    this->setStatusBar(this->statusBar);

    // setup system tray :
    this->systraySlot();

    // setup plugin manager :
    this->pluginManager = new PluginManager(this);
    this->pluginManager->loadPlugins();

    this->quitSelected = false;

    // hide main window when session is restored and systray icon is checked, else show main window :
    if ( !(kapp->isSessionRestored() && Settings::sysTray()) ||
         (kapp->isSessionRestored() && !this->kConfigGroupHandler->readMainWindowHiddenOnExit()) ) {

        this->show();
    }

}


MainWindow::~MainWindow(){

}

void MainWindow::initVariables() {
    this->core = 0;
    this->centralWidget = 0;
    this->treeView = 0;
    this->sideBar = 0;
}


void MainWindow::buildLayout(QWidget* widget) {

    QVBoxLayout* mainVBoxLayout = new QVBoxLayout(widget);
    mainVBoxLayout->setSpacing(2);
    mainVBoxLayout->setMargin(1);

    mainVBoxLayout->addWidget(this->treeView);
    mainVBoxLayout->addWidget(this->sideBar->getSideBarWidget());

}


MyStatusBar* MainWindow::getStatusBar() const {
    Q_ASSERT (this->statusBar != 0);
    return this->statusBar;
}

SideBar* MainWindow::getSideBar() const {
    Q_ASSERT (this->sideBar != 0);
    return this->sideBar;
}

CentralWidget* MainWindow::getCentralWidget() const {
    Q_ASSERT (this->centralWidget != 0);
    return this->centralWidget;
}

Core* MainWindow::getCore() const{
    Q_ASSERT (this->core != 0);
    return this->core;
}

MyTreeView* MainWindow::getTreeView() const{
    Q_ASSERT (this->treeView != 0);
    return this->treeView;
}

void MainWindow::setupActions() {

    ActionsManager* actionsManager = this->core->getActionsManager();
    ActionButtonsManager* actionButtonsManager = actionsManager->getActionButtonsManager();

    //-----------------
    // custom Actions :
    //-----------------

    // clearAction :
    KAction* clearAction = new KAction(this);
    clearAction->setText(i18n("Clear"));
    clearAction->setIcon(KIcon("edit-clear-list"));
    clearAction->setToolTip(i18n("Remove all rows"));
    clearAction->setShortcut(Qt::CTRL + Qt::Key_W);
    actionCollection()->addAction("clear", clearAction);
    connect(clearAction, SIGNAL(triggered(bool)), actionsManager, SLOT(clearSlot()));

    // startDownloadAction :
    KAction* startDownloadAction = new KAction(this);
    startDownloadAction->setText(i18n("Start"));
    startDownloadAction->setIcon(KIcon("media-playback-start"));
    startDownloadAction->setToolTip(i18n("Start download of selected rows"));
    startDownloadAction->setShortcut(Qt::CTRL + Qt::Key_S);
    startDownloadAction->setEnabled(false);
    actionCollection()->addAction("start", startDownloadAction);
    connect(startDownloadAction, SIGNAL(triggered(bool)), actionsManager, SLOT(startDownloadSlot()));
    connect(actionButtonsManager, SIGNAL(setStartButtonEnabledSignal(bool)), startDownloadAction, SLOT(setEnabled(bool)) );

    // pauseDownloadAction :
    KAction* pauseDownloadAction = new KAction(this);
    pauseDownloadAction->setText(i18n("Pause"));
    pauseDownloadAction->setIcon(KIcon("media-playback-pause"));
    pauseDownloadAction->setToolTip(i18n("Pause download of selected rows"));
    pauseDownloadAction->setShortcut(Qt::CTRL + Qt::Key_P);
    pauseDownloadAction->setEnabled(false);
    actionCollection()->addAction("pause", pauseDownloadAction);
    connect(pauseDownloadAction, SIGNAL(triggered(bool)), actionsManager, SLOT(pauseDownloadSlot()));
    connect(actionButtonsManager, SIGNAL(setPauseButtonEnabledSignal(bool)), pauseDownloadAction, SLOT(setEnabled(bool)) );

    // removeItemAction :
    KAction* removeItemAction = new KAction(this);
    removeItemAction->setText(i18n("Remove"));
    removeItemAction->setIcon(KIcon("list-remove"));
    removeItemAction->setToolTip(i18n("Remove all selected rows"));
    removeItemAction->setShortcut(Qt::Key_Delete);
    removeItemAction->setEnabled(false);
    actionCollection()->addAction("remove", removeItemAction);
    connect(removeItemAction, SIGNAL(triggered(bool)), actionsManager, SLOT(removeRowSlot()));
    connect(actionButtonsManager, SIGNAL(setMoveButtonEnabledSignal(bool)), removeItemAction, SLOT(setEnabled(bool)) );
    connect(actionButtonsManager, SIGNAL(setRemoveButtonEnabledSignal(bool)), removeItemAction, SLOT(setEnabled(bool)) );

    // moveUpAction :
    KAction* moveUpAction = new KAction(this);
    moveUpAction->setText(i18n("Up"));
    moveUpAction->setIcon(KIcon("go-up"));
    moveUpAction->setToolTip(i18n("Go up all selected rows"));
    moveUpAction->setShortcut(Qt::CTRL + Qt::Key_Up);
    moveUpAction->setEnabled(false);
    actionCollection()->addAction("moveUp", moveUpAction);
    connect(moveUpAction, SIGNAL(triggered(bool)), actionsManager, SLOT(moveUpSlot()));
    connect(actionButtonsManager, SIGNAL(setMoveButtonEnabledSignal(bool)), moveUpAction, SLOT(setEnabled(bool)) );

    // moveToTopAction :
    KAction* moveToTopAction = new KAction(this);
    moveToTopAction->setText(i18n("Top"));
    moveToTopAction->setIcon(KIcon("go-top"));
    moveToTopAction->setToolTip(i18n("Move all selected rows to the top of the list"));
    moveToTopAction->setShortcut(Qt::CTRL + Qt::Key_PageUp);
    moveToTopAction->setEnabled(false);
    actionCollection()->addAction("moveTop", moveToTopAction);
    connect(moveToTopAction, SIGNAL(triggered(bool)), actionsManager, SLOT(moveToTopSlot()));
    connect(actionButtonsManager, SIGNAL(setMoveButtonEnabledSignal(bool)), moveToTopAction, SLOT(setEnabled(bool)) );

    // moveDownAction :
    KAction* moveDownAction = new KAction(this);
    moveDownAction->setText(i18n("Down"));
    moveDownAction->setIcon(KIcon("go-down"));
    moveDownAction->setToolTip(i18n("Go down all selected rows"));
    moveDownAction->setShortcut(Qt::CTRL + Qt::Key_Down);
    moveDownAction->setEnabled(false);
    actionCollection()->addAction("moveDown", moveDownAction);
    connect(moveDownAction, SIGNAL(triggered(bool)), actionsManager, SLOT(moveDownSlot()));
    connect(actionButtonsManager, SIGNAL(setMoveButtonEnabledSignal(bool)), moveDownAction, SLOT(setEnabled(bool)) );


    // moveToBottomAction :
    KAction* moveToBottomAction = new KAction(this);
    moveToBottomAction->setText(i18n("Bottom"));
    moveToBottomAction->setIcon(KIcon("go-bottom"));
    moveToBottomAction->setToolTip(i18n("Move all selected rows to the bottom of the list"));
    moveToBottomAction->setShortcut(Qt::CTRL + Qt::Key_PageDown);
    moveToBottomAction->setEnabled(false);
    actionCollection()->addAction("moveBottom", moveToBottomAction);
    connect(moveToBottomAction, SIGNAL(triggered(bool)), actionsManager, SLOT(moveToBottomSlot()));
    connect(actionButtonsManager, SIGNAL(setMoveButtonEnabledSignal(bool)), moveToBottomAction, SLOT(setEnabled(bool)) );


    // openFolderAction :
    KAction* openFolderAction = new KAction(this);
    openFolderAction->setText(i18n("Downloads"));
    openFolderAction->setIcon(KIcon("folder-downloads"));
    openFolderAction->setToolTip(i18n("Open current download folder"));
    openFolderAction->setShortcut(Qt::CTRL + Qt::Key_D);
    openFolderAction->setEnabled(true);
    actionCollection()->addAction("downloadFolder", openFolderAction);
    connect(openFolderAction, SIGNAL(triggered(bool)), actionsManager, SLOT(openFolderSlot()));

    // shutdownAction :
    KAction* shutdownAction = new KAction(this);
    shutdownAction->setText(i18n("Shutdown"));
    shutdownAction->setIcon(KIcon("system-shutdown"));
    shutdownAction->setToolTip(i18n("Schedule system shutdown"));
    shutdownAction->setShortcut(Qt::CTRL + Qt::Key_T);
    shutdownAction->setEnabled(false);
    shutdownAction->setCheckable(true);
    actionCollection()->addAction("shutdown", shutdownAction);
    connect(shutdownAction, SIGNAL(triggered(bool)), core->getShutdownManager(), SLOT(enableSystemShutdownSlot(bool)));
    connect(core->getShutdownManager(), SIGNAL(setShutdownButtonCheckedSignal(bool)), shutdownAction, SLOT(setChecked(bool)));
    connect(core->getShutdownManager(), SIGNAL(setShutdownButtonEnabledSignal(bool)), shutdownAction, SLOT(setEnabled(bool)) );

    // startAllDownloadAction :
    KAction* startAllDownloadAction = new KAction(this);
    startAllDownloadAction->setText(i18n("Start all"));
    startAllDownloadAction->setIcon(KIcon("media-playback-start"));
    startAllDownloadAction->setToolTip(i18n("Start all paused downloads"));
    startAllDownloadAction->setEnabled(false);
    actionCollection()->addAction("startAll", startAllDownloadAction);
    connect(startAllDownloadAction, SIGNAL(triggered(bool)), actionsManager, SLOT(startAllDownloadSlot()));
    connect(actionButtonsManager, SIGNAL(setStartAllButtonEnabledSignal(bool)), startAllDownloadAction, SLOT(setEnabled(bool)) );

    // pauseAllDownloadAction :
    KAction* pauseAllDownloadAction = new KAction(this);
    pauseAllDownloadAction->setText(i18n("Pause all"));
    pauseAllDownloadAction->setIcon(KIcon("media-playback-pause"));
    pauseAllDownloadAction->setToolTip(i18n("Pause all pending downloads"));
    pauseAllDownloadAction->setEnabled(false);
    actionCollection()->addAction("pauseAll", pauseAllDownloadAction);
    connect(pauseAllDownloadAction, SIGNAL(triggered(bool)), actionsManager, SLOT(pauseAllDownloadSlot()));
    connect(actionButtonsManager, SIGNAL(setPauseAllButtonEnabledSignal(bool)), pauseAllDownloadAction, SLOT(setEnabled(bool)) );

    // retryDownloadAction :
    KAction* retryDownloadAction = new KAction(this);
    retryDownloadAction->setText(i18n("Retry"));
    retryDownloadAction->setIcon(KIcon("edit-redo"));
    retryDownloadAction->setToolTip(i18n("Retry to download selected rows"));
    retryDownloadAction->setShortcut(Qt::CTRL + Qt::Key_R);
    retryDownloadAction->setEnabled(false);
    actionCollection()->addAction("retryDownload", retryDownloadAction);
    connect(retryDownloadAction, SIGNAL(triggered(bool)), actionsManager, SLOT(retryDownloadSlot()));
    connect(actionButtonsManager, SIGNAL(setRetryButtonEnabledSignal(bool)), retryDownloadAction, SLOT(setEnabled(bool)) );

    // manualExtractAction :
    KAction* manualExtractAction = new KAction(this);
    manualExtractAction->setText(i18n("Repair and extract"));
    manualExtractAction->setIcon(KIcon("archive-extract"));
    manualExtractAction->setToolTip(i18n("Manually verify and extract selected item"));
    manualExtractAction->setShortcut(Qt::CTRL + Qt::Key_E);
    manualExtractAction->setEnabled(false);
    actionCollection()->addAction("manualExtract", manualExtractAction);
    connect(manualExtractAction, SIGNAL(triggered(bool)), actionsManager, SLOT(manualExtractSlot()));
    connect(actionButtonsManager, SIGNAL(setManualExtractActionSignal(bool)), manualExtractAction, SLOT(setEnabled(bool)) );

    // mergeNzbAction :
    KAction* mergeNzbAction = new KAction(this);
    mergeNzbAction->setText(i18n("Merge with..."));
    mergeNzbAction->setIcon(KIcon("mail-message-new"));
    mergeNzbAction->setToolTip(i18n("Merge nzb content into another nzb"));
    mergeNzbAction->setEnabled(false);
    actionCollection()->addAction("mergeNzb", mergeNzbAction);
    connect(actionButtonsManager, SIGNAL(setMergeNzbButtonEnabledSignal(bool)), mergeNzbAction, SLOT(setEnabled(bool)) );

    // add a submenu that will be filled dynamically :
    QMenu* mergeSubMenu = new QMenu(this);
    mergeNzbAction->setMenu(mergeSubMenu);

    // prepare corresponding submenu :
    connect(mergeSubMenu, SIGNAL(aboutToShow()), actionsManager->getActionMergeManager(), SLOT(mergeSubMenuAboutToShowSlot()));

    // retrieve selected action from submenu :
    connect(mergeSubMenu, SIGNAL(triggered(QAction*)), actionsManager->getActionMergeManager(), SLOT(mergeNzbActionTriggeredSlot(QAction*)));


    // renameNzbAction :
    KAction* renameNzbAction = new KAction(this);
    renameNzbAction->setText(i18n("Rename..."));
    renameNzbAction->setIcon(KIcon("edit-rename"));
    renameNzbAction->setToolTip(i18n("Rename nzb and its corresponding download folder"));
    renameNzbAction->setShortcut(Qt::Key_F2);
    renameNzbAction->setEnabled(false);
    actionCollection()->addAction("renameNzb", renameNzbAction);
    connect(renameNzbAction, SIGNAL(triggered(bool)), actionsManager->getActionRenameManager(), SLOT(renameNzbActionSlot()));
    connect(actionButtonsManager, SIGNAL(setRenameNzbButtonEnabledSignal(bool)), renameNzbAction, SLOT(setEnabled(bool)) );



    //-------------------
    // standard Actions :
    //-------------------

    // quit action :
    KStandardAction::quit(this, SLOT(quit()), actionCollection());

    // open action :
    KStandardAction::open(this, SLOT(openFile()), actionCollection());

    // settings action :
    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());

    // shown menuBar action :
    KStandardAction::showMenubar(this, SLOT(toggleShowMenuBar()), actionCollection());


    setupGUI();


}


QAction* MainWindow::getActionFromName(const QString& actionName) {

    return actionCollection()->action(actionName);

}


void MainWindow::showSettings(UtilityNamespace::PreferencesPage preferencesPage) {

    // if instance has already been created :
    if (KConfigDialog::exists("settings")) {

        emit aboutToShowSettingsSignal();

        if (this->preferencesPagesMap.contains(preferencesPage)) {

            KConfigDialog::exists("settings")->setCurrentPage(this->preferencesPagesMap.value(preferencesPage));
        }

        KConfigDialog::showDialog("settings");

    }
    else {

        // dialog instance is not et created, create it :
        KConfigDialog* dialog = new KConfigDialog(this, "settings", Settings::self());

        PreferencesGeneral* preferencesGeneral = new PreferencesGeneral();
        KPageWidgetItem* preferencesGeneralPage = dialog->addPage(preferencesGeneral, i18n("General"), "preferences-system", i18n("General Setup"));
        this->preferencesPagesMap.insert(GeneralPage, preferencesGeneralPage);

        PreferencesServer* preferencesServer = new PreferencesServer(dialog);
        KPageWidgetItem* preferencesServerPage = dialog->addPage(preferencesServer, i18n("Connection"), "network-workgroup", i18n("Setup Server Connection"));
        this->preferencesPagesMap.insert(ServerPage, preferencesServerPage);

        PreferencesPrograms* preferencesPrograms = new PreferencesPrograms();
        KPageWidgetItem* preferencesProgramsPage = dialog->addPage(preferencesPrograms, i18n("Programs"), "system-run", i18n("Setup External Programs"));
        this->preferencesPagesMap.insert(ProgramsPage, preferencesProgramsPage);

        PreferencesDisplay* preferencesDisplay = new PreferencesDisplay();
        KPageWidgetItem* preferencesDisplayPage = dialog->addPage(preferencesDisplay, i18n("Display modes"), "view-choose", i18n("Setup Display Modes"));
        this->preferencesPagesMap.insert(DisplayPage, preferencesDisplayPage);

        PreferencesShutdown* preferencesShutdown = new PreferencesShutdown(this->core);
        KPageWidgetItem* preferencesShutdownPage = dialog->addPage(preferencesShutdown, i18n("Shutdown"), "system-shutdown", i18n("Setup System Shutdown"));
        this->preferencesPagesMap.insert(ShutdownPage, preferencesShutdownPage);

        PreferencesPlugins* preferencesPlugins = new PreferencesPlugins(dialog, this->pluginManager);
        KPageWidgetItem* preferencesPluginsPage = dialog->addPage(preferencesPlugins, i18n("Plugins"), "preferences-plugin", i18n("Plugins Setup"));
        this->preferencesPagesMap.insert(PluginsPage, preferencesPluginsPage);


        connect(dialog, SIGNAL(settingsChanged(const QString&)), this->core, SLOT(updateSettingsSlot()));
        connect(dialog, SIGNAL(settingsChanged(const QString&)), this->kConfigGroupHandler, SLOT(settingsChangedSlot()));
        connect(dialog, SIGNAL(settingsChanged(const QString&)), preferencesPrograms, SLOT(aboutToShowSettingsSlot()));
        connect(dialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(systraySlot()));
        connect(this, SIGNAL(aboutToShowSettingsSignal()), preferencesPrograms, SLOT(aboutToShowSettingsSlot()));

        // show settings box :
        this->showSettings(preferencesPage);

    }


}


QSize MainWindow::sizeHint() const {
    return QSize(QApplication::desktop()->screenGeometry(this).width() / 1.5,
                 QApplication::desktop()->screenGeometry(this).height() / 2);
}





//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void MainWindow::toggleShowMenuBar() {

    if (this->menuBar()->isVisible()) {
        this->menuBar()->hide();
    }
    else {
        this->menuBar()->show();
    }
}



void MainWindow::openFile() {

    this->core->getFileOperations()->openFile();

}

void MainWindow::openFileWithFileMode(KUrl nzbUrl, UtilityNamespace::OpenFileMode openFileMode) {

    this->core->getFileOperations()->openFileWithFileMode(nzbUrl, openFileMode);

}



void MainWindow::quit() {

    // quit has been requested :
    this->quitSelected = true;

    // call queryclose() for quit confirmation :
    if (this->queryClose()) {
        kapp->quit();
    }

}



bool MainWindow::queryClose() {

    // by default quit the application :
    bool confirmQuit = true;

    // session manager is not responsible from this call :
    if (!kapp->sessionSaving()) {

        // if the main window is just closed :
        if (!this->quitSelected ) {

            // if system tray icon exists :
            if (Settings::sysTray()) {

                // display a warning message :
                KMessageBox::information( this,
                                          i18n( "<qt>Closing the main window will keep Kwooty running in the System Tray. "
                                                "Use <B>Quit</B> from the menu or the Kwooty tray icon to exit the application.</qt>" ),
                                          i18n( "Docking in System Tray" ), "hideOnCloseInfo" );

                // hide the main window and don't quit :
                this->hide();
                confirmQuit = false;
            }
            // system tray icon does not exist, ask to save data and close the application :
            else {
                this->askForSavingDownloads(confirmQuit);
            }
        }
        // quit action has been performed, ask to save data and close the application :
        else {
            this->askForSavingDownloads(confirmQuit);
        }
    }
    // session manager is about to quit, just save data silently and quit :
    else {
        core->savePendingDownloads(UtilityNamespace::ShutdownMethodUnknown, SaveSilently);
    }

    return confirmQuit;

}


bool MainWindow::queryExit() {

    this->kConfigGroupHandler->writeMainWindowHiddenOnExit(this->isHidden());
    this->sideBar->saveState();

    return true;
}


void MainWindow::askForSavingDownloads(bool& confirmQuit) {

    int answer = core->savePendingDownloads();

    if (answer == KMessageBox::Cancel) {
        this->quitSelected = false;
        confirmQuit = false;
    }

}


void MainWindow::systraySlot() {

    // remove system tray if requested by user :
    if (!Settings::sysTray() && this->sysTray) {
        delete this->sysTray;
    }
    // setup system tray if requested by user :
    else if (Settings::sysTray() && !this->sysTray) {
        this->sysTray = new SysTray(this);
    }

}


