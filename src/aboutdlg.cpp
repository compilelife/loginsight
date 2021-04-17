#include "aboutdlg.h"
#include "ui_aboutdlg.h"
#include "updater.h"
#include <QMessageBox>
#include <QDebug>

AboutDlg::AboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
    setWindowTitle("关于");

#define HINT_TPL ("发现新版本：%1, 下载地址: https://github.com/compilelife/loginsight/releases/latest")

    auto& updater = Updater::instance();
    if (updater.latestVersion().isEmpty())
        ui->versionLabel->setText(updater.currentVersion());
    else
        ui->versionLabel->setText(QString(HINT_TPL).arg(updater.latestVersion()));

    connect(&updater, &Updater::noNewVersion, [this]{
        QMessageBox::warning(this, "", "当前版本已经是最新版本");
    });
    //connect newVersionFound：找到新版本会触发MainWindow的对应slot，这里不必再写

    connect(ui->updateButton, &QPushButton::clicked, [&updater]{
        updater.checkNewVersion();
    });
}

AboutDlg::~AboutDlg()
{

}
