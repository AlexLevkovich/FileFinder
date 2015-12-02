/*******************************************************************
* kfindtreeview.cpp
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
#include <gio/gio.h>
#include "kfindtreeview.h"
#include "byteshumanizer.h"
#include "finddialog.h"

#include <QTextStream>
#include <QTextCodec>
#include <QFileInfo>
#include <QClipboard>
#include <QHeaderView>
#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QProcess>
#include <QMimeData>

#include <QFileDialog>
#include <QLocale>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QFileIconProvider>
#include <QMimeDatabase>
#include "scrollmessagebox.h"

extern QMimeDatabase  *mime_database;
#ifdef WIN32
    extern QFileIconProvider * ip;
#endif

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QFileInfo)
#endif

// Permission strings
static const char* const perm[4] = {
                  "Read-write",
                  "Read-only",
                  "Write-only",
                  "Inaccessible" };
#define RW 0
#define RO 1
#define WO 2
#define NA 3

#define MAX_CHUNK_COUNT 500

QIcon FileInfo::icon() const {
    QIcon icon;
#ifdef WIN32
    icon=ip->icon(*this);
    if (icon.isNull()) icon=ip->icon(QFileIconProvider::File);
#else
    QMimeType type = mime_database->mimeTypeForFile(filePath());
    icon = QIcon::fromTheme(type.iconName());
    if (icon.isNull()) icon = QIcon::fromTheme(type.genericIconName());
#endif
    return icon;
}

QDir FileInfo::dir() const {
    if (!isDir()) return QFileInfo::dir();
    return QDir(filePath());
}

const QString KFindItem::permString(int index) {
    switch (index) {
    case RW:
        return QObject::tr("Read-write");
    case RO:
        return QObject::tr("Read-only");
    case WO:
        return QObject::tr("Write-only");
    case NA:
        return QObject::tr("Inaccessible");
    }
    return QString();
}

//BEGIN KFindItemModel

KFindItemModel::KFindItemModel( KFindTreeView * parentView ) : QAbstractTableModel( parentView ) {
    m_view = parentView;
    items_chunk_count = 0;
    real_count = 0;
}

QVariant KFindItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ( role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return QObject::tr("Name");
            case 1:
                return QObject::tr("In Subfolder");
            case 2:
                return QObject::tr("Size");
            case 3:
                return QObject::tr("Modified");
            case 4:
                return QObject::tr("Permissions");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

void KFindItemModel::insertFileItems( const QList<FileInfo> & infos) {
    if ( infos.size() > 0 ) {
        for (int i=0;i<infos.count();i++) {
            FileInfo info = infos.at(i);
            QDir dir = info.dir();
            if (info.isDir()) dir.cdUp();
            QString subDir = m_view->reducedDir(dir.path());
            m_itemList.append(KFindItem(info,subDir));
            items_chunk_count++;
        }

        if (items_chunk_count >= MAX_CHUNK_COUNT) {
            beginInsertRows( QModelIndex(), rowCount(), rowCount()+items_chunk_count-1 );
            real_count += items_chunk_count;
            items_chunk_count = 0;
            endInsertRows();
        }
    }
}

void KFindItemModel::complete() {
    if (items_chunk_count > 0) {
        beginInsertRows( QModelIndex(), rowCount(), rowCount()+items_chunk_count-1 );
        real_count += items_chunk_count;
        items_chunk_count = 0;
        endInsertRows();
    }
}

int KFindItemModel::rowCount( const QModelIndex & parent ) const {
    if( !parent.isValid() )
        return real_count; //Return itemcount for toplevel
    else
        return 0;
}

KFindItem KFindItemModel::itemAtIndex( const QModelIndex & index ) const {
    if ( index.isValid() && rowCount() >= index.row() )
        return m_itemList.at( index.row() );

    return KFindItem();
}

QVariant KFindItemModel::data ( const QModelIndex & index, int role ) const {
    if (!index.isValid()) return QVariant();

    if (index.column() > 6 || index.row() >= rowCount() || index.row() < 0) return QVariant();

    switch( role ) {
        case Qt::DisplayRole:
        case Qt::DecorationRole:
        case Qt::UserRole:
            return m_itemList.at( index.row() ).data( index.column(), role );
        default:
            return QVariant();
    }
    return QVariant();
}

void KFindItemModel::removeItem( const QUrl & url ) {
    int itemCount = rowCount();
    for ( int i = 0; i < itemCount; i++) {
        KFindItem item = m_itemList.at(i);
        if (QUrl::fromLocalFile(item.getFileItem().filePath()) == url) {
            beginRemoveRows( QModelIndex(), i, i ); 
            m_itemList.removeAt( i );
            real_count--;
            endRemoveRows();
            return;
        }
    }
}

bool KFindItemModel::isInserted( const QUrl & url ) {
    int itemCount = rowCount();
    for ( int i = 0; i < itemCount; i++) {
        KFindItem item = m_itemList.at(i);
        if (QUrl::fromLocalFile(item.getFileItem().filePath()) == url) {
            return true;
        }
    }
    return false;
}

void KFindItemModel::clear() {
    beginRemoveRows( QModelIndex(), 0, rowCount() );
    m_itemList.clear();
    real_count = 0;
    endRemoveRows();
}

Qt::ItemFlags KFindItemModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.isValid())
        return Qt::ItemIsDragEnabled | defaultFlags;
    return defaultFlags;
}
 
QMimeData * KFindItemModel::mimeData(const QModelIndexList &indexes) const {
    QList<QUrl> uris;
    
    foreach (const QModelIndex & index, indexes) {
        if(index.isValid()) {
            if(index.column() == 0) { //Only use the first column item
                uris.append(QUrl(m_itemList.at(index.row()).getFileItem().filePath()));
            }
        }
    }

    if (uris.count() <= 0) return 0;

    QMimeData * mimeData = new QMimeData();
    QByteArray data;
    foreach (const QUrl & url, uris) {
        data += url.toString().toLocal8Bit() + "\r\n";
    }

    mimeData->setData("text/plain",data);
    return mimeData;
}

//END KFindItemModel

//BEGIN KFindItem

KFindItem::KFindItem( const FileInfo & fileItem, const QString & subDir) {
    m_subDir = subDir;

    int perm_index;
    if(fileItem.isReadable()) perm_index = fileItem.isWritable() ? RW : RO;
    else perm_index = fileItem.isWritable() ? WO : NA;
            
    m_permission = KFindItem::permString(perm_index);
    m_fileName = fileItem.fileName();
    m_fileInfo = QFileInfo(fileItem.filePath());
    m_fileSize = BytesHumanizer(fileItem.size()).toString();
    m_fileDate = fileItem.lastModified().toString(Qt::DefaultLocaleShortDate);
    m_icon = fileItem.icon();
}

QVariant KFindItem::data( int column, int role ) const {
    if( role == Qt::DecorationRole ) {
        if (column == 0) return m_icon;
        else return QVariant();
    }
        
    if( role == Qt::DisplayRole )
        switch( column ) {
            case 0:
                return m_fileName;
            case 1:
                return m_subDir;
            case 2:
                return m_fileSize;
            case 3:
                return m_fileDate;
            case 4:
                return m_permission;
            default:
                return QVariant();
        }
        
    if( role == Qt::UserRole )
        switch( column )
        {
            case 0:
                return QVariant::fromValue(m_fileInfo);
            case 2:
                return m_fileSize.toInt();
            case 3:
                return m_fileInfo.lastModified().time().toString("hh:mm:ss");
            default:
                return QVariant();
        }
    
    return QVariant();
}

class MimeProcess: public QProcess {
    Q_OBJECT
public:
    MimeProcess(const QString & filePath,QObject * parent = NULL) : QProcess(parent) {
        this->filePath = filePath;

        connect(this,SIGNAL(error(QProcess::ProcessError)),this,SLOT(slot_finished()));
        connect(this,SIGNAL(error(QProcess::ProcessError)),this,SLOT(deleteLater()));
        connect(this,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_finished(int,QProcess::ExitStatus)));
        connect(this,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(deleteLater()));

        start(QString("xdg-open %1").arg(filePath));
    }

private slots:
    void slot_finished() {
        slot_finished(1,QProcess::NormalExit);
    }

    void slot_finished(int code,QProcess::ExitStatus) {
        if (code == 0) return;

        GError *error;
        GFileInfo *file_info = NULL;
        GAppInfo *app_info = NULL;
        GFile *file = g_file_new_for_path(filePath.toLocal8Bit().constData());
        if (file != NULL) {
            file_info = g_file_query_info(file,"standard::*",(GFileQueryInfoFlags)0,NULL,&error);
            if (file_info != NULL) {
                const char *content_type = g_file_info_get_content_type (file_info);
                if (content_type != NULL) {
                    app_info = g_app_info_get_default_for_type(content_type,FALSE);
                    if (app_info != NULL) {
                        QProcess::startDetached(QString::fromLocal8Bit(QByteArray(g_app_info_get_executable(app_info))) + " " + filePath);
                    }
                }
            }
        }
        if (file != NULL) g_object_unref(file);
        if (file_info != NULL) g_object_unref(file_info);
        if (app_info != NULL) g_object_unref(app_info);
    }

private:
    QString filePath;
};

void KFindItem::execute() {
    new MimeProcess(getFileItem().filePath());
}

//END KFindItem

//BEGIN KFindTreeView

KFindTreeView::KFindTreeView( QWidget *parent) : QTreeView( parent ) {
    //Configure model and proxy model
    m_model = new KFindItemModel( this );
    m_proxyModel = new KFindSortFilterProxyModel();
    m_proxyModel->setSourceModel( m_model );
    setModel( m_proxyModel );
    
    //Configure QTreeView
    setRootIsDecorated( false );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSortingEnabled( true );
    setDragEnabled( true );
    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
                 this, SLOT(contextMenuRequested(QPoint)));
           
    connect(this,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(slotExecute(QModelIndex)));
    
    m_contextMenu.addAction(QIcon(":/images/document-open.png"),tr("Open"),this,SLOT(slotExecuteSelected()));
    m_contextMenu.addAction(QIcon(":/images/edit-copy.png"),tr("Copy"),this,SLOT(copySelection()));
    m_contextMenu.addAction(QIcon(":/images/document-open-folder.png"),tr("Open containing folder(s)"),this,SLOT(openContainingFolder()));
    m_contextMenu.addAction(QIcon(":/images/edit-delete.png"),tr("Delete"),this,SLOT(deleteSelectedFiles()));
    m_contextMenu.addSeparator();
    m_contextMenu.addAction(tr("Select All"),this,SLOT(selectAll()));
                
    header()->setStretchLastSection( true );
}

KFindTreeView::~KFindTreeView() {
    delete m_model;
    delete m_proxyModel;
}

void KFindTreeView::resizeToContents() {
    resizeColumnToContents( 0 );
    resizeColumnToContents( 1 );
    resizeColumnToContents( 2 );
    resizeColumnToContents( 3 );
}

QString KFindTreeView::reducedDir(const QString& fullDir) {
    if (fullDir.indexOf(m_baseDir)==0) {
        QString tmp=fullDir.mid(m_baseDir.length());
        if (tmp.startsWith("/")) return tmp.mid(1);
        else return tmp;
    };
    return fullDir;
}

void KFindTreeView::beginSearch(const QUrl& baseUrl) {
    qDebug() << QString("beginSearch in: %1").arg(baseUrl.path());
    m_baseDir = baseUrl.path();
    m_model->clear();
    sortByColumn(-1, Qt::AscendingOrder);
}

void KFindTreeView::endSearch() {
    m_model->complete();
    sortByColumn(0,Qt::AscendingOrder);
    resizeToContents();
    qDebug() << QString("endSearch");
}

void KFindTreeView::insertItems(const QList<FileInfo> & infos) {
    m_model->insertFileItems( infos );
}

void KFindTreeView::insertItem(const FileInfo & info) {
    m_model->insertFileItems( QList<FileInfo>() << info );
}


void KFindTreeView::removeItem(const QUrl & url) {
    QList<QUrl> list = selectedUrls();
    if (list.contains(url)) {
        m_contextMenu.hide();
    }
    m_model->removeItem(url);
}

// copy to clipboard
void KFindTreeView::copySelection() {
    QMimeData * mime = m_model->mimeData( m_proxyModel->mapSelectionToSource( selectionModel()->selection() ).indexes() );
    if (mime) {
        QClipboard * cb = qApp->clipboard();
        cb->setMimeData( mime );
    }
}

void KFindTreeView::saveResults() {
    QString filename = QFileDialog::getSaveFileName(this,tr("Save Results As"),QDir::homePath(),QString("*.txt|%1").arg(tr("Text file")));

    if (filename.isEmpty()) return;

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this,tr("Error!"),tr("Unable to save results."));
    }
    else {
        QTextStream stream( &file );
        stream.setCodec(QTextCodec::codecForLocale());
        
        QList<KFindItem> itemList = m_model->getItemList();
        Q_FOREACH(const KFindItem & item, itemList) {
            stream << item.getFileItem().filePath() << endl;
        }

        file.close();
        QMessageBox::information(this,tr("Information..."),tr("Results were saved to: %1").arg(filename));
    }
}

void KFindTreeView::openContainingFolder()
{
    QList<QUrl> uris = selectedUrls();
    
    //Generate *unique* folders
    Q_FOREACH(const QUrl & url, uris) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(url.path()).dir().path()));
    }
}

void KFindTreeView::slotExecuteSelected() {
    QModelIndexList selected = m_proxyModel->mapSelectionToSource(selectionModel()->selection()).indexes();
    if (selected.size() == 0) return;
    
    //TODO if >X add a warn ?
    Q_FOREACH(const QModelIndex & index, selected) {
        if(index.column() == 0) {
            KFindItem item = m_model->itemAtIndex(index);
            if (item.isValid()) item.execute();
        }
    }
}

void KFindTreeView::slotExecute(const QModelIndex & index) {
    if (!index.isValid()) return;
            
    QModelIndex realIndex = m_proxyModel->mapToSource(index);
    if (!realIndex.isValid()) return;
            
    KFindItem item = m_model->itemAtIndex(realIndex);
    if (item.isValid()) item.execute();
}

void KFindTreeView::contextMenuRequested(const QPoint & p) {
    if (m_proxyModel->mapSelectionToSource( selectionModel()->selection()).indexes().size() == 0) return;
    m_contextMenu.exec(mapToGlobal(p));
}

QList<QUrl> KFindTreeView::selectedUrls() {
    QList<QUrl> uris;
    
    QModelIndexList indexes = m_proxyModel->mapSelectionToSource(selectionModel()->selection()).indexes();
    Q_FOREACH(const QModelIndex & index, indexes) {
        if(index.column() == 0 && index.isValid()) {
            KFindItem item = m_model->itemAtIndex(index);
            if(item.isValid()) uris.append(QUrl::fromLocalFile(item.getFileItem().filePath()));
        }
    }
    
    return uris;
}

QString KFindTreeView::urlListToString(const QList<QUrl> & uris) const {
    QString ret;
    Q_FOREACH(const QUrl & url, uris) {
        ret += url.toLocalFile()+"\n";
    }
    return ret;
}

void KFindTreeView::deleteSelectedFiles() {
    QList<QUrl> uris = selectedUrls();
    if (uris.isEmpty()) return;

    if (ScrollMessageBox::question(this,tr("Deleting the files..."),tr("Do you want to delete the items below?\n\n%1").arg(urlListToString(uris))) != QMessageBox::Yes) return;

    Q_FOREACH(const QUrl & url, uris) {
        QFile::remove(url.path());
        removeItem(url);
    }
}

#include "kfindtreeview.moc"

//END KFindTreeView
