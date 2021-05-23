#include "register.h"
#include "httpmanager.h"
#include "webhome.h"
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <QJsonArray>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QAbstractButton>
#include "util.h"

#define TRY_LIMIT 259200 //3天

Register::Register()
{

}

QString Register::thisPlatform()
{
#if defined (Q_OS_WIN)
    return "Windows";
#elif defined (Q_OS_LINUX)
    return "Linux";
#elif defined (Q_OS_MACOS)
    return "Mac";
#else
    return "";
#endif
}

QString Register::thisMachineId()
{
    return QSysInfo::machineUniqueId();
}

Register &Register::instance()
{
    static Register impl;
    return impl;
}

void Register::init()
{
    QSettings config;

    auto licenseStr = config.value("register").toString();

    if (!licenseStr.isEmpty()) {
        auto license =  QJsonDocument::fromJson(licenseStr.toUtf8()).object();
        if (verify(license))
            mStatus = eRegistered;
    }

    if (mStatus != eRegistered) {
        auto tryFrom = config.value("tryFrom").toDateTime();
        auto passed = tryFrom.secsTo(QDateTime::currentDateTime());
        if (passed < 0)
            mLeftSecs = 0;
        else
            mLeftSecs = TRY_LIMIT - passed;

        mStatus = mLeftSecs > 0 ? eTry : eTryEnd;
    }

    if (mStatus == eTryEnd) {
        QMessageBox::critical(nullptr, "试用结束", "试用期已结束, 仅购买菜单可用，请购买后再使用");
    }
}

Register::RegisterStatus Register::getState()
{
    return mStatus;
}

QString Register::getStateStr()
{
    switch (mStatus) {
    case eRegistered:
        return "已购买";
    case eTry:
        return "试用中";
    case eTryEnd:
        return "试用结束";
    default:
        break;
    }
    return "未知状态";
}

qint64 Register::getLeftSecs()
{
    return mLeftSecs;
}

void Register::registerToServer(QString orderid, QString machineid, QString platform)
{
    QUrlQuery query({{"orderid", orderid}, {"machineid", machineid}, {"platform", platform}});
    QUrl url(WEB_REGISTER_URL);
    url.setQuery(query);

    QNetworkRequest req(url);
    HttpManager::instance().get(req, [](QNetworkReply* reply){
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
            auto body = reply->readAll();

            auto ans = QJsonDocument::fromJson(body).object();
            auto success = ans["success"].toBool(true);
            if (success) {
                //注册成功
                QSettings config;
                config.setValue("register", body);
                config.sync();

                QMessageBox mbox;
                mbox.setWindowTitle("注册成功");
                mbox.setText("注册成功, 请重启本应用");
                mbox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
                mbox.button(QMessageBox::Ok)->setText("立即重启");
                mbox.button(QMessageBox::Cancel)->setText("稍后重启");
                if (mbox.exec() == QMessageBox::Ok) {
                    relaunchApp();
                }
            } else {
                //注册失败
                QMessageBox::critical(nullptr, "注册失败", "注册失败：" + ans["reason"].toString("未知错误"));
            }

        } else {
            //通讯异常
            QMessageBox::critical(nullptr, "注册失败", "注册失败: 服务器通讯异常");
        }

    });
}

QString Register::xordecode(QJsonArray &arr, int key)
{
    quint8 keys[4];
    for (int i = 0; i < 4; i++) {
        keys[i] = key&0xFF;
        key>>=8;
    }

    QByteArray decoded;
    int j = 0;
    for (int i = 0; i < arr.size(); i++) {
        decoded.push_back(arr[i].toInt() ^ keys[j++]);
        if (j >= 4)
            j = 0;
    }

    return decoded.constData();
}

bool Register::verify(QJsonObject &license)
{
//    auto orderid = license["orderid"].toString(); //目前没去校验这个
    auto mid = license["mid"].toArray();
    auto plf = license["plf"].toArray();
    auto ordertime = license["ordertime"].toInt();

    auto machineid = xordecode(mid, ordertime);
    if (machineid != Register::thisMachineId()) {
        QMessageBox::critical(nullptr, "非法注册", "你的授权文件不是在该设备上注册的");
        return false;
    }

    auto platform = xordecode(plf, ordertime);
    if (platform != Register::thisPlatform()) {
        QMessageBox::critical(nullptr, "非法注册", "你的授权文件不能用于该操作系统，请购买对应操作系统的授权");
        return false;
    }

    return true;
}
