#include "aboutdlg.h"
#include "ui_aboutdlg.h"
#include "updater.h"
#include <QMessageBox>
#include <QDebug>
#include "webhome.h"
#include <QDesktopServices>
#include <QUrl>

AboutDlg::AboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
    setWindowTitle("关于");

    auto& updater = Updater::instance();

    ui->versionLabel->setText(updater.currentVersion());
    connect(&updater, &Updater::noNewVersion, [this]{
        QMessageBox::warning(this, "", "当前版本已经是最新版本");
    });
    //connect newVersionFound：找到新版本会触发MainWindow的对应slot，这里不必再写

    connect(ui->updateButton, &QPushButton::clicked, [&updater]{
        updater.checkNewVersion();
    });

    connect(ui->infoLabel, &QLabel::linkActivated, [](const QString& url){
        QDesktopServices::openUrl(QUrl(url));
    });
}

AboutDlg::~AboutDlg()
{

}
