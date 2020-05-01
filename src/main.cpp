#include "mainwindow.h"

#include <QApplication>
#include "../../codeeditor/codeeditor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    CodeEditor editor;
//    editor.show();
    MainWindow w;
    w.show();
    return a.exec();
}
