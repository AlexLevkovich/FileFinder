#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include "kfindtreeview.h"

namespace Ui {
class FindDialog;
}

class FindThread;

class FindDialog : public QDialog {
    Q_OBJECT
public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();

private slots:
    void on_fileOpersCheck_clicked(bool checked);
    void on_betweenRadio_clicked(bool checked);
    void on_periodRadio_clicked(bool checked);
    void on_operatorsCombo_activated(const QString &arg1);
    void on_dirButton_clicked();
    void on_findButton_clicked();
    void file_entry_found(const QString & entry);
    void endSearch();
    void on_stopButton_clicked();
    void on_closeButton_clicked();
    void on_saveAsButton_clicked();
    void showShortcuts();
    void showAbout();

protected:
    void closeEvent(QCloseEvent * e);

private:
    static int caseInsensitiveLessThan(const void * s1, const void * s2);
    static uint getBytesFromCombo(uint size,int index);

    Ui::FindDialog *ui;
    FindThread * find_thread;
};

#endif // FINDDIALOG_H
