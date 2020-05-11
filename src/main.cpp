#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/res/img/logo.png"));

    MainWindow w;
    w.show();
    if (argc > 1) {
        w.doOpenFile(argv[1]);
    }
    return a.exec();
}
