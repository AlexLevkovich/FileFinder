#include "finddialog.h"
#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMimeDatabase>
#include <QFileIconProvider>

QSettings *theSettings = NULL;
QMimeDatabase  * mime_database;
#ifdef WIN32
    QFileIconProvider * ip;
#endif

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("FileFinder");
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,QCoreApplication::organizationName(),QCoreApplication::applicationName());
    theSettings = &settings;

    QTranslator m_translator;
    QString lang = QLocale::system().name().split("_").at(0);
    if(!m_translator.load("filefinder_" + lang, TRANS_DIR2))
        m_translator.load("filefinder_" + lang, TRANS_DIR1);
    QApplication::installTranslator(&m_translator);

    QTranslator m_translator2;
    if (m_translator2.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&m_translator2);
    }

    mime_database = new QMimeDatabase();
#ifdef WIN32
    ip = new QFileIconProvider();
#endif

    FindDialog w;
    w.show();

    return a.exec();
}
