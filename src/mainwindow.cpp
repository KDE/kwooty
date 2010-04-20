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

#include <KSaveFile>
#include <KDebug>

#include <QTextStream>
#include <QtGui>
#include "settings.h"
#include "mystatusbar.h"
#include "mytreeview.h"
#include "centralwidget.h"
#include "shutdownmanager.h"
#include "fileoperations.h"
#include "preferences/preferencesserver.h"
#include "preferences/preferencesgeneral.h"
#include "preferences/preferencesprograms.h"
#include "preferences/preferencesdisplay.h"
#include "preferences/preferencesshutdown.h"


MainWindow::MainWindow(QWidget* parent): KXmlGuiWindow(parent), fileName(QString())
{

    //setup statusBar :
    this->statusBar = new MyStatusBar(this);
    this->setStatusBar(this->statusBar);

    // create the user interface :
    QWidget* widget = new QWidget(this);
    this->centralWidget = new CentralWidget(widget, this->statusBar);

    // get treeview instance :
    treeView = centralWidget->getTreeView();

    this->setCentralWidget(widget);

    this->setupActions();


}


MainWindow::~MainWindow()
{

}


void MainWindow::setupActions()
{

    //-------------------
    //custom Actions :
    //-------------------
    //clearAction
    KAction* clearAction = new KAction(this);
    clearAction->setText(i18n("Clear"));
    clearAction->setIcon(KIcon("edit-clear-list"));
    clearAction->setToolTip(i18n("Remove all rows"));
    clearAction->setShortcut(Qt::CTRL + Qt::Key_W);
    actionCollection()->addAction("clear", clearAction);
    connect(clearAction, SIGNAL(triggered(bool)), treeView, SLOT(clearSlot()));

    //startDownloadAction
    KAction* startDownloadAction = new KAction(this);
    startDownloadAction->setText(i18n("Start"));
    startDownloadAction->setIcon(KIcon("media-playback-start"));
    startDownloadAction->setToolTip(i18n("Start download of selected rows"));
    startDownloadAction->setShortcut(Qt::CTRL + Qt::Key_S);
    startDownloadAction->setEnabled(false);
    actionCollection()->addAction("start", startDownloadAction);
    connect(startDownloadAction, SIGNAL(triggered(bool)), centralWidget, SLOT(startDownloadSlot()));
    connect(treeView, SIGNAL(setStartButtonEnabledSignal(bool)), startDownloadAction, SLOT(setEnabled(bool)) );

    //pauseDownloadAction
    KAction* pauseDownloadAction = new KAction(this);
    pauseDownloadAction->setText(i18n("Pause"));
    pauseDownloadAction->setIcon(KIcon("media-playback-pause"));
    pauseDownloadAction->setToolTip(i18n("Pause download of selected rows"));
    pauseDownloadAction->setShortcut(Qt::CTRL + Qt::Key_P);
    pauseDownloadAction->setEnabled(false);
    actionCollection()->addAction("pause", pauseDownloadAction);
    connect(pauseDownloadAction, SIGNAL(triggered(bool)), centralWidget, SLOT(pauseDownloadSlot()));
    connect(treeView, SIGNAL(setPauseButtonEnabledSignal(bool)), pauseDownloadAction, SLOT(setEnabled(bool)) );

    //removeItemAction
    KAction* removeItemAction = new KAction(this);
    removeItemAction->setText(i18n("Remove"));
    removeItemAction->setIcon(KIcon("list-remove"));
    removeItemAction->setToolTip(i18n("Remove all selected rows"));
    removeItemAction->setShortcut(Qt::Key_Delete);
    removeItemAction->setEnabled(false);
    actionCollection()->addAction("remove", removeItemAction);
    connect(removeItemAction, SIGNAL(triggered(bool)), treeView, SLOT(removeRowSlot()));
    connect(treeView, SIGNAL(setMoveButtonEnabledSignal(bool)), removeItemAction, SLOT(setEnabled(bool)) );
    connect(treeView, SIGNAL(setRemoveButtonEnabledSignal(bool)), removeItemAction, SLOT(setEnabled(bool)) );

    //moveUpAction
    KAction* moveUpAction = new KAction(this);
    moveUpAction->setText(i18n("Up"));
    moveUpAction->setIcon(KIcon("go-up"));
    moveUpAction->setToolTip(i18n("Go up all selected rows"));
    moveUpAction->setShortcut(Qt::CTRL + Qt::Key_Up);
    moveUpAction->setEnabled(false);
    actionCollection()->addAction("moveUp", moveUpAction);
    connect(moveUpAction, SIGNAL(triggered(bool)), treeView, SLOT(moveUpSlot()));
    connect(treeView, SIGNAL(setMoveButtonEnabledSignal(bool)), moveUpAction, SLOT(setEnabled(bool)) );

    //moveToTopAction
    KAction* moveToTopAction = new KAction(this);
    moveToTopAction->setText(i18n("Top"));
    moveToTopAction->setIcon(KIcon("go-top"));
    moveToTopAction->setToolTip(i18n("Move all selected rows to the top of the list"));
    moveToTopAction->setShortcut(Qt::CTRL + Qt::Key_PageUp);
    moveToTopAction->setEnabled(false);
    actionCollection()->addAction("moveTop", moveToTopAction);
    connect(moveToTopAction, SIGNAL(triggered(bool)), treeView, SLOT(moveToTopSlot()));
    connect(treeView, SIGNAL(setMoveButtonEnabledSignal(bool)), moveToTopAction, SLOT(setEnabled(bool)) );

    //moveDownAction
    KAction* moveDownAction = new KAction(this);
    moveDownAction->setText(i18n("Down"));
    moveDownAction->setIcon(KIcon("go-down"));
    moveDownAction->setToolTip(i18n("Go down all selected rows"));
    moveDownAction->setShortcut(Qt::CTRL + Qt::Key_Down);
    moveDownAction->setEnabled(false);
    actionCollection()->addAction("moveDown", moveDownAction);
    connect(moveDownAction, SIGNAL(triggered(bool)), treeView, SLOT(moveDownSlot()));
    connect(treeView, SIGNAL(setMoveButtonEnabledSignal(bool)), moveDownAction, SLOT(setEnabled(bool)) );


    //moveToBottomAction
    KAction* moveToBottomAction = new KAction(this);
    moveToBottomAction->setText(i18n("Bottom"));
    moveToBottomAction->setIcon(KIcon("go-bottom"));
    moveToBottomAction->setToolTip(i18n("Move all selected rows to the bottom of the list"));
    moveToBottomAction->setShortcut(Qt::CTRL + Qt::Key_PageDown);
    moveToBottomAction->setEnabled(false);
    actionCollection()->addAction("moveBottom", moveToBottomAction);
    connect(moveToBottomAction, SIGNAL(triggered(bool)), treeView, SLOT(moveToBottomSlot()));
    connect(treeView, SIGNAL(setMoveButtonEnabledSignal(bool)), moveToBottomAction, SLOT(setEnabled(bool)) );


    //openFolderAction
    KAction* openFolderAction = new KAction(this);
    openFolderAction->setText(i18n("Downloads"));
    openFolderAction->setIcon(KIcon("folder-downloads"));
    openFolderAction->setToolTip(i18n("Open current download folder"));
    openFolderAction->setShortcut(Qt::CTRL + Qt::Key_D);
    openFolderAction->setEnabled(true);
    actionCollection()->addAction("downloadFolder", openFolderAction);
    connect(openFolderAction, SIGNAL(triggered(bool)), treeView, SLOT(openFolderSlot()));

    //shutdownAction
    KAction* shutdownAction = new KAction(this);
    shutdownAction->setText(i18n("Shutdown"));
    shutdownAction->setIcon(KIcon("system-shutdown"));
    shutdownAction->setToolTip(i18n("Schedule system shutdown"));
    shutdownAction->setShortcut(Qt::CTRL + Qt::Key_T);
    shutdownAction->setEnabled(false);
    shutdownAction->setCheckable(true);
    actionCollection()->addAction("shutdown", shutdownAction);
    connect(shutdownAction, SIGNAL(triggered(bool)), centralWidget->getShutdownManager(), SLOT(enableSystemShutdownSlot(bool)));
    connect(centralWidget->getShutdownManager(), SIGNAL(setShutdownButtonCheckedSignal(bool)), shutdownAction, SLOT(setChecked(bool)));
    connect(centralWidget->getShutdownManager(), SIGNAL(setShutdownButtonEnabledSignal(bool)), shutdownAction, SLOT(setEnabled(bool)) );


    //-------------------
    //standard Actions :
    //-------------------

    // quitAction
    KStandardAction::quit(this, SLOT(quit()), actionCollection());

    // closeAction
    //KStandardAction::close(kapp, SLOT(close()), actionCollection());

    // openAction
    KStandardAction::open(this, SLOT(openFile()), actionCollection());

    // SettingsAction
    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());


    setupGUI();

}



void MainWindow::showSettings(){

    // if instance has already been created :
    if (KConfigDialog::exists("settings")) {

        emit aboutToShowSettingsSignal();
        KConfigDialog::showDialog("settings");

    }
    else {

        // dialog instance is not et created, create it :
        KConfigDialog* dialog = new KConfigDialog(this, "settings", Settings::self());

        PreferencesGeneral* preferencesGeneral = new PreferencesGeneral();
        dialog->addPage(preferencesGeneral, i18n("General"), "preferences-system", i18n("General Setup"));

        PreferencesServer* preferencesServer = new PreferencesServer();
        dialog->addPage(preferencesServer, i18n("Connection"), "network-workgroup", i18n("Setup Server Connection"));

        PreferencesPrograms* preferencesPrograms = new PreferencesPrograms();
        dialog->addPage(preferencesPrograms, i18n("Programs"), "system-run", i18n("Setup External Programs"));

        PreferencesDisplay* preferencesDisplay = new PreferencesDisplay();
        dialog->addPage(preferencesDisplay, i18n("Display modes"), "view-choose", i18n("Setup Display Modes"));

        PreferencesShutdown* preferencesShutdown = new PreferencesShutdown(this->centralWidget);
        dialog->addPage(preferencesShutdown, i18n("Shutdown"), "system-shutdown", i18n("Setup System Shutdown"));

        connect( dialog, SIGNAL(settingsChanged(const QString&)), centralWidget, SLOT(updateSettingsSlot()) );
        connect( dialog, SIGNAL(settingsChanged(const QString&)), preferencesPrograms, SLOT(aboutToShowSettingsSlot()) );
        connect( this, SIGNAL(aboutToShowSettingsSignal()), preferencesPrograms, SLOT(aboutToShowSettingsSlot()) );

        // show settings box :
        this->showSettings();

    }


}


//============================================================================================================//
//                                               SLOTS                                                        //
//============================================================================================================//

void MainWindow::openFile() {

    this->centralWidget->getFileOperations()->openFile();

}

void MainWindow::openFileWithFileMode(KUrl nzbUrl, UtilityNamespace::OpenFileMode openFileMode) {

    this->centralWidget->getFileOperations()->openFileWithFileMode(nzbUrl, openFileMode);

}


void MainWindow::quit() {
    // ask to save pending downloads when quitting action performed :
    centralWidget->savePendingDownloads();
    kapp->quit();
}


bool MainWindow::queryClose() {
    // ask to save pending downloads when closing action performed :
    centralWidget->savePendingDownloads();
    return true;
}


