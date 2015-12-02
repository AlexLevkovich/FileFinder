#ifndef FINDTHREAD_H
#define FINDTHREAD_H

#include <QThread>
#include <QRegExp>
#include <QMutex>
#include <QDir>
#include <QDateTime>
#include <QMimeDatabase>

class QDirIterator;
class QTimer;

class FindThreadParms {
public:
    enum SIZE_OPER {
        NONE = 0,
        AT_LEAST,
        AT_MOST,
        AQUAL_TO
    };

    inline FindThreadParms() {
        file_mask.setPattern("*");
        setCaseSensitivity(false);
        use_sub_dirs = false;
        use_hidden = false;
        textCaseSensitivity = false;
        file_mask.setPatternSyntax(QRegExp::Wildcard);
        bytes = 0;
        size_oper  = NONE;
    }

    inline FindThreadParms(const QRegExp & mask) {
        setFileMask(mask);
        setCaseSensitivity(false);
        use_sub_dirs = false;
        use_hidden = false;
        textCaseSensitivity = false;
        bytes = 0;
        size_oper  = NONE;
    }

    inline void setCaseSensitivity(bool flag) {
        file_mask.setCaseSensitivity(flag?Qt::CaseSensitive:Qt::CaseInsensitive);
    }

    inline bool isCaseSensitive() {
        return (file_mask.caseSensitivity() == Qt::CaseSensitive);
    }

    inline void setUseSubDirs(bool flag) {
        use_sub_dirs = flag;
    }

    inline bool useSubDirs() {
        return use_sub_dirs;
    }

    inline void setUseHidden(bool flag) {
        use_hidden = flag;
    }

    inline bool useHidden() {
        return use_hidden;
    }

    inline void setFileMask(const QRegExp & mask) {
        file_mask = mask;
        file_mask.setPatternSyntax(QRegExp::Wildcard);
    }

    inline void setPattern(const QString & str) {
        file_mask.setPattern(str);
    }

    inline QRegExp fileMask() const {
        return file_mask;
    }

    inline void setFindDir(const QDir & dir) {
        this->dir = dir;
    }

    inline QDir findDir() const {
        return dir;
    }

    inline QString mimeType() const {
        return mime_type;
    }

    inline void setMimeType(const QString & type) {
        mime_type = type;
    }

    inline QString text() const {
        return in_text;
    }

    inline void setText(const QString & in_text) {
        this->in_text = in_text;
    }

    inline void setTextCaseSensitivity(bool flag) {
        textCaseSensitivity = flag;
    }

    inline bool isTextCaseSensitive() {
        return textCaseSensitivity;
    }

    inline void setUseBinaryFiles(bool flag) {
        useBinary = flag;
    }

    inline bool useBinaryFiles() {
        return useBinary;
    }

    inline QDateTime toDate() const {
        return to_date;
    }

    inline void setToDate(const QDateTime & date) {
        to_date = date;
    }

    inline QDateTime fromDate() const {
        return from_date;
    }

    inline void setFromDate(const QDateTime & date) {
        from_date = date;
    }

    SIZE_OPER sizeOper() const {
        return size_oper;
    }

    uint bytesToCompare() const {
        return bytes;
    }

    void setFileSizeToCompare(SIZE_OPER size_oper,uint bytes) {
        this->size_oper = size_oper;
        this->bytes = bytes;
    }

    inline QString fileOwner() const {
        return owner;
    }

    inline void setFileOwner(const QString & in_text) {
        owner = in_text;
    }

    inline QString fileGroup() const {
        return group;
    }

    inline void setFileGroup(const QString & in_text) {
        group = in_text;
    }

private:
    QRegExp file_mask;
    QDir dir;
    bool use_sub_dirs;
    bool case_sensitive;
    bool use_hidden;
    QString mime_type;
    QString in_text;
    bool textCaseSensitivity;
    bool useBinary;
    QDateTime from_date;
    QDateTime to_date;
    uint bytes;
    SIZE_OPER size_oper;
    QString owner;
    QString group;
};

class FindThread : public QThread {
    Q_OBJECT
public:
    FindThread(const FindThreadParms & parms,QObject * parent = 0);
    ~FindThread();

signals:
    void entry_found(const QString &);

private slots:
    void do_next();

private:
    bool isTextExists(const QString & file_name,const QString & text,bool caseSensitive);
    bool isSequentialFile(const QString & file_name);
    bool hasBinaryData(const QString & file_name);
    bool isBufferBinaryData(const QByteArray & data);

    FindThreadParms m_parms;
    QDirIterator * p_files;
    QTimer * p_timer;
    QMimeDatabase db;
    bool use_mime;
    QString buffer;
};

#endif // FINDTHREAD_H
