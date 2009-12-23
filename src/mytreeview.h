#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <KUrl>

#include <QTreeView>
#include <QModelIndex>
#include <QStandardItem>

#include <QDragEnterEvent>
#include <QDragMoveEvent>


#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class StandardItemModel;

class MyTreeView : public QTreeView
{
    Q_OBJECT

public:
    MyTreeView(QWidget*, CentralWidget*);
    void moveRow(bool);


private:
    CentralWidget* centralWidget;
    StandardItemModel* downloadModel;

    void setHeaderLabels();
    void setupConnections();

protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);


signals:
    void setMoveButtonEnabledSignal(bool);
    void setPauseButtonEnabledSignal(bool);
    void setStartButtonEnabledSignal(bool);
    void setRemoveButtonEnabledSignal(bool);
    void statusBarFileSizeUpdateSignal(StatusBarUpdateType);
    void recalculateNzbSizeSignal(const QModelIndex);
    void changePar2FilesStatusSignal(const QModelIndex, UtilityNamespace::ItemStatus);
    void openFileByDragAndDropSignal(KUrl);

public slots:
    void selectedItemSlot();
    void moveToTopSlot();
    void moveToBottomSlot();
    void removeRowSlot();
    void clearSlot();
    void settingsChangedSlot();

    // #QTBUG-5201
#if QT_VERSION == 0x040503
    void dataChangedSlot(QStandardItem*);
#endif

private slots:


};

#endif // MYTREEVIEW_H
