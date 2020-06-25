#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include "searchedit.h"
#include <QListWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include "taglistwidget.h"

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

    if (!keys.contains("gotoEOF")) {
        config.setValue("gotoEOF", false);
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

    createDefaultSettings();

//    MainWindow w;
//    w.show();
//    if (argc > 1) {
//        w.doOpenFile(argv[1]);
//    }



    TagListWidget* list = new TagListWidget;
    list->setMinimumHeight(35);
    list->setMaximumHeight(35);
    list->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    for (int i = 0; i < 5; i++)
    {
        list->addTag(QString("keyword %1").arg(i+1), Qt::blue);
    }

    QWidget widget;
    auto box = new QHBoxLayout();
    auto edit = new SearchEdit();
    edit->setMinimumHeight(35);
    box->addWidget(edit);
    box->addWidget(list);
    box->setMargin(5);
    box->setSpacing(10);
    widget.setLayout(box);
    widget.show();

    return a.exec();
}
