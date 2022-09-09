QT += quick websockets widgets network

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        coreboot.cpp \
        linehighlighter.cpp \
        main.cpp \
        nativehelper.cpp \
        textcodec.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    coreboot.h \
    linehighlighter.h \
    nativehelper.h \
    textcodec.h

DESTDIR = $$OUT_PWD # 控制windows将exe输出到build目录下，而非build/debug下

copy_files.files = $$files($$PWD/bin/*)
copy_files.path = $$OUT_PWD
COPIES += copy_files

linux {
    QMAKE_POST_LINK="chmod +x $$OUT_PWD/*.linux"
}
macx {
    QMAKE_POST_LINK="chmod +x $$OUT_PWD/*.macosx; cp $$OUT_PWD/*.macosx app.app/Contents/MacOS"
ICON = Icon.icns
QMAKE_INFO_PLIST = Info.plist
}

win32 {
RC_ICONS = logo.ico
}
msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}
