#include "findthread.h"
#include <QDirIterator>
#include <QMimeType>
#include <QTextStream>
#include <QTimer>
#include <QDebug>

FindThread::FindThread(const FindThreadParms & parms,QObject * parent) : QThread(parent) {
    m_parms = parms;

    QDir::Filters filters = 0;
    use_mime = true;
    if (m_parms.mimeType() == "all/all") {
        filters = QDir::Dirs | QDir::Files;
        use_mime = false;
    }
    else if (m_parms.mimeType() == "all/allfiles") {
        filters = QDir::Files;
        use_mime = false;
    }
    else if (m_parms.mimeType() == "inode/directory") {
        filters = QDir::Dirs;
        use_mime = false;
    }
    else filters = QDir::Files;
    if (m_parms.isCaseSensitive()) filters |= QDir::CaseSensitive;
    if (m_parms.useHidden()) filters |= QDir::Hidden;
    p_files = new QDirIterator(m_parms.findDir().path(),filters,m_parms.useSubDirs()?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags);
    p_timer = new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(do_next()));
    p_timer->start(0);
}

FindThread::~FindThread() {
    delete p_files;
    delete p_timer;
}

void FindThread::do_next() {
    if (!p_files->hasNext()) {
        quit();
        return;
    }

    QString file_path = p_files->next();
    QFileInfo file_info(file_path);
    QString file_name = file_info.fileName();
    if (file_name == "." || file_name == "..") return;
    if (!m_parms.fileMask().exactMatch(file_name)) return;
    if (use_mime && !db.mimeTypeForFile(file_path).inherits(m_parms.mimeType())) return;
    if (!m_parms.text().isEmpty()) {
        if (isSequentialFile(file_path)) return;
        if (!isTextExists(file_path,m_parms.text(),m_parms.isTextCaseSensitive())) return;
    }
    if (!m_parms.fromDate().isNull() && !m_parms.toDate().isNull()) {
        if (file_info.lastModified() > m_parms.toDate() ||
            file_info.lastModified() < m_parms.fromDate()) return;
    }
    qint64 size = file_info.size();
    switch ((int)m_parms.sizeOper()) {
    case FindThreadParms::AT_LEAST:
        if (!file_info.isFile() || (size < m_parms.bytesToCompare())) return;
        break;
    case FindThreadParms::AT_MOST:
        if (!file_info.isFile() || (size > m_parms.bytesToCompare())) return;
        break;
    case FindThreadParms::AQUAL_TO:
        if (!file_info.isFile() || (size != m_parms.bytesToCompare())) return;
        break;
    }
    if (!m_parms.fileOwner().isEmpty()) {
        if (m_parms.fileOwner() != file_info.owner()) return;
    }
    if (!m_parms.fileGroup().isEmpty()) {
        if (m_parms.fileGroup() != file_info.group()) return;
    }
    emit entry_found(file_path);
}

#define READ_BUFFER 4096

bool FindThread::isTextExists(const QString & file_name,const QString & text,bool caseSensitive) {
    QFile file(file_name);
    if (!file.open(QFile::ReadOnly)) return false;
    QTextStream stream(&file);
    buffer.truncate(0);
    QString _buffer;
    bool doTruncate = true;
    while (!stream.atEnd()) {
        _buffer = stream.read(READ_BUFFER);
        if (!_buffer.isNull()) {
            buffer.append(_buffer);
            if (buffer.indexOf(text,0,caseSensitive?Qt::CaseSensitive:Qt::CaseInsensitive) >= 0) return true;
            doTruncate = true;
            for (int len=(text.length()-1);len>=1;len--) {
                if (buffer.endsWith(text.left(len),caseSensitive?Qt::CaseSensitive:Qt::CaseInsensitive)) {
                    doTruncate = false;
                    break;
                }
            }
            if (doTruncate) buffer.truncate(0);
        }
    }

    return false;
}

bool FindThread::isSequentialFile(const QString & file_name) {
    QFile file(file_name);
    if (!file.open(QFile::ReadOnly)) return true;

    return file.isSequential();
}

bool FindThread::hasBinaryData(const QString & file_name) {
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly)) return false;
    const QByteArray data = file.read(32);
    return isBufferBinaryData(data);
}

bool FindThread::isBufferBinaryData(const QByteArray & data) {
    const char* p = data.data();
    const int end = qMin(32, data.size());
    for (int i = 0; i < end; ++i) {
        if ((unsigned char)(p[i]) < 32 && p[i] != 9 && p[i] != 10 && p[i] != 13) // ASCII control character
            return true;
    }
    return false;
}
