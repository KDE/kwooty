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
#include <KFileDialog>
#include <KConfigDialog>
#include <KPageDialog>
#include <KPageWidgetItem>
#include <KMessageBox>
#include <KIO/NetAccess>
#include <KSaveFile>
#include <KDebug>

#include <QTextStream>
#include <QtGui>

#include "settings.h"
#include "mystatusbar.h"
#include "centralwidget.h"
#include "preferences/preferencesserver.h"
#include "preferences/preferencesgeneral.h"
#include "preferences/preferencesprograms.h"
#include "preferences/preferencesdisplay.h"



MainWindow::MainWindow(QWidget *parent): KXmlGuiWindow(parent), fileName(QString())
{

    //setup statusBar :
    statusBar = new MyStatusBar(this);
    this->setStatusBar(statusBar);

    // create the user interface :
    QWidget* widget = new QWidget(this);
    centralWidget = new CentralWidget(widget, statusBar);

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
    connect(clearAction, SIGNAL(triggered(bool)), centralWidget, SLOT(clearSlot()));

    //startDownloadAction
    KAction* startDownloadAction = new KAction(this);
    startDownloadAction->setText(i18n("Start"));
    startDownloadAction->setIcon(KIcon("media-playback-start"));
    startDownloadAction->setToolTip(i18n("Start download of selected rows"));
    startDownloadAction->setShortcut(Qt::CTRL + Qt::Key_S);
    startDownloadAction->setEnabled(false);
    actionCollection()->addAction("start", startDownloadAction);
    connect(startDownloadAction, SIGNAL(triggered(bool)), centralWidget, SLOT(startDownloadSlot()));
    connect(centralWidget, SIGNAL(setStartButtonEnabledSignal(bool)), startDownloadAction, SLOT(setEnabled(bool)) );

    //pauseDownloadAction
    KAction* pauseDownloadAction = new KAction(this);
    pauseDownloadAction->setText(i18n("Pause"));
    pauseDownloadAction->setIcon(KIcon("media-playback-pause"));
    pauseDownloadAction->setToolTip(i18n("Pause download of selected rows"));
    pauseDownloadAction->setShortcut(Qt::CTRL + Qt::Key_P);
    pauseDownloadAction->setEnabled(false);
    actionCollection()->addAction("pause", pauseDownloadAction);
    connect(pauseDownloadAction, SIGNAL(triggered(bool)), centralWidget, SLOT(pauseDownloadSlot()));
    connect(centralWidget, SIGNAL(setPauseButtonEnabledSignal(bool)), pauseDownloadAction, SLOT(setEnabled(bool)) );

    //removeItemAction
    KAction* removeItemAction = new KAction(this);
    removeItemAction->setText(i18n("Remove"));
    removeItemAction->setIcon(KIcon("list-remove"));
    removeItemAction->setToolTip(i18n("Remove all selected rows"));
    removeItemAction->setShortcut(Qt::Key_Delete);
    removeItemAction->setEnabled(false);
    actionCollection()->addAction("remove", removeItemAction);
    connect(removeItemAction, SIGNAL(triggered(bool)), centralWidget, SLOT(removeRowSlot()));
    connect(centralWidget, SIGNAL(setMoveButtonEnabledSignal(bool)), removeItemAction, SLOT(setEnabled(bool)) );
    connect(centralWidget, SIGNAL(setRemoveButtonEnabledSignal(bool)), removeItemAction, SLOT(setEnabled(bool)) );

    //moveToTopAction
    KAction* moveToTopAction = new KAction(this);
    moveToTopAction->setText(i18n("Top"));
    moveToTopAction->setIcon(KIcon("go-top"));
    moveToTopAction->setToolTip(i18n("Move all selected rows to the top of the list"));
    moveToTopAction->setShortcut(Qt::CTRL + Qt::Key_Up);
    moveToTopAction->setEnabled(false);
    actionCollection()->addAction("moveTop", moveToTopAction);
    connect(moveToTopAction, SIGNAL(triggered(bool)), centralWidget, SLOT(moveToTopSlot()));
    connect(centralWidget, SIGNAL(setMoveButtonEnabledSignal(bool)), moveToTopAction, SLOT(setEnabled(bool)) );

    //moveToTopBottom
    KAction* moveToBottomAction = new KAction(this);
    moveToBottomAction->setText(i18n("Bottom"));
    moveToBottomAction->setIcon(KIcon("go-bottom"));
    moveToBottomAction->setToolTip(i18n("Move all selected rows to the bottom of the list"));
    moveToBottomAction->setShortcut(Qt::CTRL + Qt::Key_Down);
    moveToBottomAction->setEnabled(false);
    actionCollection()->addAction("moveBottom", moveToBottomAction);
    connect(moveToBottomAction, SIGNAL(triggered(bool)), centralWidget, SLOT(moveToBottomSlot()));
    connect(centralWidget, SIGNAL(setMoveButtonEnabledSignal(bool)), moveToBottomAction, SLOT(setEnabled(bool)) );


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
        KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());

        PreferencesGeneral* preferencesGeneral = new PreferencesGeneral();
        dialog->addPage( preferencesGeneral, i18n("General"), "preferences-system", i18n("General Setup"));

        PreferencesServer* preferencesServer = new PreferencesServer();
        dialog->addPage( preferencesServer, i18n("Connection"), "network-workgroup", i18n("Setup Server Connection"));

        PreferencesPrograms* preferencesPrograms = new PreferencesPrograms();
        dialog->addPage( preferencesPrograms, i18n("Programs"), "system-run", i18n("Setup External Programs"));

        PreferencesDisplay* preferencesDisplay = new PreferencesDisplay();
        dialog->addPage( preferencesDisplay, i18n("Display modes"), "view-choose", i18n("Setup display modes"));

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

    bool isWrongUrl = false;

    QStringList fileNameFromDialogList = KFileDialog::getOpenFileNames(KUrl(), "*.nzb|" + i18n("nzb files"), this);

    // process selected file(s) :
    for (int i = 0; i < fileNameFromDialogList.size(); i++) {

        QString fileNameFromDialog = fileNameFromDialogList.at(i);

        if (!fileNameFromDialog.isNull() || !fileNameFromDialog.isEmpty()) {

            this->openUrl(KUrl(fileNameFromDialog), isWrongUrl, UtilityNamespace::OpenNormal);

        } // end of iteration loop


        // If url cannot be reached open an error message box
        if (isWrongUrl){
            KMessageBox::error(this, KIO::NetAccess::lastErrorString());
        }


    }

}


void MainWindow::openFileByExternalApp(KUrl url) {

    bool isWrongUrl = false;

    // if file is opened by file or internet browser :
    this->openUrl(url, isWrongUrl, UtilityNamespace::OpenWith);

    // If url cannot be reached open an error message box
    if (isWrongUrl){
        KMessageBox::error(this, KIO::NetAccess::lastErrorString());
    }

}



void MainWindow::openUrl(KUrl url, bool& isWrongUrl, UtilityNamespace::OpenFileMode openFileMode) {

    QString downloadFile;

    if(KIO::NetAccess::download(url, downloadFile, this)){

        QFile file(downloadFile);

        // Open the nzb file :
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            KMessageBox::error(this, KIO::NetAccess::lastErrorString());
        }

        // add nzbFile data to the view :
        centralWidget->handleNzbFile(file);

        file.close();        

        // copy nzb file in its associated download folder if file has been open has been triggered by another app  :
        if (Settings::openWith() &&
            (openFileMode == UtilityNamespace::OpenWith)) {

            //remove .nzb extension to file name :
            QFileInfo fileInfo(file.fileName());
            QString nzbBaseName = fileInfo.completeBaseName();

            // create download folder :
            QString downloadFolderPath = Settings::completedFolder().path() + '/' + nzbBaseName;
            Utility::createFolder(downloadFolderPath);

            // copy nzb file in created download folder :
            file.copy(downloadFolderPath + '/' + url.fileName());

        }

        // remove temporary downloaded file :
        KIO::NetAccess::removeTempFile(downloadFile);

    }
    // the url can not be opened
    else {
        isWrongUrl = true;
    }

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


