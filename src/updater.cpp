#include "updater.h"
#include <QNetworkRequest>
#include "version.h"
#include "webhome.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "httpmanager.h"

Updater::Updater(QObject *parent) : QObject(parent), mCurrentVersion(VERSION)
{
    mCurrentVersion = VERSION;
}

Updater &Updater::instance()
{
    static Updater updater;
    return updater;
}

void Updater::checkNewVersion()
{
    QNetworkRequest req(QUrl(WEB_GET_VERSION_URL));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute,false);
    req.setAttribute(QNetworkRequest::BackgroundRequestAttribute, true);
    mHasCheckUpdate = true;

    HttpManager::instance().get(req, [this](QNetworkReply* reply){
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
            auto ans = QJsonDocument::fromJson(reply->readAll()).object();
            auto version = ans["version"].toString();
            mChangeLog = ans["change"].toString();

            qDebug()<<version;

            if (version == VERSION)
                emit noNewVersion();
            else{
                mLatestVersion = version;
                emit newVersionFound(version);
            }
        }
    });
}
