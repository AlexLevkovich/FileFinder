#ifndef KFINDSORTFILTERPROXYMODEL_H
#define KFINDSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class KFindSortFilterProxyModel: public QSortFilterProxyModel {
    Q_OBJECT

    public:
        KFindSortFilterProxyModel(QObject * parent = 0) : QSortFilterProxyModel(parent) {};

    protected:
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

};

#endif // KFINDSORTFILTERPROXYMODEL_H
