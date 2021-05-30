#include "welcomepage.h"

#include <QGridLayout>
#include <QLabel>
#include "usercontrol.h"
#include <QSettings>
#include "mainwindow.h"
#include <QFileInfo>
#include <QDir>
#include "version.h"
#include "updater.h"
#include <QMessageBox>
#include <QDesktopServices>
#include "webhome.h"
#include "util.h"
#include "newversiondlg.h"

WelcomePage::WelcomePage(MainWindow *parent) : QWidget(parent), mMainWnd(parent)
{
    mTitleStyle = "style=\"color:#303030\"";
    auto& updater = Updater::instance();
    connect(&updater, &Updater::newVersionFound, [this](const QString& version){
        NewVersionDlg().exec();

        rebuildVersionArea();
    });

    connect(&updater, &Updater::noNewVersion, [this]{
        rebuildVersionArea();
    });
}

QWidget *WelcomePage::buildRecentArea(const QString& name, const QString &key, const char* slot)
{
    QSettings config;
    auto lst = config.value(key).toList();

    QStringList items;
    for (auto&& item : lst) {
        QFileInfo fi(item.toString());
        auto&& text = QString("<p><a href=%2>%1</a> %3</p>").arg(fi.fileName()).arg(fi.absoluteFilePath()).arg(makeShortPath(fi.absolutePath(), 40));
        items.push_back(text);
    }

    auto label = createAreaLabel();
    if (items.isEmpty()){
        label->setText(QString("<h1 %2>最近的%1</h1><p>无</p>").arg(name).arg(mTitleStyle));
    }
    else{
        auto itemsStr = items.join("");
        auto&& text = QString("<h1 %3>最近的%1</h1>%2").arg(name).arg(itemsStr).arg(mTitleStyle);
        label->setText(text);
    }

    connect(label, SIGNAL(linkActivated(const QString&)), mMainWnd, slot);

    return label;
}

void WelcomePage::rebuildVersionArea()
{
    auto box = (QGridLayout*)layout();
    mVersionWidget->deleteLater();
    auto widget = buildVersionArea();
    box->replaceWidget(mVersionWidget, widget);
    mVersionWidget = widget;
}

void WelcomePage::showEvent(QShowEvent *)
{
    //四个区
    //左上：启动
    //左下：最近
    //右上：帮助
    //右下：版本、注册

    auto old = layout();
    if (old)
        QWidget().setLayout(old);

    auto box = new QGridLayout;
    box->setMargin(30);

    box->addWidget(buildActionsArea("启动", {UserControl::OpenFileIntent, UserControl::OpenClipboardIntent}), 0, 0);
    box->setRowMinimumHeight(1, 50);
    box->addWidget(buildRecentArea("文件", "recentFile", SLOT(doOpenFile(const QString&))), 2, 0);
    box->setRowMinimumHeight(3, 50);
    box->addWidget(buildRecentArea("工程", "recentPrj", SLOT(doOpenPrj(const QString&))), 4, 0);

    box->setColumnMinimumWidth(1, 20);
    box->addWidget(buildActionsArea("帮助",{
                                        UserControl::UsageIntent,
                                        UserControl::ShortcutIntent
                                    }), 0, 2);
    mVersionWidget = buildVersionArea();
    box->addWidget(mVersionWidget, 2, 2);

    setLayout(box);

    if (!Updater::instance().hasCheckUpdate()) {
        QSettings config;
        if (config.value("checkUpdate").toBool())
            Updater::instance().checkNewVersion();
    }
}

QWidget *WelcomePage::buildVersionArea()
{
    //注册状态
    auto&& registerStr = QString("当前是开源版，<a href=%1>点击试用专业版</a>").arg(WEB_DOWNLOAD_URL);
    auto registerStyle = "";

    //检查更新，或者显示新版本，或者无新版本
    QString updateStr;
    QSettings config;
    auto& updater = Updater::instance();
    if (Updater::instance().hasCheckUpdate()) {
        if (updater.latestVersion().isEmpty()) {
            updateStr = "没有新版本";
        } else {
            updateStr = QString("找到新版本(专业版): %1，点击<a href=%2>前往下载</a>").arg(updater.latestVersion()).arg(WEB_DOWNLOAD_URL);
        }
    } else {
        updateStr = "<a href=checkUpdate>检查新版本</a>";
    }

    auto label = createAreaLabel();

    auto&& text = QString("<h1 %1>软件状态</h1> <p %2>%3<p> <p>当前版本：%4</p> <p>%5</p>")
            .arg(mTitleStyle)
            .arg(registerStyle).arg(registerStr)
            .arg(VERSION)
            .arg(updateStr);

    label->setText(text);

    connect(label, &QLabel::linkActivated, [](const QString& link){
        if (link == "checkUpdate")
            Updater::instance().checkNewVersion();
        else
            QDesktopServices::openUrl(QUrl(link));
    });

    return label;
}

QWidget *WelcomePage::buildActionsArea(const QString &name, const QVector<int> &actions)
{
    QStringList actionStrList;
    for (auto&& intent : actions) {
        auto action = UserControl::instance().actionFor((UserControl::Intent)(intent));
        if (action)
            actionStrList.append(QString("<p><a href=%1>%2</a></p>").arg(intent).arg(action->text()));
    }

    auto label = createAreaLabel();

    label->setText(QString("<h1 %1>%2</h1> %3").arg(mTitleStyle).arg(name).arg(actionStrList.join("")));
    connect(label, &QLabel::linkActivated, [](const QString& intentStr){
        auto intent = (UserControl::Intent)(intentStr.toInt());
        UserControl::instance().actionFor(intent)->trigger();
    });

    return label;
}

QLabel *WelcomePage::createAreaLabel()
{
    auto label = new QLabel;
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    return label;
}
