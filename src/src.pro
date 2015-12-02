#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T14:51:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

lessThan(QT_MAJOR_VERSION, 5): {
INCLUDEPATH += ./mimetypes
INCLUDEPATH += ./io
}

CONFIG += link_pkgconfig
PKGCONFIG += gobject-2.0 gio-2.0

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr/local
}

win32 {
    INSTALL_PREFIX = .
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/filefinder

DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

TARGET = FileFinder
TEMPLATE = app subdirs

RC_FILE = win_icon.rc

SOURCES += main.cpp\
        finddialog.cpp \
    historycompleter.cpp \
    fancylineedit.cpp \
    kfindsortfilterproxymodel.cpp \
    byteshumanizer.cpp \
    kfindtreeview.cpp \
    findthread.cpp \
    folderchooser.cpp \
    scrollmessagebox.cpp

HEADERS  += finddialog.h \
    historycompleter.h \
    fancylineedit.h \
    kfindsortfilterproxymodel.h \
    byteshumanizer.h \
    kfindtreeview.h \
    findthread.h \
    folderchooser.h \
    scrollmessagebox.h

lessThan(QT_MAJOR_VERSION, 5): {
HEADERS *= io/qstandardpaths.h
SOURCES *= io/qstandardpaths.cpp

macx {
    SOURCES *= io/qstandardpaths_mac.cpp
} else:unix {
    SOURCES *= io/qstandardpaths_unix.cpp
} else:win32 {
    SOURCES *= io/qstandardpaths_win.cpp
}

HEADERS += \
        mimetypes/qmimedatabase.h \
        mimetypes/qmimetype.h \
        mimetypes/qmimemagicrulematcher_p.h \
        mimetypes/qmimetype_p.h \
        mimetypes/qmimetypeparser_p.h \
        mimetypes/qmimedatabase_p.h \
        mimetypes/qmimemagicrule_p.h \
        mimetypes/qmimeglobpattern_p.h \
        mimetypes/qmimeprovider_p.h

SOURCES += \
        mimetypes/qmimedatabase.cpp \
        mimetypes/qmimetype.cpp \
        mimetypes/qmimemagicrulematcher.cpp \
        mimetypes/qmimetypeparser.cpp \
        mimetypes/qmimemagicrule.cpp \
        mimetypes/qmimeglobpattern.cpp \
        mimetypes/qmimeprovider.cpp

RESOURCES += \
        mimetypes/mimetypes.qrc
}

FORMS    += finddialog.ui \
    folderchooser.ui

SUBDIRS += mimetypes-qt4

RESOURCES += \
    src.qrc

TRANSLATIONS = $$PWD/translations/filefinder_ru.ts \
               $$PWD/translations/filefinder_be.ts

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
LRELEASE = $$[QT_INSTALL_BINS]/lrelease

updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/src.pro

QMAKE_EXTRA_TARGETS += updatets

updateqm.depends = updatets
updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.variable_out = PRE_TARGETDEPS
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

qm.files = $$TRANS_DIR1/*.qm
qm.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/filefinder/
qm.CONFIG += no_check_exist

desktop.files = FileFinder.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

icon.files = images/finder.png
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/

INSTALLS += target qm desktop icon
