QT       += core gui concurrent network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = loginsight

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdlg.cpp \
    choosecolormenu.cpp \
    documenttab.cpp \
    feedbackdlg.cpp \
    filesource.cpp \
    filterdialog.cpp \
    focusmanager.cpp \
    highlighter.cpp \
    history.cpp \
    httpmanager.cpp \
    ilog.cpp \
    ilogsource.cpp \
    logedit.cpp \
    logviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    memlog.cpp \
    newversiondlg.cpp \
    progressdialog.cpp \
    recent.cpp \
    searchbar.cpp \
    settingsdialog.cpp \
    sublog.cpp \
    taglistwidget.cpp \
    timeline.cpp \
    timenode.cpp \
    timenodebody.cpp \
    timenodehead.cpp \
    updater.cpp \
    usercontrol.cpp \
    util.cpp \
    welcomepage.cpp

HEADERS += \
    aboutdlg.h \
    choosecolormenu.h \
    documenttab.h \
    events.h \
    feedbackdlg.h \
    filesource.h \
    filterdialog.h \
    focusmanager.h \
    highlighter.h \
    history.h \
    httpmanager.h \
    ilog.h \
    ilogsource.h \
    logedit.h \
    logviewer.h \
    longtimeoperation.h \
    mainwindow.h \
    memlog.h \
    newversiondlg.h \
    progressdialog.h \
    recent.h \
    searchbar.h \
    settingsdialog.h \
    sublog.h \
    taglistwidget.h \
    timeline.h \
    timenode.h \
    timenodebody.h \
    timenodehead.h \
    updater.h \
    usercontrol.h \
    util.h \
    version.h \
    webhome.h \
    welcomepage.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    res.qrc

FORMS += \
    aboutdlg.ui \
    feedbackdlg.ui \
    settingsdialog.ui

macx {
ICON = Icon.icns
}

win32 {
RC_ICONS = logo.ico
}
