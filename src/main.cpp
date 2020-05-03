#include "mainwindow.h"

#include <QApplication>
#include "../../codeeditor/codeeditor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/res/img/logo.png"));

    MainWindow w;
    w.show();
    return a.exec();
}
