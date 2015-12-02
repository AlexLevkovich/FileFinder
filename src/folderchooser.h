#ifndef FOLDERCHOOSER_H
#define FOLDERCHOOSER_H

#include <QDialog>

namespace Ui {
class FolderChooser;
}

class FolderChooser : public QDialog {
    Q_OBJECT

private:
    FolderChooser(const QString & start_folder,QWidget *parent = 0);
    QString folderPath() const;
public:
    ~FolderChooser();
    static const QString getExistingDirectory(const QString & start_folder = QString(),QWidget *parent = 0);

private slots:
    void selectionChanged();

private:
    Ui::FolderChooser *ui;
};

#endif // FOLDERCHOOSER_H
