#include "mainwindow.h"

#include <QApplication>
#include <QThread>
#include <QThreadPool>
#include <QSettings>

static void createDefaultSettings()
{
    QSettings config;
    auto keys = config.allKeys();

    if (!keys.contains("editorFont")) {
        config.setValue("editorFont", "monospace");
    }

    if (!keys.contains("editorFontSize")) {
        config.setValue("editorFontSize", 12);
    }

    if (!keys.contains("caseSensitive")) {
        config.setValue("caseSensitive", true);
    }

    if (!keys.contains("checkUpdate")) {
        config.setValue("checkUpdate", true);
    }

    if (!keys.contains("tryFrom")) {
        config.setValue("tryFrom", QDateTime::currentDateTime());
    }

    if (!keys.contains("recentFile")) {
        config.setValue("recentFile", QStringList{});
    }

    if (!keys.contains("recentPrj")) {
        config.setValue("recentPrj", QStringList{});
    }

    if (!keys.contains("wrap")) {
        config.setValue("wrap", false);
    }

    if (!keys.contains("register")) {
        config.setValue("register", "");
    }

    config.sync();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("compilelife");
    QCoreApplication::setOrganizationDomain("compilelife.com");
    QCoreApplication::setApplicationName("loginsight");
    a.setWindowIcon(QIcon(":/res/img/logo.png"));

    auto idealCpuCount = QThread::idealThreadCount();
    qDebug()<<"cpu count: "<<idealCpuCount;
    if (idealCpuCount >= 2) {
        QThreadPool::globalInstance()->setMaxThreadCount(2);
    }

    createDefaultSettings();


    MainWindow w;
    w.show();
    if (argc > 1) {
        w.doOpenFile(argv[1]);
    }

    return a.exec();
}
