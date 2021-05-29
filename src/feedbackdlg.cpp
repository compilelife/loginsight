#include "feedbackdlg.h"
#include "ui_feedbackdlg.h"
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QUrl>
#include <QFile>

FeedbackDlg::FeedbackDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FeedbackDlg)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("发送");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        auto title = ui->titleEdit->text();
        auto content = ui->contentEdit->toPlainText();

        extern QFile gLogFile;
        if (gLogFile.isOpen()) {
            QFile f(gLogFile.fileName());
            if (f.open(QIODevice::ReadOnly)) {
                QString flavor("开源版本");
                QString log = f.readAll();
                content += QString("\n\n\n以下是此次loginsight的运行信息\n========================\n%1\n%2").arg(flavor).arg(log);
            }
        }

        if (ui->useBrCheckBox->isChecked()) {
            content.replace('\n', "</br>");//Foxmail似乎只能识别这个
        } else {
            content.replace('\n', "\r\n"); //标准协议是这样的
        }

        auto url = QUrl(QString("mailto:compilelife@163.com?subject=%1&body=%2").arg(title, content));
        QDesktopServices::openUrl(url);
    });
}

FeedbackDlg::~FeedbackDlg()
{
    delete ui;
}
