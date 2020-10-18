QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdlg.cpp \
    backgroundrunner.cpp \
    backgroundtaskcontainer.cpp \
    choosecolormenu.cpp \
    highlighter.cpp \
    history.cpp \
    log.cpp \
    logtextedit.cpp \
    main.cpp \
    mainwindow.cpp \
    progressdialog.cpp \
    searchedit.cpp \
    settingsdialog.cpp \
    shortcuthelpdlg.cpp \
    taglistwidget.cpp \
    timeline.cpp \
    timenode.cpp \
    timenodebody.cpp \
    timenodehead.cpp \
    toast.cpp \
    updater.cpp \
    recent.cpp

HEADERS += \
    LongtimeOperation.h \
    aboutdlg.h \
    backgroundrunner.h \
    backgroundtaskcontainer.h \
    choosecolormenu.h \
    highlighter.h \
    history.h \
    log.h \
    logtextedit.h \
    mainwindow.h \
    progressdialog.h \
    searchedit.h \
    searchwidget_copy.h \
    settingsdialog.h \
    shortcuthelpdlg.h \
    taglistwidget.h \
    timeline.h \
    timenode.h \
    timenodebody.h \
    timenodehead.h \
    toast.h \
    updater.h \
    version.h \
    recent.h

FORMS += \
    aboutdlg.ui \
    mainwindow.ui \
    progressdialog.ui \
    settingsdialog.ui \
    shortcuthelpdlg.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

macx {
ICON = Icon.icns
}

win32 {
SOURCES += \
    win/strcasestr.c
RC_ICONS = logo.ico
}

