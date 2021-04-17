#include "updater.h"
#include <QNetworkRequest>
#include "version.h"

Updater::Updater(QObject *parent) : QObject(parent), mCurrentVersion(VERSION)
{
    connect(&mNetwork, &QNetworkAccessManager::finished, this, &Updater::httpReplied);
}

Updater &Updater::instance()
{
    static Updater updater;
    return updater;
}

void Updater::checkNewVersion()
{
    QNetworkRequest req(QUrl("https://github.com/compilelife/loginsight/releases/latest"));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute,false);
    req.setAttribute(QNetworkRequest::BackgroundRequestAttribute, true);

    mNetwork.get(req);
}

void Updater::httpReplied(QNetworkReply *reply)
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
            emit noNewVersion();
        else{
            mLatestVersion = version;
            emit newVersionFound(version);
        }
    }

    reply->deleteLater();
}
