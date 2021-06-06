#include "newversiondlg.h"
#include <QVBoxLayout>
#include "updater.h"
#include <QLabel>
#include "webhome.h"
#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>

NewVersionDlg::NewVersionDlg()
{
    auto& updater = Updater::instance();

    auto box = new QVBoxLayout;
    box->addWidget(new QLabel("找到新版本: "+updater.latestVersion()));

    {
        auto l = new QLabel;
        l->setTextFormat(Qt::MarkdownText);
        l->setText(updater.changeLog());
        box->addWidget(l);

        connect(l, &QLabel::linkActivated, [](const QString& link){
            QUrl url(QString("%1/manual/%2").arg(WEB_PAGE,link));
            QDesktopServices::openUrl(url);
        });
    }

    {
        auto okBtn = new QPushButton("立即下载");
        connect(okBtn, &QPushButton::clicked, [this]{
            QDesktopServices::openUrl(QUrl(WEB_DOWNLOAD_URL));
            accept();
        });
        okBtn->setAutoDefault(true);
        auto cancelBtn = new QPushButton("稍后");
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

        auto hbox = new QHBoxLayout;
        hbox->addWidget(okBtn);
        hbox->addWidget(cancelBtn);
        box->addLayout(hbox);
    }

    setLayout(box);
}
