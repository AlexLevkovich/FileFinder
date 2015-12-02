#include "kfindsortfilterproxymodel.h"
#include <QFileInfo>

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QFileInfo)
#endif

bool KFindSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    if(left.column() == 2 || left.column() == 3) {
        qulonglong leftData = sourceModel()->data( left, Qt::UserRole ).toULongLong();
        qulonglong rightData = sourceModel()->data( right, Qt::UserRole ).toULongLong();
        return leftData < rightData;
    }
    else if (left.column() == 0) {
        QFileInfo leftData = sourceModel()->data(left,Qt::UserRole).value<QFileInfo>();
        QFileInfo rightData = sourceModel()->data(right,Qt::UserRole).value<QFileInfo>();
        if ((leftData.isDir() && rightData.isDir()) ||
            (!leftData.isDir() && !rightData.isDir())) {
            return QSortFilterProxyModel::lessThan(left, right);
        }
        else if (leftData.isDir() && !rightData.isDir()) return true;
        else if (!leftData.isDir() && rightData.isDir()) return false;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}


