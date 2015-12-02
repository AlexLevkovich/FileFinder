/*******************************************************************
* kfindtreeview.h
* Copyright 2009    Dario Andres Rodriguez <andresbajotierra@gmail.com>
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* 
******************************************************************/

#ifndef KFINDTREEVIEW__H
#define KFINDTREEVIEW__H

#include <QTreeView>
#include <QAbstractTableModel>
#include "kfindsortfilterproxymodel.h"
#include <QDragMoveEvent>

#include <QUrl>
#include <QIcon>
#include <QDebug>
#include <QFileInfo>
#include <QMenu>

class QMimeDatabase;
class KFindTreeView;
class QMimeDatabase;

class FileInfo : public QFileInfo {
public:
    inline FileInfo() : QFileInfo() { }
    inline FileInfo(const QString & file) : QFileInfo(file) { }
    inline FileInfo(const QFile & file) : QFileInfo(file) { }
    inline FileInfo(const QDir & dir,const QString & file) : QFileInfo(dir,file) { }
    inline FileInfo(const QFileInfo & fileinfo) : QFileInfo(fileinfo) { }

    QIcon icon() const;
    QDir dir() const;
};

class KFindItem {
    public:
        explicit KFindItem( const FileInfo & = FileInfo(), const QString & subDir = QString());
        
        QVariant data(int column, int role) const;
        
        QFileInfo getFileItem() const { return m_fileInfo; }
        bool isValid() const { return m_fileInfo.isDir() || m_fileInfo.isFile(); }
        static const QString permString(int index);
        void execute();
        
    private:
        QString         m_fileName;
        QFileInfo       m_fileInfo;
        QString         m_fileDate;
        QString         m_fileSize;
        QString         m_subDir;
        QString         m_permission;
        QIcon           m_icon;
};
 
class KFindItemModel: public QAbstractTableModel {
    public:
        KFindItemModel( KFindTreeView* parent);

        void insertFileItems( const QList<FileInfo> &);

        void removeItem(const QUrl &);
        bool isInserted(const QUrl &);
        void complete();
        
        void clear();
        
        Qt::DropActions supportedDropActions() const { return Qt::CopyAction | Qt::MoveAction; }
        
        Qt::ItemFlags flags(const QModelIndex &) const;
        QMimeData * mimeData(const QModelIndexList &) const;
        
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const {  Q_UNUSED(parent); return 5; }
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        
        KFindItem itemAtIndex( const QModelIndex & index ) const;
        
        QList<KFindItem> getItemList() const { return m_itemList; }
        
    private:
        QList<KFindItem>    m_itemList;
        KFindTreeView*        m_view;
        int items_chunk_count;
        int real_count;
};

class KFindTreeView: public QTreeView {
    Q_OBJECT
    public:
        KFindTreeView( QWidget * parent);
        ~KFindTreeView();

        void beginSearch(const QUrl& baseUrl);
        void endSearch();

        void insertItems(const QList<FileInfo> &);
        void insertItem(const FileInfo &);
        void removeItem(const QUrl & url);
        
        bool isInserted(const QUrl & url) { return m_model->isInserted( url ); }
        
        QString reducedDir(const QString& fullDir);
        
        int itemCount() { return m_model->rowCount(); }
        void saveResults();
        
    public Q_SLOTS:
        void copySelection();
        void contextMenuRequested( const QPoint & p );

    private Q_SLOTS:
        QList<QUrl> selectedUrls();
        void deleteSelectedFiles();
        void slotExecute( const QModelIndex & index );
        void slotExecuteSelected();
        void openContainingFolder();
        
    protected:
        void dragMoveEvent( QDragMoveEvent *e ) { e->accept(); }

    private:
        void resizeToContents();
        QString urlListToString(const QList<QUrl> & uris) const;
        
        QString                     m_baseDir;
        
        KFindItemModel *            m_model;
        KFindSortFilterProxyModel * m_proxyModel;
        QMenu                       m_contextMenu;
};

#endif
