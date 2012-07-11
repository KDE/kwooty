#ifndef CATEGORIESMODEL_H
#define CATEGORIESMODEL_H

#include <QStandardItemModel>

#include "mimedata.h"

class CategoriesModel : public QStandardItemModel {

    Q_OBJECT

public:

    // custom roles used for storing data in items :
    enum CategoriesRoles {
        MimeRole  = Qt::UserRole + 1,
    };

    // column definitions :
    enum ColumnNumber {
        ColumnCategory,
        ColumnTarget
    };


    CategoriesModel(QObject*);

    MimeData loadMimeData(QStandardItem*);
    MimeData loadMimeData(const QModelIndex&);
    QList<MimeData> retrieveMimeDataListFromItem(QStandardItem*);
    QStandardItem* retrieveItemFromCategory(const QString&, QStandardItem* = 0);
    QString getMainCategory(const QModelIndex&);
    QString getMainCategory(QStandardItem*);
    int retrieveLexicalTextPosition(const QString&, QStandardItem*);
    bool isDuplicateSubCategory(QStandardItem*, const QString&);
    bool isSelectedItemParent(QStandardItem*);
    bool isSelectedItemParent(const QModelIndex&);
    void storeMimeData(QStandardItem*, MimeData);
    void addParentCategoryListToModel(const QStringList&);

    QStandardItem* getCategoryItem(QStandardItem*);
    QStandardItem* getCategoryItem(const QModelIndex&);
    QStandardItem* getTargetItem(QStandardItem*);
    QStandardItem* getColumnItem(const QModelIndex&, CategoriesModel::ColumnNumber);
    QStandardItem* getParentItem(const QModelIndex&);


private:

    bool stringPos(const QString&, const QString&, int&);

signals:

public slots:

};

#endif // CATEGORIESMODEL_H
