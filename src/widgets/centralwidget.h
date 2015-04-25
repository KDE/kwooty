#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>

#include <KDialog>
#include <QPointer>

#include "utilities/utility.h"
using namespace UtilityNamespace;

class MainWindow;


class CentralWidget : public QWidget {

    Q_OBJECT

public:
    CentralWidget(MainWindow*);

    QString extractPasswordRequired(const QString&, bool&);
    int displayRestoreMessageBox();
    int displaySaveMessageBox(SaveFileBehavior);
    int displayAboutToShutdownMessageBox(const QString&);
    int displayRemoveAllFilesMessageBox();
    int displayRemoveSelectedFilesMessageBox();
    int displayMergeItemsMessageBox(const QString&, const QString&);
    bool isDialogExisting();
    void saveFileError(const int);
    void displaySorryMessageBox(const QString&);
    void closeAboutToShutdownMessageBox();
    void displayNzbHandleErrorMessageBox(const QString&);


private:
    int saveErrorButtonCode;
    QPointer<KDialog> aboutToShutdownDialog;


Q_SIGNALS:

    
public Q_SLOTS:


    
};

#endif // CENTRALWIDGET_H
