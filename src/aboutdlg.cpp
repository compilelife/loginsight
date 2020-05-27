#include "aboutdlg.h"
#include "ui_aboutdlg.h"

#include <QDebug>
#include <QNetworkRequest>

#define VERSION "v0.3"

AboutDlg::AboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
    setWindowTitle("关于");
    ui->versionLabel->setText(QString("当前版本: %1").arg(VERSION));

    connect(ui->updateButton, &QAbstractButton::clicked, this, &AboutDlg::checkUpdate);
    connect(&mNetwork, &QNetworkAccessManager::finished, this, &AboutDlg::httpReplied);
}

AboutDlg::~AboutDlg()
{
    delete ui;
}

void AboutDlg::checkUpdate()
{
    QNetworkRequest req(QUrl("https://github.com/compilelife/loginsight/releases/latest"));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute,false);
    req.setAttribute(QNetworkRequest::BackgroundRequestAttribute, true);

    mNetwork.get(req);
}

void AboutDlg::httpReplied(QNetworkReply *reply)
{
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302) {
        QString body(reply->readAll());
        auto begin = body.indexOf("tag/");
        if (begin < 0) {
            qDebug()<<"tag/ not found";
            return;
        }
        auto end = body.indexOf("\">", begin);
        if (end < 0) {
            qDebug()<<"\"> not found";
            return;
        }
        begin+=4;
        auto version = body.mid(begin, end - begin);
        if (version == VERSION)
            ui->versionLabel->setText("当前已经是最新版本");
        else
            ui->versionLabel->setText("发现新版本："+version+", 下载地址: https://github.com/compilelife/loginsight/releases/latest");
    }

    reply->deleteLater();
}
