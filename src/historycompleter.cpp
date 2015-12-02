/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "historycompleter.h"

#include <QSettings>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <math.h>

#define max(a,b) ((a>b)?a:b)
extern QSettings *theSettings;

class HistoryCompleterPrivate : public QAbstractListModel {
public:
    HistoryCompleterPrivate() : maxLines(30), lineEdit(0) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    void clearHistory();
    void saveEntry(const QString &str);

    QStringList list;
    QString historyKey;
    int maxLines;
    QLineEdit *lineEdit;
};

class HistoryLineDelegate : public QItemDelegate {
public:
    HistoryLineDelegate() : pixmap(":/images/editclear.png") {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect,option.palette.highlight());
            painter->setPen(option.palette.color(QPalette::HighlightedText));
        }
        else {
            painter->fillRect(option.rect,option.palette.color(!(int)fmod((double)index.row(),(double)2)?QPalette::AlternateBase:QPalette::Base));
            painter->setPen(option.palette.color(QPalette::Text));
        }
        painter->drawText(QStyle::alignedRect(option.direction, Qt::AlignLeft | Qt::AlignVCenter , QSize(option.rect.width()-pixmap.width(),option.rect.height()) , option.rect),index.data(Qt::DisplayRole).toString());
        painter->drawPixmap(QStyle::alignedRect(option.direction, Qt::AlignRight | Qt::AlignVCenter , pixmap.size(), option.rect),pixmap.scaled(option.decorationSize));
    }

    QPixmap pixmap;
};

class HistoryLineView : public QListView {
public:
    HistoryLineView(HistoryCompleterPrivate *model_) : model(model_) {}

private:
    void mousePressEvent(QMouseEvent *event) {
        int rr= event->x();
        if (layoutDirection() == Qt::LeftToRight)
            rr = viewport()->width() - event->x();
        if (rr < iconSize().width()) {
            model->removeRow(indexAt(event->pos()).row());
            return;
        }
        QListView::mousePressEvent(event);
    }

    HistoryCompleterPrivate *model;
};

int HistoryCompleterPrivate::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : list.count();
}

QVariant HistoryCompleterPrivate::data(const QModelIndex &index, int role) const {
    if (index.row() >= list.count() || index.column() != 0)
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return list.at(index.row());
    return QVariant();
}

bool HistoryCompleterPrivate::removeRows(int row, int count, const QModelIndex &parent) {
    if (row + count > list.count())
        return false;
    beginRemoveRows(parent, row, row + count -1);
    for (int i = 0; i < count; ++i)
        list.removeAt(row);
    theSettings->setValue(historyKey, list);
    endRemoveRows();
    return true;
}

void HistoryCompleterPrivate::clearHistory() {
    beginResetModel();
    list.clear();
    endResetModel();
}

void HistoryCompleterPrivate::saveEntry(const QString &str) {
    const QString &entry = str.trimmed();
    int removeIndex = list.indexOf(entry);
    if (removeIndex != -1)
        removeRow(removeIndex);
    beginInsertRows (QModelIndex(), list.count(), list.count());
    list.prepend(entry);
    list = list.mid(0, maxLines);
    endInsertRows();
    theSettings->setValue(historyKey, list);
}

HistoryCompleter::HistoryCompleter(QLineEdit *lineEdit, const QString &historyKey, QObject *parent)
    : QCompleter(parent),
      d(new HistoryCompleterPrivate) {
    d->historyKey = QLatin1String("CompleterHistory/") + historyKey;
    d->list = theSettings->value(d->historyKey).toStringList();
    d->lineEdit = lineEdit;
    if (d->list.count())
        lineEdit->setText(d->list.at(0));

    setModel(d);
    setPopup(new HistoryLineView(d));
    popup()->setItemDelegate(new HistoryLineDelegate);

    lineEdit->installEventFilter(this);
    popup()->installEventFilter(this);

    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(saveHistory()));
}

bool HistoryCompleter::removeHistoryItem(int index) {
    return d->removeRow(index);
}

HistoryCompleter::~HistoryCompleter() {
    delete d;
}

bool HistoryCompleter::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress
            && static_cast<QKeyEvent *>(event)->key() == Qt::Key_Down
            && !popup()->isVisible() && obj == d->lineEdit) {
        setCompletionPrefix(QString());
        complete();
    }
    return QCompleter::eventFilter(obj, event);
}

int HistoryCompleter::historySize() const {
    return d->rowCount();
}

int HistoryCompleter::maximalHistorySize() const {
    return d->maxLines;
}

void HistoryCompleter::setMaximalHistorySize(int numberOfEntries) {
    d->maxLines = numberOfEntries;
}

void HistoryCompleter::clearHistory() {
    d->clearHistory();
}

void HistoryCompleter::saveHistory() {
    d->saveEntry(d->lineEdit->text());
}

