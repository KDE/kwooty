#include "centralwidget.h"

#include <KMessageBox>
#include <KPasswordDialog>

#include "mainwindow.h"
#include "datarestorer.h"
#include "kwootysettings.h"


CentralWidget::CentralWidget(MainWindow* parent) : QWidget(parent) {

    // init button code that avoid to display one message box per nntp client instance error :
    this->saveErrorButtonCode = 0;

}



bool CentralWidget::isDialogExisting() {
    return !(this->saveErrorButtonCode == 0);
}


void CentralWidget::saveFileError(const int fromProcessing) {

    // notify user with a message box (and avoid multiple message box instances):
    if (this->saveErrorButtonCode == 0) {

        QString saveErrorFolder;

        if (fromProcessing == DuringDecode) {
            saveErrorFolder = i18n("download folder");
        }
        if (fromProcessing == DuringDownload) {
            saveErrorFolder = i18n("temporary folder");
        }


        this->saveErrorButtonCode = KMessageBox::Cancel;
        this->saveErrorButtonCode = KMessageBox::messageBox(this,
                                                            KMessageBox::Sorry,
                                                            i18n("Write error in <b>%1</b>: disk drive may be full.<br>Downloads have been suspended.",
                                                                 saveErrorFolder),
                                                            i18n("Write error"));


        if (this->saveErrorButtonCode == KMessageBox::Ok) {
            this->saveErrorButtonCode = 0;
        }

    }

}


QString CentralWidget::extractPasswordRequired(const QString& currentArchiveFileName, bool& passwordEntered) {

    QString password;

    KPasswordDialog kPasswordDialog(this);
    kPasswordDialog.setPrompt(i18n("The archive <b>%1</b> is password protected. <br>Please enter the password to extract the file.",  currentArchiveFileName));

    // if password has been entered :
    if(kPasswordDialog.exec()) {
        passwordEntered = true;
        password = kPasswordDialog.password();
    }
    else {
        passwordEntered = false;
    }

    return password;

}


void CentralWidget::displayNzbHandleErrorMessageBox(const QString& fileName) {

    KMessageBox::messageBox(this,
                            KMessageBox::Sorry,
                            i18n("The file <b>%1</b> can not be processed",
                                 fileName),
                            i18n("File process error"));

}


int CentralWidget::displayRestoreMessageBox() {

    int answer = KMessageBox::Yes;

    // ask question if confirmRestoreSilently is checked:
    if (Settings::restoreDownloadsMethods() == DataRestorer::WithConfirmation) {

        answer = KMessageBox::messageBox(this,
                                         KMessageBox::QuestionYesNo,
                                         i18n("Reload pending downloads from previous session ?"));
    }

    return answer;

}


int CentralWidget::displaySaveMessageBox(SaveFileBehavior saveFileBehavior) {

    int answer = KMessageBox::Yes;

    if (saveFileBehavior == SaveNotSilently) {

        // ask question if confirmSaveSilently is checked:
        if (Settings::saveDownloadsMethods() == DataRestorer::WithConfirmation) {

            answer = KMessageBox::messageBox(this,
                                             KMessageBox::QuestionYesNoCancel,
                                             i18n("Save pending downloads from current session ?"));
        }

    }

    return answer;
}


void CentralWidget::displaySorryMessageBox(const QString& message) {

    KMessageBox::messageBox(this,
                            KMessageBox::Sorry,
                            message);

}


int CentralWidget::displayRemoveAllFilesMessageBox() {

    return KMessageBox::messageBox(this,
                                   KMessageBox::QuestionYesNo,
                                   i18n("Remove all files from queue ?"));

}


int CentralWidget::displayRemoveSelectedFilesMessageBox() {

    return KMessageBox::messageBox(this,
                                   KMessageBox::QuestionYesNo,
                                   i18n("Remove selected files from queue ?"));
}


int CentralWidget::displayMergeItemsMessageBox(const QString& selectedNzbFileName, const QString& targetNzbFileName) {

    return KMessageBox::messageBox(this,
                                   KMessageBox::QuestionYesNo,
                                   i18n("Merge content of %1 into %2 ?", "<br><b>" + selectedNzbFileName + "</b><br>",
                                                                         "<br><b>" + targetNzbFileName + "</b>"));

}



int CentralWidget::displayAboutToShutdownMessageBox(const QString& shutdownMethodText) {

    // create kdialog :
    this->aboutToShutdownDialog = new KDialog(this, Qt::Dialog);
    this->aboutToShutdownDialog->setCaption(i18n("Warning"));
    this->aboutToShutdownDialog->setButtons(KDialog::Yes | KDialog::No);
    this->aboutToShutdownDialog->setModal(true);

    // display text for continue button
    KGuiItem buttonContinue = KStandardGuiItem::cont();
    buttonContinue.setText(shutdownMethodText);
    this->aboutToShutdownDialog->setButtonGuiItem(KDialog::Yes, buttonContinue);

    // set cancel button :
    this->aboutToShutdownDialog->setButtonGuiItem(KDialog::No, KStandardGuiItem::cancel());

    // display kmessagebox :
    bool checkboxReturn = false;
    QString status = i18nc("%1 = shutdown/suspend to RAM/suspend to disk",
                           "Kwooty is about to %1 system. Continue?", shutdownMethodText.toLower());
    return KMessageBox::createKMessageBox(this->aboutToShutdownDialog,
                                          QMessageBox::Warning,
                                          status,
                                          QStringList(),
                                          QString(),
                                          &checkboxReturn,
                                          KMessageBox::Notify);

}


void CentralWidget::closeAboutToShutdownMessageBox() {

    if (this->aboutToShutdownDialog) {
        this->aboutToShutdownDialog->reject();
    }

}

