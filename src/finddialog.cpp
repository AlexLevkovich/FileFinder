#include "finddialog.h"
#include "ui_finddialog.h"
#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QMessageBox>
#include <QMimeDatabase>
#include "findthread.h"
#include "byteshumanizer.h"
#include "folderchooser.h"

extern QSettings *theSettings;
extern QMimeDatabase  *mime_database;
const char * finder_version = "1.0";

FindDialog::FindDialog(QWidget *parent) :  QDialog(parent),  ui(new Ui::FindDialog) {
    ui->setupUi(this);

    setWindowFlags(Qt::Widget);
    find_thread = NULL;
    ui->dirEdit->setHistoryCompleter("dir_completer");
    ui->containingTextEdit->setHistoryCompleter("containing_text_completer");
    ui->fileMaskEdit->setHistoryCompleter("file_mask_completer");
    ui->groupEdit->setHistoryCompleter("group_completer");
    ui->ownerEdit->setHistoryCompleter("user_completer");

    if (!QFileInfo(ui->dirEdit->text()).exists()) {
        ui->dirEdit->setFocus();
        ui->dirEdit->setText(QDir::homePath());
    }

    ui->helpButton->setMenu(new QMenu(this));
    ui->helpButton->menu()->addAction(QIcon(":/images/help-hint.png"),tr("Shortcuts..."),this,SLOT(showShortcuts()));
    ui->helpButton->setDefaultAction(ui->helpButton->menu()->addAction(QIcon(":/images/finder.png"),tr("About..."),this,SLOT(showAbout())));
    ui->helpButton->setText(tr("Help"));

    QVector<QMimeType> v_mimes = mime_database->allMimeTypes().toVector();
    ui->mimeCombo->addItem(tr("All files and Folders"),"all/all");
    ui->mimeCombo->addItem(tr("All files"),"all/allfiles");

    ::qsort(v_mimes.data(),v_mimes.count(),sizeof(QMimeType),caseInsensitiveLessThan);

    for (int i=0;i<v_mimes.count();i++) {
        ui->mimeCombo->addItem(v_mimes[i].comment(),v_mimes[i].name());
    }
    ui->stopButton->setEnabled(false);

    ui->subFoldersCheck->setChecked(theSettings->value("file_name_subfolders",false).toBool());
    ui->sensitiveCheck->setChecked(theSettings->value("file_name_case",false).toBool());
    ui->hiddenCheck->setChecked(theSettings->value("file_name_hidden",false).toBool());
    ui->findSensitiveCheck->setChecked(theSettings->value("text_case",false).toBool());
    ui->binaryCheck->setChecked(theSettings->value("include_binary",false).toBool());
    ui->operatorsCombo->setCurrentIndex(theSettings->value("operators_combo",0).toInt());
    ui->sizeCombo->setCurrentIndex(theSettings->value("operators_size_combo",0).toInt());
    ui->sizeSpin->setValue(theSettings->value("operators_size_spin",0).toInt());
    ui->fromDate->setDateTime(theSettings->value("from_date",QDateTime(QDate(2000,1,1))).toDateTime());
    ui->toDate->setDateTime(theSettings->value("to_date",QDateTime(QDate(2100,1,1))).toDateTime());
    ui->fileOpersCheck->setChecked(theSettings->value("opers_check",false).toBool());
    on_fileOpersCheck_clicked(ui->fileOpersCheck->isChecked());
    ui->periodsCombo->setCurrentIndex(theSettings->value("periods_combo",0).toInt());
    ui->periodSpin->setValue(theSettings->value("period_spin",0).toInt());
    ui->betweenRadio->setChecked(theSettings->value("between_radio",true).toBool());
    ui->periodRadio->setChecked(theSettings->value("period_radio",false).toBool());

    int index = ui->mimeCombo->findData(theSettings->value("mime_type").toString());
    if (index >= 0) ui->mimeCombo->setCurrentIndex(index);
    on_operatorsCombo_activated(ui->operatorsCombo->currentText());
}

FindDialog::~FindDialog() {
    delete ui;
    if (find_thread != NULL && find_thread->isRunning()) {
        find_thread->quit();
        find_thread->wait();
    }
}

int FindDialog::caseInsensitiveLessThan(const void * s1, const void * s2) {
    QMimeType * t1 = (QMimeType *)s1;
    QMimeType * t2 = (QMimeType *)s2;
    return QString::compare(t1->comment(),t2->comment(),Qt::CaseInsensitive);
}

void FindDialog::on_fileOpersCheck_clicked(bool checked) {
    ui->betweenRadio->setEnabled(checked);
    ui->periodRadio->setEnabled(checked);
    ui->fromDate->setEnabled(checked);
    ui->toDate->setEnabled(checked);
    ui->periodsCombo->setEnabled(checked);
    ui->periodSpin->setEnabled(checked);
}

void FindDialog::on_betweenRadio_clicked(bool checked) {
    ui->periodRadio->setChecked(!checked);
}

void FindDialog::on_periodRadio_clicked(bool checked) {
    ui->betweenRadio->setChecked(!checked);
}

void FindDialog::on_operatorsCombo_activated(const QString &arg1) {
    ui->sizeCombo->setEnabled(!arg1.isEmpty());
    ui->sizeSpin->setEnabled(!arg1.isEmpty());
}

void FindDialog::on_dirButton_clicked() {
    QString dir = FolderChooser::getExistingDirectory(ui->dirEdit->text(),this);
    if (dir.isEmpty()) return;
    ui->dirEdit->setFocus();
    ui->dirEdit->setText(dir);
}

uint FindDialog::getBytesFromCombo(uint size,int index) {
    QString suffix("B");
    switch (index) {
    case 1:
        suffix = "KiB";
        break;
    case 2:
        suffix = "MiB";
        break;
    case 3:
        suffix = "GiB";
        break;
    }

    return (uint)BytesHumanizer(QString("%1 %2").arg(size).arg(suffix)).value();
}

void FindDialog::on_findButton_clicked() {
    FindThreadParms parms;
    parms.setPattern(ui->fileMaskEdit->text());
    parms.setFindDir(QDir(ui->dirEdit->text()));
    parms.setCaseSensitivity(ui->sensitiveCheck->isChecked());
    parms.setUseHidden(ui->hiddenCheck->isChecked());
    parms.setUseSubDirs(ui->subFoldersCheck->isChecked());
    parms.setMimeType(ui->mimeCombo->itemData(ui->mimeCombo->currentIndex()).toString());
    parms.setText(ui->containingTextEdit->text());
    parms.setTextCaseSensitivity(ui->findSensitiveCheck->isChecked());
    parms.setUseBinaryFiles(ui->binaryCheck->isChecked());
    if (ui->fileOpersCheck->isChecked()) {
        if (ui->betweenRadio->isChecked()) {
            parms.setFromDate(ui->fromDate->dateTime());
            parms.setToDate(ui->toDate->dateTime());
        }
        else {
            QDateTime toDate = QDateTime::currentDateTime();
            parms.setToDate(ui->toDate->dateTime());
            switch (ui->periodsCombo->currentIndex()) {
            case 0:
                parms.setFromDate(toDate.addSecs(-ui->periodSpin->value()*60));
                break;
            case 1:
                parms.setFromDate(toDate.addSecs(-ui->periodSpin->value()*3600));
                break;
            case 2:
                parms.setFromDate(toDate.addDays(-ui->periodSpin->value()));
                break;
            case 3:
                parms.setFromDate(toDate.addMonths(-ui->periodSpin->value()));
                break;
            case 4:
                parms.setFromDate(toDate.addYears(-ui->periodSpin->value()));
                break;
            }
        }
    }
    parms.setFileSizeToCompare((FindThreadParms::SIZE_OPER)ui->operatorsCombo->currentIndex(),getBytesFromCombo(ui->sizeSpin->value(),ui->sizeCombo->currentIndex()));
    parms.setFileOwner(ui->ownerEdit->text());
    parms.setFileGroup(ui->groupEdit->text());
    find_thread = new FindThread(parms,this);
    connect(find_thread,SIGNAL(entry_found(const QString &)),this,SLOT(file_entry_found(const QString &)));
    connect(find_thread,SIGNAL(finished()),this,SLOT(endSearch()),Qt::QueuedConnection);
    ui->treeView->beginSearch(QUrl(ui->dirEdit->text()));
    find_thread->start();
    ui->stopButton->setEnabled(true);
    ui->findButton->setEnabled(false);
    ui->tabWidget->setEnabled(false);
}

void FindDialog::endSearch() {
    delete find_thread;
    find_thread = NULL;
    ui->treeView->endSearch();
    ui->stopButton->setEnabled(false);
    ui->findButton->setEnabled(true);
    ui->tabWidget->setEnabled(true);
}

void FindDialog::file_entry_found(const QString & entry) {
    ui->treeView->insertItem(FileInfo(entry));
}

void FindDialog::on_stopButton_clicked() {
    find_thread->quit();
}

void FindDialog::on_closeButton_clicked() {
    qApp->postEvent(this,new QCloseEvent());
}

void FindDialog::closeEvent(QCloseEvent * e) {
    QDialog::closeEvent(e);
    theSettings->setValue("file_name_subfolders",ui->subFoldersCheck->isChecked());
    theSettings->setValue("file_name_case",ui->sensitiveCheck->isChecked());
    theSettings->setValue("file_name_hidden",ui->hiddenCheck->isChecked());
    theSettings->setValue("mime_type",ui->mimeCombo->itemData(ui->mimeCombo->currentIndex()));
    theSettings->setValue("text_case",ui->findSensitiveCheck->isChecked());
    theSettings->setValue("include_binary",ui->binaryCheck->isChecked());
    theSettings->setValue("operators_combo",ui->operatorsCombo->currentIndex());
    theSettings->setValue("operators_size_spin",ui->sizeSpin->value());
    theSettings->setValue("operators_size_combo",ui->sizeCombo->currentIndex());
    theSettings->setValue("from_date",ui->fromDate->dateTime());
    theSettings->setValue("to_date",ui->toDate->dateTime());
    theSettings->setValue("opers_check",ui->fileOpersCheck->isChecked());
    theSettings->setValue("period_spin",ui->periodSpin->value());
    theSettings->setValue("periods_combo",ui->periodsCombo->currentIndex());
    theSettings->setValue("between_radio",ui->betweenRadio->isChecked());
    theSettings->setValue("period_radio",ui->periodRadio->isChecked());
}

void FindDialog::on_saveAsButton_clicked() {
    ui->treeView->saveResults();
}

void FindDialog::showShortcuts() {
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Help..."));
    msgBox.setText(tr("The list of shortcuts:"));
    msgBox.setInformativeText(tr("F3\tStart the search\nAlt+End\tStop the search\nAlt+S\tSave the search results\nAlt+Q\tClose the application"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void FindDialog::showAbout() {
    QMessageBox::about(this,tr("About FileFinder..."),tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">FileFinder</span> is simple search tool like KDE's KFind.</p>"
                                                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">FileFinder version is %1.</p>"
                                                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Developer: Alex Levkovich (<a href=\"mailto:alevkovich@tut.by\"><span style=\" text-decoration: underline; color:#0057ae;\">alevkovich@tut.by</span></a>)</p>"
                                                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">License: GPL</p>").arg(finder_version));

}
