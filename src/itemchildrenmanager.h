#ifndef ITEMCHILDRENMANAGER_H
#define ITEMCHILDRENMANAGER_H

#include <QObject>
#include "itemabstractupdater.h"
#include "utility.h"
using namespace UtilityNamespace;

class CentralWidget;
class StandardItemModel;


class ItemChildrenManager : public ItemAbstractUpdater {

    Q_OBJECT

public:
    ItemChildrenManager(CentralWidget*, ItemParentUpdater*);

private:
    CentralWidget* parent;
    void setupConnections();
    bool displayIcons;
    bool smartPar2Download;

signals:
    void downloadWaitingPar2Signal();


public slots:
    void setIconToFileNameItemSlot(const QModelIndex);
    void changePar2FilesStatusSlot(const QModelIndex, UtilityNamespace::ItemStatus);
    void settingsChangedSlot();


private slots:


};

#endif // ITEMCHILDRENMANAGER_H
